#include "instruction.h"

namespace kpl::inst
{
	std::ostream& Instruction::dump(std::ostream& os) const
	{
		return os << "[a: " << a()
			<< "; b: " << b()
			<< "; c: " << c()
			<< "; kb: " << kb()
			<< "; kc: " << kc()
			<< "; bx: " << bx()
			<< "; sbx: " << sbx()
			<< "; ax: " << ax()
			<< "; sax: " << sax()
			<< "]";
	}

	Instruction& Instruction::b(int value)
	{
		if (value >= 0)
			_inst = (_inst & b_mask) | static_cast<Instruction>((value & 0xff) << 15);
		else
		{
			_inst = (_inst & b_mask) | static_cast<Instruction>(((-value - 1) & 0xff) << 15) | (0x1 << 14);
		}

		return *this;
	}

	Instruction& Instruction::c(int value)
	{
		if (value >= 0)
			_inst = (_inst & c_mask) | static_cast<Instruction>((value & 0xff) << 24);
		else
		{
			_inst = (_inst & c_mask) | static_cast<Instruction>(((-value - 1) & 0xff) << 24) | (0x1 << 23);
		}

		return *this;
	}

	Instruction& Instruction::sbx(int value)
	{
		if (value >= 0)
			_inst = (_inst & bx_mask) | static_cast<Instruction>((value & 0x1ffff) << 15);
		else
		{
			_inst = (_inst & bx_mask) | static_cast<Instruction>((value & 0x1ffff) << 15) | (0x1 << 14);
		}

		return *this;
	}

	Instruction& Instruction::sax(int value)
	{
		if (value >= 0)
			_inst = (_inst & ax_mask) | static_cast<Instruction>((value & 0x1ffffff) << 7);
		else
		{
			_inst = (_inst & ax_mask) | static_cast<Instruction>((value & 0x1ffffff) << 7) | (0x1 << 6);
		}

		return *this;
	}
}




namespace kpl::inst
{
	void InstructionList::_destroy()
	{
		for (Node* node = _front(), *next; node != &_ghost; node = next)
		{
			next = node->next;
			delete node;
		}

		_ghost.next = _ghost.prev = &_ghost;
	}

	InstructionList& InstructionList::_copy(const InstructionList& list, bool reset)
	{
		if (reset)
			_destroy();

		for (const Node* node = list._front(); node != &list._ghost; node = node->next)
		{
			Node* newnode = new Node{ *node };
			if (_ghost.next == &_ghost)
			{
				newnode->next = &_ghost;
				newnode->prev = &_ghost;

				_ghost.next = newnode;
				_ghost.prev = newnode;
			}
			else
			{
				newnode->next = &_ghost;
				newnode->prev = _ghost.prev;

				_ghost.prev->next = newnode;
				_ghost.prev = newnode;
			}
		}

		_size = list._size;
		return *this;
	}

	InstructionList& InstructionList::_move(InstructionList&& list, bool reset) noexcept
	{
		if (reset)
			_destroy();

		_ghost = list._ghost;
		_size = list._size;

		list._ghost = { .next = &list._ghost, .prev = &list._ghost };
		list._size = 0;

		return *this;
	}

	InstructionList::operator std::vector<Instruction>() const
	{
		std::vector<Instruction> v;
		v.reserve(_size);
		for (const Node* node = _front(); node != &_ghost; node = node->next)
			v.push_back(node->instruction);

		return v;
	}

	InstructionList::Location InstructionList::push_front(const Instruction& inst)
	{
		Node* node = new Node;
		node->instruction = inst;

		if (_front() == &_ghost)
		{
			node->next = &_ghost;
			node->prev = &_ghost;

			_ghost.next = node;
			_ghost.prev = node;
		}
		else
		{
			node->prev = &_ghost;
			node->next = _ghost.next;

			_ghost.next->prev = node;
			_ghost.next = node;
		}

		return ++_size, node;
	}

	InstructionList::Location InstructionList::push_back(const Instruction& inst)
	{
		Node* node = new Node;
		node->instruction = inst;

		if (_front() == &_ghost)
		{
			node->next = &_ghost;
			node->prev = &_ghost;

			_ghost.next = node;
			_ghost.prev = node;
		}
		else
		{
			node->next = &_ghost;
			node->prev = _ghost.prev;

			_ghost.prev->next = node;
			_ghost.prev = node;
		}

		return ++_size, node;
	}

	InstructionList::Location InstructionList::insert_before(Location loc, const Instruction& inst)
	{
		if (_front() == &_ghost || loc._node == _front())
			return push_front(inst);

		if (loc == end())
			return push_back(inst);

		Node* next = const_cast<Node*>(loc._node);
		Node* node = new Node;
		node->instruction = inst;

		node->next = next;
		node->prev = next->prev;

		next->prev->next = node;
		next->prev = node;

		return ++_size, node;
	}

	InstructionList::Location InstructionList::insert_after(Location loc, const Instruction& inst)
	{
		if (_back() == &_ghost || loc._node == _back())
			return push_back(inst);

		if (loc == end())
			return push_front(inst);

		Node* prev = const_cast<Node*>(loc._node);
		Node* node = new Node;
		node->instruction = inst;

		node->next = prev->next;
		node->prev = prev;

		prev->next->prev = node;
		prev->next = node;

		return ++_size, node;
	}

	InstructionList::Location InstructionList::replace(Location loc, const Instruction& inst)
	{
		if(loc != end())
			const_cast<Node*>(loc._node)->instruction = inst;
		return loc;
	}

	void InstructionList::swap(Location l_left, Location l_right)
	{
		if (l_left == end() || l_right == end())
			return;

		Node* left = const_cast<Node*>(l_left._node);
		Node* right = const_cast<Node*>(l_right._node);
		Instruction aux = left->instruction;

		left->instruction = right->instruction;
		right->instruction = aux;
	}

	InstructionList::Location InstructionList::erase(Location loc)
	{
		if (loc == end() || !_front())
			return end();

		if (_front() == _back())
		{
			if (loc._node != _front())
				return end();

			delete _front();
			_ghost.next = _ghost.prev = &_ghost;

			loc = end();
		}
		else if(loc._node == _front())
		{
			Node* node = loc._node;
			node->next->prev = &_ghost;
			_ghost.next = node->next;

			delete node;
			loc = _front();
		}
		else if (loc._node == _back())
		{
			Node* node = loc._node;
			node->prev->next = &_ghost;
			_ghost.prev = node->prev;

			delete node;
			loc = _back();
		}
		else
		{
			Node* node = loc._node;

			node->next->prev = node->prev;
			node->prev->next = node->next;

			loc = node->next;
			delete node;
		}

		return --_size, loc;
	}









	InstructionList::const_iterator::const_iterator(const iterator& it) : _node{ it._node } {}
	InstructionList::const_iterator::const_iterator(iterator&& it) noexcept : _node{ it._node } {}

	InstructionList::const_iterator& InstructionList::const_iterator::operator= (const iterator& right) { return _node = right._node, *this; }
	InstructionList::const_iterator& InstructionList::const_iterator::operator= (iterator&& right) noexcept { return _node = right._node, *this; }

	bool InstructionList::const_iterator::operator== (const iterator& right) const { return _node == right._node; }
	bool InstructionList::const_iterator::operator!= (const iterator& right) const { return _node != right._node; }

	bool InstructionList::iterator::operator== (const const_iterator& right) const { return _node == right._node; }
	bool InstructionList::iterator::operator!= (const const_iterator& right) const { return _node != right._node; }
}

std::ostream& operator<< (std::ostream& left, const kpl::inst::Instruction& right)
{
	return right.dump(left);
}
