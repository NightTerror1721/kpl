#pragma once

#include "common.h"

namespace kpl::opcode
{
	enum class id : UInt8
	{
		NOP,
		MOVE,
		LOADL_K,
		LOAD_BOOL,
		LOAD_NULL
	};
}
