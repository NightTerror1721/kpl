#include "common.h"
#include "kplstate.h"
#include "mheap.h"
#include "data_types.h"
#include "instruction.h"
#include "chunk.h"
#include "runtime.h"

using namespace kpl;
using namespace kpl::inst;

static constexpr int size = sizeof(kpl::MemoryBlock);

std::vector<ChunkConstant> program_consts();
InstructionList program_code();

int main(int argc, char** argv)
{
	MemoryHeap heap;

	Value obj = heap.make_object();
	obj.object().insert({ "hola", 50LL });
	obj.object()["power"] = 50;

	KPLState state;

	InstructionList insts = program_code();

	Chunk chunk;
	chunk.builder()
		.registers(2)
		.chunks({})
		.constants(program_consts())
		.instructions(insts)
		.build();

	type::Function func(chunk);
	Value result = runtime::execute(state, func, type::literal::Null);
	std::cout << result.to_string() << std::endl;

	return 0;
}

std::vector<ChunkConstant> program_consts()
{
	return {
		256LL,
		"Hello World"
	};
}

InstructionList program_code()
{
	InstructionList inst;
	return inst
		//<< Instruction::add(0, -1, -1)
		<< Instruction::load_k(0, 1)
		<< Instruction::return_(true, -2);
}
