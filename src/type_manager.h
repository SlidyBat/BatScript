#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "ast.h"
#include "errorsys.h"
#include "type.h"

namespace Bat
{
	class TypeManager
	{
	public:
		TypeManager();
		PrimitiveType* NewPrimitive( PrimitiveKind primkind );
		ArrayType* NewArray( Type* inner );
		FunctionType* NewFunction( FunctionSignature* sig );
		NamedType* NewNamed( const std::string& name );
	private:
		Type* GetType( const std::string& name ) const;
		bool AddType( const std::string& name, std::unique_ptr<Type> type );
	private:
		std::unordered_map<std::string, std::unique_ptr<Type>> m_mapTypes;
	};

	extern TypeManager typeman;

	inline Type* TypeSpecifierToType( const TypeSpecifier& type )
	{
		Type* t = nullptr;
		switch( type.TypeName().type )
		{
		case TOKEN_INT:    t = typeman.NewPrimitive( PrimitiveKind::Int ); break;
		case TOKEN_FLOAT:  t = typeman.NewPrimitive( PrimitiveKind::Float ); break;
		case TOKEN_BOOL:   t = typeman.NewPrimitive( PrimitiveKind::Bool ); break;
		case TOKEN_STRING: t = typeman.NewPrimitive( PrimitiveKind::String ); break;
		case TOKEN_VOID:   t = typeman.NewPrimitive( PrimitiveKind::Void ); break;

		default:
			assert( !"Invalid type token" );
			return nullptr;
		}

		for( size_t i = 0; i < type.Rank(); i++ )
			t = typeman.NewArray( t );

		return t;
	}
}