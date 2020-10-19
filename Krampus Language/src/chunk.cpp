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

		chunk->_constants = reinterpret_cast<Value*>(chunk->_data);
		chunk->_chunks = reinterpret_cast<Chunk**>(chunk->_constants + chunk->_constant_count);
		chunk->_code = reinterpret_cast<InstructionCode*>(chunk->_chunks + chunk->_chunk_count);

		Offset offset = 0;
		for (const Value& c : _constants)
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
