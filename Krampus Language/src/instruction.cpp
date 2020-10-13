#include "instruction.h"

namespace kpl::inst
{
	Instruction& Instruction::b(int value)
	{
		if (value >= 0)
			_inst = (_inst & b_mask) | static_cast<Instruction>((value & 0xff) << 15);
		else
		{
			unsigned int uvalue = static_cast<unsigned int>(value);
			_inst = (_inst & b_mask) | static_cast<Instruction>((uvalue & 0xff) << 15) | (0x1 << 14);
		}

		return *this;
	}

	Instruction& Instruction::c(int value)
	{
		if (value >= 0)
			_inst = (_inst & c_mask) | static_cast<Instruction>((value & 0xff) << 24);
		else
		{
			unsigned int uvalue = static_cast<unsigned int>(value);
			_inst = (_inst & c_mask) | static_cast<Instruction>((uvalue & 0xff) << 24) | (0x1 << 23);
		}

		return *this;
	}

	Instruction& Instruction::sbx(int value)
	{
		if (value >= 0)
			_inst = (_inst & bx_mask) | static_cast<Instruction>((value & 0x1ffff) << 15);
		else
		{
			unsigned int uvalue = static_cast<unsigned int>(value);
			_inst = (_inst & bx_mask) | static_cast<Instruction>((uvalue & 0x1ffff) << 15) | (0x1 << 14);
		}

		return *this;
	}

	Instruction& Instruction::sax(int value)
	{
		if (value >= 0)
			_inst = (_inst & ax_mask) | static_cast<Instruction>((value & 0x1ffffff) << 7);
		else
		{
			unsigned int uvalue = static_cast<unsigned int>(value);
			_inst = (_inst & ax_mask) | static_cast<Instruction>((uvalue & 0x1ffffff) << 7) | (0x1 << 6);
		}

		return *this;
	}
}
