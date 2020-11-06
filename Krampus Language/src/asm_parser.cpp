#include "asm_parser.h"

namespace kpl::assembler::parser
{
	Number::Number() :
		_decimal{ false },
		_value{ .integral = 0 }
	{}

	Number::Number(Int64 value) :
		_decimal{ false },
		_value{ .integral = value }
	{}

	Number::Number(double value) :
		_decimal{ true },
		_value{ .floating = value }
	{}

	Number& Number::operator= (Int64 value)
	{
		_decimal = false;
		_value.integral = value;
		return *this;
	}

	Number& Number::operator= (double value)
	{
		_decimal = true;
		_value.floating = value;
		return *this;
	}

	Int64 Number::integral() const
	{
		return _decimal ? static_cast<Int64>(_value.floating) : _value.integral;
	}
	double Number::floating() const
	{
		return _decimal ? _value.floating : static_cast<double>(_value.integral);
	}
}


namespace kpl::assembler::parser
{
	Element::Element(Type type) :
		_type{ type },
		_data{ .invalid = nullptr }
	{}

	void Element::destroy()
	{
		if (_type == Type::String)
			_data.string.~basic_string();
	}

	Element& Element::copy(const Element& elem, bool reset)
	{
		if (reset)
			destroy();

		_type = elem._type;
		switch (elem._type)
		{
			case Type::Invalid: _data.invalid = nullptr; break;
			case Type::Keyword: _data.keyword = elem._data.keyword; break;
			case Type::Number: utils::construct_copy(_data.number, elem._data.number); break;
			case Type::String: utils::construct_copy(_data.string, elem._data.string); break;
			case Type::Index: _data.index = elem._data.index; break;
			case Type::Opcode: _data.opcode = elem._data.opcode; break;
		}

		return *this;
	}

	Element& Element::move(Element&& elem, bool reset)
	{
		if (reset)
			destroy();

		_type = elem._type;
		switch (elem._type)
		{
			case Type::Invalid: _data.invalid = nullptr; break;
			case Type::Keyword: _data.keyword = elem._data.keyword; break;
			case Type::Number: utils::construct_move(_data.number, std::move(elem._data.number)); break;
			case Type::String: utils::construct_move(_data.string, std::move(elem._data.string)); break;
			case Type::Index: _data.index = elem._data.index; break;
			case Type::Opcode: _data.opcode = elem._data.opcode; break;
		}

		return *this;
	}
}





namespace kpl::assembler::parser
{
	static constexpr const char* keyword_name(Keyword keyword)
	{
		switch (keyword)
		{
			case Keyword::Chunks: return "chunks";
			case Keyword::Constants: return "constants";
			case Keyword::Registers: return "registers";
			case Keyword::Code: return "code";
		}
		return "<invalid-keyword>";
	}

	static const utils::EnumDict<Keyword> keywords(&keyword_name, Keyword::Chunks, Keyword::Code);
	static const utils::EnumDict<opcode::id> opcodes(&opcode::name, opcode::id::NOP, opcode::id::RETURN);

	ParserException error(utils::DataReader& reader, const char* msg)
	{
		return { reader.current_line(), msg };
	}

	ParserException error(utils::DataReader& reader, const std::string& msg)
	{
		return { reader.current_line(), msg };
	}
}






namespace kpl::assembler::parser
{
	UInt8 read_char_to_hex_byte(utils::DataReader& reader, char character, bool left_value)
	{
		UInt8 value = 0;
		if (character >= '0' && character <= '9')
			value = static_cast<UInt8>(character - '0');
		else if (character >= 'a' && character <= 'f')
			value = static_cast<UInt8>(character - 'a');
		else if (character >= 'A' && character <= 'F')
			value = static_cast<UInt8>(character - 'A');
		else error(reader, "Invalid ASCII value in string");

		if (left_value)
			value <<= 4;
		return value;
	}

