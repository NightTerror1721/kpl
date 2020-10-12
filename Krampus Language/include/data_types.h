#pragma once

#include "common.h"

namespace kpl
{
	class MemoryHeap;

	enum class DataType
	{
		Null,

		Integer,
		Float,
		Boolean,
		String,

		Array,

		List,

		Object,

		Callable,

		Userdata,
		Internal
	};


	namespace type
	{
		class Null;
		class Integer;
		class Float;
		class Boolean;
		class String;
		class Array;
		class List;
		class Object;
		class Callable;
		class Userdata;
	}


	class Value
	{
	private:
		mutable struct {
			Size size;
			Offset next;
			Offset prev;
			UInt32 refs;
		} _header;

	private:
		const DataType _type;

	protected:
#pragma warning(push)
#pragma warning(disable : 26495)
		inline Value(DataType type) : _type{ type } {}
#pragma warning(pop)

	public:
		Value(const Value&) = delete;
		Value(Value&&) noexcept = delete;

		Value& operator= (const Value&) = delete;
		Value& operator= (Value&&) noexcept = delete;

	public:
		inline DataType type() const { return _type; }

		void increase_refcount() const;
		void decrease_refcount() const;

		std::string to_string();

	public:
		friend class MemoryHeap;
	};

	namespace type::literal
	{
		extern Value* const Null;
		extern Value* const True;
		extern Value* const False;
	}
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
		inline ValueReference(Value* value) : _value{ value } { if (value) value->increase_refcount(); }
		inline ValueReference(const ValueReference& ref) : _value{ ref._value } { if (_value) _value->increase_refcount(); }
		inline ValueReference(ValueReference&& ref) noexcept : _value{ ref._value } { ref._value = nullptr; }
		inline ~ValueReference() { if (_value) _value->decrease_refcount(); _value = nullptr; }

		inline ValueReference& operator= (Value* value)
		{
			if (_value)
				_value->decrease_refcount();
			_value = value;
			if (value)
				value->increase_refcount();
			return *this;
		}
		inline ValueReference& operator= (const ValueReference& right)
		{
			if (_value)
				_value->decrease_refcount();
			_value = right._value;
			if (_value)
				_value->increase_refcount();
			return *this;
		}
		inline ValueReference& operator= (ValueReference&& right) noexcept
		{
			if (_value)
				_value->decrease_refcount();
			_value = right._value;
			right._value = nullptr;
			return *this;
		}
		inline ValueReference& operator= (decltype(nullptr))
		{
			if (_value)
			{
				_value->decrease_refcount();
				_value = nullptr;
			}
			return *this;
		}

		inline operator Value* () const { return _value ? _value : type::literal::Null; }
		inline operator const Value* () const { return _value ? _value : type::literal::Null; }

		inline Value* operator-> () { return _value ? _value : type::literal::Null; }
		inline const Value* operator-> () const { return _value ? _value : type::literal::Null; }

		inline operator bool() const { return _value; }
		inline bool operator! () const { return !_value; }
	};
}



namespace kpl::type
{
	class Null : public Value
	{
	public:
		Null(const Null&) = delete;
		Null(Null&&) noexcept = delete;

		Null& operator= (const Null&) = delete;
		Null& operator= (Null&&) = delete;

	private:
		inline Null() : Value{ DataType::Null } {}

	public:
		static const Null instance;

		friend class Value;
	};
}



namespace kpl::type
{
	class Integer : public Value
	{
	private:
		Int64 _value;

	public:
		inline Integer() : Value{ DataType::Integer }, _value{ 0 } {}
		inline Integer(Int64 value) : Value{ DataType::Integer }, _value{ value } {}
		Integer(const Integer&) = delete;
		Integer(Integer&&) noexcept = delete;

		Integer& operator= (const Integer&) = delete;
		Integer& operator= (Integer&&) noexcept = delete;

		friend class Value;
	};
}



namespace kpl::type
{
	class Float : public Value
	{
	private:
		double _value;

	public:
		inline Float() : Value{ DataType::Float }, _value{ 0 } {}
		inline Float(double value) : Value{ DataType::Float }, _value{ value } {}
		Float(const Float&) = delete;
		Float(Float&&) noexcept = delete;

		Float& operator= (const Float&) = delete;
		Float& operator= (Float&&) = delete;

		friend class Value;
	};
}



namespace kpl::type
{
	class Boolean : public Value
	{
	private:
		bool _state;

	public:
		Boolean(const Boolean&) = delete;
		Boolean(Boolean&&) noexcept = delete;

		Boolean& operator= (const Boolean) = delete;
		Boolean& operator= (Boolean&&) noexcept = delete;

