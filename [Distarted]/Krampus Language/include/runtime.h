#pragma once

#include "kplstate.h"
#include "instruction.h"
#include "data_types.h"

namespace kpl::runtime
{
	typedef ValueReference Register;
	typedef type::Function Function;

	struct CallInfo
	{
		Register* top;
		Register* bottom;
		Function* function;
		CallInfo* prev;
	};

	class CallStack
	{
	public:
		static constexpr int default_size = 65536 * sizeof(CallInfo);

	private:
		CallInfo* _stack;

	public:
		CallStack(const CallStack&) = delete;
		CallStack(CallStack&&) = delete;

		CallStack& operator= (const CallStack&) = delete;
		CallStack& operator= (CallStack&&) = delete;

		CallStack(Size size = default_size);
		~CallStack();
		

	};
}
