#pragma once

#include "common.h"
#include "opcode.h"


#define __KPL_INST_ARG_OPCODE(_Inst) static_cast<kpl::opcode::id>((_Inst) & 0x3f)

#define __KPL_INST_ARG_A(_Inst) (static_cast<unsigned int>((_Inst) >> 6) & 0xff)
#define __KPL_INST_ARG_B(_Inst) (static_cast<unsigned int>((_Inst) >> 15) & 0xff)
#define __KPL_INST_ARG_C(_Inst) (static_cast<unsigned int>((_Inst) >> 24) & 0xff)

#define __KPL_INST_ARG_KB(_Inst) (((_Inst) >> 14) & 0x1)
#define __KPL_INST_ARG_KC(_Inst) (((_Inst) >> 23) & 0x1)

#define __KPL_INST_ARG_BX(_Inst) static_cast<unsigned int>(((_Inst) >> 14) & 0x3ffff)
#define __KPL_INST_ARG_SBX(_Inst) ((((_Inst) >> 14) & 0x1) ? (((_Inst) >> 15) & 0x1ffff) | 0xffff8000 : (((_Inst) >> 15) | 0x1ffff))
#define __KPL_INST_ARG_AX(_Inst) static_cast<unsigned int>(((_Inst) >> 6) & 0x3ffffff)
#define __KPL_INST_ARG_SAX(_Inst) ((((_Inst) >> 6) & 0x1) ? (((_Inst) >> 7) & 0x1ffffff) | 0xffffff80 : (((_Inst) >> 7) | 0x1ffffff))


namespace kpl::inst::arg
{
	constexpr kpl::opcode::id opcode(InstructionCode inst) { return __KPL_INST_ARG_OPCODE(inst); }

	constexpr unsigned int a(InstructionCode inst) { return __KPL_INST_ARG_A(inst); }
	constexpr unsigned int b(InstructionCode inst) { return __KPL_INST_ARG_B(inst); }
	constexpr unsigned int c(InstructionCode inst) { return __KPL_INST_ARG_C(inst); }

	constexpr bool kb(InstructionCode inst) { return __KPL_INST_ARG_KB(inst); }
	constexpr bool kc(InstructionCode inst) { return __KPL_INST_ARG_KC(inst); }

	constexpr unsigned int bx(InstructionCode inst) { return __KPL_INST_ARG_BX(inst); }
	constexpr int sbx(InstructionCode inst) { return __KPL_INST_ARG_SBX(inst); }
	constexpr unsigned int ax(InstructionCode inst) { return __KPL_INST_ARG_AX(inst); }
	constexpr int sax(InstructionCode inst) { return __KPL_INST_ARG_SAX(inst); }
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

		std::ostream& dump(std::ostream& os) const;

		Instruction& b(int value);
		Instruction& c(int value);
		Instruction& sbx(int value);
		Instruction& sax(int value);


		inline Instruction& opcode(kpl::opcode::id op) { return _inst = (_inst & opcode_mask) | static_cast<InstructionCode>(static_cast<int>(op) & 0x3f), *this; }
		inline Instruction& a(unsigned int value) { return _inst = (_inst & a_mask) | static_cast<InstructionCode>((value & 0xff) << 6), *this; }
		inline Instruction& bx(unsigned int value) { return _inst = (_inst & bx_mask) | static_cast<InstructionCode>((value & 0x3ffff) << 14), *this; }
		inline Instruction& ax(unsigned int value) { return _inst = (_inst & ax_mask) | static_cast<InstructionCode>((value & 0x3ffffff) << 6), *this; }

		inline Instruction& b(unsigned int value, bool is_constant) { return b(is_constant ? -static_cast<int>(value + 1) : static_cast<int>(value)); }
		inline Instruction& c(unsigned int value, bool is_constant) { return c(is_constant ? -static_cast<int>(value + 1) : static_cast<int>(value)); }

		inline kpl::opcode::id opcode() const { return arg::opcode(_inst); }
		inline unsigned int a() const { return arg::a(_inst); }
		inline unsigned int b() const { return arg::b(_inst); }
		inline unsigned int c() const { return arg::c(_inst); }
		inline bool kb() const { return arg::kb(_inst); }
		inline bool kc() const { return arg::kc(_inst); }
		inline unsigned int bx() const { return arg::bx(_inst); }
		inline int sbx() const { return arg::sbx(_inst); }
		inline unsigned int ax() const { return arg::ax(_inst); }
		inline int sax() const { return arg::sax(_inst); }


		inline Instruction(InstructionCode inst) : _inst{ inst } {}
		inline Instruction& operator= (InstructionCode inst) { return _inst = inst, *this; }
		inline operator InstructionCode() const { return _inst; }


