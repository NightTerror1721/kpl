#pragma once

#include "mheap.h"
#include "data_types.h"
#include "runtime.h"

namespace kpl
{
	class GlobalsManager
	{
	private:
		std::unordered_map<std::string, Value> _values;
		Value _nullvalue = nullptr;

	public:
		GlobalsManager() = default;
		GlobalsManager(const GlobalsManager&) = default;
		GlobalsManager(GlobalsManager&&) noexcept = default;
		~GlobalsManager() = default;

		GlobalsManager& operator= (const GlobalsManager&) = default;
		GlobalsManager& operator= (GlobalsManager&&) noexcept = default;

		const Value& get_value(const std::string& name) const;

		inline void set_value(const std::string& name, const Value& value) { _values[name] = value; }
		inline void set_value(const std::string& name, Value&& value) { _values[name] = std::move(value); }
		inline void delete_value(const std::string& name) { _values.erase(name); }
	};



	class KPLState
	{
	public:
		friend void runtime::execute(KPLState& state, Function& function);

	private:
		MemoryHeap _heap;
		GlobalsManager _globals;
		runtime::CallStack _calls;
		runtime::RegisterStack _regs;
	};
}
