#include "data_types.h"
#include "mheap.h"
#include "chunk.h"

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

	std::string Value::to_string() const
	{
		switch (_type)
		{
			case DataType::Null:
				return "null";

			case DataType::Integer:
				return std::to_string(reinterpret_cast<const type::Integer*>(this)->_value);

			case DataType::Float:
				return std::to_string(reinterpret_cast<const type::Float*>(this)->_value);

			case DataType::Boolean:
				return reinterpret_cast<const type::Boolean*>(this)->_state ? "true" : "false";

			case DataType::Array: {
				const type::Array* array = reinterpret_cast<const type::Array*>(this);
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
				const type::List* list = reinterpret_cast<const type::List*>(this);
				if (list->_size == 0)
					return "[]";

				std::stringstream ss;
				ss << "[";

				for (type::List::Node* node = list->_front; node; node = node->next)
				{
					if (node != list->_front)
						ss << ", ";
					ss << node->value->to_string();
				}

				return ss << "]", ss.str();
			}

			case DataType::Object: {
				const type::Object* object = reinterpret_cast<const type::Object*>(this);
				if (object->_size == 0)
					return "{}";

				std::stringstream ss;
				ss << "{";

				for (Offset i = 0; i < object->_table->size; ++i)
				{
					for (type::Object::Entry* entry = object->_table->entries[i]; entry; entry = entry->next)
					{
						if (entry != object->_table->entries[i])
							ss << ", ";
						ss << entry->name->to_string() << " = " << entry->value->to_string();
					}
				}

				return ss << "}", ss.str();
			}
		}

		return "";
	}

	Value* Value::get_property(const Value* name)
	{
		return type::literal::Null;
	}

	void Value::set_property(const Value* name, Value* value)
	{

	}

	void Value::del_property(const Value* name)
	{

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

	Int64 String::hashcode(const std::string& str)
	{
		auto hash = std::hash<std::string>();
		return hash(str);
	}

	Int64 String::hashcode() const
	{
		if (_hashcode == 0 && _str)
			_hashcode = hashcode(_str, _size);
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
	Object::Entry::Entry(MemoryHeap* heap, const std::string& name, Value* value, Entry* next) :
		Value{ DataType::Internal },
		name{ heap->make_string(name) },
		value{ value },
		next{ next }
	{
		increase_refcount();
		this->name->increase_refcount();
	}

	Object::Entry* Object::make_entry(const std::string& name, Value* value, Entry* next)
	{
		Entry* entry = reinterpret_cast<Entry*>(_heap->malloc(sizeof(Entry)));
		return __KPL_CONSTRUCT(entry, Entry, _heap, name, value, next);
	}

	void Object::delete_entry(Entry* entry)
	{
		entry->value = nullptr;
		if(entry->name)
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

	Value* Object::Table::insert(Object* object, const Value* v_name, Value* value)
	{
		if (v_name == nullptr)
			v_name = literal::Null;

		if (v_name->type() == DataType::String)
		{
			const String* name = reinterpret_cast<const String*>(v_name);
			Offset index = hash(name);

			if (!entries[index])
			{
				entries[index] = object->make_entry(v_name->to_string(), value);
				++object->_size;
			}
			else
			{
				const String* name = reinterpret_cast<const String*>(v_name);
				for (Entry* entry = entries[index]; entry; entry = entry->next)
					if (entry->name->compare(name) == 0)
					{
						Value* old = entry->value;
						entry->value = value;
						return old;
					}

				Entry* entry = object->make_entry(name->to_string(), value);

				entry->next = entries[index];
				entries[index] = entry;
				++object->_size;
			}
		}
		else
		{
			insert(object, v_name->to_string(), value);
		}

		return literal::Null;
	}

	Value* Object::Table::insert(Object* object, const std::string& name, Value* value)
	{
		Offset index = hash(name);

		if (!entries[index])
		{
			entries[index] = object->make_entry(name, value);
			++object->_size;
		}
		else
		{
			for (Entry* entry = entries[index]; entry; entry = entry->next)
				if (entry->name->compare(name) == 0)
				{
					Value* old = entry->value;
					entry->value = value;
					return old;
				}

			Entry* entry = object->make_entry(name, value);

			entry->next = entries[index];
			entries[index] = entry;
			++object->_size;
		}

		return literal::Null;
	}

	Value* Object::Table::insert(Object* object, const char* name, Value* value)
	{
		Offset index = hash(name);

		if (!entries[index])
		{
			entries[index] = object->make_entry(name, value);
			++object->_size;
		}
		else
		{
			for (Entry* entry = entries[index]; entry; entry = entry->next)
				if (entry->name->compare(name) == 0)
				{
					Value* old = entry->value;
					entry->value = value;
					return old;
				}

			Entry* entry = object->make_entry(name, value);

			entry->next = entries[index];
			entries[index] = entry;
			++object->_size;
		}

		return literal::Null;
	}

	void Object::Table::move(Entry* entry)
	{
		Offset index = hash(entry->name);

		if (!entries[index])
			entries[index] = entry;
		else
		{
			entry->next = entries[index];
			entries[index] = entry;
		}
	}

	Object::Entry* Object::Table::find(const Value* v_name)
	{
		v_name = !v_name ? literal::Null : v_name;
		if (v_name->type() == DataType::String)
		{
			const String* name = reinterpret_cast<const String*>(v_name);
			Offset index = hash(name);

			for (Entry* entry = entries[index]; entry; entry = entry->next)
				if (entry->name->compare(name) == 0)
					return entry;
			return nullptr;
		}
		else
		{
			std::string name = v_name->to_string();
			Offset index = hash(name);

			for (Entry* entry = entries[index]; entry; entry = entry->next)
				if (entry->name->compare(name) == 0)
					return entry;
			return nullptr;
		}
	}

	Object::Entry* Object::Table::find(const char* name)
	{
		Offset index = hash(name);

		for (Entry* entry = entries[index]; entry; entry = entry->next)
			if (entry->name->compare(name) == 0)
				return entry;
		return nullptr;
	}

	Object::Entry* Object::Table::find(const std::string& name)
	{
		Offset index = hash(name);

		for (Entry* entry = entries[index]; entry; entry = entry->next)
			if (entry->name->compare(name) == 0)
				return entry;
		return nullptr;
	}

	void Object::Table::remove(Object* object, const Value* v_name)
	{
		Entry* entry, *prev = nullptr;
		Offset index;

		v_name = !v_name ? literal::Null : v_name;
		if (v_name->type() == DataType::String)
		{
			const String* name = reinterpret_cast<const String*>(v_name);
			index = hash(name);

			for (entry = entries[index]; entry; prev = entry, entry = entry->next)
				if (entry->name->compare(name) == 0)
					break;
		}
		else
		{
			std::string name = v_name->to_string();
			index = hash(name);

			for (entry = entries[index]; entry; prev = entry, entry = entry->next)
				if (entry->name->compare(name) == 0)
					break;
		}

		if (!entry)
			return;

		if (entry == entries[index])
			entries[index] = entry->next;
		else prev->next = entry->next;

		object->delete_entry(entry);
		--object->_size;
	}

	void Object::Table::remove(Object* object, const char* name)
	{
		Entry* entry, *prev = nullptr;
		Offset index = hash(name);

		for (entry = entries[index]; entry; prev = entry, entry = entry->next)
			if (entry->name->compare(name) == 0)
			{
				if (entry == entries[index])
					entries[index] = entry->next;
				else prev->next = entry->next;

				object->delete_entry(entry);
				--object->_size;

				return;
			}
	}

	void Object::Table::remove(Object* object, const std::string& name)
	{
		Entry* entry, *prev = nullptr;
		Offset index = hash(name);

		for (entry = entries[index]; entry; prev = entry, entry = entry->next)
			if (entry->name->compare(name) == 0)
			{
				if (entry == entries[index])
					entries[index] = entry->next;
				else prev->next = entry->next;

				object->delete_entry(entry);
				--object->_size;

				return;
			}
	}

	void Object::Table::delete_node(Object* object, Offset node_idx)
	{
		if (entries[node_idx])
		{
			for (Entry* entry = entries[node_idx], *next; entry; entry = next)
			{
				next = entry->next;
				object->delete_entry(entry);
				--object->_size;
			}
			entries[node_idx] = nullptr;
		}
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
				table->delete_node(this, i);
		_heap->free(table);
	}

	void Object::check_capacity()
	{
		if (!_table)
			_table = make_table(default_capacity);
		else
		{
			Size load_factor = static_cast<Size>(_table->size * 0.75);
			if (_size >= load_factor)
				re_hash();
		}
	}

	void Object::re_hash()
	{
		Table* newtable = make_table(_table->size * 2);
		for (Offset i = 0; i < _table->size; ++i)
		{
			for (Entry* entry = _table->entries[i], *next; entry; entry = next)
			{
				next = entry->next;
				newtable->move(entry);
			}
			_table->entries[i] = nullptr;
		}

		_heap->free(_table);
		_table = newtable;
	}



	Object::Object(MemoryHeap* heap, Value* class_) :
		Value{ DataType::Object },
		_heap{ heap },
		_table{ nullptr },
		_size{ 0 },
		_class{ class_ ? class_ : literal::Null }
	{}

	Object::~Object()
	{
		delete_table(_table);
	}

	void Object::clear()
	{
		for (Offset i = 0; i < _table->size; ++i)
			_table->delete_node(this, i);
	}
}

namespace kpl::type
{
	Function::Function(MemoryHeap* heap, Chunk* chunk, Value* locals, ValueReference* constants) :
		Value{ DataType::Callable },
		_heap{ heap },
		_chunk{ chunk },
		_locals{ locals },
		_constants{ constants }
	{}

	void Function::alloc_constant(Offset index)
	{
		_constants[index] = _chunk->constant(index).make_value(_heap);
	}
}
