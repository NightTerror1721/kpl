#include "kplstate.h"

namespace kpl
{
	Value* GlobalsManager::get_value(const std::string& name)
	{
		auto it = _values.find(name);
		return it == _values.end() ? MemoryHeap::literal_null() : it->second;
	}
}