	public:
		typedef utils::RangedInt<255> A;
		typedef utils::RangedInt<255> B;
		typedef utils::RangedInt<255> C;
		typedef utils::RangedInt<255, -256> KB;
		typedef utils::RangedInt<255, -256> KC;
		typedef utils::RangedInt<(1 << 18) - 1> Bx;
		typedef utils::RangedInt<(1 << 17) - 1, -((1 << 17) - 1)> sBx;
		typedef utils::RangedInt<(1 << 26) - 1> Ax;
		typedef utils::RangedInt<(1 << 25) - 1, -((1 << 25) - 1)> sAx;

		static inline Instruction nop() { return Instruction().opcode(opcode::id::NOP); }

		static inline Instruction move(A dst_reg, B src_reg)
		{
			return Instruction().opcode(opcode::id::MOVE).a(dst_reg).b(src_reg);
		}

		static inline Instruction load_k(A dst_reg, Bx src_cnst)
		{
			return Instruction().opcode(opcode::id::LOAD_K).a(dst_reg).bx(src_cnst);
		}

		static inline Instruction load_bool(A dst_reg, B bool_value, C skip_next)
		{
			return Instruction().opcode(opcode::id::LOAD_BOOL).a(dst_reg).b(bool_value).c(skip_next);
		}

		static inline Instruction load_null(A first_reg, B last_reg)
		{
			return Instruction().opcode(opcode::id::LOAD_BOOL).a(first_reg).b(last_reg);
		}

		static inline Instruction load_int(A dst_reg, sBx value)
		{
			return Instruction().opcode(opcode::id::LOAD_INT).a(dst_reg).bx(value);
		}

		static inline Instruction get_global(A dst_reg, KB symbol)
		{
			return Instruction().opcode(opcode::id::GET_GLOBAL).a(dst_reg).b(symbol);
		}

		static inline Instruction get_local(A dst_reg, KB symbol)
		{
			return Instruction().opcode(opcode::id::GET_LOCAL).a(dst_reg).b(symbol);
		}

		static inline Instruction get_prop(A dst_reg, KB src, KC symbol)
		{
			return Instruction().opcode(opcode::id::GET_PROP).a(dst_reg).b(src).c(symbol);
		}

		static inline Instruction set_global(KB symbol, KC value)
		{
			return Instruction().opcode(opcode::id::SET_GLOBAL).b(symbol).c(value);
		}

		static inline Instruction set_local(KB symbol, KC value)
		{
			return Instruction().opcode(opcode::id::SET_LOCAL).b(symbol).c(value);
		}

		static inline Instruction set_prop(A obj_reg, KB symbol, KC value)
		{
			return Instruction().opcode(opcode::id::SET_PROP).a(obj_reg).b(symbol).c(value);
		}

		static inline Instruction new_array(A dst_reg, KB size)
		{
			return Instruction().opcode(opcode::id::NEW_ARRAY).a(dst_reg).b(size);
		}

		static inline Instruction new_list(A dst_reg)
		{
			return Instruction().opcode(opcode::id::NEW_LIST).a(dst_reg);
		}

		static inline Instruction new_object(A dst_reg, KB class_, C has_class)
		{
			return Instruction().opcode(opcode::id::NEW_OBJECT).a(dst_reg).b(class_).c(has_class);
		}

		static inline Instruction new_object(A al_reg, B first_reg, C last_reg)
		{
			return Instruction().opcode(opcode::id::SET_AL).a(al_reg).b(first_reg).c(last_reg);
		}

		static inline Instruction self(A dst_reg)
		{
			return Instruction().opcode(opcode::id::SELF).a(dst_reg);
		}

		static inline Instruction add(A dst_reg, KB left_reg, KC right_reg)
		{
			return Instruction().opcode(opcode::id::ADD).a(dst_reg).b(left_reg).c(right_reg);
		}

		static inline Instruction sub(A dst_reg, KB left_reg, KC right_reg)
		{
			return Instruction().opcode(opcode::id::SUB).a(dst_reg).b(left_reg).c(right_reg);
		}

		static inline Instruction mul(A dst_reg, KB left_reg, KC right_reg)
		{
			return Instruction().opcode(opcode::id::MUL).a(dst_reg).b(left_reg).c(right_reg);
		}

		static inline Instruction div(A dst_reg, KB left_reg, KC right_reg)
		{
			return Instruction().opcode(opcode::id::DIV).a(dst_reg).b(left_reg).c(right_reg);
		}

		static inline Instruction idiv(A dst_reg, KB left_reg, KC right_reg)
		{
			return Instruction().opcode(opcode::id::IDIV).a(dst_reg).b(left_reg).c(right_reg);
		}

