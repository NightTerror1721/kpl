#pragma once

#include "common.h"
#include "opcode.h"
#include "static_array.h"
#include "iodata.h"

namespace kpl::assembler::parser
{
	class ParserException : std::exception
	{
	private:
		Offset _line;

	public:
		inline ParserException(Offset line, const char* msg) : exception(msg), _line{ line } {}
		inline ParserException(Offset line, const std::string& msg) : exception(msg.c_str()), _line{ line } {}

		inline Offset line() const { return _line; }
	};

	enum class Keyword
	{
		Chunks,
		Constants,
		Registers,
		Code
	};

	class Number
	{
	private:
		bool _decimal;
		union {
			Int64 integral;
			double floating;
		} _value;

	public:
		Number();
		Number(Int64 value);
		Number(double value);

		Number& operator= (Int64 value);
		Number& operator= (double value);

		Int64 integral() const;
		double floating() const;

	public:
		Number(const Number&) = default;
		Number(Number&&) noexcept = default;
		~Number() = default;

		Number& operator= (const Number&) = default;
		Number& operator= (Number&&) noexcept = default;

		template<std::integral _Ty>
		inline Number(_Ty value) : Number(static_cast<Int64>(value)) {}

		inline Number(float value) : Number(static_cast<double>(value)) {}

		template<std::integral _Ty>
		inline Number& operator= (_Ty value) { return operator= (static_cast<Int64>(value)); }

		inline Number& operator= (float value) { return operator= (static_cast<double>(value)); }

		inline operator bool() const { return _value.integral; }
		inline bool operator! () const { return !_value.integral; }

		inline bool isInteger() const { return !_decimal; }
		inline bool isFloat() const { return _decimal; }
	};

	class Element
	{
	public:
		enum class Type
		{
			Invalid,
			Keyword,
			Number,
			String,
			Index,
			Opcode
		};

	private:
		Type _type;
		union Data
		{
			decltype(nullptr) invalid;
			Keyword keyword;
			Number number;
			std::string string;
			Offset index;
			opcode::id opcode;

			inline ~Data() {}
		} _data;

	private:
		void destroy();
		Element& copy(const Element& elem, bool reset);
		Element& move(Element&& elem, bool reset);

		Element(Type type);

	public:
		inline Element() : Element(Type::Invalid) {}
		inline Element(const Element& elem) : Element() { copy(elem, false); }
		inline Element(Element&& elem) noexcept : Element() { move(std::move(elem), false); }
		inline ~Element() { destroy(); }

		inline Element& operator= (const Element& right) { return copy(right, true); }
		inline Element& operator= (Element&& right) noexcept { return move(std::move(right), true); }

		inline Type type() const { return _type; }

		inline bool isInvalid() const { return _type == Type::Invalid; }
		inline bool isKeyword() const { return _type == Type::Keyword; }
		inline bool isNumber() const { return _type == Type::Number; }
		inline bool isString() const { return _type == Type::String; }
		inline bool isIndex() const { return _type == Type::Index; }
		inline bool isOpcode() const { return _type == Type::Opcode; }

		inline Element(Keyword value) : Element(Type::Keyword) { _data.keyword = value; }
		inline Element(const Number& value) : Element(Type::Number) { utils::construct_copy(_data.number, value); }
		inline Element(Number&& value) : Element(Type::Number) { utils::construct_move(_data.number, std::move(value)); }
		inline Element(const std::string& value) : Element(Type::Number) { utils::construct_copy(_data.string, value); }
		inline Element(std::string&& value) : Element(Type::Number) { utils::construct_move(_data.string, std::move(value)); }
		inline Element(const char* value) : Element(Type::Number) { utils::construct_move(_data.string, std::string(value)); }
		inline Element(Offset value) : Element(Type::Index) { _data.index = value; }
		inline Element(opcode::id value) : Element(Type::Opcode) { _data.opcode = value; }

		static inline Element invalid() { return Element(); }
		static inline Element keyword(Keyword value) { return value; }
		static inline Element number(const Number& value) { return value; }
		static inline Element number(Number&& value) { return std::move(value); }
		static inline Element string(const std::string& value) { return value; }
		static inline Element string(std::string&& value) { return std::move(value); }
		static inline Element index(Offset value) { return value; }
		static inline Element opcode(opcode::id value) { return value; }

		inline Keyword keyword() const { return _data.keyword; }
		inline const Number& number() const { return _data.number; }
		inline const std::string& string() const { return _data.string; }
		inline Offset index() const { return _data.index; }
		inline opcode::id opcode() const { return _data.opcode; }
	};

	typedef utils::StaticArray<Element> ElementArray;





	ElementArray parse_line(utils::DataReader& reader);
}
