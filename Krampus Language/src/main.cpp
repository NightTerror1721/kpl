#include "common.h"
#include "mheap.h"
#include "instruction.h"

int main(int argc, char** argv)
{
	kpl::MemoryHeap heap;

	kpl::inst::Instruction inst;

	inst.a(1).b(2).c(-1);

	std::cout << inst.c() << std::endl;

	return 0;
}
