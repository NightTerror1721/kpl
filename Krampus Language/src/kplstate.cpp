#include "kplstate.h"

namespace kpl
{
	Value* GlobalsManager::get_value(const std::string& name)
	{
		auto it = _values.find(name);
		return it == _values.end() ? type::literal::Null : static_cast<Value*>(it->second);
	}
}
