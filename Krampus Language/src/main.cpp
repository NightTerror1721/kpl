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

int main(int argc, char** argv)
{
	MemoryHeap heap;

	Value obj = heap.make_object();
	obj.object().insert({ "hola", 50LL });
	obj.object()["power"] = 50;

	KPLState state;

	InstructionList insts;
	insts << Instruction::load_k(0, 0) << Instruction::load_int(1, 256);

	Chunk chunk;
	chunk.builder()
		.registers(2)
		.chunks({})
		.constants({ Value("hello world") })
		.instructions(insts)
		.build();

	type::Function func(chunk);
	runtime::execute(state, func);


	return 0;
}
