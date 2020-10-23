#include "mheap.h"
#include "data_types.h"


namespace kpl
{
	MemoryHeap::MemoryHeap() :
		_front{ nullptr },
		_back{ nullptr }
	{}

	MemoryHeap::~MemoryHeap()
	{
		for (MemoryBlock* block = _front, *next; block; block = next)
		{
			next = block->_next;
			delete_block(block);
		}
	}

	MemoryBlock* MemoryHeap::malloc(Size size, void (*destructor)(void*))
	{
		MemoryBlock* block = utils::malloc<MemoryBlock>(sizeof(MemoryBlock) + size);
		block->_size = sizeof(MemoryBlock) + size;
		block->_next = _front;
		block->_prev = nullptr;
		block->_refs = 0;
		block->_destructor = destructor;

		if (!_front)
			_front = _back = block;
		else
		{
			_front->_prev = block;
			_front = block;
		}

		return block;
	}

	void MemoryHeap::free(MemoryBlock* block)
	{
		if (!_front)
			return;

		if (_front == _back)
		{
			_front = _back = nullptr;
		}
		else if (_front == block)
		{
			block->_next->_prev = nullptr;
			_front = block->_next;
		}
		else if (_back == block)
		{
			block->_prev->_next = nullptr;
			_back = block->_prev;
		}
		else
		{
			block->_next->_prev = block->_prev;
			block->_prev->_next = block->_next;
		}

		delete_block(block);
	}

	void MemoryHeap::garbage_collector()
	{
		for (MemoryBlock* block = _front, *next; block; block = next)
		{
			next = block->_next;
			if (block->_refs == 0)
				free(block + 1);
		}
	}

	void MemoryHeap::delete_block(MemoryBlock* block)
	{
		if (block->_destructor)
			block->_destructor(block + 1);
		utils::free(block);
	}



#define instanceof(_Type, _Destructor) malloc(sizeof(_Type), (_Destructor))
#define construct(_Block, _Type, ...) KPLVirtualObject::attach(__KPL_CONSTRUCT(reinterpret_cast<_Type*>(_Block + 1), _Type, __VA_ARGS__), (_Block))

	type::String* MemoryHeap::make_string(const char* str)
	{
		MemoryBlock* block = instanceof(type::String, &type::String::_mheap_delete);
		if(str)
			return construct(block, type::String, str);
		else return construct(block, type::String);
	}
	type::String* MemoryHeap::make_string(const std::string& str)
	{
		MemoryBlock* block = instanceof(type::String , &type::String::_mheap_delete);
		return construct(block, type::String, str);
	}



	type::Array* MemoryHeap::make_array(Size length)
	{
		MemoryBlock* block = instanceof(type::Array, &type::Array::_mheap_delete);
		return construct(block, type::Array, length);
	}
	type::Array* MemoryHeap::make_array(const Value* array, Size length)
	{
		MemoryBlock* block = instanceof(type::Array, &type::Array::_mheap_delete);
		return construct(block, type::Array, array, length);
	}
	type::Array* MemoryHeap::make_array(const std::vector<Value>& vector)
	{
		MemoryBlock* block = instanceof(type::Array, &type::Array::_mheap_delete);
		return construct(block, type::Array, vector);
	}
	type::Array* MemoryHeap::make_array(std::vector<Value>&& vector)
	{
		MemoryBlock* block = instanceof(type::Array, &type::Array::_mheap_delete);
		return construct(block, type::Array, std::move(vector));
	}
	type::Array* MemoryHeap::make_array(const std::initializer_list<Value>& args)
	{
		MemoryBlock* block = instanceof(type::Array, &type::Array::_mheap_delete);
		return construct(block, type::Array, args);
	}



	type::List* MemoryHeap::make_list()
	{
		MemoryBlock* block = instanceof(type::List, &type::List::_mheap_delete);
		return construct(block, type::List);
	}

	type::List* MemoryHeap::make_list(const type::List& src)
	{
		MemoryBlock* block = instanceof(type::List, &type::List::_mheap_delete);
		return construct(block, type::List, src);
	}




	type::Object* MemoryHeap::make_object()
	{
		MemoryBlock* block = instanceof(type::Object, &type::Object::_mheap_delete);
		return construct(block, type::Object, type::literal::Null);
	}

	type::Object* MemoryHeap::make_object(const Value& class_)
	{
		MemoryBlock* block = instanceof(type::Object, &type::Object::_mheap_delete);
		return construct(block, type::Object, class_);
	}
}
