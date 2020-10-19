#pragma once

#include "common.h"
#include "mheap.h"

#define inc_ref(_Obj) (_Obj)->increase_reference_count()
#define dec_ref(_Obj) (_Obj)->decrease_reference_count()

namespace kpl
{
	enum class DataType
	{
		Null,
		Integer,
		Float,
		Boolean,
		String,
		Array,
		List,
		Object,
		Function,
		Userdata
	};

	class Value
	{
	private:
		DataType _type;
		union {
			type::Null null;
			type::Integer integral;
			type::Float floating;
			type::Boolean boolean;
			type::String* string;
			type::Array* array;
			type::List* list;
			type::Object* object;
			type::Function* function;
			type::Userdata* userdata;
			void* raw_ptr;
		} _value;

	private:
		void try_inc_ref();
		void try_dec_ref();

	public:
		inline Value() : _type{ DataType::Null }, _value{ .null = nullptr } {}
		inline Value(type::Null) : _type{ DataType::Null }, _value{ .null = nullptr } {}
		inline Value(type::Integer value) : _type{ DataType::Integer }, _value{ .integral = value } {}
		inline Value(type::Float value) : _type{ DataType::Float }, _value{ .floating = value } {}
		inline Value(type::Boolean value) : _type{ DataType::Boolean }, _value{ .boolean = value } {}

		Value(type::String* value);
		Value(type::Array* value);
		Value(type::List* value);
		Value(type::Object* value);
		Value(type::Function* value);
		Value(type::Userdata* value);

		template<std::integral _Ty>
		inline Value(_Ty value) : Value{ static_cast<type::Integer>(value) } {}

		inline Value(const Value& value) : _type{ value._type }, _value{ value._value } { try_inc_ref(); }
		inline Value(Value&& value) noexcept : _type{ value._type }, _value{ std::move(value._value) } {}

		inline ~Value()
		{
			try_dec_ref();
			_type = DataType::Null;
		}

		inline Value& operator= (type::Null) { try_dec_ref(); return _type = DataType::Null, *this; }
		inline Value& operator= (type::Integer right) { try_dec_ref(); return _type = DataType::Integer, _value.integral = right, *this; }
		inline Value& operator= (type::Float right) { try_dec_ref(); return _type = DataType::Float, _value.floating = right, *this; }
		inline Value& operator= (type::Boolean right) { try_dec_ref(); return _type = DataType::Boolean, _value.boolean = right, *this; }

		Value& operator= (type::String* right);
		Value& operator= (type::Array* right);
		Value& operator= (type::List* right);
		Value& operator= (type::Object* right);
		Value& operator= (type::Function* right);
		Value& operator= (type::Userdata* right);

		template<std::integral _Ty>
		inline Value& operator= (_Ty right) { return *this = static_cast<type::Integer>(right); }

		inline Value& operator= (const Value& right) { try_dec_ref(); _type = right._type, _value = right._value; try_inc_ref(); return *this; }
		inline Value& operator= (Value&& right) noexcept { try_dec_ref(); return _type = right._type, _value = std::move(right._value), *this; }


		inline DataType type() const { return _type; }
		
		template<typename _Ty>
		requires std::same_as<type::Null, _Ty> ||
			std::same_as<type::Integer, _Ty> ||
			std::same_as<type::Float, _Ty> ||
			std::same_as<type::Boolean, _Ty> ||
			std::same_as<type::String*, _Ty> ||
			std::same_as<type::Array*, _Ty> ||
			std::same_as<type::List*, _Ty> ||
			std::same_as<type::Object*, _Ty> ||
			std::same_as<type::Function*, _Ty> ||
			std::same_as<type::Userdata*, _Ty>
		inline _Ty as()
		{
			if constexpr (std::same_as<type::Null, _Ty>) return nullptr;
			else if constexpr (std::same_as<type::Integer, _Ty>) return _value.integral;
			else if constexpr (std::same_as<type::Float, _Ty>) return _value.floating;
			else if constexpr (std::same_as<type::Boolean, _Ty>) return _value.boolean;
			else if constexpr (std::same_as<type::String*, _Ty>) return _value.string;
			else if constexpr (std::same_as<type::Array*, _Ty>) return _value.array;
			else if constexpr (std::same_as<type::List*, _Ty>) return _value.list;
			else if constexpr (std::same_as<type::Object*, _Ty>) return _value.object;
			else if constexpr (std::same_as<type::Function*, _Ty>) return _value.function;
			else if constexpr (std::same_as<type::Userdata*, _Ty>) return _value.userdata;
			else return _Ty{};
		}

		inline type::Integer integral() const { return _value.integral; }
		inline type::Float floating() const { return _value.floating; }
		inline type::Boolean boolean() const { return _value.boolean; }
		inline type::String& string() const { return *_value.string; }
		inline type::Array& array() const { return *_value.array; }
		inline type::List& list() const { return *_value.list; }
		inline type::Object& object() const { return *_value.object; }
		inline type::Function& function() const { return *_value.function; }
		inline type::Userdata& userdata() const { return *_value.userdata; }



		void set_property(const std::string& name, const Value& value);
		const Value& get_property(const std::string& name) const;
		void del_property(const std::string& name);

		std::string to_string() const;
	};

	namespace type::literal
	{
		extern const Value Null;
		extern const Value True;
		extern const Value False;
		extern const Value One;
		extern const Value Zero;
		extern const Value Minusone;
	}
}



