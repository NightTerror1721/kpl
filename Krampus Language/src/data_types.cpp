#include "data_types.h"
#include "runtime.h"
#include "kplstate.h"
#include "object_utils.h"

namespace kpl
{
	static inline BadValueOperation op_error(const char* operation, const Value& left, const Value& right)
	{
		return { std::string("Cannot") + operation + " " + data_type_name(left.type()) + "with " + data_type_name(right.type()) };
	}


	Value Value::runtime_add(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.integral + right._value.integral;
					case DataType::Float:
						return static_cast<double>(_value.integral) + right._value.floating;
					default: goto error;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.floating + static_cast<double>(right._value.integral);
					case DataType::Float:
						return _value.floating + right._value.floating;
					default: goto error;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String: {
				switch (right._type)
				{
					case DataType::String:
						return state.heap().make_string(*_value.string + *right._value.string);
					default: goto error;
				}
			} break;
			case DataType::Array: {
				switch (right._type)
				{
					case DataType::Array:
						return type::Array::runtime_concat(*_value.array, *right._value.array, state.heap());
					case DataType::List:
						return type::Array::runtime_concat(*_value.array, *right._value.list, state.heap());
					default: goto error;
				}
			} break;
			case DataType::List: {
				switch (right._type)
				{
					case DataType::Array:
						return type::List::runtime_concat(*_value.list, *right._value.array, state.heap());
					case DataType::List:
						return type::List::runtime_concat(*_value.list, *right._value.list, state.heap());
					default: goto error;
				}
			} break;
			case DataType::Object:
				return invoke(state, std::string(obj::special_property::operator_add), right);
			case DataType::Function: break;
			case DataType::Userdata: break;
		}

		error:
		throw op_error("add", *this, right);
	}
	//Value Value::runtime_sub(const Value& right, KPLState& state) const;
	//Value Value::runtime_mul(const Value& right, KPLState& state) const;
	//Value Value::runtime_div(const Value& right, KPLState& state) const;
	//Value Value::runtime_idiv(const Value& right, KPLState& state) const;
	//Value Value::runtime_mod(const Value& right, KPLState& state) const;

	//Value Value::runtime_eq(const Value& right, KPLState& state) const;
	//Value Value::runtime_ne(const Value& right, KPLState& state) const;
	//Value Value::runtime_gr(const Value& right, KPLState& state) const;
	//Value Value::runtime_ls(const Value& right, KPLState& state) const;
	//Value Value::runtime_ge(const Value& right, KPLState& state) const;
	//Value Value::runtime_le(const Value& right, KPLState& state) const;

	//Value Value::runtime_shl(const Value& right, KPLState& state) const;
	//Value Value::runtime_shr(const Value& right, KPLState& state) const;
	//Value Value::runtime_band(const Value& right, KPLState& state) const;
	//Value Value::runtime_bor(const Value& right, KPLState& state) const;
	//Value Value::runtime_xor(const Value& right, KPLState& state) const;

	//Value Value::runtime_length(KPLState& state) const;
	//Value Value::runtime_not(KPLState& state) const;
	//Value Value::runtime_bnot(KPLState& state) const;
	//Value Value::runtime_neg(KPLState& state) const;

	//Value Value::runtime_subscrived_get(const Value& index, MemoryHeap& heap) const;
	//Value Value::runtime_subscrived_set(const Value& index, const Value& value, MemoryHeap& heap);



	void Value::set_property(const std::string& name, const Value& value)
	{
		switch (_type)
		{
			case DataType::Null: break;
			case DataType::Integer: break;
			case DataType::Float: break;
			case DataType::Boolean: break;
			case DataType::String: break;
			case DataType::Array: break;
			case DataType::List: break;
			case DataType::Object:
				_value.object->set_property(name, value);
				break;
			case DataType::Function: break;
			case DataType::Userdata: break;
		}
	}

	const Value& Value::get_property(const std::string& name) const
	{
		switch (_type)
		{
			case DataType::Null: break;
			case DataType::Integer: break;
			case DataType::Float: break;
			case DataType::Boolean: break;
			case DataType::String: break;
			case DataType::Array: break;
			case DataType::List: break;
			case DataType::Object:
				return _value.object->get_property(name);
			case DataType::Function: break;
			case DataType::Userdata: break;
		}

		return type::literal::Null;
	}

	void Value::del_property(const std::string& name)
	{
		switch (_type)
		{
			case DataType::Null: break;
			case DataType::Integer: break;
			case DataType::Float: break;
			case DataType::Boolean: break;
			case DataType::String: break;
			case DataType::Array: break;
			case DataType::List: break;
			case DataType::Object:
				_value.object->del_property(name);
				break;
			case DataType::Function: break;
			case DataType::Userdata: break;
		}
	}



	std::string Value::to_string() const
	{
		switch (_type)
		{
			case DataType::Null: return "null";
			case DataType::Integer: return std::to_string(_value.integral);
			case DataType::Float: return std::to_string(_value.floating);
			case DataType::Boolean: return _value.boolean ? "true" : "false";
			case DataType::String: return *_value.string;
			case DataType::Array: return _value.array->to_string();
			case DataType::List: return _value.list->to_string();
			case DataType::Object: return _value.object->to_string();
			case DataType::Function: return _value.function->to_string();
			case DataType::Userdata: return _value.userdata->to_string();
		}

		return "";
	}

	Int64 Value::to_integer() const
	{
		switch (_type)
		{
			case DataType::Null: return 0;
			case DataType::Integer: return _value.integral;
			case DataType::Float: return static_cast<Int64>(_value.floating);
			case DataType::Boolean: return _value.boolean;
			case DataType::String: return std::stoll(*_value.string);
			case DataType::Array: return 0;
			case DataType::List: return 0;
			case DataType::Object: return 0;
			case DataType::Function: return 0;
			case DataType::Userdata: return 0;
		}

		return 0;
	}



	Value Value::runtime_call(KPLState& state, const runtime::Arguments& args) const
	{
		return nullptr;
	}

	Value Value::call(KPLState& state, const runtime::Parameters& args) const
	{
		return nullptr;
	}

	Value Value::runtime_invoke(KPLState& state, const std::string& name, const runtime::Arguments& args) const
	{
		return get_property(name).runtime_call(state, { args, *this });
	}

	Value Value::runtime_invoke(KPLState& state, const Value& name, const runtime::Arguments& args) const
	{
		return get_property(name).runtime_call(state, { args, *this });
	}

	Value Value::invoke(KPLState& state, const std::string& name, const runtime::Parameters& args) const
	{
		return get_property(name).call(state, args.self(*this));
	}

	Value Value::invoke(KPLState& state, const Value& name, const runtime::Parameters& args) const
	{
		return get_property(name).call(state, args.self(*this));
	}
}



