#include "runtime.h"
#include "chunk.h"
#include "kplstate.h"

namespace kpl::runtime
{
	CallStack::CallStack(Size size) :
		_base{ utils::malloc<CallInfo>(size) },
		_top{ nullptr }
	{}

	CallStack::~CallStack()
	{
		if (_base)
			utils::free(_base);

		_base = _top = nullptr;
	}

	CallInfo* CallStack::push(RegisterStack& regs, Function& function, Offset instruction)
	{
		CallInfo* info = _top + 1;

		info->prev = _top;
		info->function = &function;
		info->bottom = regs._bottom;
		info->top = regs._top;
		info->instruction = instruction;

		_top = info;

		return info;
	}

	CallInfo* CallStack::pop()
	{
		CallInfo* info = _top;
		_top = info->prev;

		return info;
	}

	void CallStack::push_native()
	{
		CallInfo* info = !_top ? _base : _top + 1;

		info->prev = _top;
		info->function = nullptr;
		info->bottom = nullptr;
		info->top = nullptr;
		info->instruction = 0;

		_top = info;
	}
}



namespace kpl::runtime
{
	RegisterStack::RegisterStack(Size size) :
		_base{ utils::malloc<Register>(size) },
		_bottom{ nullptr },
		_regs{ nullptr },
		_top{ nullptr }
	{}

	RegisterStack::~RegisterStack()
	{
		if (_base)
			utils::free(_base);

		_base = _bottom = _regs = _top = nullptr;
	}

	void RegisterStack::set(const CallInfo& info)
	{
		if (info.function)
		{
			_bottom = info.bottom;
			_regs = _bottom + 1;
			_top = info.top;
		}
		else
		{
			CallInfo* prev = info.prev;
			if (prev)
			{
				_bottom = prev->bottom;
				_regs = _bottom + 1;
				_top = prev->top;
			}
			else _bottom = _regs = _top = nullptr;
		}
	}

	void RegisterStack::set(const Function& func, const Value& self, int bottom_reg, unsigned int args)
	{
		unsigned int regs = func.chunk().register_count();
		if (regs > 0)
		{
			if (!_top)
			{
				_bottom = _base;
				_regs = _bottom + 1;
				_top = _regs + (regs - 1);
			}
			else
			{
				_bottom = bottom_reg < 0 ? (_top + 1) : _bottom + bottom_reg;
				_regs = _bottom + 1;
				_top = _regs + (regs - 1);
			}

			if(args < regs)
				std::memset(_regs + args, 0, sizeof(*_regs) * (regs - args));
		}
		else if(_top)
		{
			_bottom = bottom_reg < 0 ? (_top + 1) : _bottom + bottom_reg;
			_regs = _bottom + 1;
			_top = _regs;
		}

		*_bottom = self;
	}

	void RegisterStack::push_args(const CallArguments& args, unsigned int max_args)
	{
		if (args)
		{
			const Value* args_data = args.data();
			const Size len = args.size();
			for (Offset i = 0; i < max_args && i < len; ++i)
				_regs[i] = args_data[i];
		}
	}

	void RegisterStack::close()
	{
		for (Register* r = _bottom; r <= _top; ++r)
			r->invalidate();
	}
}




namespace kpl::runtime
{
	struct RuntimeState
	{
		InstructionCode inst;
		Offset inst_offset;

		Function* function;
		Chunk* chunk;

		Value* ret_value;
		bool end;
	};



#define end_inst goto next_instruction
#define to_end goto runtime_end

#define A __KPL_INST_ARG_A(runtime.inst)
#define B __KPL_INST_ARG_B(runtime.inst)
#define C __KPL_INST_ARG_C(runtime.inst)

#define KB __KPL_INST_ARG_KB(runtime.inst)
#define KC __KPL_INST_ARG_KC(runtime.inst)

#define Bx __KPL_INST_ARG_BX(runtime.inst)
#define sBx __KPL_INST_ARG_SBX(runtime.inst)

#define Ax __KPL_INST_ARG_AX(runtime.inst)
#define sAx __KPL_INST_ARG_SAX(runtime.inst)

#define REGS state._regs
#define R(_Index) REGS.reg(_Index)

#define Kst(_Index) runtime.chunk->constant(_Index)

#define RK(_Index, _KstTest) (_KstTest ? Kst(_Index) : R(_Index))
#define RKB RK(B, KB)
#define RKC RK(C, KC)


