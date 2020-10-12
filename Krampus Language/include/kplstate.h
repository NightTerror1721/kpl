#pragma once

#include "mheap.h"
#include "data_types.h"

namespace kpl
{
	class GlobalsManager
	{
	private:
		std::unordered_map<std::string, ValueReference> _values;

	public:
		GlobalsManager() = default;
		GlobalsManager(const GlobalsManager&) = default;
		GlobalsManager(GlobalsManager&&) noexcept = default;
		~GlobalsManager() = default;

		GlobalsManager& operator= (const GlobalsManager&) = default;
		GlobalsManager& operator= (GlobalsManager&&) noexcept = default;

		Value* get_value(const std::string& name);

		inline void set_value(const std::string& name, Value* value) { _values[name] = value; }
		inline void delete_value(const std::string& name) { _values.erase(name); }
	};



	class KPLState
	{
	private:
		MemoryHeap _heap;
		GlobalsManager _globals;
	};
}
