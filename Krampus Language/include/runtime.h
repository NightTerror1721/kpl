#pragma once

#include "instruction.h"
#include "data_types.h"

namespace kpl::runtime
{
	typedef Value Register;
	typedef type::Function Function;

	struct CallInfo
	{
		Register* top;
		Register* bottom;
		Function* function;
		CallInfo* prev;
		Offset instruction;
	};

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
		
		CallInfo* push(Function& function, unsigned int from_register, Offset instruction);
		CallInfo* pop();

		void push_native();
	};

	class RegisterStack
	{
	public:
		static constexpr int default_size = 65536 * sizeof(Register);

	private:
		Register* _base;

		Register* _top;
		Register* _bottom;

	public:
		RegisterStack(const RegisterStack&) = delete;
		RegisterStack(RegisterStack&&) = delete;

		RegisterStack& operator= (const RegisterStack&) = delete;
		RegisterStack& operator= (RegisterStack&&) = delete;

		RegisterStack(Size size = default_size);
		~RegisterStack();

		void set(const CallInfo& info);

		inline void write(unsigned int id, const Value& value) { _bottom[id] = value; }
		inline const Value& read(unsigned int id) { return _bottom[id]; }
		inline void move(unsigned int dst, unsigned int src) { _bottom[dst] = _bottom[src]; }

		inline Value& reg(unsigned int id) { return _bottom[id]; }
		inline const Value& reg(unsigned int id) const { return _bottom[id]; }
	};





	void execute(KPLState& state, Function& function);
}
