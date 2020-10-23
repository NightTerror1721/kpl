#pragma once

#include "common.h"

namespace kpl::opcode
{
	enum class id : UInt8
	{
		NOP,		//
		MOVE,		// A B
		LOAD_K,		// A Bx
		LOAD_BOOL,	// A B C
		LOAD_NULL,	// A B
		LOAD_INT,	// A sBx
		GET_GLOBAL,	// A KB
		GET_LOCAL,	// A KB
		GET_PROP,	// A KB KC
		SET_GLOBAL,	// KB KC
		SET_LOCAL,	// KB KC
		SET_PROP,	// A KB KC
		NEW_ARRAY,	// A KB
		NEW_LIST,	// A
		NEW_OBJECT,	// A KB C
		SET_AL,		// A B C
		SELF,		// A



		CALL,		// A B
		INVOKE,		// A KB C
		RETURN,		// A KB
	};
}
