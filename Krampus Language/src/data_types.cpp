#include "data_types.h"
#include "mheap.h"

namespace kpl::type
{
	String::String(MemoryHeap* heap, const char* const str, Size len) :
		Value{ DataType::String },
		_heap{ heap },
		_size{ len },
		_str{ len == 0 ? nullptr : reinterpret_cast<char*>(heap->malloc(sizeof(char) * len)) },
		_hashcode{ 0 }
	{
		if(len > 0)
			std::memcpy(_str, str, len);
	}

	String::String(const String& str) : String{ str._heap, str._str, str._size } {}
	String::String(String&& str) noexcept;

	String& String::operator= (const String& right);
	String& String::operator= (String&& right) noexcept;
}
