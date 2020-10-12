#include "common.h"
#include "mheap.h"

int main(int argc, char** argv)
{
	kpl::MemoryHeap heap;

	kpl::type::String* str = heap.make_string("hello world");
	heap.free(str);

	return 0;
}
