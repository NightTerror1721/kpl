#pragma once

#include "common.h"

namespace kpl::type
{
	typedef decltype(nullptr) Null;
	typedef Int64 Integer;
	typedef double Float;
	typedef bool Boolean;
	class String;
	class Array;
	class List;
	class Object;
	class Function;
	class Userdata;
}

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

		Userdata,
		Internal
	};

	class Reference
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
			void* internal;
		} _value;

	public:
		inline Reference() : _type{ DataType::Null }, _value{ .null = nullptr } {}
		inline Reference(type::Null) : _type{ DataType::Null }, _value{ .null = nullptr } {}
		inline Reference(type::Integer value) : _type{ DataType::Integer }, _value{ .integral = value } {}
		inline Reference(type::Float value) : _type{ DataType::Float }, _value{ .floating = value } {}
	};
}
