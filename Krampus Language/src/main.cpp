#include "common.h"
#include "mheap.h"
#include "data_types.h"

static constexpr int size = sizeof(kpl::MemoryBlock);

int main(int argc, char** argv)
{
	kpl::MemoryHeap heap;

	kpl::Value obj = heap.make_object();
	obj.object().insert({ "hola", 50LL });
	obj.object()["power"] = 50;

	return 0;
}