		static inline Instruction mod(A dst_reg, KB left_reg, KC right_reg)
		{
			return Instruction().opcode(opcode::id::MOD).a(dst_reg).b(left_reg).c(right_reg);
		}

		static inline Instruction eq(KB left, KC right)
		{
			return Instruction().opcode(opcode::id::EQ).b(left).c(right);
		}

		static inline Instruction ne(KB left, KC right)
		{
			return Instruction().opcode(opcode::id::NE).b(left).c(right);
		}

		static inline Instruction gr(KB left, KC right)
		{
			return Instruction().opcode(opcode::id::GR).b(left).c(right);
		}

		static inline Instruction ls(KB left, KC right)
		{
			return Instruction().opcode(opcode::id::LS).b(left).c(right);
		}

		static inline Instruction ge(KB left, KC right)
		{
			return Instruction().opcode(opcode::id::GE).b(left).c(right);
		}

		static inline Instruction le(KB left, KC right)
		{
			return Instruction().opcode(opcode::id::LE).b(left).c(right);
		}

		static inline Instruction shl(A dst_reg, KB left, KC right)
		{
			return Instruction().opcode(opcode::id::SHL).a(dst_reg).b(left).c(right);
		}

		static inline Instruction shr(A dst_reg, KB left, KC right)
		{
			return Instruction().opcode(opcode::id::SHR).a(dst_reg).b(left).c(right);
		}

		static inline Instruction band(A dst_reg, KB left, KC right)
		{
			return Instruction().opcode(opcode::id::BAND).a(dst_reg).b(left).c(right);
		}

		static inline Instruction bor(A dst_reg, KB left, KC right)
		{
			return Instruction().opcode(opcode::id::BOR).a(dst_reg).b(left).c(right);
		}

		static inline Instruction xor_(A dst_reg, KB left, KC right)
		{
			return Instruction().opcode(opcode::id::XOR).a(dst_reg).b(left).c(right);
		}

		static inline Instruction bnot(A dst_reg, KB right)
		{
			return Instruction().opcode(opcode::id::BNOT).a(dst_reg).b(right);
		}

		static inline Instruction not_(A dst_reg, KB right)
		{
			return Instruction().opcode(opcode::id::NOT).a(dst_reg).b(right);
		}

		static inline Instruction neg(A dst_reg, KB right)
		{
			return Instruction().opcode(opcode::id::NEG).a(dst_reg).b(right);
		}

		static inline Instruction len(A dst_reg, KB right)
		{
			return Instruction().opcode(opcode::id::LEN).a(dst_reg).b(right);
		}

		static inline Instruction in(A dst_reg, KB left, KC right)
		{
			return Instruction().opcode(opcode::id::IN).a(dst_reg).b(left).c(right);
		}

		static inline Instruction instanceof(A dst_reg, KB left, KC right)
		{
			return Instruction().opcode(opcode::id::INSTANCEOF).a(dst_reg).b(left).c(right);
		}

		static inline Instruction get(A dst_reg, KB base, KC index)
		{
			return Instruction().opcode(opcode::id::INSTANCEOF).a(dst_reg).b(base).c(index);
		}

		static inline Instruction set(A base, KB index, KC value)
		{
			return Instruction().opcode(opcode::id::INSTANCEOF).a(base).b(index).c(value);
		}

		static inline Instruction set(Ax target)
		{
			return Instruction().opcode(opcode::id::INSTANCEOF).ax(target);
		}



		static inline Instruction call(A func, B args)
		{
			return Instruction().opcode(opcode::id::CALL).a(func).b(args);
		}

		static inline Instruction invoke(A func, KB symbol, C args)
		{
			return Instruction().opcode(opcode::id::INVOKE).a(func).b(symbol).c(args);
		}

		static inline Instruction return_(A return_any, KB value)
		{
			return Instruction().opcode(opcode::id::RETURN).a(return_any).b(value);
		}
	};
}




namespace kpl::inst
{
	class InstructionList
	{
	private:
		struct Node
		{
			Instruction instruction;
			Node* next = nullptr;
			Node* prev = nullptr;
		};

	public:
		class iterator;
		class const_iterator;
		typedef iterator Location;

	private:
		Node _ghost;
		Size _size;

	private:
		void _destroy();
		InstructionList& _copy(const InstructionList& list, bool reset);
		InstructionList& _move(InstructionList&& list, bool reset) noexcept;

		inline Node* _front() { return _ghost.next; }
		inline const Node* _front() const { return _ghost.next; }

		inline Node* _back() { return _ghost.prev; }
		inline const Node* _back() const { return _ghost.prev; }

	public:
		inline InstructionList() :
			_ghost{ .next = &_ghost, .prev = &_ghost },
			_size{ 0 }
		{}
		inline InstructionList(const InstructionList& list) : InstructionList() { _copy(list, false); }
		inline InstructionList(InstructionList&& list) noexcept : InstructionList() { _move(std::move(list), false); }
		inline ~InstructionList() { _destroy(); }

