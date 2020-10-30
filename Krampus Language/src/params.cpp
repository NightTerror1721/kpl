#include "params.h"
#include "data_types.h"

namespace kpl
{
	CallArguments::CallArguments(const std::initializer_list<ConstWeakValueReference>& list) :
		_size{ list.size() },
		_args{ _size > 0 ? new Value[_size] : nullptr },
		_allocated{ true }
	{
		if (_args)
		{
			auto it = list.begin();
			const auto end = list.end();
			for (Offset i = 0; i < _size; ++i, ++it)
				_args[i] = *it;
		}
	}

	CallArguments::CallArguments(const std::vector<ConstWeakValueReference>& vector) :
		_size{ vector.size() },
		_args{ _size > 0 ? new Value[_size] : nullptr },
		_allocated{ true }
	{
		if (_args)
		{
			const ConstWeakValueReference* data = vector.data();
			for (Offset i = 0; i < _size; ++i, ++data)
				_args[i] = *data;
		}
	}

	CallArguments::CallArguments(const Value& arg) :
		_args{ new Value[1] },
		_size{ 1 },
		_allocated{ true }
	{
		_args[0] = arg;
	}

	CallArguments::~CallArguments()
	{
		if (_allocated && _args)
			delete[] _args;
	}

	const Value& CallArguments::operator[] (Offset index) const
	{
		if (index < _size)
			return _args[index];
		return type::literal::Null;
	}
}

