#include "chunk.h"

namespace kpl
{
	ChunkConstant::ChunkConstant(const char* string) :
		_type{ Type::String },
		_value{}
	{
		Size len = std::strlen(string);
		_value.string = new char[len + 1];
		_value.string_len = len;

		if (len > 0)
			std::memcpy(const_cast<char*>(_value.string), string, len);
		const_cast<char*>(_value.string)[len] = '\0';

	}

	ChunkConstant::ChunkConstant(const char* string, Size count) :
		_type{ Type::String },
		_value{}
	{
		_value.string = new char[count + 1];
		_value.string_len = count;

		if (count > 0)
			std::memcpy(const_cast<char*>(_value.string), string, count);
		const_cast<char*>(_value.string)[count] = '\0';
	}

	ChunkConstant::ChunkConstant(const std::string& string) :
		_type{ Type::String },
		_value{}
	{
		Size len = string.size();
		_value.string = new char[len + 1];
		_value.string_len = len;

		if (len > 0)
			std::memcpy(const_cast<char*>(_value.string), string.data(), len);
		const_cast<char*>(_value.string)[len] = '\0';
	}

	ChunkConstant::ChunkConstant(const Value& value) :
		ChunkConstant{}
	{
		switch (value.type())
		{
			default:
			case DataType::Null:
				_type = Type::Null;
				_value.null = nullptr;
				break;

			case DataType::Integer:
				_type = Type::Integer;
				_value.integral = value.integral();
				break;

			case DataType::Float:
				_type = Type::Float;
				_value.floating = value.floating();
				break;

			case DataType::Boolean:
				_type = Type::Boolean;
				_value.boolean = value.boolean();
				break;

			case DataType::String:
				utils::construct(*this, value.string());
				break;
		}
	}

	ChunkConstant::ChunkConstant(const ChunkConstant& right) :
		_type{ right._type },
		_value{}
	{
		switch (_type)
		{
			case Type::Null: _value.null = nullptr; break;
			case Type::Integer: _value.integral = right._value.integral; break;
			case Type::Float: _value.floating = right._value.floating; break;
			case Type::Boolean: _value.boolean = right._value.boolean; break;
			case Type::String:
				utils::construct(*this, right._value.string, right._value.string_len);
				break;
		}
	}

	ChunkConstant::ChunkConstant(ChunkConstant&& right) noexcept :
		_type{ right._type },
		_value{}
	{
		switch (_type)
		{
			case Type::Null: _value.null = nullptr; break;
			case Type::Integer: _value.integral = right._value.integral; break;
			case Type::Float: _value.floating = right._value.floating; break;
			case Type::Boolean: _value.boolean = right._value.boolean; break;
			case Type::String:
				_value.string = right._value.string;
				_value.string_len = right._value.string_len;
				break;
		}
		right._type = Type::Null;
	}

	ChunkConstant::~ChunkConstant()
	{
		if (_type == Type::String)
			delete[] _value.string;
		_type = Type::Null;
	}

	ChunkConstant& ChunkConstant::operator= (const ChunkConstant& right)
	{
		return utils::construct_copy(utils::destroy(*this), right);
	}
	ChunkConstant& ChunkConstant::operator= (ChunkConstant&& right) noexcept
	{
		return utils::construct_move(utils::destroy(*this), std::move(right));
	}

	Value ChunkConstant::to_value() const
	{
		Value value;
		switch (_type)
		{
			case Type::Null:
				value = nullptr;
				break;

			case Type::Integer:
				value = _value.integral;
				break;

			case Type::Float:
				value = _value.floating;
				break;

			case Type::Boolean:
				value = _value.boolean;
				break;

			case Type::String:
				value = new type::String(_value.string, _value.string_len);
				break;
		}

		return value;
	}
}



namespace kpl
{
	Chunk* ChunkBuilder::build(Chunk* chunk)
	{
		if (!chunk)
			chunk = _chunk;

		if (!chunk)
			return nullptr;

		utils::destroy(*chunk);
		utils::construct(*chunk);

		chunk->_constant_count = _constants.size();
		chunk->_chunk_count = _chunks.size();
		chunk->_code_count = _instructions.size();
		chunk->_register_count = static_cast<unsigned int>(_registers);

		chunk->_data = utils::malloc(Chunk::chunk_object_size(chunk->_constant_count, chunk->_chunk_count, chunk->_code_count));

		chunk->_constants = reinterpret_cast<Value*>(chunk->_data);
		chunk->_chunks = reinterpret_cast<Chunk**>(chunk->_constants + chunk->_constant_count);
		chunk->_code = reinterpret_cast<InstructionCode*>(chunk->_chunks + chunk->_chunk_count);

		Offset offset = 0;
		for (const ChunkConstant& c : _constants)
			chunk->_constants[offset++] = c.to_value();

		if(!_chunks.empty())
			std::memcpy(chunk->_chunks, _chunks.data(), chunk->_chunk_count);
		
		offset = 0;
		for (const inst::Instruction& inst : _instructions)
			chunk->_code[offset++] = inst;

		return chunk;
	}
}




namespace kpl
{
	Chunk::~Chunk()
	{
		if (_data)
		{
			for (Offset i = 0; i < _constant_count; ++i)
			{
				_constants[i].force_destructor_call();
				utils::destroy(_constants[i]);
			}

			for (Offset i = 0; i < _chunk_count; ++i)
				delete _chunks[i];

			utils::free(_data);
		}

		std::memset(this, 0, sizeof(*this));
	}
}