	static inline bool end_call(RuntimeState& runtime, CallStack& calls, RegisterStack& regs, const Register* ret_reg)
	{
		CallInfo* info = calls.top();
		runtime.function = info->function;
		runtime.chunk = runtime.function ? &runtime.function->chunk() : nullptr;
		runtime.inst_offset = info->instruction;
		if (runtime.end = !runtime.function)
		{
			if (ret_reg)
				*runtime.ret_value = *ret_reg;
			else *runtime.ret_value = nullptr;
		}

		regs.close();
		regs.set(*info);
		calls.pop();

		return runtime.end;
	}

	Value execute(KPLState& state, Function& function, const Value& self, const CallArguments& args)
	{
		Value ret_value;
		RuntimeState runtime;
		state._calls.push_native();
		state._regs.set(function, self);

		runtime.chunk = &function.chunk();
		runtime.function = &function;
		runtime.inst_offset = 0;
		runtime.ret_value = &ret_value;
		runtime.end = false;

		state._regs.push_args(args, runtime.chunk->register_count());
		state._regs.set_self(self);

	next_instruction:
		runtime.inst = runtime.chunk->instruction(runtime.inst_offset++);

	exec_instruction:
		std::cout << static_cast<inst::Instruction>(runtime.inst) << std::endl;

		switch (__KPL_INST_ARG_OPCODE(runtime.inst))
		{
			case opcode::id::NOP:
				end_inst;

			case opcode::id::MOVE:
				REGS.move(A, B);
				end_inst;

			case opcode::id::LOAD_K:
				R(A) = Kst(Bx);
				end_inst;

			case opcode::id::LOAD_BOOL:
				R(A) = B ? true : false;
				if (C)
					runtime.inst++;
				end_inst;

			case opcode::id::LOAD_NULL: {
				unsigned int to = B;
				for (unsigned int i = A; i <= to; ++i)
					R(i) = nullptr;
			} end_inst;

			case opcode::id::LOAD_INT:
				R(A) = static_cast<type::Integer>(sBx);
				end_inst;

			case opcode::id::GET_GLOBAL:
				R(A) = state._globals.get_value(RKB);
				end_inst;

			case opcode::id::GET_LOCAL:
				R(A) = runtime.function->get_local(RKB);
				end_inst;

			case opcode::id::GET_PROP:
				R(A) = RKB.get_property(RKC);
				end_inst;

			case opcode::id::SET_GLOBAL:
				state._globals.set_value(RKB, RKC);
				end_inst;

			case opcode::id::SET_LOCAL:
				runtime.function->set_local(RKB, RKC);
				end_inst;

			case opcode::id::SET_PROP:
				R(A).set_property(RKB, RKC);
				end_inst;

			case opcode::id::NEW_ARRAY:
				R(A) = state._heap.make_array(static_cast<Size>(RKB.to_integer()));
				end_inst;

			case opcode::id::NEW_LIST:
				R(A) = state._heap.make_list();
				end_inst;

			case opcode::id::NEW_OBJECT:
				if (C)
					R(A) = state._heap.make_object(RKB);
				else R(A) = state._heap.make_object();
				end_inst;

			case opcode::id::SET_AL: {
				const Value& iterable = R(A);

				switch (iterable.type())
				{
					case DataType::Array: {
						type::Array& array = iterable.array();
						Offset offset = 0;
						for (Register* r = &R(B), *end = &R(C); r <= end; ++r)
							array[offset++] = *r;
					} break;

					case DataType::List:
						type::List& list = iterable.list();
						for (Register* r = &R(B), *end = &R(C); r <= end; ++r)
							list.push_back(*r);
						break;
				}
			} end_inst;

			case opcode::id::SELF:
				REGS.write(A, REGS.self());
				end_inst;

			case opcode::id::ADD:
				R(A) = RKB.runtime_add(RKC, state);
				end_inst;

			case opcode::id::SUB:
				R(A) = RKB.runtime_sub(RKC, state);
				end_inst;

			case opcode::id::MUL:
				R(A) = RKB.runtime_mul(RKC, state);
				end_inst;

			case opcode::id::DIV:
				R(A) = RKB.runtime_div(RKC, state);
				end_inst;

			case opcode::id::IDIV:
				R(A) = RKB.runtime_idiv(RKC, state);
				end_inst;

			case opcode::id::MOD:
				R(A) = RKB.runtime_mod(RKC, state);
				end_inst;

			case opcode::id::EQ:
				if (RKB.runtime_eq(RKC, state).to_bool())
					++runtime.inst_offset;
				end_inst;

			case opcode::id::NE:
				if (RKB.runtime_ne(RKC, state).to_bool())
					++runtime.inst_offset;
				end_inst;

			case opcode::id::GR:
				if (RKB.runtime_gr(RKC, state).to_bool())
					++runtime.inst_offset;
				end_inst;

			case opcode::id::LS:
				if (RKB.runtime_ls(RKC, state).to_bool())
					++runtime.inst_offset;
				end_inst;

			case opcode::id::GE:
				if (RKB.runtime_ge(RKC, state).to_bool())
					++runtime.inst_offset;
				end_inst;

			case opcode::id::LE:
				if (RKB.runtime_le(RKC, state).to_bool())
					++runtime.inst_offset;
				end_inst;

			case opcode::id::SHL:
				R(A) = RKB.runtime_shl(RKC, state);
				end_inst;

			case opcode::id::SHR:
				R(A) = RKB.runtime_shr(RKC, state);
				end_inst;

			case opcode::id::BAND:
				R(A) = RKB.runtime_band(RKC, state);
				end_inst;

			case opcode::id::BOR:
				R(A) = RKB.runtime_bor(RKC, state);
				end_inst;

			case opcode::id::XOR:
				R(A) = RKB.runtime_xor(RKC, state);
				end_inst;

			case opcode::id::BNOT:
				R(A) = RKB.runtime_bnot(state);
				end_inst;

			case opcode::id::NOT:
				R(A) = RKB.runtime_not(state);
				end_inst;

			case opcode::id::NEG:
				R(A) = RKB.runtime_neg(state);
				end_inst;

			case opcode::id::LEN:
				R(A) = RKB.runtime_length(state);
				end_inst;

			case opcode::id::IN:
				R(A) = RKB.runtime_in(RKC, state);
				end_inst;

			case opcode::id::INSTANCEOF:
				R(A) = RKB.runtime_instanceof(RKC, state);
				end_inst;

			case opcode::id::GET:
				R(A) = RKB.runtime_subscrived_get(RKC, state);
				end_inst;

			case opcode::id::SET:
				R(A).runtime_subscrived_set(RKB, RKC, state);
				end_inst;

			case opcode::id::JP:
				runtime.inst_offset = Ax;
				runtime.inst = runtime.chunk->instruction(runtime.inst_offset);
				goto exec_instruction;

			case opcode::id::TEST:
				if (RKB.to_bool() == static_cast<bool>(B))
					runtime.inst_offset++;
				end_inst;

			case opcode::id::TEST_SET:
				if (RKB.to_bool() == static_cast<bool>(B))
					runtime.inst_offset++;
				else R(A) = RKB;
				end_inst;

			case opcode::id::CALL: {
				Value& callable = R(A);
				if (callable.type() == DataType::Function)
				{
					state._calls.push(state._regs, *runtime.function, runtime.inst_offset);
					
					runtime.function = &callable.function();
					runtime.chunk = &runtime.function->chunk();
					runtime.inst_offset = 0;

					state._regs.set(*runtime.function, type::literal::Null, static_cast<int>(A), B);
				}
				else
				{
					callable.runtime_call(state, type::literal::Null, { (&callable + 1), B });
				}
			} end_inst;

			case opcode::id::INVOKE: {
				Value& object = R(A);
				object.invoke(state, RKB, { (&object + 1), C });
			} end_inst;

			case opcode::id::RETURN:
				if (A)
					REGS.write(0, RKB);
				else REGS.reg(0) = nullptr;
				if (end_call(runtime, state._calls, state._regs, &R(0)))
					to_end;
				end_inst;
		}

		runtime_end:
		return ret_value;
	}
}
