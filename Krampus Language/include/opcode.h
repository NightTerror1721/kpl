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



	static constexpr const char* name(id opcode_id)
	{
		switch (opcode_id)
		{
			case id::NOP: return "nop";
			case id::MOVE: return "move";
			case id::LOAD_K: return "load_k";
			case id::LOAD_BOOL: return "load_bool";
			case id::LOAD_NULL: return "load_null";
			case id::LOAD_INT: return "load_int";
			case id::GET_GLOBAL: return "get_global";
			case id::GET_LOCAL: return "get_local";
			case id::GET_PROP: return "get_prop";
			case id::SET_GLOBAL: return "set_global";
			case id::SET_LOCAL: return "set_local";
			case id::SET_PROP: return "set_prop";
			case id::NEW_ARRAY: return "new_array";
			case id::NEW_LIST: return "new_list";
			case id::NEW_OBJECT: return "new_object";
			case id::SET_AL: return "set_al";
			case id::SELF: return "self";
			case id::ADD: return "add";
			case id::SUB: return "sub";
			case id::MUL: return "mul";
			case id::DIV: return "div";
			case id::IDIV: return "idiv";
			case id::MOD: return "mod";
			case id::EQ: return "eq";
			case id::NE: return "ne";
			case id::GR: return "gr";
			case id::LS: return "ls";
			case id::GE: return "ge";
			case id::LE: return "le";
			case id::SHL: return "shl";
			case id::SHR: return "shr";
			case id::BAND: return "band";
			case id::BOR: return "bor";
			case id::XOR: return "xor";
			case id::BNOT: return "bnot";
			case id::NOT: return "not";
			case id::NEG: return "neg";
			case id::LEN: return "len";
			case id::IN: return "in";
			case id::INSTANCEOF: return "instanceof";
			case id::GET: return "get";
			case id::SET: return "set";
			case id::JP: return "jp";
			case id::TEST: return "test";
			case id::TEST_SET: return "test_set";
			case id::CALL: return "call";
			case id::INVOKE: return "invoke";
			case id::RETURN: return "return";
		}

		return "<unknown-opcode>";
	}
}
