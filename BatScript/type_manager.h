#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "type.h"

namespace Bat
{
	class TypeManager
	{
	public:
		TypeManager();
		PrimitiveType* NewPrimitive( PrimitiveKind primkind );
		ArrayType* NewArray( Type* inner, size_t size );
		NamedType* NewNamed( const std::string& name );
	private:
		Type* GetType( const std::string& name ) const;
		bool AddType( const std::string& name, std::unique_ptr<Type> type );
	private:
		std::unordered_map<std::string, std::unique_ptr<Type>> m_mapTypes;
	};

	extern TypeManager typeman;
}