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

	CallInfo* CallStack::push(Function& function, unsigned int from_register, Offset instruction)
	{
		CallInfo* info = _top + 1;

		info->prev = _top;
		info->function = &function;
		info->bottom = _top->bottom + from_register;
		info->top = info->bottom + function.chunk().register_count();
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
		CallInfo* info = _top;

		info->prev = _top;
		info->function = nullptr;
		info->bottom = nullptr;
		info->top = nullptr;
		info->instruction = 0;

		_top = info + 1;
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

		_base = _bottom = _top = nullptr;
	}

	void RegisterStack::set(const CallInfo& info)
	{
		if (info.function)
		{
			_bottom = info.bottom;
			_top = info.top;
		}
		else
		{
			CallInfo* prev = info.prev;
			if (prev)
			{
				_bottom = prev->bottom;
				_top = prev->top;
			}
			else _bottom = _top = nullptr;
		}
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

#define A inst::arg::a(runtime.inst)
#define B inst::arg::b(runtime.inst)
#define C inst::arg::c(runtime.inst)

#define KB inst::arg::kb(runtime.inst)
#define KC inst::arg::kc(runtime.inst)

#define Bx inst::arg::bx(runtime.inst)
#define sBx inst::arg::sbx(runtime.inst)

#define Ax inst::arg::bx(runtime.inst)
#define sAx inst::arg::sbx(runtime.inst)

#define REGS state._regs
#define R(_Index) REGS.reg(_Index)

#define Kst(_Index) runtime.chunk->constant(_Index)


	void execute(KPLState& state, Function& function)
	{
		RuntimeState runtime;
		state._calls.push_native();

		runtime.chunk = &function.chunk();
		runtime.function = &function;
		runtime.inst_offset = 0;

		execute_instruction:
		switch (inst::arg::opcode(runtime.inst = runtime.chunk->instruction(runtime.inst_offset++)))
		{
			case opcode::id::NOP:
				end_inst;

			case opcode::id::MOVE:
				REGS.move(A, B);
				end_inst;

			case opcode::id::LOADL_K:
				R(A) = Kst(Bx);
				end_inst;

			case opcode::id::LOAD_BOOL:
				R(A) = B ? true : false;
				if (C)
					runtime.inst++;
				end_inst;

			case opcode::id::LOAD_NULL: {
				unsigned int reg = A;
				for (unsigned int i = B; i > 0; --i)
					R(i) = nullptr;
			} end_inst;

		}
	}
}
