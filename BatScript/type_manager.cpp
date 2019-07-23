#include "type_manager.h"

namespace Bat
{
	TypeManager typeman;

	TypeManager::TypeManager()
	{
		AddType( "bool", std::make_unique<PrimitiveType>( PrimitiveKind::Bool ) );
		AddType( "int", std::make_unique<PrimitiveType>( PrimitiveKind::Int ) );
		AddType( "float", std::make_unique<PrimitiveType>( PrimitiveKind::Float ) );
		AddType( "string", std::make_unique<PrimitiveType>( PrimitiveKind::String ) );
	}

	bool TypeManager::AddType( const std::string& name, std::unique_ptr<Type> type )
	{
		auto it = m_mapTypes.find( name );
		if( it != m_mapTypes.end() )
		{
			return false;
		}

		m_mapTypes[name] = std::move( type );
		return true;
	}

	Type* TypeManager::GetType( const std::string& name )
	{
		auto it = m_mapTypes.find( name );
		if( it != m_mapTypes.end() )
		{
			return it->second.get();
		}

		return nullptr;
	}
	PrimitiveType* TypeManager::GetType( PrimitiveKind primkind )
	{
		switch( primkind )
		{
			case PrimitiveKind::Bool:
				return GetType( "bool" )->ToPrimitive();
			case PrimitiveKind::Float:
				return GetType( "float" )->ToPrimitive();
			case PrimitiveKind::Int:
				return GetType( "int" )->ToPrimitive();
			case PrimitiveKind::String:
				return GetType( "string" )->ToPrimitive();
		}

		assert( false );
		return nullptr;
	}
}
