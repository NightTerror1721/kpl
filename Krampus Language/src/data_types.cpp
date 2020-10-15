#include "data_types.h"

namespace kpl::type
{
	void String::_mheap_delete(void* block)
	{
		reinterpret_cast<String*>(block)->~String();
	}
}



namespace kpl::type
{
	void Array::_mheap_delete(void* block) { reinterpret_cast<Array*>(block)->~Array(); }

	Array::Array(Size length) :
		_array{ length > 0 ? new Value[length] : nullptr },
		_length{ length }
	{}
	Array::Array(const Value* array, Size length) :
		Array(length)
	{
		for (Offset i = 0; i < length; ++i)
			_array[i] = array[i];
	}
	Array::Array(const std::vector<Value>& vector) :
		Array(vector.data(), vector.size())
	{}
	Array::Array(std::vector<Value>&& vector) :
		Array(vector.size())
	{
		Value* array = vector.data();
		for (Offset i = 0; i < _length; ++i)
			_array[i] = std::move(array[i]);
	}
	Array::Array(const std::initializer_list<Value>& args) :
		Array(args.size())
	{
		Offset i = 0;
		for (const Value& arg : args)
			_array[i++] = arg;
	}
	Array::~Array()
	{
		if (_length > 0)
			delete[] _array;
	}
}



namespace kpl::type
{
	void List::_mheap_delete(void* block) { reinterpret_cast<List*>(block)->~List(); }
}



namespace kpl::type
{
	void Object::_mheap_delete(void* block) { reinterpret_cast<Object*>(block)->~Object(); }
}