	private:
		inline Boolean(bool state) : Value{ DataType::Boolean }, _state{ state } {}

	public:
		static const Boolean true_instance;
		static const Boolean false_instance;

		friend class Value;
	};
}



namespace kpl::type
{
	class String : public Value
	{
	private:
		const char* _str;
		Size _size;
		mutable Int64 _hashcode;

	public:
		String(const char* str_addr, Size str_size);
		String(const String&) = delete;
		String(String&&) noexcept = delete;

		String& operator= (const String&) = delete;
		String& operator= (String&&) noexcept = delete;

		inline String() :
			Value{ DataType::String },
			_str{ nullptr },
			_size{ 0 },
			_hashcode{ 0 }
		{}

		Int64 hashcode() const;

		String* copy(MemoryHeap* heap) const;

		friend class Value;
	};
}



namespace kpl::type
{
	class Array : public Value
	{
	private:
		ValueReference* _array;
		Size _size;

	public:
		Array(ValueReference* array_addr, Size array_size);
		Array(const Array&) = delete;
		Array(Array&&) noexcept = delete;

		Array& operator= (const Array&) = delete;
		Array& operator= (Array&&) noexcept = delete;

		inline Array() :
			Value{ DataType::Array },
			_array{ nullptr },
			_size{ 0 }
		{}

	public:
		inline Size length() const { return _size; }

		inline void set(Offset index, Value* value) { _array[index] = value; }
		inline Value* get(Offset index) { return _array[index]; }

		friend class Value;
	};
}



namespace kpl::type
{
	class List : public Value
	{
	private:
		struct Node : public Value
		{
			ValueReference value;
			Node* next;
			Node* prev;

			inline Node(Value* value = nullptr, Node* next = nullptr, Node* prev = nullptr) :
				Value{ DataType::Internal },
				value{ value ? value : literal::Null },
				next{ next },
				prev{ prev }
			{
				increase_refcount();
			};

			Node(const Node&) = delete;
			Node(Node&&) noexcept = delete;
			Node& operator= (const Node&) = delete;
			Node& operator= (Node&&) noexcept = delete;

			inline Value* safe_value() { return !value ? literal::Null : static_cast<Value*>(value); }
		};

	private:
		MemoryHeap* _heap;
		Node* _front;
		Node* _back;
		Size _size;

	public:
		List(const List&) = delete;
		List(List&&) noexcept = delete;

		List& operator= (const List&) = delete;
		List& operator= (List&&) noexcept = delete;

		inline List(MemoryHeap* heap) :
			Value{ DataType::List },
			_heap{ heap },
			_front{ nullptr },
			_back{ nullptr },
			_size{ 0 }
		{}

	public:
		void push_back(Value* value);
		void push_front(Value* value);
		void insert(Offset index, Value* value);
		void set(Offset index, Value* value);

		//Value* contains(Value* value) const;

		Value* get(Offset index) const;

		Value* erase(Offset index);
		//Value* erase(Value* value);
		void clear();

		//Value* index_of(Value* value) const;

	public:
		inline ~List() { clear(); }

		inline bool empty() const { return !_front; }
		inline Size size() const { return _size; }

		inline Value* front() const { return _front ? _front->value : nullptr; }
		inline Value* back() const { return _back ? _back->value : nullptr; }

	private:
		Node* make_node(Value* value, Node* next = nullptr, Node* prev = nullptr);
		void delete_node(Node* node);
		Node* find_node(Offset index) const;

	public:
		friend class Value;
	};
}



namespace kpl::type
{
	class Object : public Value
	{
	private:
		struct Entry : public Value
		{
			String* const name;
			ValueReference value;

			Entry(const Entry&) = delete;
			Entry(Entry&&) noexcept = delete;

			Entry& operator= (const Entry&) = delete;
			Entry& operator= (Entry&&) noexcept = delete;

			Entry(MemoryHeap* heap, const Value* name, Value* value);
		};

		struct Table : public Value
		{
			MemoryHeap* const heap;
			Entry** const entries;
			Size const size;

			Table(const Table&) = delete;
			Table(Table&&) noexcept = delete;

			Table& operator= (const Table&) = delete;
			Table& operator= (Table&&) noexcept = delete;

			Table(MemoryHeap* heap, Entry** entries, Size size);
		};

	private:
		MemoryHeap* _heap;

	public:
		Object(const Object&) = delete;
		Object(Object&&) noexcept = delete;

		Object& operator= (const Object&) = delete;
		Object& operator= (Object&&) noexcept = delete;

	private:
		Entry* make_entry(const Value* name, Value* value);
		void delete_entry(Entry* entry);

		Table* make_table(Size size);
		void delete_table(Table* table);

	public:
		friend class Value;
	};
}
