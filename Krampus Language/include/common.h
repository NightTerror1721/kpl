#pragma once

#include <unordered_map>
#include <algorithm>
#include <exception>
#include <iostream>
#include <compare>
#include <utility>
#include <sstream>
#include <string>
#include <vector>
#include <list>

#ifndef __cpp_lib_concepts
#define __cpp_lib_concepts
#endif
#include <concepts>


#define __KPL_CONSTRUCT(_Object, _Type, ...) new (_Object) _Type(__VA_ARGS__)



namespace kpl
{
	typedef std::uint8_t UInt8;
	typedef std::uint16_t UInt16;
	typedef std::uint32_t UInt32;
	typedef std::uint64_t UInt64;

	typedef std::int8_t Int8;
	typedef std::int16_t Int16;
	typedef std::int32_t Int32;
	typedef std::int64_t Int64;

	typedef std::size_t Size;
	typedef std::size_t Offset;

	typedef std::byte Byte;

	typedef UInt32 InstructionCode;
}



namespace kpl
{
	class KPLState;

	class Value;

	namespace type
	{
		typedef decltype(nullptr) Null;
		typedef Int64 Integer;
		typedef double Float;
		typedef bool Boolean;
		class String;
		class Array;
		class List;
		class Object;
		class Function;
		class Userdata;
	}

	class Chunk;
}



namespace kpl::utils
{
	static constexpr Offset invalid_offset = static_cast<Offset>(-1LL);
}



namespace kpl::utils
{
	template<typename _Ty = void>
	inline _Ty* malloc(const Size size) { return reinterpret_cast<_Ty*>(::operator new(size)); }

	inline void free(void* const ptr) { ::operator delete(ptr); }

	template<typename _Ty, typename... _Args>
	inline _Ty& construct(_Ty& object, _Args&&... args) { return new (&object) _Ty{ std::forward<_Args>(args)... }, object; }

	template<typename _Ty>
	inline _Ty& construct_copy(_Ty& dst, const _Ty& src) { return construct<_Ty, const _Ty&>(dst, src); }

	template<typename _Ty>
	inline _Ty& construct_move(_Ty& dst, _Ty&& src) { return construct<_Ty, _Ty&&>(dst, std::move(src)); }

	template<typename _Ty>
	inline _Ty& destroy(_Ty& object) { return object.~_Ty(), object; }

	inline std::ptrdiff_t addr_diff(const void* const left, const void* const right)
	{
		return reinterpret_cast<const Byte*>(left) - reinterpret_cast<const Byte*>(right);
	}

	inline Offset offset_of(const void* const base_ptr, const void* const target_ptr)
	{
		return static_cast<Offset>(reinterpret_cast<const Byte*>(base_ptr) - reinterpret_cast<const Byte*>(target_ptr));
	}

	template<typename _Ty = void>
	inline _Ty* addr_diff_add(void* const ptr, const std::ptrdiff_t diff)
	{
		return reinterpret_cast<_Ty*>(reinterpret_cast<Byte*>(ptr) + diff);
	}

	template<typename _Ty = void>
	inline const _Ty* addr_diff_add(const void* const ptr, const std::ptrdiff_t diff)
	{
		return reinterpret_cast<const _Ty*>(reinterpret_cast<const Byte*>(ptr) + diff);
	}

	template<typename _Ty = void>
	inline _Ty* at_offset(void* const ptr, const Offset offset)
	{
		return reinterpret_cast<_Ty*>(reinterpret_cast<Byte*>(ptr) + offset);
	}

	template<typename _Ty = void>
	inline const _Ty* at_offset(const void* const ptr, const Offset offset)
	{
		return reinterpret_cast<const _Ty*>(reinterpret_cast<const Byte*>(ptr) + offset);
	}

	template<typename _ValueTy, typename _MinTy = _ValueTy, typename _MaxTy = _ValueTy>
	constexpr _ValueTy clamp(_ValueTy value, _MinTy min, _MaxTy max)
	{
		if constexpr (std::same_as<_MinTy, _ValueTy>)
		{
			if constexpr (std::same_as<_MaxTy, _ValueTy>)
				return std::min(max, std::max(min, value));
			else return std::min(static_cast<_ValueTy>(max), std::max(min, value));
		}
		else if constexpr (std::same_as<_MaxTy, _ValueTy>)
			return std::min(max, std::max(static_cast<_ValueTy>(min), value));
		else return std::min(static_cast<_ValueTy>(max), std::max(static_cast<_ValueTy>(min), value));
	}
}



namespace kpl::utils
{
	template <Int64 _Max, Int64 _Min = 0>
	class RangedInt
	{
	public:
		static constexpr Int64 max = _Max;
		static constexpr Int64 min = _Min;

	private:
		Int64 _value = min;

	public:
		RangedInt() = default;
		RangedInt(const RangedInt&) = default;
		RangedInt(RangedInt&&) noexcept = default;
		~RangedInt() = default;

		RangedInt& operator= (const RangedInt&) = default;
		RangedInt& operator= (RangedInt&&) noexcept = default;

		template<std::integral _Ty>
		RangedInt(_Ty value) : _value{ clamp(static_cast<Int64>(value), min, max) } {}
		RangedInt(bool value) : _value{ static_cast<Int64>(value) } {}

		template<std::integral _Ty>
		RangedInt& operator= (_Ty value) { return _value = clamp(static_cast<Int64>(value), min, max), *this; }
		RangedInt& operator= (bool value) { return _value = static_cast<Int64>(value), *this; }

		inline operator bool() const { return _value; }
		inline bool operator! () const { return !_value; }

		bool operator== (const RangedInt&) const = default;
		auto operator<=> (const RangedInt&) const = default;

		template<std::integral _Ty>
		operator _Ty() const { return static_cast<_Ty>(_value); }

		template<std::integral _Ty>
		friend inline bool operator== (const RangedInt& left, _Ty right) { return left._value == static_cast<Int64>(right); }
		template<std::integral _Ty>
		friend inline bool operator== (_Ty left, const RangedInt& right) { return right._value == static_cast<Int64>(left); }

		template<std::integral _Ty>
		friend inline bool operator!= (const RangedInt& left, _Ty right) { return left._value != static_cast<Int64>(right); }
		template<std::integral _Ty>
		friend inline bool operator!= (_Ty left, const RangedInt& right) { return right._value != static_cast<Int64>(left); }

		template<std::integral _Ty>
		friend std::strong_ordering operator<=> (const RangedInt& left, _Ty right) { return left._value <=> static_cast<Int64>(right); }
		template<std::integral _Ty>
		friend std::strong_ordering operator<=> (_Ty left, const RangedInt& right) { return right._value <=> static_cast<Int64>(left); }
	};
}
