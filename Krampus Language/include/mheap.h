#pragma once

#include "common.h"
#include "data_types.h"

namespace kpl
{
	struct MemoryHeapHeader
	{
		Size size;
		Offset next;
		Offset prev;
		UInt32 refs;
	};

	class MemoryHeap
	{
	private:
		using Header = MemoryHeapHeader;

		static constexpr Size max_dup_size = 1024 * 1024 * 512;
		static constexpr Size header_size = sizeof(Header);

	public:
		static constexpr Size default_capacity = 1024 * 32;
		static constexpr Size default_min_capacity = default_capacity;
		static constexpr Size default_max_capacity = 1024 * 1024 * 8;

	private:
		Header* _head;
		Header* _tail;
		Offset _offset;
		Size _capacity;
		Size _minCapacity;
		Size _maxCapacity;
		void* _data;

	public:
		MemoryHeap(Size max_capacity = default_max_capacity, Size min_capacity = default_min_capacity);
		MemoryHeap(const MemoryHeap&) = delete;
		MemoryHeap(MemoryHeap&&) noexcept = default;
		~MemoryHeap();

		MemoryHeap& operator= (const MemoryHeap&) = delete;
		MemoryHeap& operator= (MemoryHeap&&) noexcept = default;

		Value* malloc(Size size);

		void free(Value* ptr);

		void garbage_collector();

		static inline MemoryHeapHeader* get_header(void* ptr) { return reinterpret_cast<Header*>(ptr) - 1; }
		static inline const MemoryHeapHeader* get_header(const void* ptr) { return reinterpret_cast<const Header*>(ptr) - 1; }

		static void increase_reference(Value* const ptr);
		static void decrease_reference(Value* const ptr);

	private:
		void ensure_capacity(Size capacity);
		bool check_free_space(Size size);

		inline Header* get_header(Offset offset)
		{
			return offset == utils::invalid_offset ? nullptr : utils::at_offset<Header>(_data, offset);
		}

		inline Offset header_offset(Header* header)
		{
			return !header ? utils::invalid_offset : utils::offset_of(_data, header);
		}

		inline Header* next_header(Header* header)
		{
			return header->next == utils::invalid_offset ? nullptr : utils::at_offset<Header>(_data, header->next);
		}
		inline Header* prev_header(Header* header)
		{
			return header->prev == utils::invalid_offset ? nullptr : utils::at_offset<Header>(_data, header->prev);
		}

	public:
		static inline type::Null* literal_null() { const_cast<type::Null*>(&type::constant::null); }

		static inline type::Boolean* literal_boolean(bool state)
		{
			return const_cast<type::Boolean*>(state ? &type::constant::True : &type::constant::False);
		}

	public:
		inline type::Null* make_null() { return const_cast<type::Null*>(&type::constant::null); }

		inline type::Boolean* make_boolean(bool state)
		{
			return const_cast<type::Boolean*>(state ? &type::constant::True : &type::constant::False);
		}

		template<std::integral _Ty>
		inline type::Integer* make_integer(_Ty value)
		{
			type::Integer* obj = reinterpret_cast<type::Integer*>(malloc(sizeof(type::Integer)));

			if constexpr (std::same_as<_Ty, Int64>)
				return __KPL_CONSTRUCT(obj, type::Integer, value), obj;
			else return __KPL_CONSTRUCT(obj, type::Integer, static_cast<Int64>(value)), obj;
		}
		inline type::Integer* make_integer() { return make_integer<Int64>(0); }

		inline type::Float* make_float(double value = 0)
		{
			type::Float* obj = reinterpret_cast<type::Float*>(malloc(sizeof(type::Float)));
			return __KPL_CONSTRUCT(obj, type::Float, value), obj;
		}
		inline type::Float* make_float(float value) { return make_float(static_cast<double>(value)); }
	};
}

namespace kpl
{
	class ValueReference
	{
	private:
		Value* _value;

	public:
		inline ValueReference() : _value{ nullptr } {}
		inline ValueReference(decltype(nullptr)) : _value{} {}
		inline ValueReference(Value* value) : _value{ value } { if (value) MemoryHeap::increase_reference(value); }
		inline ValueReference(const ValueReference& ref) : _value{ ref._value } { if (_value) MemoryHeap::increase_reference(_value); }
		inline ValueReference(ValueReference&& ref) noexcept : _value{ ref._value } { ref._value = nullptr; }
		inline ~ValueReference() { if (_value) MemoryHeap::decrease_reference(_value); _value = nullptr; }

		inline ValueReference& operator= (Value* value)
		{
			if (_value)
				MemoryHeap::decrease_reference(_value);
			_value = value;
			if (value)
				MemoryHeap::increase_reference(value);
			return *this;
		}
		inline ValueReference& operator= (const ValueReference& right)
		{
			if (_value)
				MemoryHeap::decrease_reference(_value);
			_value = right._value;
			if (_value)
				MemoryHeap::increase_reference(_value);
			return *this;
		}
		inline ValueReference& operator= (ValueReference&& right) noexcept
		{
			if (_value)
				MemoryHeap::decrease_reference(_value);
			_value = right._value;
			right._value = nullptr;
			return *this;
		}
		inline ValueReference& operator= (decltype(nullptr))
		{
			if (_value)
			{
				MemoryHeap::decrease_reference(_value);
				_value = nullptr;
			}
			return *this;
		}

		inline operator Value* () const { return _value; }
		inline operator const Value* () const { return _value; }

		inline Value* operator-> () { return _value; }
		inline const Value* operator-> () const { return _value; }

		inline operator bool() const { return _value; }
		inline bool operator! () const { return !_value; }
	};
}
