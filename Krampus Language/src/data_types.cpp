#include "data_types.h"
#include "runtime.h"
#include "kplstate.h"
#include "object_utils.h"

namespace kpl
{
	void Value::force_destructor_call()
	{
		switch (_type)
		{
			case DataType::String:
				_value.string->~String();
				break;

			case DataType::Array:
				_value.array->~Array();
				break;

			case DataType::List:
				_value.list->~List();
				break;

			case DataType::Object:
				_value.object->~Object();
				break;

			case DataType::Function:
				_value.function->~Function();
				break;
		}
	}



	static inline BadValueOperation op_error(const char* operation, const Value& left, const Value& right)
	{
		return { std::string("Cannot") + operation + " " + data_type_name(left.type()) + " with " + data_type_name(right.type()) };
	}

	static inline BadValueOperation op_error(const char* operation, const Value& right)
	{
		return { std::string("Cannot") + operation + " " + " with " + data_type_name(right.type()) };
	}

	static inline BadValueOperation op_invalid_index(Int64 index, Offset max)
	{
		std::stringstream ss;
		ss << "Index out of range. Valid range [0, " << (max) << "]. But found: " << index << ".";
		return { ss.str() };
	}


	namespace special_props
	{
		static const std::string operator_add = obj::special_property::operator_add;
		static const std::string operator_sub = obj::special_property::operator_sub;
		static const std::string operator_mul = obj::special_property::operator_mul;
		static const std::string operator_div = obj::special_property::operator_div;
		static const std::string operator_idiv = obj::special_property::operator_idiv;
		static const std::string operator_mod = obj::special_property::operator_mod;

		static const std::string operator_eq = obj::special_property::operator_eq;
		static const std::string operator_noeq = obj::special_property::operator_noeq;
		static const std::string operator_gr = obj::special_property::operator_gr;
		static const std::string operator_ls = obj::special_property::operator_ls;
		static const std::string operator_ge = obj::special_property::operator_ge;
		static const std::string operator_le = obj::special_property::operator_le;

		static const std::string operator_shl = obj::special_property::operator_shl;
		static const std::string operator_shr = obj::special_property::operator_shr;
		static const std::string operator_band = obj::special_property::operator_band;
		static const std::string operator_bor = obj::special_property::operator_bor;
		static const std::string operator_bnot = obj::special_property::operator_bnot;

		static const std::string operator_len = obj::special_property::operator_len;
		static const std::string operator_not = obj::special_property::operator_not;
		static const std::string operator_neg = obj::special_property::operator_neg;
		static const std::string operator_xor = obj::special_property::operator_xor;

		static const std::string operator_in = obj::special_property::operator_in;

		static const std::string operator_get = obj::special_property::operator_get;
		static const std::string operator_set = obj::special_property::operator_set;

		static const std::string operator_call = obj::special_property::operator_call;


