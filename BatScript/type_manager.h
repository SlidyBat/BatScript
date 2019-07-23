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

		bool AddType( const std::string& name, std::unique_ptr<Type> type );
		Type* GetType( const std::string& name );
		PrimitiveType* GetType( PrimitiveKind primkind );
	private:
		std::unordered_map<std::string, std::unique_ptr<Type>> m_mapTypes;
	};

	extern TypeManager typeman;
}