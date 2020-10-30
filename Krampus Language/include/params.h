#pragma once

#include "common.h"

namespace kpl
{
	class CallArguments
	{
	private:
		Size _size;
		Value* _args;
		bool _allocated;

	public:
		inline CallArguments(Value* args, Size count) :
			_args{ args },
			_size{ count },
			_allocated{ false }
		{}

		inline CallArguments() :
			_args{ nullptr },
			_size{ 0 },
			_allocated{ false }
		{}

		inline operator bool() const { return _size; }
		inline bool operator! () const { return !_size; }

		inline bool empty() const { return !_size; }
		inline Size size() const { return _size; }

		inline const Value* data() const { return _args; }

	public:
		CallArguments(const std::initializer_list<ConstWeakValueReference>& list);
		CallArguments(const std::vector<ConstWeakValueReference>& vector);
		CallArguments(const Value& arg);
		~CallArguments();

		const Value& operator[] (Offset index) const;
	};
}
