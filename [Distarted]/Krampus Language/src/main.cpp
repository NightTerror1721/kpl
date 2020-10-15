#include "common.h"
#include "mheap.h"
#include "instruction.h"

int main(int argc, char** argv)
{
	kpl::MemoryHeap heap;

	kpl::inst::Instruction inst;

	inst.sax(-32);

	std::cout << inst.sax() << std::endl;

	return 0;
}
