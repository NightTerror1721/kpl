#pragma once

#include "common.h"

namespace kpl
{
	class MemoryHeap;

	enum class DataType
	{
		Null,

		Integer,
		Float,
		Boolean,
		String,

		Callable,

		Array,

		Object,

		Userdata
	};


	class Value
	{
	private:
		DataType _type;

	protected:
		constexpr Value(DataType type) : _type{ type } {}

	public:
		inline DataType type() const { return _type; }
	};
}



namespace kpl::type
{
	class Null : public Value
	{
	public:
		constexpr Null() : Value{ DataType::Null } {}
		constexpr Null(const Null&) : Value{ DataType::Null } {}
		constexpr Null(Null&&) noexcept : Value{ DataType::Null } {}

		constexpr Null& operator= (const Null&) { return *this; }
		constexpr Null& operator= (Null&&) noexcept { return *this; }
	};

	namespace constant { static constexpr Null null; }
}



namespace kpl::type
{
	class Integer : public Value
	{
	private:
		Int64 _value;

	public:
		constexpr Integer() : Value{ DataType::Integer }, _value{ 0 } {}
		constexpr Integer(Int64 value) : Value{ DataType::Integer }, _value{ value } {}
		constexpr Integer(const Integer& v) : Value{ DataType::Integer }, _value{ v._value } {}
		constexpr Integer(Integer&& v) noexcept : Value{ DataType::Integer }, _value{ v._value } {}

		constexpr Integer& operator= (const Integer& right) { return _value = right._value, *this; }
		constexpr Integer& operator= (Integer&& right) noexcept { return _value = right._value, *this; }
	};
}



namespace kpl::type
{
	class Float : public Value
	{
	private:
		double _value;

	public:
		constexpr Float() : Value{ DataType::Float }, _value{ 0 } {}
		constexpr Float(double value) : Value{ DataType::Float }, _value{ value } {}
		constexpr Float(const Float& v) : Value{ DataType::Float }, _value{ v._value } {}
		constexpr Float(Float&& v) noexcept : Value{ DataType::Float }, _value{ v._value } {}

		constexpr Float& operator= (const Float& right) { return _value = right._value, *this; }
		constexpr Float& operator= (Float&& right) noexcept { return _value = right._value, *this; }
	};
}



namespace kpl::type
{
	class Boolean : public Value
	{
	private:
		bool _state;

	public:
		constexpr Boolean() : Value{ DataType::Boolean }, _state{ 0 } {}
		constexpr Boolean(bool state) : Value{ DataType::Boolean }, _state{ state } {}
		constexpr Boolean(const Boolean& s) : Value{ DataType::Boolean }, _state{ s._state } {}
		constexpr Boolean(Boolean&& s) noexcept : Value{ DataType::Boolean }, _state{ s._state } {}

		constexpr Boolean& operator= (const Boolean right) { return _state = right._state, *this; }
		constexpr Boolean& operator= (Boolean&& right) noexcept { return _state = right._state, *this; }
	};

	namespace constant { static constexpr Boolean True, False; }
}



namespace kpl::type
{
	class String : public Value
	{
	private:
		MemoryHeap* _heap;
		char* _str;
		Size _size;
		Int64 _hashcode;

	public:
		String(MemoryHeap* heap, const char* const str, Size len);
		String(const String& str);
		String(String&& str) noexcept;

		String& operator= (const String& right);
		String& operator= (String&& right) noexcept;

		inline String(MemoryHeap* heap) :
			Value{ DataType::String },
			_heap{ heap },
			_str{ nullptr },
			_size{ 0 },
			_hashcode{ 0 }
		{}
		inline String(MemoryHeap* heap, const char* const str) : String{ heap, str, std::strlen(str) } {}
		inline String(MemoryHeap* heap, const std::string& str) : String{ heap, str.c_str(), str.size() } {}
	};
}
