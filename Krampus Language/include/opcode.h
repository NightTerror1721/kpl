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
		ADD,		// A KB KC
		SUB,		// A KB KC
		MUL,		// A KB KC
		DIV,		// A KB KC
		IDIV,		// A KB KC
		MOD,		// A KB KC
		EQ,			// KB KC
		NE,			// KB KC
		GR,			// KB KC
		LS,			// KB KC
		GE,			// KB KC
		LE,			// KB KC
		SHL,		// KB KC
		SHR,		// KB KC
		BAND,		// A KB KC
		BOR,		// A KB KC
		XOR,		// A KB KC
		BNOT,		// A KB
		NOT,		// A KB
		NEG,		// A KB
		LEN,		// A KB
		IN,			// A KB KC
		INSTANCEOF,	// A KB KC
		GET,		// A KB KC
		SET,		// A KB KC
		JP,			// Ax
		TEST,		// KB C
		TEST_SET,	// A KB C
		CALL,		// A B
		INVOKE,		// A KB C
		RETURN,		// A KB
	};
}
