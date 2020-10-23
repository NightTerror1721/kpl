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

		void push_params(const Parameters& params, unsigned int args);

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

	class Arguments
	{
	private:
		const Register* const _self;
		const Register* const _regs;
		const Size _size;

	public:
		inline Arguments(const Register* args, Size count) :
			_self{ &type::literal::Null },
			_regs{ args },
			_size{ count }
		{}

		inline Arguments(const Arguments& args, const Value& self) :
			_self{ &self },
			_regs{ args._regs },
			_size{ args._size }
		{}

		inline operator bool() const { return _size; }
		inline bool operator! () const { return !_size; }

		inline bool empty() const { return !_size; }
		inline Size size() const { return _size; }

		inline const Value& operator[] (unsigned int index) const { return index >= _size ? type::literal::Null : _regs[index]; }

		inline const Value& at(unsigned int index) const { return index >= _size ? type::literal::Null : _regs[index]; }

		inline const Value& self() const { return _self; }
	};

	class Parameters
	{
	private:
		mutable const Value* _self = &type::literal::Null;
		std::vector<const Value*> _pars;

	private:
		Parameters& _ensure(Size size);

	public:
		class Param
		{
		private:
			const Value** _param;

			inline Param(const Value** param) : _param{ param } {}

		public:
			inline Param& operator= (const Value& right) { return *_param = &right, *this; }

			inline operator const Value& () const { return **_param; }

			friend class Parameters;
		};

	public:
		Parameters() = default;
		Parameters(const Parameters&) = default;
		Parameters(Parameters&&) noexcept = default;
		~Parameters() = default;

		Parameters& operator= (const Parameters&) = default;
		Parameters& operator= (Parameters&&) noexcept = default;

		inline Parameters(const std::vector<const Value*>& pars) : _pars{ pars } {}
		inline Parameters(std::vector<const Value*>&& pars) noexcept : _pars{ std::move(pars) } {}

		Parameters(const std::initializer_list<ConstWeakValueReference> pars);

		inline Parameters(const Value& arg) : Parameters{ arg } {}

		inline operator bool() const { return !_pars.empty(); }
		inline bool operator! () const { return _pars.empty(); }

		inline bool empty() const { return _pars.empty(); }
		inline Size size() const { return _pars.size(); }

		inline Param operator[] (unsigned int index) { return &_ensure(index + 1)._pars.at(index); }
		inline const Param operator[] (unsigned int index) const { return const_cast<const Value**>(&_pars.at(index)); }

		inline void push(const Value& param) { _pars.push_back(&param); }

		inline void set(unsigned int index, const Value& param) { _ensure(index + 1)._pars.at(index) = &param; }

		void fill(unsigned int size, const Value& value = type::literal::Null);
		void fill(unsigned int from, unsigned int to, const Value& value = type::literal::Null);

		inline Parameters& self(const Value& value) { return _self = &value, *this; }
		inline const Parameters& self(const Value& value) const { return _self = &value, *this; }
		inline const Value& self() const { return *_self; }

		inline const std::vector<const Value*>& vector() const { return _pars; }
	};




	void execute(KPLState& state, Function& function, const Parameters& args = Parameters());
}
