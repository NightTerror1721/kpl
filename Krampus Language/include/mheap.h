#pragma once

#include "common.h"
#include "data_types.h"

namespace kpl
{
	class MemoryHeap
	{
	private:
		static constexpr Size max_dup_size = 1024 * 1024 * 512;

	public:
		static constexpr Size default_capacity = 1024 * 32;
		static constexpr Size default_min_capacity = default_capacity;
		static constexpr Size default_max_capacity = 1024 * 1024 * 8;

	private:
		Value* _head;
		Value* _tail;
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

	private:
		void ensure_capacity(Size capacity);
		bool check_free_space(Size size);

		void destroy_object(Value* const obj);

		inline Value* get_value(Offset offset)
		{
			return offset == utils::invalid_offset ? nullptr : utils::at_offset<Value>(_data, offset);
		}

		inline Offset value_offset(Value* value)
		{
			return !value ? utils::invalid_offset : utils::offset_of(_data, value);
		}

		inline Value* next_value(Value* value)
		{
			return value->_header.next == utils::invalid_offset ? nullptr : utils::at_offset<Value>(_data, value->_header.next);
		}
		inline Value* prev_value(Value* value)
		{
			return value->_header.prev == utils::invalid_offset ? nullptr : utils::at_offset<Value>(_data, value->_header.prev);
		}

		template<typename _Ty>
		requires std::derived_from<_Ty, Value>
		inline _Ty* make_instance() { return reinterpret_cast<_Ty*>(malloc(sizeof(_Ty))); }

		template<typename _Ty, typename _ExtraTy = void>
		requires std::derived_from<_Ty, Value>
		inline _Ty* make_instance(Size extra_size, _ExtraTy** extra)
		{
			_Ty* obj = reinterpret_cast<_Ty*>(malloc(sizeof(_Ty) + extra_size));
			return *extra = reinterpret_cast<_ExtraTy*>(reinterpret_cast<Byte*>(obj) + sizeof(_Ty)), obj;
		}

		template<typename _Ty>
		requires std::derived_from<_Ty, Value>
		inline _Ty* construct_instance() { return __KPL_CONSTRUCT(reinterpret_cast<_Ty*>(malloc(sizeof(_Ty))), _Ty); }

		template<typename _Ty, typename _ArgTy>
		requires std::derived_from<_Ty, Value>
		inline _Ty* construct_instance(_ArgTy arg)
		{
			return __KPL_CONSTRUCT(reinterpret_cast<_Ty*>(malloc(sizeof(_Ty))), _Ty, arg);
		}

	public:
		inline type::Null* make_null() { return reinterpret_cast<type::Null*>(type::literal::Null); }

		inline type::Boolean* make_boolean(bool state)
		{
			return reinterpret_cast<type::Boolean*>(state ? type::literal::True : type::literal::False);
		}

		template<std::integral _Ty>
		inline type::Integer* make_integer(_Ty value)
		{
			type::Integer* obj = make_instance<type::Integer>();

			if constexpr (std::same_as<_Ty, Int64>)
				return construct_instance<type::Integer, Int64>(value);
			else return construct_instance<type::Integer>(static_cast<Int64>(value));
		}
		inline type::Integer* make_integer() { return make_integer<Int64>(0); }

		inline type::Float* make_float(double value = 0) { return construct_instance<type::Float>(value); }
		inline type::Float* make_float(float value) { return construct_instance<type::Float>(static_cast<double>(value)); }

		type::String* make_string(const char* str, Size length);

		inline type::String* make_string(const char* str) { return make_string(str, std::strlen(str)); }
		inline type::String* make_string(const std::string& str) { return make_string(str.c_str(), str.size()); }
		inline type::String* make_string() { return make_string(nullptr, 0); }

		type::Array* make_array(Size length);

		inline type::List* make_list() { return construct_instance<type::List>(this); }

		inline type::Object* make_object(Value* _class = nullptr) { return __KPL_CONSTRUCT(reinterpret_cast<type::Object*>(malloc(sizeof(type::Object))), type::Object, this, _class); }
	};
}
