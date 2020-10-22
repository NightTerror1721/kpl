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

	void RegisterStack::push_params(const Parameters& params, unsigned int args)
	{
		const auto& v = params.vector();
		auto it = v.begin();
		const auto& end = v.end();
		for (unsigned int i = 0; i < args && it != end; ++i)
			_regs[i] = **it;
	}

	void RegisterStack::close()
	{
		for (Register* r = _bottom; r <= _top; ++r)
			r->invalidate();
	}
}



namespace kpl::runtime
{
	Parameters::Parameters(const std::initializer_list<ConstWeakValueReference> pars) :
		_pars{ pars.size() }
	{
		for (const Value& par : pars)
			_pars.push_back(&par);
	}

	Parameters& Parameters::_ensure(Size size)
	{
		if (_pars.size() < size)
			_pars.resize(size, &type::literal::Null);
		return *this;
	}

	void Parameters::fill(unsigned int size, const Value& value)
	{
		_ensure(size);
		std::fill_n(_pars.begin(), size, &value);
	}
	void Parameters::fill(unsigned int from, unsigned int to, const Value& value)
	{
		_ensure(to);
		std::fill_n(_pars.begin() + from, to - from, &value);
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
	};



#define end_inst goto execute_instruction

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

#define RK(_Index, _KstTest) (_KstTest ? R(_Index) : Kst(_Index))
#define RKB RK(B, KB)
#define RKC RK(C, KC)


	static inline void end_call(RuntimeState& runtime, CallStack& calls, RegisterStack& regs)
	{
		CallInfo* info = calls.top();
		runtime.function = info->function;
		runtime.chunk = &runtime.function->chunk();
		runtime.inst_offset = info->instruction;

		regs.close();
		regs.set(*info);
		calls.pop();
	}

	void execute(KPLState& state, Function& function, const Parameters& args)
	{
		RuntimeState runtime;
		state._calls.push_native();
		state._regs.set(function, args.self());

		runtime.chunk = &function.chunk();
		runtime.function = &function;
		runtime.inst_offset = 0;

		if (args)
			state._regs.push_params(args, runtime.chunk->register_count());
		state._regs.set_self(args.self());

		execute_instruction:
		switch (__KPL_INST_ARG_OPCODE(runtime.inst = runtime.chunk->instruction(runtime.inst_offset++)))
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





			case opcode::id::CALL: {
				const Value& callable = R(A);
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
					callable.runtime_call({ &R(A), B, &type::literal::Null });
				}
			} end_inst;


			case opcode::id::RETURN:
				if (A)
					REGS.write(0, RKB);
				else REGS.reg(0) = nullptr;
				end_call(runtime, state._calls, state._regs);
				end_inst;

		}
	}
}
