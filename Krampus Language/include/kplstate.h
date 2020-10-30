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

		inline void set_value(const Value& name, const Value& value)
		{
			if (name.type() == DataType::String)
				_values[name.string()] = value;
			else _values[name.to_string()] = value;
		}
		inline const Value& get_value(const Value& name) const
		{
			if (name.type() == DataType::String)
				return get_value(name.string());
			return get_value(name.to_string());
		}
		inline void delete_value(const Value& name)
		{
			if (name.type() == DataType::String)
				_values.erase(name.string());
			else _values.erase(name.to_string());
		}
	};



	class KPLState : public MemoryHeap
	{
	public:
		friend Value runtime::execute(KPLState& state, Function& function, const Value& self, const CallArguments& args);

	private:
		MemoryHeap _heap;
		GlobalsManager _globals;
		runtime::CallStack _calls;
		runtime::RegisterStack _regs;

	public:
		KPLState() = default;
		~KPLState() = default;
	};
}
