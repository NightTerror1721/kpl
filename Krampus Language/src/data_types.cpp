#include "data_types.h"
#include "mheap.h"

#include <string>
#include <sstream>

namespace kpl::type::literal
{
	Value* const Null = const_cast<type::Null*>(&Null::instance);
	Value* const True = const_cast<type::Boolean*>(&Boolean::true_instance);
	Value* const False = const_cast<type::Boolean*>(&Boolean::false_instance);
}

namespace kpl
{
	void Value::increase_refcount() const
	{
		switch (_type)
		{
			case DataType::Null:
			case DataType::Boolean:
				return;
		}

		if (_header.refs < static_cast<decltype(_header.refs)>(-1))
			++_header.refs;
	}

	void Value::decrease_refcount() const
	{
		switch (_type)
		{
			case DataType::Null:
			case DataType::Boolean:
				return;
		}

		if (_header.refs > 0)
			--_header.refs;
	}

	std::string Value::to_string()
	{
		switch (_type)
		{
			case DataType::Null:
				return "null";

			case DataType::Integer:
				return std::to_string(reinterpret_cast<type::Integer*>(this)->_value);

			case DataType::Float:
				return std::to_string(reinterpret_cast<type::Float*>(this)->_value);

			case DataType::Boolean:
				return reinterpret_cast<type::Boolean*>(this)->_state ? "true" : "false";

			case DataType::Array: {
				type::Array* array = reinterpret_cast<type::Array*>(this);
				if (array->_size == 0)
					return "[]";

				std::stringstream ss;
				ss << "[";

				for (Offset i = 0; i < array->_size; i++)
				{
					if (i > 0)
						ss << ", ";
					ss << array->_array[i]->to_string();
				}

				return ss << "]", ss.str();
			}

			case DataType::List: {
				/*type::List* array = reinterpret_cast<type::List*>(this);
				if (array->_size == 0)
					return "[]";

				std::stringstream ss;
				ss << "[";

				for (Offset i = 0; i < array->_size; i++)
				{
					if (i > 0)
						ss << ", ";
					ss << array->_array[i]->to_cppstring();
				}

				return ss << "]", ss.str();*/
			}
		}

		return "";
	}
}

namespace kpl::type { const Null Null::instance; }

namespace kpl::type
{
	const Boolean Boolean::true_instance{ true };
	const Boolean Boolean::false_instance{ false };
}

namespace kpl::type
{
	String::String(const char* str_addr, Size str_size) :
		Value{ DataType::String },
		_str{ str_addr },
		_size{ str_size },
		_hashcode{ 0 }
	{}

	Int64 String::hashcode() const
	{
		if (_hashcode == 0 && _str)
		{
			std::string str{ _str, _size };
			auto hash = std::hash<std::string>();
			_hashcode = hash(str);
		}
		return _hashcode;
	}

	String* String::copy(MemoryHeap* heap) const
	{
		if (!_str)
			return heap->make_string();
		return heap->make_string(_str, _size);
	}
}

namespace kpl::type
{
	Array::Array(ValueReference* array_addr, Size array_size) :
		Value{ DataType::Array },
		_array{ array_addr },
		_size{ array_size }
	{}
}

namespace kpl::type
{
	List::Node* List::make_node(Value* value, Node* next, Node* prev)
	{
		Node* n = reinterpret_cast<Node*>(_heap->malloc(sizeof(Node)));
		return __KPL_CONSTRUCT(n, Node, value, next, prev);
	}

	void List::delete_node(Node* node) { node->value = nullptr, _heap->free(node); }

	List::Node* List::find_node(Offset index) const
	{
		Node* node;

		if(index > _size / 2)
			for (node = _back, index -= _size - 1; node && index < 0; node = node->prev, ++index);
		else for (node = _front; node && index > 0; node = node->next, --index);

		return node;
	}

	void List::push_back(Value* value)
	{
		if (!_back)
			_front = _back = make_node(value);
		else
		{
			Node* node = make_node(value, nullptr, _back);
			_back->next = node;
			_back = node;
		}
		++_size;
	}

	void List::push_front(Value* value)
	{
		if(!_front)
			_front = _back = make_node(value);
		else
		{
			Node* node = make_node(value, _front);
			_front->prev = node;
			_front = node;
		}
		++_size;
	}

	void List::insert(Offset index, Value* value)
	{
		if (index == 0)
			push_front(value);
		else if (index >= _size)
			push_back(value);

		Node* prev = find_node(index);
		Node* node = make_node(value, prev->next, prev);
		node->next->prev = node;
		prev->next = node;

		++_size;
	}

	void List::set(Offset index, Value* value)
	{
		Node* node = find_node(index);
		if (node)
			node->value = value;
	}

	Value* List::get(Offset index) const
	{
		Node* node = find_node(index);
		return node ? node->safe_value() : literal::Null;
	}

	Value* List::erase(Offset index)
	{
		if (!_front)
			return literal::Null;

		Value* value;
		if (_front == _back)
		{
			value = _front->safe_value();
			delete_node(_front);
			_front = _back = nullptr;
		}
		else if (index == 0)
		{
			Node* node = _front;
			_front = node->next;
			_front->prev = nullptr;

			value = node->safe_value();
			delete_node(node);
		}
		else if (index >= _size - 1)
		{
			Node* node = _back;
			_back = node->prev;
			_back->next = nullptr;

			value = node->safe_value();
			delete_node(node);
		}
		else
		{
			Node* node = find_node(index);
			node->prev->next = node->next;
			node->next->prev = node->prev;

			value = node->safe_value();
			delete_node(node);
		}

		return --_size, value;
	}

	void List::clear()
	{
		if (_front)
		{
			for (Node* node = _front, *next; node; node = next)
			{
				next = node->next;
				delete_node(node);
			}

			_front = _back = nullptr;
			_size = 0;
		}
	}
}

namespace kpl::type
{
	Object::Entry::Entry(MemoryHeap* heap, const Value* name, Value* value) :
		Value{ DataType::Internal },
		name{ name->type() == DataType::String
				? reinterpret_cast<const String*>(name)->copy(heap)
				: heap->make_string(value->to_string())
		},
		value{ value }
	{
		increase_refcount();
		name->increase_refcount();
	}

	Object::Entry* Object::make_entry(const Value* name, Value* value)
	{
		Entry* entry = reinterpret_cast<Entry*>(_heap->malloc(sizeof(Entry)));
		return __KPL_CONSTRUCT(entry, Entry, _heap, name, value);
	}

	void Object::delete_entry(Entry* entry)
	{
		entry->value = nullptr;
		_heap->free(entry->name);
		_heap->free(entry);
	}

	Object::Table::Table(MemoryHeap* heap, Entry** entries, Size size) :
		Value{ DataType::Internal },
		heap{ heap },
		entries{ entries },
		size{ size }
	{
		increase_refcount();
	}

	Object::Table* Object::make_table(Size size)
	{
		Table* table = reinterpret_cast<Table*>(_heap->malloc(sizeof(Table) + sizeof(Entry*) * size));
		Entry* entries = reinterpret_cast<Entry*>(table + 1);
		return __KPL_CONSTRUCT(table, Table, _heap, &entries, size);
	}

	void Object::delete_table(Table* table)
	{
		for (Offset i = 0; i < table->size; ++i)
			if(table->entries[i])
				delete_entry(table->entries[i]);
		_heap->free(table);
	}
}