namespace kpl::type
{
	class String : public KPLVirtualObject, public std::string
	{
	public:
		static void _mheap_delete(void* block);

	public:
		inline String() : std::string() {}
		inline String(const char* str) : std::string(str) {}
		inline String(const std::string& str) : std::string(str) {}
		inline String(std::string&& str) : std::string(std::move(str)) {}
		~String() = default;
	};
}



namespace kpl::type
{
	class Array : public KPLVirtualObject
	{
	private:
		Value* _array;
		Size _length;

	public:
		static void _mheap_delete(void* block);

	public:
		explicit Array(Size length);
		Array(const Value* array, Size length);
		Array(const std::vector<Value>& vector);
		Array(std::vector<Value>&& vector);
		Array(const std::initializer_list<Value>& args);
		~Array();

		inline Size length() const { return _length; }
		inline Size size() const { return _length; }
		inline bool empty() const { return _length == 0; }

		inline Value& operator[] (Offset index) { return _array[index]; }
		inline const Value& operator[] (Offset index) const { return _array[index]; }

		std::string to_string() const;
	};
}



namespace kpl::type
{
	class List : public KPLVirtualObject, public std::list<Value>
	{
	public:
		static void _mheap_delete(void* block);

	public:
		inline List() : list{} {}
		~List() = default;

		std::string to_string() const;
	};
}



namespace kpl::type
{
	class Object : public KPLVirtualObject, public std::unordered_map<std::string, Value>
	{
	public:
		static void _mheap_delete(void* block);

	public:
		inline Object() : unordered_map{} {}
		~Object() = default;

		std::string to_string() const;
	};
}



namespace kpl::type
{
	class Function : public KPLVirtualObject
	{
	private:
		Chunk* _chunk;
		Value _locals;

	public:
		static void _mheap_delete(void* block);

	public:
		Function(Chunk& chunk, Value* locals = nullptr) : 
			_chunk{ &chunk },
			_locals{ locals ? *locals : nullptr }
		{}
		~Function() = default;

		std::string to_string() const;

		inline Chunk& chunk() { return *_chunk; }
		inline const Chunk& chunk() const { return *_chunk; }

		inline Value& locals() { return _locals; }
		inline const Value& locals() const { return _locals; }

		inline void set_local(const std::string& name, const Value& value) { _locals.set_property(name, value); }
		inline const Value& get_local(const std::string& name) const { return _locals.get_property(name); }
		inline void del_local(const std::string& name) { _locals.del_property(name); }
	};
}



namespace kpl::type
{
	class Userdata : public KPLVirtualObject
	{
	public:
		std::string to_string() const;
	};
}



namespace kpl
{
	inline Value::Value(type::String* value) : _type{ DataType::String }, _value{ .string = value } { value->increase_reference_count(); }
	inline Value::Value(type::Array* value) : _type{ DataType::Array }, _value{ .array = value } { value->increase_reference_count(); }
	inline Value::Value(type::List* value) : _type{ DataType::List }, _value{ .list = value } { value->increase_reference_count(); }
	inline Value::Value(type::Object* value) : _type{ DataType::Object }, _value{ .object = value } { value->increase_reference_count(); }
	inline Value::Value(type::Function* value) : _type{ DataType::Function }, _value{ .function = value } { value->increase_reference_count(); }
	inline Value::Value(type::Userdata* value) : _type{ DataType::Userdata }, _value{ .userdata = value } { value->increase_reference_count(); }

	inline Value& Value::operator= (type::String* right) { try_dec_ref(); return _type = DataType::String, _value.string = right, inc_ref(right), * this; }
	inline Value& Value::operator= (type::Array* right) { try_dec_ref(); return _type = DataType::Array, _value.array = right, inc_ref(right), * this; }
	inline Value& Value::operator= (type::List* right) { try_dec_ref(); return _type = DataType::List, _value.list = right, inc_ref(right), * this; }
	inline Value& Value::operator= (type::Object* right) { try_dec_ref(); return _type = DataType::Object, _value.object = right, inc_ref(right), * this; }
	inline Value& Value::operator= (type::Function* right) { try_dec_ref(); return _type = DataType::Function, _value.function = right, inc_ref(right), * this; }
	inline Value& Value::operator= (type::Userdata* right) { try_dec_ref(); return _type = DataType::Userdata, _value.userdata = right, inc_ref(right), * this; }

	inline void Value::try_inc_ref()
	{
		switch (_type)
		{
			case DataType::String: inc_ref(_value.string); break;
			case DataType::Array: inc_ref(_value.array); break;
			case DataType::List: inc_ref(_value.list); break;
			case DataType::Object: inc_ref(_value.object); break;
			case DataType::Function: inc_ref(_value.function); break;
			case DataType::Userdata: inc_ref(_value.userdata); break;
		}
	}
	inline void Value::try_dec_ref()
	{
		switch (_type)
		{
			case DataType::String: dec_ref(_value.string); break;
			case DataType::Array: dec_ref(_value.array); break;
			case DataType::List: dec_ref(_value.list); break;
			case DataType::Object: dec_ref(_value.object); break;
			case DataType::Function: dec_ref(_value.function); break;
			case DataType::Userdata: dec_ref(_value.userdata); break;
		}
	}
}

#undef inc_ref
#undef dec_ref
