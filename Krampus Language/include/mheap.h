#pragma once

#include "common.h"

namespace kpl
{
	class MemoryHeap;

	class MemoryBlock
	{
	private:
		Size _size;
		MemoryBlock* _next;
		MemoryBlock* _prev;
		unsigned int _refs;
		void (*_destructor)(void*);

	public:
#pragma warning(push)
#pragma warning(disable : 26495)
		inline MemoryBlock() {};
#pragma warning(pop)

		MemoryBlock(const MemoryBlock&) = delete;
		MemoryBlock(MemoryBlock&&) noexcept = delete;
		~MemoryBlock() = default;

		MemoryBlock& operator= (const MemoryBlock&) = delete;
		MemoryBlock& operator= (MemoryBlock&) noexcept = delete;

		friend class MemoryHeap;

	public:
		template<typename _Ty = void>
		inline _Ty* block_data() { return reinterpret_cast<_Ty*>(this + 1); }

		inline void increase_reference_count() { if (_refs < static_cast<decltype(_refs)>(-1)) ++_refs; }
		inline void decrease_reference_count() { if (_refs > 0) --_refs; }
	};

	

	class KPLVirtualObject
	{
	private:
		MemoryBlock* _block = nullptr;

	public:
		KPLVirtualObject() = default;
		KPLVirtualObject(const KPLVirtualObject&) = delete;
		KPLVirtualObject(KPLVirtualObject&&) noexcept = delete;
		
		KPLVirtualObject& operator= (const KPLVirtualObject&) = delete;
		KPLVirtualObject& operator= (KPLVirtualObject&&) = delete;

		inline void increase_reference_count() { if (_block) _block->increase_reference_count(); }
		inline void decrease_reference_count() { if (_block) _block->decrease_reference_count(); }

		friend class MemoryHeap;

	private:
		template<typename _Ty>
		requires std::derived_from<_Ty, KPLVirtualObject>
		static inline _Ty* attach(_Ty* obj, MemoryBlock* block) { return obj->_block = block, obj; }
	};



	class MemoryHeap
	{
	private:
		MemoryBlock* _front;
		MemoryBlock* _back;

	public:
		MemoryHeap(const MemoryHeap&) = delete;
		MemoryHeap(MemoryHeap&&) noexcept = delete;

		MemoryHeap& operator= (const MemoryHeap&) = delete;
		MemoryHeap& operator= (MemoryHeap&) noexcept = delete;

	public:
		MemoryHeap();
		~MemoryHeap();

		MemoryBlock* malloc(Size size, void (*destructor)(void*) = nullptr);

		void free(MemoryBlock* block);

		void garbage_collector();

	private:
		void delete_block(MemoryBlock* block);

	public:
		type::String* make_string(const char* str = nullptr);
		type::String* make_string(const std::string& str);

		type::Array* make_array(Size length);
		type::Array* make_array(const Value* array, Size length);
		type::Array* make_array(const std::vector<Value>& vector);
		type::Array* make_array(std::vector<Value>&& vector);
		type::Array* make_array(const std::initializer_list<Value>& args);

		type::List* make_list();

		type::Object* make_object();
	};
}
