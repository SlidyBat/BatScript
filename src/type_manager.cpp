#include "type_manager.h"

namespace Bat
{
	TypeManager typeman;

	TypeManager::TypeManager()
	{
		AddType( "void", std::make_unique<PrimitiveType>( PrimitiveKind::Void ) );
		AddType( "bool", std::make_unique<PrimitiveType>( PrimitiveKind::Bool ) );
		AddType( "int", std::make_unique<PrimitiveType>( PrimitiveKind::Int ) );
		AddType( "float", std::make_unique<PrimitiveType>( PrimitiveKind::Float ) );
		AddType( "string", std::make_unique<PrimitiveType>( PrimitiveKind::String ) );
	}

	Type* TypeManager::GetType( const std::string& name ) const
	{
		auto it = m_mapTypes.find( name );
		if( it != m_mapTypes.end() )
		{
			return it->second.get();
		}

		return nullptr;
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

	PrimitiveType* TypeManager::NewPrimitive( PrimitiveKind primkind )
	{
		switch( primkind )
		{
		case PrimitiveKind::Void:
			return GetType( "void" )->ToPrimitive();
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

	ArrayType* TypeManager::NewArray( Type* inner )
	{
		std::string type_name = inner->ToString() + "[]";

		if( Type* existing = GetType( type_name ) )
		{
			return existing->AsArray();
		}

		auto new_array = std::make_unique<ArrayType>( inner );
		auto res = new_array.get();
		if( !AddType( type_name, std::move( new_array ) ) )
		{
			return nullptr;
		}
		return res;
	}

	FunctionType* TypeManager::NewFunction( FunctionSignature* sig )
	{
		return new FunctionType( sig );
	}

	ClassType* TypeManager::NewClass( ClassDecl* decl, SymbolTable* symtab )
	{
		const std::string& name = decl->Name().lexeme;
		if( Type* existing = GetType( name ) )
		{
			return nullptr;
		}

		auto new_class = std::make_unique<ClassType>( name, symtab );
		auto res = new_class.get();
		if( !AddType( name, std::move( new_class ) ) )
		{
			return nullptr;
		}

		return res;
	}
}