	char read_ascii_char(utils::DataReader& reader)
	{
		char a0 = reader.next();
		char a1 = reader.next();

		return static_cast<char>(read_char_to_hex_byte(reader, a0, true) | read_char_to_hex_byte(reader, a1, false));
	}

	Element parse_element(const std::string& text)
	{
		if (keywords.has(text))
			return Element::keyword(keywords[text]);

		if (opcodes.has(text))
			return Element::opcode(opcodes[text]);

		try
		{
			Int64 value = std::stoll(text);
			return Element::number(value);
		}
		catch (const std::exception& ex) {}

		try
		{
			double value = std::stod(text);
			return Element::number(value);
		}
		catch (const std::exception& ex) {}

		return Element::invalid();
	}

	Element parse_index(utils::DataReader& reader)
	{
		std::stringstream ss;

		while (reader)
		{
			char c = reader.next();
			switch (c)
			{
				case utils::DataReader::invalid_char:
					throw error(reader, "Invalid character in index.");

				case utils::DataReader::newline:
				case '\r':
					throw error(reader, "Invalid newline character in index.");

				case ']':
					goto decode;

				default:
					if (c >= '0' && c <= '9')
						ss << c;
					else throw error(reader, "Invalid character in index.");
					break;
			}
		}

		throw error(reader, "Invalid index.");

		decode:
		std::string text = ss.str();

		try
		{
			Int64 value = std::stoull(text);
			return Element::index(value);
		}
		catch (const std::exception& ex)
		{
			throw error(reader, "Invalid index.");
		}
	}

	Element parse_string(utils::DataReader& reader)
	{
		std::stringstream ss;

		while (reader)
		{
			char c = reader.next();
			switch (c)
			{
				case utils::DataReader::invalid_char:
					throw error(reader, "Invalid character in string.");

				case utils::DataReader::newline:
				case '\r':
					throw error(reader, "Invalid newline character in string.");

				case '\"':
					goto end;

				case '\\': {
					c = reader.next();
					switch (c)
					{
						case 'n': ss << '\n'; break;
						case 'r': ss << '\r'; break;
						case 't': ss << '\t'; break;
						case '0': ss << '\0'; break;
						case '\'': ss << '\''; break;
						case '\"': ss << '\"'; break;
						case '\\': ss << '\\'; break;
						case 'a': ss << read_ascii_char(reader); break;
						default:
							error(reader, "Invalid character after scaped character.");
					}
				} break;

				default:
					ss << c;
					break;
			}
		}

		end:
		return Element::string(ss.str());
	}

	std::pair<Element, bool> read_element(utils::DataReader& reader)
	{
		#define dump_buffer(_Buf) if((_Buf).tellp() > 0) return reader.prev(), std::pair{ parse_element((_Buf).str()), false }

		std::stringstream ss;

		while (reader)
		{
			char c = reader.next();
			switch (c)
			{
				case utils::DataReader::invalid_char:
				case utils::DataReader::newline:
					dump_buffer(ss);
					return { {}, true };

				case '\r':
					break;

				case ' ':
				case '\t':
					dump_buffer(ss);
					break;

				case ';':
					dump_buffer(ss);
					reader.skip_line();
					return { {}, true };

				case '\"':
					dump_buffer(ss);
					return { parse_string(reader), false };

				case '[':
					dump_buffer(ss);
					return { parse_index(reader), false };

				default:
					ss << c;
					break;
			}
		}

		std::string text = ss.str();
		if (text.empty())
			return { {}, true };
		return { parse_element(text), true };

		#undef dump_buffer
	}

	ElementArray parse_line(utils::DataReader& reader)
	{
		std::vector<Element> elements;

		while (reader)
		{
			auto elem = read_element(reader);
			if (!elem.first.isInvalid())
				elements.push_back(std::move(elem.first));

			if (elem.second)
				break;
		}

		return elements;
	}
}
