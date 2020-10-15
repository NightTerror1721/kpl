#pragma once

#include "mheap.h"
#include "data_types.h"
#include "instruction.h"

namespace kpl
{
	class Chunk;

	class ChunkBuilder
	{
	private:
		Chunk* _chunk = nullptr;
		std::vector<Chunk::Constant> _constants;
		std::vector<Chunk*> _chunks;
		std::vector<inst::Instruction> _instructions;
		UInt8 _registers;

	public:
		ChunkBuilder() = default;
		ChunkBuilder(const ChunkBuilder&) = default;
		ChunkBuilder(ChunkBuilder&&) noexcept = default;
		~ChunkBuilder() = default;

		ChunkBuilder& operator= (const ChunkBuilder&) = default;
		ChunkBuilder& operator= (ChunkBuilder&&) noexcept = default;

		inline ChunkBuilder(Chunk* chunk) :
			_chunk{ chunk },
			_constants{},
			_chunks{},
			_instructions{}
		{}

		inline ChunkBuilder& constants(const std::vector<Chunk::Constant>& constants) { return _constants = constants, *this; }
		inline ChunkBuilder& constants(std::vector<Chunk::Constant>&& constants) { return _constants = std::move(constants), *this; }

		inline ChunkBuilder& chunks(const std::vector<Chunk*>& chunks) { return _chunks = chunks, *this; }
		inline ChunkBuilder& chunks(std::vector<Chunk*>&& chunks) { return _chunks = std::move(chunks), *this; }

		inline ChunkBuilder& instructions(const std::vector<inst::Instruction>& instructions) { return _instructions = instructions, *this; }
		inline ChunkBuilder& instructions(std::vector<inst::Instruction>&& instructions) { return _instructions = std::move(instructions), *this; }

		inline ChunkBuilder& registers(unsigned int count) { return _registers = static_cast<UInt8>(utils::clamp(count, 0, 255)), *this; }


		Chunk* build(Chunk* chunk = nullptr);

	public:
		inline ChunkBuilder& operator<< (const std::vector<Chunk::Constant>& right) { return constants(right); }
		inline ChunkBuilder& operator<< (std::vector<Chunk::Constant>&& right) { return constants(std::move(right)); }

		inline ChunkBuilder& operator<< (const std::vector<Chunk*>& right) { return chunks(right); }
		inline ChunkBuilder& operator<< (std::vector<Chunk*>&& right) { return chunks(std::move(right)); }

		inline ChunkBuilder& operator<< (const std::vector<inst::Instruction>& right) { return instructions(right); }
		inline ChunkBuilder& operator<< (std::vector<inst::Instruction>&& right) { return instructions(std::move(right)); }

		inline ChunkBuilder& operator<< (unsigned int right) { return registers(right); }
	};




	class Chunk
	{
	public:
		enum class ConstantType { Null, Integer, Float, Boolean, String };

		class Constant
		{
			ConstantType _type;
			union {
				Int64 integral;
				double floating;
				bool boolean;
				const char* string;
			} _value;

		private:
			static const char* make_string(const char* str);
			void destroy();
			Constant& copy(const Constant& c, bool reset);
			Constant& move(Constant&& c, bool reset);

		public:
			inline Constant() : _type{ ConstantType::Null }, _value{} {}
			inline Constant(decltype(nullptr)) : _type{ ConstantType::Null }, _value{} {}
			inline Constant(Int64 value) : _type{ ConstantType::Integer }, _value{ .integral = value } {}
			inline Constant(double value) : _type{ ConstantType::Float }, _value{ .floating = value } {}
			inline Constant(bool value) : _type{ ConstantType::Boolean }, _value{ .boolean = value } {}
			inline Constant(const char* value) :
				_type{ value ? ConstantType::String : ConstantType::Null },
				_value{ .string = value ? make_string(value) : nullptr }
			{}

			inline Constant(const Constant& c) : Constant{} { copy(c, false); }
			inline Constant(Constant&& c) noexcept : Constant{} { move(std::move(c), false); }

			inline ~Constant() { destroy(); }

			inline Constant& operator= (const Constant& right) { return copy(right, true); }
			inline Constant& operator= (Constant&& right) noexcept { return move(std::move(right), true); }

			inline Constant& operator= (decltype(nullptr)) { return destroy(), *this; }
			inline Constant& operator= (Int64 value) { return destroy(), _type = ConstantType::Integer, _value.integral = value, *this; }
			inline Constant& operator= (double value) { return destroy(), _type = ConstantType::Float, _value.floating = value, *this; }
			inline Constant& operator= (bool value) { return destroy(), _type = ConstantType::Boolean, _value.boolean = value, *this; }
			inline Constant& operator= (const char* value)
			{
				destroy();
				_type = value ? ConstantType::String : ConstantType::Null;
				_value.string = value ? make_string(value) : nullptr;
				return *this;
			}

			inline ConstantType type() const { return _type; }

			inline Int64 integral() const { return _value.integral; }
			inline double floating() const { return _value.floating; }
			inline bool boolean() const { return _value.boolean; }
			inline const char* string() const { return _value.string; }

			Value* make_value(MemoryHeap* heap) const;
		};

	private:
		Constant* _constants;
		Size _constant_count;

		Chunk** _chunks;
		Size _chunk_count;

		unsigned int _register_count;

		InstructionCode* _code;
		Size _code_count;

		void* _data;

	private:
		static constexpr int constant_size = sizeof(*_constants);
		static constexpr int chunk_size = sizeof(*_chunks);
		static constexpr int instruction_size = sizeof(*_code);
		static constexpr int chunk_object_size(Size constants, Size chunks, unsigned int code)
		{
			return constants * constant_size + chunks * chunk_size + code * instruction_size;
		}

	public:
		inline Chunk() :
			_constants{ nullptr },
			_constant_count{ 0 },
			_chunks{ nullptr },
			_chunk_count{ 0 },
			_register_count{ 0 },
			_code{ nullptr },
			_code_count{ 0 },
			_data{ nullptr }
		{}
		~Chunk();

		Chunk(const Chunk&) = delete;
		Chunk(Chunk&&) noexcept = delete;

		Chunk& operator= (const Chunk&) = delete;
		Chunk operator= (Chunk&&) noexcept = delete;

		inline Size constants_count() const { return _constant_count; }
		inline const Constant& constant(Offset index) const { return _constants[index]; }

		inline Size chunk_count() const { return _chunk_count; }
		inline Chunk* chunk(Offset index) const { return _chunks[index]; }

		inline Size register_count() const { return _register_count; }

		friend class ChunkBuilder;
	};
}
