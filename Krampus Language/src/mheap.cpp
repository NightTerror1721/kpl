#include "mheap.h"

namespace kpl
{
	MemoryHeap::MemoryHeap(Size max_capacity, Size min_capacity) :
		_head{ nullptr },
		_tail{ nullptr },
		_offset{ 0 },
		_capacity{ 0 },
		_minCapacity{ std::max(default_min_capacity, std::min(min_capacity, max_capacity)) },
		_maxCapacity{ std::min(_minCapacity, std::max(min_capacity, max_capacity)) },
		_data{ nullptr }
	{
		ensure_capacity(_minCapacity);
	}

	MemoryHeap::~MemoryHeap()
	{
		if (_data)
		{
			utils::free(_data);
			_data = nullptr;
		}

		_head = _tail = nullptr;
		_offset = 0;
		_capacity = 0;
	}

	void MemoryHeap::ensure_capacity(Size capacity)
	{
		capacity = utils::clamp(capacity, _minCapacity, _maxCapacity);

		if (!_data)
		{
			if (capacity == 0)
				return;

			_data = utils::malloc(capacity);
			_capacity = capacity;
		}
		else
		{
			if (capacity == 0)
			{
				utils::destroy(*this);
			}
			else
			{
				if (capacity == _capacity)
					return;

				std::ptrdiff_t head_off = utils::addr_diff(_head, _data);
				std::ptrdiff_t tail_off = utils::addr_diff(_tail, _data);

				void* newdata = utils::malloc(capacity);
				if (capacity > _capacity)
					std::memcpy(newdata, _data, _capacity);
				else std::memcpy(newdata, _data, capacity);

				utils::free(_data);
				_data = newdata;
				_capacity = capacity;
				_head = utils::addr_diff_add<Value>(_data, head_off);
				_tail = utils::addr_diff_add<Value>(_data, tail_off);

				if (_capacity > _offset)
					_offset = _capacity;
			}
		}
	}

	bool MemoryHeap::check_free_space(Size size)
	{
		if (_offset + size <= _capacity)
			return true;

		garbage_collector();
		if (_offset + size <= _capacity)
			return true;

		if (_capacity > max_dup_size)
			ensure_capacity(_capacity + max_dup_size);
		else ensure_capacity(_capacity * 2);

		return _offset + size <= _capacity;
	}

	Value* MemoryHeap::malloc(Size size)
	{
		if (!check_free_space(size))
			return nullptr;

		Value* value = get_value(_offset);
		value->_header.size = size;
		value->_header.refs = 0;
		value->_header.next = utils::invalid_offset;
		value->_header.prev = value_offset(_tail);

		if (!_head)
			_head = _tail = value;
		else
		{
			_tail->_header.next = value_offset(value);
			_tail = value;
		}

		_offset += size;

		return value;
	}

	void MemoryHeap::free(Value* value)
	{
		Value* next = next_value(value), *prev = prev_value(value);

		destroy_object(value);
		
		if (value == _head)
		{
			if (next)
			{
				next->_header.prev = utils::invalid_offset;
				_head = next;
			}
			else _head = _tail = nullptr;
		}
		else if (value == _tail)
		{
			if (prev)
			{
				prev->_header.next = utils::invalid_offset;
				_tail = prev;
			}
			else _head = _tail = nullptr;
		}
		else
		{
			prev->_header.next = value_offset(next);
			next->_header.prev = value_offset(prev);
		}
	}

	void MemoryHeap::destroy_object(Value* const obj)
	{
		/*switch (obj->type())
		{
		}*/
	}

	void MemoryHeap::garbage_collector()
	{
		if (!_head)
			return;

		Offset offset = 0;
		Value* value = _head;
		Value* head = nullptr;
		Value* tail = nullptr;
		Value* next_value_pos = value;
		bool first = true;

		while (value)
		{
			if (value->_header.refs > 0)
			{
				if (value < next_value_pos)
				{
					std::memcpy(next_value_pos, value, value->_header.size);
					value = next_value_pos;
				}
				if (first)
				{
					first = false;
					head = value;
				}

				offset += value->_header.size;
				next_value_pos = utils::at_offset<Value>(next_value_pos, value->_header.size);
				tail = value;
				value = next_value(value);
			}
			else
			{
				Value* to_delete = value;
				value = next_value(value);
				free(to_delete);
			}
		}

		_head = head;
		_tail = tail;
		_offset = offset;
	}


	type::String* MemoryHeap::make_string(const char* cstr, Size length)
	{
		type::String* obj;
		if (length == 0)
		{
			obj = construct_instance<type::String>();
		}
		else
		{
			const char* str;
			obj = make_instance<type::String>(length, &str);
			__KPL_CONSTRUCT(obj, type::String, str, length);
			std::memcpy(const_cast<char*>(str), cstr, length);
		}
		return obj;
	}

	type::Array* MemoryHeap::make_array(Size length)
	{
		type::Array* obj;
		if (length == 0)
		{
			obj = construct_instance<type::Array>();
		}
		else
		{
			ValueReference* array;
			obj = make_instance<type::Array>(sizeof(ValueReference) * length, &array);
			for (Offset i = 0; i < length; i++)
				__KPL_CONSTRUCT(array + i, ValueReference);
			__KPL_CONSTRUCT(obj, type::Array, array, length);
		}
		return obj;
	}
}
