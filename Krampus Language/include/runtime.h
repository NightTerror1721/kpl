#pragma once

#include "instruction.h"
#include "data_types.h"

namespace kpl::runtime
{
	typedef Value Register;
	typedef type::Function Function;
	class Parameters;

	struct CallInfo
	{
		Register* top;
		Register* bottom;
		Function* function;
		CallInfo* prev;
		Offset instruction;
	};

	class RegisterStack;

	class CallStack
	{
	public:
		static constexpr int default_size = 32768 * sizeof(CallInfo);

	private:
		CallInfo* _base;
		CallInfo* _top;

	public:
		CallStack(const CallStack&) = delete;
		CallStack(CallStack&&) = delete;

		CallStack& operator= (const CallStack&) = delete;
		CallStack& operator= (CallStack&&) = delete;

		CallStack(Size size = default_size);
		~CallStack();
		
		CallInfo* push(RegisterStack& regs, Function& function, Offset instruction);
		CallInfo* pop();

		void push_native();

		inline CallInfo* top() { return _top; }
	};

	class RegisterStack
	{
	public:
		static constexpr int default_size = 65536 * sizeof(Register);

	private:
		Register* _base;

		Register* _top;
		Register* _regs;
		Register* _bottom;

	public:
		RegisterStack(const RegisterStack&) = delete;
		RegisterStack(RegisterStack&&) = delete;

		RegisterStack& operator= (const RegisterStack&) = delete;
		RegisterStack& operator= (RegisterStack&&) = delete;

		RegisterStack(Size size = default_size);
		~RegisterStack();

		void set(const CallInfo& info);
		void set(const Function& func, const Value& self, int bottom_reg = -1, unsigned int args = 0);

		void push_args(const CallArguments& args, unsigned int max_args);

		void close();

		inline void write(unsigned int id, const Value& value) { _regs[id] = value; }
		inline const Value& read(unsigned int id) { return _regs[id]; }
		inline void move(unsigned int dst, unsigned int src) { _regs[dst] = _regs[src]; }

		inline Value& reg(unsigned int id) { return _regs[id]; }
		inline const Value& reg(unsigned int id) const { return _regs[id]; }

		inline Value& self() { return *_bottom; }
		inline const Value& self() const { return *_bottom; }

		inline void set_self(const Value& value) { *_bottom = value; }

		friend class CallStack;
	};




	Value execute(KPLState& state, Function& function, const Value& self, const CallArguments& args = CallArguments());
}
