#pragma once

#include <string>
#include <cassert>
#include "token.h"

#define TYPE_KINDS(_) \
	_(Primitive) \
	_(Array) \
	_(Function) \
	_(Named) \

namespace Bat
{
	class FunctionSignature;

#define _(type) class type##Type;
	TYPE_KINDS(_)
#undef _

	enum class TypeKind
	{
#define _(type) type,
		TYPE_KINDS(_)
#undef _
	};

	enum class TypeQualifiers
	{
		NONE = 0,
		CONST = (1 << 0),
	};

	class Type
	{
	public:
		Type( TypeKind kind )
			:
			m_Kind( kind )
		{}

		TypeKind Kind() const { return m_Kind; }
		virtual size_t Size() const = 0;

		virtual std::string ToString() const = 0;

#define _(type) \
		bool Is##type() const { return Kind() == TypeKind::##type; } \
		type##Type* As##type() { assert( Is##type() ); return (type##Type*)this; } \
		const type##Type* As##type() const { assert( Is##type() ); return (const type##Type*)this; } \
		type##Type* To##type() { if( !Is##type() ) return nullptr; return (type##Type*)this; } \
		const type##Type* To##type() const { if( !Is##type() ) return nullptr; return (const type##Type*)this; }
		TYPE_KINDS(_)
#undef _
	private:
		TypeKind m_Kind;
	};


	enum class PrimitiveKind
	{
		Void,
		Bool,
		Int,
		Float,
		String
	};

	class PrimitiveType : public Type
	{
	public:
		PrimitiveType( PrimitiveKind kind )
			:
			m_PrimitiveKind( kind ),
			Type( TypeKind::Primitive )
		{}

		virtual std::string ToString() const override
		{
			switch( PrimKind() )
			{
				case PrimitiveKind::Bool:
					return "bool";
				case PrimitiveKind::Int:
					return "int";
				case PrimitiveKind::Float:
					return "float";
				case PrimitiveKind::String:
					return "string";
				default:
					return "<error-type>";
			}
		}

		virtual size_t Size() const override
		{
			return sizeof( int64_t );
		}

		PrimitiveKind PrimKind() const { return m_PrimitiveKind; }
	private:
		PrimitiveKind m_PrimitiveKind;
	};

	class ArrayType : public Type
	{
	public:
		static constexpr size_t UNSIZED = 0;
	public:
		ArrayType( Type* inner, size_t size = UNSIZED )
			:
			m_pInner( inner ),
			m_iFixedSize( size ),
			Type( TypeKind::Array )
		{}

		virtual std::string ToString() const override
		{
			return Inner()->ToString() + "[" +
				(HasFixedSize() ? std::to_string(FixedSize()) : std::string())
				+ "]";
		}

		virtual size_t Size() const override
		{
			if( HasFixedSize() )
			{
				return FixedSize() * Inner()->Size();
			}
			
			return sizeof( int64_t );
		}

		Type* Inner() { return m_pInner; }
		const Type* Inner() const { return m_pInner; }
		bool HasFixedSize() const { return m_iFixedSize > 0; }
		size_t FixedSize() const { return m_iFixedSize; }
	private:
		Type* m_pInner;
		size_t m_iFixedSize;
	};

	class FunctionType : public Type
	{
	public:
		// Need to add function name/return to FunctionSignature
		FunctionType( FunctionSignature* signature )
			:
			m_pSignature( signature ),
			Type( TypeKind::Function )
		{}

		virtual std::string ToString() const override
		{
			return "function";
		}

		virtual size_t Size() const override
		{
			return sizeof( int64_t );
		}

		FunctionSignature* Signature() { return m_pSignature; }
		const FunctionSignature* Signature() const { return m_pSignature; }
	private:
		FunctionSignature* m_pSignature;
	};

	class NamedType : public Type
	{
	public:
		NamedType( const std::string& name )
			:
			m_szName( name ),
			Type( TypeKind::Named )
		{}

		virtual std::string ToString() const override
		{
			return m_szName;
		}

		virtual size_t Size() const override
		{
			assert( false );
			return 0;
		}

		const std::string& Name() const { return m_szName; }
	private:
		std::string m_szName;
	};

	inline bool IsSameType( Type* a, Type* b )
	{
		if( a == b )
		{
			return true;
		}

		if( a->Kind() != b->Kind() )
		{
			return false;
		}

		if( a->IsPrimitive() && b->IsPrimitive() &&
			a->AsPrimitive()->PrimKind() == b->ToPrimitive()->PrimKind() )
		{
			return true;
		}

		if( a->IsArray() && b->IsArray() )
		{
			return IsSameType( a->ToArray()->Inner(), b->ToArray()->Inner() ) &&
				a->ToArray()->FixedSize() == b->ToArray()->FixedSize();
		}

		if( a->IsNamed() && b->IsNamed() )
		{
			return a->ToNamed()->Name() == b->ToNamed()->Name();
		}

		return false;
	}
}