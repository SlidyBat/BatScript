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
		ClassType* NewClass( ClassDecl* decl, class SymbolTable* symtab );

		Type* GetType( const std::string& name ) const;
	private:
		bool AddType( const std::string& name, std::unique_ptr<Type> type );
	private:
		std::unordered_map<std::string, std::unique_ptr<Type>> m_mapTypes;
	};

	extern TypeManager typeman;

	inline Type* TypeSpecifierToType( const TypeSpecifier& type )
	{
		Type* t = typeman.GetType( type.TypeName().lexeme );
		if ( !t )
			return nullptr;

		for( size_t i = 0; i < type.Rank(); i++ )
			t = typeman.NewArray( t );

		return t;
	}
}