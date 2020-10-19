#include "kplstate.h"

namespace kpl
{
	const Value& GlobalsManager::get_value(const std::string& name) const
	{
		auto it = _values.find(name);
		return it == _values.end() ? _nullvalue : it->second;
	}
}