		static const std::string constructor = obj::special_property::constructor;
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
						return static_cast<type::Float>(_value.integral) + right._value.floating;
					default: goto error;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.floating + static_cast<type::Float>(right._value.integral);
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
						return state.make_string(*_value.string + *right._value.string);
					default: goto error;
				}
			} break;
			case DataType::Array: {
				switch (right._type)
				{
					case DataType::Array:
						return type::Array::runtime_concat(*_value.array, *right._value.array, state);
					case DataType::List:
						return type::Array::runtime_concat(*_value.array, *right._value.list, state);
					default: goto error;
				}
			} break;
			case DataType::List: {
				switch (right._type)
				{
					case DataType::Array:
						return type::List::runtime_concat(*_value.list, *right._value.array, state);
					case DataType::List:
						return type::List::runtime_concat(*_value.list, *right._value.list, state);
					default: goto error;
				}
			} break;
			case DataType::Object:
				return invoke(state, special_props::operator_add, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_add, right);
		}

		error:
		throw op_error("add", *this, right);
	}
	Value Value::runtime_sub(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.integral - right._value.integral;
					case DataType::Float:
						return static_cast<type::Float>(_value.integral) - right._value.floating;
					default: goto error;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.floating - static_cast<type::Float>(right._value.integral);
					case DataType::Float:
						return _value.floating - right._value.floating;
					default: goto error;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String: goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_sub, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_sub, right);
		}

		error:
		throw op_error("sub", *this, right);
	}
	Value Value::runtime_mul(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.integral * right._value.integral;
					case DataType::Float:
						return static_cast<type::Float>(_value.integral) * right._value.floating;
					default: goto error;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.floating * static_cast<type::Float>(right._value.integral);
					case DataType::Float:
						return _value.floating * right._value.floating;
					default: goto error;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String: {
				switch (right._type)
				{
					case DataType::Integer:
						return type::String::runtime_nclone(*_value.string, right._value.integral, state);
					case DataType::Float:
						return type::String::runtime_nclone(*_value.string, static_cast<type::Integer>(right._value.floating), state);
					default: goto error;
				}
			} break;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_mul, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_mul, right);
		}

		error:
		throw op_error("mul", *this, right);
	}
	Value Value::runtime_div(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Float>(_value.integral) / static_cast<type::Float>(right._value.integral);
					case DataType::Float:
						return static_cast<type::Float>(_value.integral) / right._value.floating;
					default: goto error;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.floating / static_cast<type::Float>(right._value.integral);
					case DataType::Float:
						return _value.floating / right._value.floating;
					default: goto error;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String: goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_div, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_div, right);
		}

		error:
		throw op_error("div", *this, right);
	}
	Value Value::runtime_idiv(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.integral / right._value.integral;
					case DataType::Float:
						return static_cast<type::Integer>(_value.integral / right._value.floating);
					default: goto error;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Integer>(_value.floating / static_cast<type::Float>(right._value.integral));
					case DataType::Float:
						return static_cast<type::Integer>(_value.floating / right._value.floating);
					default: goto error;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String: goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_idiv, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_idiv, right);
		}

		error:
		throw op_error("idiv", *this, right);
	}
	Value Value::runtime_mod(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.integral % right._value.integral;
					case DataType::Float:
						return _value.integral % static_cast<type::Integer>(right._value.floating);
					default: goto error;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Integer>(_value.floating) % right._value.integral;
					case DataType::Float:
						return static_cast<type::Integer>(_value.floating) % static_cast<type::Integer>(right._value.floating);
					default: goto error;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String: goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_mod, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_mod, right);
		}

		error:
		throw op_error("mod", *this, right);
	}

	Value Value::runtime_eq(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null:
				return static_cast<type::Boolean>(right._type == DataType::Null);
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.integral == right._value.integral);
					case DataType::Float:
						return static_cast<type::Boolean>(static_cast<type::Float>(_value.integral) == right._value.floating);
					default:
						return false;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.floating == static_cast<type::Float>(right._value.integral));
					case DataType::Float:
						return static_cast<type::Boolean>(_value.floating == right._value.floating);
					default:
						return false;
				}
			} break;
			case DataType::Boolean:
				return right._type == DataType::Boolean && _value.boolean == right._value.boolean;
			case DataType::String:
				return right._type == DataType::String && *_value.string == *right._value.string;
			case DataType::Array:
				return right._type == DataType::Array && type::Array::runtime_eq(*_value.array, *right._value.array, state);
			case DataType::List:
				return right._type == DataType::List && type::List::runtime_eq(*_value.list, *right._value.list, state);
			case DataType::Object:
				if (right._type == DataType::Object)
				{
					const Value& prop = _value.object->get_property(special_props::operator_eq);
					if (prop.isNull())
						return _value.object == right._value.object;
					return prop.runtime_call(state, *this, right);
				}
				return false;
			case DataType::Function:
				return right._type == DataType::Function && _value.function == right._value.function;
			case DataType::Userdata:
				if (right._type == DataType::Userdata)
				{
					const Value& prop = _value.userdata->get_property(special_props::operator_eq);
					if (prop.isNull())
						return _value.userdata == right._value.userdata;
					return prop.runtime_call(state, *this, right);
				}
				return false;
		}

		return false;
	}
	Value Value::runtime_ne(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null:
				return static_cast<type::Boolean>(right._type != DataType::Null);
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.integral != right._value.integral);
					case DataType::Float:
						return static_cast<type::Boolean>(static_cast<type::Float>(_value.integral) != right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.floating != static_cast<type::Float>(right._value.integral));
					case DataType::Float:
						return static_cast<type::Boolean>(_value.floating != right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Boolean:
				return right._type != DataType::Boolean || _value.boolean != right._value.boolean;
			case DataType::String:
				return right._type != DataType::String || *_value.string != *right._value.string;
			case DataType::Array:
				return right._type != DataType::Array || type::Array::runtime_ne(*_value.array, *right._value.array, state);
			case DataType::List:
				return right._type != DataType::List || type::List::runtime_ne(*_value.list, *right._value.list, state);
			case DataType::Object:
				if (right._type == DataType::Object)
				{
					const Value* prop = &_value.object->get_property(special_props::operator_noeq);
					if (prop->isNull())
					{
						prop = &_value.object->get_property(special_props::operator_eq);
						if(prop->isNull())
							return _value.object != right._value.object;
						return !prop->runtime_call(state, *this, right);
					}
					return prop->runtime_call(state, *this, right);
				}
				return true;
			case DataType::Function:
				return right._type == DataType::Function && _value.function == right._value.function;
			case DataType::Userdata:
				if (right._type == DataType::Userdata)
				{
					const Value* prop = &_value.userdata->get_property(special_props::operator_noeq);
					if (prop->isNull())
					{
						prop = &_value.userdata->get_property(special_props::operator_eq);
						if(prop->isNull())
							return _value.userdata == right._value.userdata;
						return !prop->runtime_call(state, *this, right);
					}
					return prop->runtime_call(state, *this, right);
				}
				return true;
		}

		return true;
	}
	Value Value::runtime_gr(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.integral > right._value.integral);
					case DataType::Float:
						return static_cast<type::Boolean>(static_cast<type::Float>(_value.integral) > right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.floating > static_cast<type::Float>(right._value.integral));
					case DataType::Float:
						return static_cast<type::Boolean>(_value.floating > right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String:
				return right._type != DataType::String || *_value.string > *right._value.string;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_gr, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_gr, right);
		}

		error:
		throw op_error("gr", *this, right);
	}
	Value Value::runtime_ls(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.integral < right._value.integral);
					case DataType::Float:
						return static_cast<type::Boolean>(static_cast<type::Float>(_value.integral) < right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.floating < static_cast<type::Float>(right._value.integral));
					case DataType::Float:
						return static_cast<type::Boolean>(_value.floating < right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String:
				return right._type != DataType::String || *_value.string < *right._value.string;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_ls, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_ls, right);
		}

		error:
		throw op_error("ls", *this, right);
	}
	Value Value::runtime_ge(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.integral >= right._value.integral);
					case DataType::Float:
						return static_cast<type::Boolean>(static_cast<type::Float>(_value.integral) >= right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.floating >= static_cast<type::Float>(right._value.integral));
					case DataType::Float:
						return static_cast<type::Boolean>(_value.floating >= right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String:
				return right._type != DataType::String || *_value.string >= *right._value.string;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_ge, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_ge, right);
		}

		error:
		throw op_error("ge", *this, right);
	}
	Value Value::runtime_le(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.integral <= right._value.integral);
					case DataType::Float:
						return static_cast<type::Boolean>(static_cast<type::Float>(_value.integral) <= right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Boolean>(_value.floating <= static_cast<type::Float>(right._value.integral));
					case DataType::Float:
						return static_cast<type::Boolean>(_value.floating <= right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String:
				return right._type != DataType::String || *_value.string <= *right._value.string;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_le, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_le, right);
		}

		error:
		throw op_error("le", *this, right);
	}

	Value Value::runtime_shl(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.integral << right._value.integral;
					case DataType::Float:
						return _value.integral << static_cast<type::Integer>(right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Integer>(_value.floating) << right._value.integral;
					case DataType::Float:
						return static_cast<type::Integer>(_value.floating) << static_cast<type::Integer>(right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String:goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_shl, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_shl, right);
		}

		error:
		throw op_error("shl", *this, right);
	}
	Value Value::runtime_shr(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.integral >> right._value.integral;
					case DataType::Float:
						return _value.integral >> static_cast<type::Integer>(right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Integer>(_value.floating) >> right._value.integral;
					case DataType::Float:
						return static_cast<type::Integer>(_value.floating) >> static_cast<type::Integer>(right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String:goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_shr, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_shr, right);
		}

		error:
		throw op_error("shr", *this, right);
	}
	Value Value::runtime_band(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.integral & right._value.integral;
					case DataType::Float:
						return _value.integral & static_cast<type::Integer>(right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Integer>(_value.floating) & right._value.integral;
					case DataType::Float:
						return static_cast<type::Integer>(_value.floating) & static_cast<type::Integer>(right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String:goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_band, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_band, right);
		}

		error:
		throw op_error("band", *this, right);
	}
	Value Value::runtime_bor(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.integral | right._value.integral;
					case DataType::Float:
						return _value.integral | static_cast<type::Integer>(right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Integer>(_value.floating) | right._value.integral;
					case DataType::Float:
						return static_cast<type::Integer>(_value.floating) | static_cast<type::Integer>(right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String:goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_bor, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_bor, right);
		}

		error:
		throw op_error("bor", *this, right);
	}
	Value Value::runtime_xor(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: {
				switch (right._type)
				{
					case DataType::Integer:
						return _value.integral ^ right._value.integral;
					case DataType::Float:
						return _value.integral ^ static_cast<type::Integer>(right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Float:  {
				switch (right._type)
				{
					case DataType::Integer:
						return static_cast<type::Integer>(_value.floating) ^ right._value.integral;
					case DataType::Float:
						return static_cast<type::Integer>(_value.floating) ^ static_cast<type::Integer>(right._value.floating);
					default:
						return true;
				}
			} break;
			case DataType::Boolean: goto error;
			case DataType::String:goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_xor, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_xor, right);
		}

		error:
		throw op_error("xor", *this, right);
	}

	Value Value::runtime_length(KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: goto error;
			case DataType::Float:  goto error;
			case DataType::Boolean: goto error;
			case DataType::String:
				return static_cast<type::Integer>(_value.string->size());
			case DataType::Array:
				return static_cast<type::Integer>(_value.array->length());
			case DataType::List:
				return static_cast<type::Integer>(_value.list->size());
			case DataType::Object:
				return invoke(state, special_props::operator_len);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_len);
		}

		error:
		throw op_error("length", *this);
	}
	Value Value::runtime_not(KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null:
				return true;
			case DataType::Integer:
				return static_cast<type::Boolean>(!_value.integral);
			case DataType::Float: 
				return static_cast<type::Boolean>(!_value.floating);
			case DataType::Boolean:
				return !_value.boolean;
			case DataType::String:
				return _value.string->empty();
			case DataType::Array:
				return _value.array->empty();
			case DataType::List:
				return _value.list->empty();
			case DataType::Object:
				return invoke(state, special_props::operator_not);
			case DataType::Function:
				return false;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_not);
		}

		return false;
	}
	Value Value::runtime_bnot(KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer:
				return ~_value.integral;
			case DataType::Float: 
				return static_cast<type::Integer>(!_value.floating);
			case DataType::Boolean: goto error;
			case DataType::String: goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_bnot);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_bnot);
		}

		error:
		throw op_error("bnot", *this);
	}
	Value Value::runtime_neg(KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer:
				return -_value.integral;
			case DataType::Float: 
				return -_value.floating;
			case DataType::Boolean: goto error;
			case DataType::String: goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_neg);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_neg);
		}

		error:
		throw op_error("neg", *this);
	}

	Value Value::runtime_in(const Value& right, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: goto error;
			case DataType::Float:  goto error;
			case DataType::Boolean: goto error;
			case DataType::String:
				return type::String::runtime_in(*_value.string, right);
			case DataType::Array:
				return type::Array::runtime_in(*_value.array, right, state);
			case DataType::List:
				return type::List::runtime_in(*_value.list, right, state);
			case DataType::Object:
				return invoke(state, special_props::operator_in);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_in);
		}

		error:
		throw op_error("in", *this);
	}
	Value Value::runtime_instanceof(const Value& right, KPLState& state) const
	{
		return _type == DataType::Object && _value.object->is_instance_of(right);
	}

	Value Value::runtime_subscrived_get(const Value& index, KPLState& state) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: goto error;
			case DataType::Float:  goto error;
			case DataType::Boolean: goto error;
			case DataType::String: {
				char c = _value.string->at(static_cast<Offset>(index.to_integer()));
				return state.make_string(&c, 1);
			} break;
			case DataType::Array:
				return (*_value.array)[static_cast<Offset>(index.to_integer())];
			case DataType::List: {
				Int64 idx = index.to_integer();
				if (idx >= _value.list->size())
					throw op_invalid_index(idx, _value.list->size() - 1);

				auto it = _value.list->begin();
				for (; idx > 0; ++it);
				return *it;
			} break;
			case DataType::Object:
				return invoke(state, special_props::operator_get);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_get);
		}

		error:
		throw op_error("in", *this);
	}
	Value Value::runtime_subscrived_set(const Value& index, const Value& right, KPLState& state)
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: goto error;
			case DataType::Float:  goto error;
			case DataType::Boolean: goto error;
			case DataType::String: goto error;
			case DataType::Array:
				return (*_value.array)[static_cast<Offset>(index.to_integer())] = right;
			case DataType::List: {
				Int64 idx = index.to_integer();
				if (idx >= _value.list->size())
					throw op_invalid_index(idx, _value.list->size() - 1);

				auto it = _value.list->begin();
				for (; idx > 0; ++it);
				return (*it) = right;
			} break;
			case DataType::Object:
				return invoke(state, special_props::operator_set, right);
			case DataType::Function: goto error;
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_set, right);
		}

		error:
		throw op_error("in", *this);
	}



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
				//_value.userdata
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

	bool Value::to_bool() const
	{
		switch (_type)
		{
			case DataType::Null: return false;
			case DataType::Integer: return _value.integral;
			case DataType::Float: return static_cast<bool>(_value.floating);
			case DataType::Boolean: return _value.boolean;
			case DataType::String: return !_value.string->empty();
			case DataType::Array: return !_value.array->empty();
			case DataType::List: return !_value.list->empty();
			case DataType::Object: return !_value.object->empty();
			case DataType::Function: return true;
			case DataType::Userdata: return true;
		}

		return false;
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



	Value Value::runtime_call(KPLState& state, const Value& self, const CallArguments& args) const
	{
		switch (_type)
		{
			case DataType::Null: goto error;
			case DataType::Integer: goto error;
			case DataType::Float:  goto error;
			case DataType::Boolean: goto error;
			case DataType::String: goto error;
			case DataType::Array: goto error;
			case DataType::List: goto error;
			case DataType::Object:
				return invoke(state, special_props::operator_call, args);
			case DataType::Function:
				return runtime::execute(state, *_value.function, self, args);
			case DataType::Userdata:
				return _value.userdata->invoke(state, special_props::operator_call, args);
		}

		error:
		throw op_error("call", *this);
	}

	Value Value::invoke(KPLState& state, const std::string& name, const CallArguments& args) const
	{
		return get_property(name).runtime_call(state, *this, args);
	}

	Value Value::invoke(KPLState& state, const Value& name, const CallArguments& args) const
	{
		return get_property(name).runtime_call(state, *this, args);
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

	Value String::runtime_nclone(const String& string, Integer times, MemoryHeap& heap)
	{
		if (string.empty() || times <= 0)
			return heap.make_string("");

		if (times == 1)
			return heap.make_string(string);

		Size len = string.size();
		const char* original = string.c_str();
		char* str = new char[len];

		for (Offset i = 0; i < len; i += len)
			std::memcpy(str + i, original, len);

		String* newstr = heap.make_string(str);
		delete[] str;

		return newstr;
	}

	type::Boolean String::runtime_in(const String& left, const Value& right)
	{
		if (!right.isString())
			return false;

		return left.find(right.string()) != std::string::npos;
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

	type::Boolean Array::runtime_eq(const Array& left, const Array& right, KPLState& state)
	{
		if (left._length != right._length)
			return false;

		Size len = left._length;
		for (Offset i = 0; i < len; ++i)
			if (!left._array[i].runtime_eq(right._array[i], state))
				return false;

		return true;
	}

	type::Boolean Array::runtime_ne(const Array& left, const Array& right, KPLState& state)
	{
		if (left._length != right._length)
			return true;

		Size len = left._length;
		for (Offset i = 0; i < len; ++i)
			if (!left._array[i].runtime_eq(right._array[i], state))
				return true;

		return false;
	}

	type::Boolean Array::runtime_in(const Array& left, const Value& right, KPLState& state)
	{
		Size len = left._length;
		for (Offset i = 0; i < len; ++i)
			if (left._array[i].runtime_eq(right, state))
				return true;
		return false;
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

	type::Boolean List::runtime_eq(const List& left, const List& right, KPLState& state)
	{
		if (left.size() != right.size())
			return false;

		const auto lend = left.end();
		const auto rend = right.end();
		for (auto lit = left.begin(), rit = right.begin(); lit != lend && rit != rend; ++lit, ++rit)
			if (!lit->runtime_eq(*rit, state))
				return false;

		return true;
	}

	type::Boolean List::runtime_ne(const List& left, const List& right, KPLState& state)
	{
		if (left.size() != right.size())
			return true;

		const auto lend = left.end();
		const auto rend = right.end();
		for (auto lit = left.begin(), rit = right.begin(); lit != lend && rit != rend; ++lit, ++rit)
			if (!lit->runtime_eq(*rit, state))
				return true;

		return false;
	}

	type::Boolean List::runtime_in(const List& left, const Value& right, KPLState& state)
	{
		return std::find_if(left.begin(), left.end(), [&right, &state](const Value& value) { return value.runtime_eq(right, state); }) != left.end();
	}
}



namespace kpl::type
{
	void Object::_mheap_delete(void* block) { reinterpret_cast<Object*>(block)->~Object(); }

	Object::Object(const Value* parents, const Size count) :
		unordered_map(),
		_class(),
		_parents{ count > 0 ? new Value[count] : nullptr },
		_parentSize{ count }
	{
		if (_parents)
			for (Offset i = 0; i < _parentSize; ++i)
				_parents[i] = parents[i];
	}

	Object::~Object()
	{
		if (_parents)
			delete[] _parents;
	}


	bool Object::is_instance_of(const Value& value)
	{
		if (!_class)
			return false;

		if (!_class.isObject())
			return _class.object().is_same_or_parent(value);

		return _class.addr() == value.addr();
	}

	bool Object::is_same_or_parent(const Value& value)
	{
		if (value.isObject())
		{
			Object& obj = value.object();
			if (this == &obj)
				return true;

			if(obj._parents)
				for (Offset i = 0; i < obj._parentSize; ++i)
				{
					Value& parent = obj._parents[i];
					if (parent.isObject())
					{
						if (parent.object().is_same_or_parent(value))
							return true;
					}
					else if (parent.addr() == this)
						return true;
				}
		}
		return false;
	}

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

		if(!_class.isNull())
			return _class.get_property(name);

		else if (_parentSize > 0)
			for (Offset i = 0; i < _parentSize; ++i)
			{
				const Value& prop = _parents[i].get_property(name);
				if (!prop.isNull())
					return prop;
			}

		return literal::Null;
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
	Value Userdata::Meta::invoke(KPLState& state, const Value& self, const std::string& property_name, const CallArguments& args)
	{
		return get_property(property_name).runtime_call(state, self, args);
	}

	Value Userdata::Meta::invoke(KPLState& state, const Value& self, const Value& property_name, const CallArguments& args)
	{
		return get_property(property_name).runtime_call(state, self, args);
	}



	std::string Userdata::to_string() const
	{
		std::stringstream ss;
		ss << "userdata@" << this;
		return ss.str();
	}

	const Value& Userdata::get_property(const std::string& name) const
	{
		const Value& value = inner_get_property(name);
		if (value.isNull())
			return literal::Null;

		if (!_meta)
			return literal::Null;

		return _meta->get_property(name);
	}

	void Userdata::set_property(const std::string name, const Value& value)
	{
		if (_meta)
			inner_set_property(name, value);
	}

	void Userdata::del_property(const std::string& name)
	{
		if (_meta)
			inner_del_property(name);
	}

	Value Userdata::invoke(KPLState& state, const std::string& property_name, const CallArguments& args)
	{
		return get_property(property_name).runtime_call(state, this, args);
	}

	Value Userdata::invoke(KPLState& state, const Value& property_name, const CallArguments& args)
	{
		return get_property(property_name).runtime_call(state, this, args);
	}
}