namespace kpl::type::literal
{
	const Value Null = nullptr;
	const Value True = true;
	const Value False = false;
	const Value One = 1LL;
	const Value Zero = 0LL;
	const Value Minusone = -1LL;
}



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

	std::string Array::to_string() const
	{
		if (_length == 0)
			return "[]";

		std::stringstream ss;
		ss << "[";

		for (Offset i = 0; i < _length; ++i)
		{
			if (i > 0)
				ss << ", ";
			ss << _array[i].to_string();
		}
		
		return ss << "]", ss.str();
	}

	Value Array::runtime_concat(const Array& left, const Array& right, MemoryHeap& heap)
	{
		Value value = heap.make_array(left._length + right._length);
		Array& array = value.array();

		if (array._length > 0)
		{
			Offset offset = 0;
			for (offset = 0; offset < left._length; ++offset)
				array._array[offset] = left._array[offset];

			for (Offset i = 0; i < right._length; ++i)
				array._array[offset + i] = right._array[i];
		}
		return value;
	}

	Value Array::runtime_concat(const Array& left, const List& right, MemoryHeap& heap)
	{
		Value value = heap.make_array(left._length + right.size());
		Array& array = value.array();

		if (array._length > 0)
		{
			Offset offset = 0;
			for (offset = 0; offset < left._length; ++offset)
				array._array[offset] = left._array[offset];

			for (const Value& elem : right)
				array._array[offset++] = elem;
		}
		return value;
	}
}



namespace kpl::type
{
	void List::_mheap_delete(void* block) { reinterpret_cast<List*>(block)->~List(); }

	std::string List::to_string() const
	{
		if (empty())
			return "[]";

		std::stringstream ss;
		ss << "[";

		bool first = true;
		for (const Value& value : *this)
		{
			if (first)
				first = false;
			else ss << ", ";
			ss << value.to_string();
		}

		return ss << "]", ss.str();
	}

	Value List::runtime_concat(const List& left, const List& right, MemoryHeap& heap)
	{
		Value value = heap.make_list(left);
		List& list = value.list();

		for (const Value& elem : right)
			list.push_back(elem);

		return value;
	}

	Value List::runtime_concat(const List& left, const Array& right, MemoryHeap& heap)
	{
		Value value = heap.make_list(left);
		List& list = value.list();

		if (!right.empty())
		{
			Size len = right.length();
			for (Offset i = 0; i < len; ++i)
				list.push_back(right[i]);
		}

		return value;
	}
}



namespace kpl::type
{
	void Object::_mheap_delete(void* block) { reinterpret_cast<Object*>(block)->~Object(); }

	std::string Object::to_string() const
	{
		if (empty())
			return "{}";

		std::stringstream ss;
		ss << "{";

		bool first = true;
		for (const auto& property : *this)
		{
			if (first)
				first = false;
			else ss << ", ";
			ss << property.first << ":" << property.second.to_string();
		}

		return ss << "}", ss.str();
	}

	const Value& Object::get_property(const std::string& name) const
	{
		const auto& it = find(name);
		if (it != end())
			return it->second;

		return _class.get_property(name);
	}
}



namespace kpl::type
{
	void Function::_mheap_delete(void* block) { reinterpret_cast<Function*>(block)->~Function(); }

	std::string Function::to_string() const
	{
		std::stringstream ss;
		ss << "function@" << this;
		return ss.str();
	}
}



namespace kpl::type
{
	std::string Userdata::to_string() const
	{
		std::stringstream ss;
		ss << "userdata@" << this;
		return ss.str();
	}
}
