#pragma once

#include "common.h"
#include "opcode.h"

namespace kpl { typedef UInt32 InstructionCode; }

namespace kpl::inst::arg
{
	constexpr kpl::op::Opcode opcode(InstructionCode inst) { return static_cast<kpl::op::Opcode>(inst & 0x3f); }

	constexpr unsigned int a(InstructionCode inst) { return static_cast<unsigned int>(inst >> 6) & 0xff; }
	constexpr int b(InstructionCode inst) { return static_cast<int>((inst >> 15) & 0xff) | (((inst >> 14) & 0x1) << 31); }
	constexpr int c(InstructionCode inst) { return static_cast<int>((inst >> 24) & 0xff) | (((inst >> 23) & 0x1) << 31); }

	constexpr unsigned int bx(InstructionCode inst) { return static_cast<unsigned int>((inst >> 14) & 0x3ffff); }
	constexpr int sbx(InstructionCode inst) { return static_cast<int>((inst >> 15) & 0x1ffff) | (((inst >> 14) & 0x1) << 31); }

	constexpr unsigned int ax(InstructionCode inst) { return static_cast<unsigned int>((inst >> 6) & 0x3ffffff); }
	constexpr int sax(InstructionCode inst) { return static_cast<int>((inst >> 7) & 0x1ffffff) | (((inst >> 6) & 0x1) << 31); }
}

namespace kpl::inst
{
	class Instruction
	{
	private:
		static constexpr InstructionCode opcode_mask	= 0xffffffc0;
		static constexpr InstructionCode a_mask			= 0xffffc03f;
		static constexpr InstructionCode b_mask			= 0xff803fff;
		static constexpr InstructionCode c_mask			= 0x007fffff;
		static constexpr InstructionCode bx_mask		= 0x00003fff;
		static constexpr InstructionCode sbx_mask		= 0x00003fff;
		static constexpr InstructionCode ax_mask		= 0x0000003f;
		static constexpr InstructionCode sax_mask		= 0x0000003f;

	private:
		InstructionCode _inst = 0;

	public:
		Instruction() = default;
		Instruction(const Instruction&) = default;
		Instruction(Instruction&&) noexcept = default;
		~Instruction() = default;

		Instruction& operator= (const Instruction&) = default;
		Instruction& operator= (Instruction&&) noexcept = default;

		Instruction& b(int value);
		Instruction& c(int value);
		Instruction& sbx(int value);
		Instruction& sax(int value);


		inline Instruction& opcode(kpl::op::Opcode op) { return _inst = (_inst & opcode_mask) | static_cast<InstructionCode>(static_cast<int>(op) & 0x3f), *this; }
		inline Instruction& a(unsigned int value) { return _inst = (_inst & a_mask) | static_cast<InstructionCode>((value & 0xff) << 6), *this; }
		inline Instruction& bx(unsigned int value) { return _inst = (_inst & bx_mask) | static_cast<InstructionCode>((value & 0x3ffff) << 14), *this; }
		inline Instruction& ax(unsigned int value) { return _inst = (_inst & ax_mask) | static_cast<InstructionCode>((value & 0x3ffffff) << 6), *this; }

		inline kpl::op::Opcode opcode() const { return arg::opcode(_inst); }
		inline unsigned int a() const { return arg::a(_inst); }
		inline int b() const { return arg::b(_inst); }
		inline int c() const { return arg::c(_inst); }
		inline unsigned int bx() const { return arg::bx(_inst); }
		inline int sbx() const { return arg::sbx(_inst); }
		inline unsigned int ax() const { return arg::ax(_inst); }
		inline int sax() const { return arg::sax(_inst); }


		inline Instruction(InstructionCode inst) : _inst{ inst } {}
		inline Instruction& operator= (InstructionCode inst) { return _inst = inst, *this; }
		inline operator InstructionCode() const { return _inst; }
	};
}
