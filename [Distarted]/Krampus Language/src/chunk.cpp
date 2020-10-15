#include "chunk.h"

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

		chunk->_data = utils::malloc(Chunk::chunk_object_size(chunk->_constant_count, chunk->_chunk_count, chunk->_register_count));

		chunk->_constants = reinterpret_cast<Chunk::Constant*>(chunk->_data);
		chunk->_chunks = reinterpret_cast<Chunk**>(chunk->_constants + chunk->_constant_count);
		chunk->_code = reinterpret_cast<InstructionCode*>(chunk->_chunks + chunk->_chunk_count);

		Offset offset = 0;
		for (const Chunk::Constant& c : _constants)
			chunk->_constants[offset++] = c;

		std::memcpy(chunk->_chunks, _chunks.data(), chunk->_chunk_count);
		
		offset = 0;
		for (const inst::Instruction& inst : _instructions)
			chunk->_code[offset++] = inst;

		return chunk;
	}
}



namespace kpl
{
	const char* Chunk::Constant::make_string(const char* str)
	{
		Size len = std::strlen(str);
		char* newstr = utils::malloc<char>(len + 1);
		if (len > 0)
			std::memcpy(newstr, str, len);
		newstr[len] = '\0';
		return newstr;
	}

	void Chunk::Constant::destroy()
	{
		if (_type == ConstantType::String && _value.string)
			utils::free(const_cast<char*>(_value.string));

		_type = ConstantType::Null;
	}

	Chunk::Constant& Chunk::Constant::copy(const Constant& c, bool reset)
	{
		if (reset)
			destroy();

		_type = c._type;
		if (c._type == ConstantType::String)
			_value.string = make_string(c._value.string);
		else _value = c._value;

		return *this;
	}

	Chunk::Constant& Chunk::Constant::move(Constant&& c, bool reset)
	{
		if (reset)
			destroy();

		_type = c._type;
		_value = std::move(c._value);

		return *this;
	}

	Value* Chunk::Constant::make_value(MemoryHeap* heap) const
	{
		switch (_type)
		{
			case ConstantType::Null: return type::literal::Null;
			case ConstantType::Integer: return heap->make_integer(_value.integral);
			case ConstantType::Float: return heap->make_float(_value.floating);
			case ConstantType::Boolean: return _value.boolean ? type::literal::True : type::literal::False;
			case ConstantType::String: return heap->make_string(_value.string);
		}

		return type::literal::Null;
	}
}




namespace kpl
{
	Chunk::~Chunk()
	{
		if (_data)
		{
			for (Offset i = 0; i < _constant_count; ++i)
				utils::destroy(_constants[i]);

			for (Offset i = 0; i < _chunk_count; ++i)
				delete _chunks[i];

			utils::free(_data);
		}

		std::memset(this, 0, sizeof(*this));
	}
}