		inline InstructionList& operator= (const InstructionList& right) { return _copy(right, true); }
		inline InstructionList& operator= (InstructionList&& right) noexcept { return _move(std::move(right), true); }

		inline operator bool() const { return _ghost.next; }
		inline bool operator! () const { return !_ghost.next; }

		inline bool empty() const { return !_ghost.next; }
		inline Size size() const { return _size; }

		inline Instruction& front() { return _front()->instruction; }
		inline const Instruction& front() const { return _front()->instruction; }

		inline Instruction& back() { return _back()->instruction; }
		inline const Instruction& back() const { return _back()->instruction; }

		operator std::vector<Instruction>() const;

		Location push_front(const Instruction& inst);
		Location push_back(const Instruction& inst);

		Location insert_before(Location loc, const Instruction& inst);
		Location insert_after(Location loc, const Instruction& inst);

		Location replace(Location loc, const Instruction& inst);

		void swap(Location left, Location right);

		Location erase(Location loc);

		inline void clear() { _destroy(); }

		inline InstructionList& operator<< (const Instruction& right) { return push_back(right), *this; }
		inline InstructionList& operator<< (Instruction&& right) { return push_back(std::move(right)), *this; }

	public:
		class const_iterator
		{
		private:
			const Node* _node;

		public:
			// iterator traits
			using difference_type = std::ptrdiff_t;
			using value_type = const Instruction;
			using pointer = const Instruction*;
			using reference = const Instruction&;
			using iterator_category = std::bidirectional_iterator_tag;

		public:
			const_iterator(const Node* node) : _node{ node } {}
			const_iterator(const const_iterator&) = default;
			const_iterator(const_iterator&&) noexcept = default;
			~const_iterator() = default;

			const_iterator& operator= (const const_iterator&) = default;
			const_iterator& operator= (const_iterator&&) noexcept = default;

			const_iterator(const iterator& it);
			const_iterator(iterator&& it) noexcept;

			const_iterator& operator= (const iterator& right);
			const_iterator& operator= (iterator&& right) noexcept;

			bool operator== (const const_iterator& right) const { return _node == right._node; }
			bool operator!= (const const_iterator& right) const { return _node == right._node; }

			bool operator== (const iterator& right) const;
			bool operator!= (const iterator& right) const;

			const_iterator& operator++ () { return _node = _node->next, *this; }
			const_iterator operator++ (int) { const_iterator it = *this; return _node = _node->next, it; }

			const_iterator& operator-- () { return _node = _node->prev, *this; }
			const_iterator operator-- (int) { const_iterator it = *this; return _node = _node->prev, it; }

			reference operator* () const { return _node->instruction; }

			pointer operator-> () const { return &_node->instruction; }

			friend class iterator;
			friend class InstructionList;
		};

		class iterator
		{
		private:
			Node* _node;

		public:
			// iterator traits
			using difference_type = std::ptrdiff_t;
			using value_type = Instruction;
			using pointer = Instruction*;
			using reference = Instruction&;
			using iterator_category = std::bidirectional_iterator_tag;

		public:
			iterator(Node* node) : _node{ node } {}
			iterator(const iterator&) = default;
			iterator(iterator&&) noexcept = default;
			~iterator() = default;

			iterator& operator= (const iterator&) = default;
			iterator& operator= (iterator&&) noexcept = default;

			bool operator== (const iterator& right) const { return _node == right._node; }
			bool operator!= (const iterator& right) const { return _node == right._node; }

			bool operator== (const const_iterator& right) const;
			bool operator!= (const const_iterator& right) const;

			iterator& operator++ () { return _node = _node->next, *this; }
			iterator operator++ (int) { iterator it = *this; return _node = _node->next, it; }

			iterator& operator-- () { return _node = _node->prev, *this; }
			iterator operator-- (int) { iterator it = *this; return _node = _node->prev, it; }

			reference operator* () const { return _node->instruction; }

			pointer operator-> () const { return &_node->instruction; }

			friend class const_iterator;
			friend class InstructionList;
		};

		inline iterator begin() { return _ghost.next ? _ghost.next : &_ghost; }
		inline const_iterator begin() const { return _ghost.next ? _ghost.next : &_ghost; }
		inline const_iterator cbegin() const { return _ghost.next ? _ghost.next : &_ghost; }

		inline iterator end() { return &_ghost; }
		inline const_iterator end() const { return &_ghost; }
		inline const_iterator cend() const { return &_ghost; }
	};
}

std::ostream& operator<< (std::ostream& left, const kpl::inst::Instruction& right);
