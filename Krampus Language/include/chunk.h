#pragma once

#include "mheap.h"
#include "data_types.h"
#include "instruction.h"

namespace kpl
{
	class ChunkConstant
	{
	public:
		enum class Type { Null, Integer, Float, Boolean, String };

	private:
		Type _type;
		union {
			decltype(nullptr) null;
			Int64 integral;
			double floating;
			bool boolean;
			struct { const char* string; Size string_len; };
		} _value;

	public:
		inline ChunkConstant() : _type{ Type::Null }, _value{ .null = nullptr } {}
		inline ChunkConstant(decltype(nullptr)) : _type{ Type::Null }, _value{ .null = nullptr } {}
		inline ChunkConstant(Int64 value) : _type{ Type::Integer }, _value{ .integral = value } {}
		inline ChunkConstant(float value) : _type{ Type::Float }, _value{ .floating = static_cast<double>(value) } {}
		inline ChunkConstant(double value) : _type{ Type::Float }, _value{ .floating = value } {}
		inline ChunkConstant(bool value) : _type{ Type::Boolean }, _value{ .boolean = value } {}

		template<std::integral _Ty>
		inline ChunkConstant(_Ty value) : _type{ Type::Integer }, _value{ .integral = static_cast<Int64>(value) } {}

		inline Type type() const { return _type; }

		ChunkConstant(const char* string);
		ChunkConstant(const char* string, Size count);
		ChunkConstant(const std::string& string);

		ChunkConstant(const Value& value);

		ChunkConstant(const ChunkConstant& right);
		ChunkConstant(ChunkConstant&& right) noexcept;
		~ChunkConstant();

		ChunkConstant& operator= (const ChunkConstant& right);
		ChunkConstant& operator= (ChunkConstant&& right) noexcept;

		Value to_value() const;
	};



	class ChunkBuilder
	{
	private:
		Chunk* _chunk = nullptr;
		std::vector<ChunkConstant> _constants;
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
			_instructions{},
			_registers{ 0 }
		{}

		inline ChunkBuilder& constants(const std::vector<ChunkConstant>& constants) { return _constants = constants, *this; }
		inline ChunkBuilder& constants(std::vector<ChunkConstant>&& constants) { return _constants = std::move(constants), *this; }

		inline ChunkBuilder& chunks(const std::vector<Chunk*>& chunks) { return _chunks = chunks, *this; }
		inline ChunkBuilder& chunks(std::vector<Chunk*>&& chunks) { return _chunks = std::move(chunks), *this; }

		inline ChunkBuilder& instructions(const std::vector<inst::Instruction>& instructions) { return _instructions = instructions, *this; }
		inline ChunkBuilder& instructions(std::vector<inst::Instruction>&& instructions) { return _instructions = std::move(instructions), *this; }

		inline ChunkBuilder& registers(unsigned int count) { return _registers = static_cast<UInt8>(utils::clamp(count, 0, 255)), *this; }


		Chunk* build(Chunk* chunk = nullptr);

	public:
		inline ChunkBuilder& operator<< (const std::vector<ChunkConstant>& right) { return constants(right); }
		inline ChunkBuilder& operator<< (std::vector<ChunkConstant>&& right) { return constants(std::move(right)); }

		inline ChunkBuilder& operator<< (const std::vector<Chunk*>& right) { return chunks(right); }
		inline ChunkBuilder& operator<< (std::vector<Chunk*>&& right) { return chunks(std::move(right)); }

		inline ChunkBuilder& operator<< (const std::vector<inst::Instruction>& right) { return instructions(right); }
		inline ChunkBuilder& operator<< (std::vector<inst::Instruction>&& right) { return instructions(std::move(right)); }

		inline ChunkBuilder& operator<< (unsigned int right) { return registers(right); }
	};




	class Chunk
	{
	private:
		Value* _constants;
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
		inline const Value& constant(Offset index) const { return _constants[index]; }

		inline Size chunk_count() const { return _chunk_count; }
		inline Chunk* chunk(Offset index) const { return _chunks[index]; }

		inline Size register_count() const { return _register_count; }

		inline InstructionCode instruction(Offset index) const { return _code[index]; }
		inline Size instruction_count() const { return _code_count; }

		inline ChunkBuilder builder() { return { this }; }
		static inline ChunkBuilder builder(Chunk* chunk) { return { chunk }; }

		friend class ChunkBuilder;
	};
}
