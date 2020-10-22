#pragma once

#include "common.h"

namespace kpl::obj::special_property
{
	static constexpr const char* operator_add = "__add__";
	static constexpr const char* operator_sub = "__sub__";
	static constexpr const char* operator_mul = "__mul__";
	static constexpr const char* operator_div = "__div__";
	static constexpr const char* operator_idiv = "__idiv__";
	static constexpr const char* operator_mod = "__mod__";

	static constexpr const char* operator_eq = "__eq__";
	static constexpr const char* operator_noeq = "__ne__";
	static constexpr const char* operator_gr = "__gr__";
	static constexpr const char* operator_ls = "__ls__";
	static constexpr const char* operator_ge = "__ge__";
	static constexpr const char* operator_le = "__le__";

	static constexpr const char* operator_shl = "__shl__";
	static constexpr const char* operator_shr = "__shr__";
	static constexpr const char* operator_band = "__band__";
	static constexpr const char* operator_bor = "__bor__";
	static constexpr const char* operator_bnot = "__bnot__";

	static constexpr const char* operator_not = "__not__";
	static constexpr const char* operator_neg = "__neg__";
	static constexpr const char* operator_xor = "__xor__";


	static constexpr const char* constructor = "__constructor__";
}
