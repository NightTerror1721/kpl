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
				_head = utils::addr_diff_add<Header>(_data, head_off);
				_tail = utils::addr_diff_add<Header>(_data, tail_off);

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
		Size realsize = size + header_size;

		if (!check_free_space(realsize))
			return nullptr;

		Header* header = get_header(_offset);
		header->size = size;
		header->refs = 0;
		header->next = utils::invalid_offset;
		header->prev = header_offset(_tail);

		if (!_head)
			_head = _tail = header;
		else
		{
			_tail->next = header_offset(header);
			_tail = header;
		}

		_offset += realsize;

		return reinterpret_cast<Value*>(header + 1);
	}

	void MemoryHeap::free(Value* ptr)
	{
		Header* header = get_header(ptr);
		Header* next = next_header(header), *prev = prev_header(header);
		
		if (header == _head)
		{
			if (next)
			{
				next->prev = utils::invalid_offset;
				_head = next;
			}
			else _head = _tail = nullptr;
		}
		else if (header == _tail)
		{
			if (prev)
			{
				prev->next = utils::invalid_offset;
				_tail = prev;
			}
			else _head = _tail = nullptr;
		}
		else
		{
			prev->next = header_offset(next);
			next->prev = header_offset(prev);
		}
	}

	void MemoryHeap::garbage_collector()
	{
		if (!_head)
			return;

		Offset offset = 0;
		Header* header = _head;
		Header* head = nullptr;
		Header* tail = nullptr;
		Header* next_header_pos = header;
		bool first = true;

		while (header)
		{
			if (header->refs > 0)
			{
				if (header < next_header_pos)
				{
					std::memcpy(next_header_pos, header, header_size + header->size);
					header = next_header_pos;
				}
				if (first)
				{
					first = false;
					head = header;
				}

				offset = header_size + header->size;
				next_header_pos = utils::at_offset<Header>(next_header_pos, header_size + header->size);
				tail = header;
				header = next_header(header);
			}
			else
			{
				Header* to_delete = header;
				header = next_header(header);
				free(reinterpret_cast<Value*>(to_delete + 1));
			}
		}

		_head = head;
		_tail = tail;
		_offset = offset;
	}

	void MemoryHeap::increase_reference(Value* const ptr)
	{
		switch (ptr->type())
		{
			case DataType::Null:
			case DataType::Boolean:
				return;
		}

		Header* header = get_header(ptr);
		if (header->refs < static_cast<decltype(header->refs)>(-1))
			++header->refs;
	}

	void MemoryHeap::decrease_reference(Value* const ptr)
	{
		switch (ptr->type())
		{
			case DataType::Null:
			case DataType::Boolean:
				return;
		}

		Header* header = get_header(ptr);
		if (header->refs > 0)
			--header->refs;
	}
}
