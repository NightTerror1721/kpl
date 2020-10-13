#pragma once

#include "data_types.h"

namespace kpl
{
	class ChunkBuilder;

	class Chunk
	{
	private:
		MemoryHeap* const _heap;

		Value** _constants;
		Size _constant_count;

		Chunk** _chunks;
		Size _chunk_count;

		Size _register_count;

		void* _code;
		Size _code_count;

		void* _data;

	public:
		inline Chunk(MemoryHeap& heap) :
			_heap{ &heap },
			_constants{ nullptr },
			_constant_count{ 0 },
			_chunks{ nullptr },
			_chunk_count{ 0 },
			_register_count{ 0 },
			_code{ nullptr },
			_code_count{ 0 },
			_data{ nullptr }
		{}

		Chunk(const Chunk&) = delete;
		Chunk(Chunk&&) noexcept = delete;

		Chunk& operator= (const Chunk&) = delete;
		Chunk operator= (Chunk&&) noexcept = delete;

		inline Size constants_count() const { return _constant_count; }
		inline Value* constant(Offset index) const { return _constants[index]; }

		inline Size chunk_count() const { return _chunk_count; }
		inline Chunk* chunk(Offset index) const { return _chunks[index]; }

		inline Size register_count() const { return _register_count; }
	};



	class ChunkBuilder
	{
	private:
		std::vector<Value*> _constants;
		std::vector<Chunk*> _chunks;
	};
}
