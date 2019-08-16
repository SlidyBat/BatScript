#pragma once

#include <string>
#include <cassert>

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

		const std::string& Name() const { return m_szName; }
	private:
		std::string m_szName;
	};
}