#include "symbol_table.h"

namespace Bat
{
	SymbolTable::SymbolTable( SymbolTable* enclosing )
		:
		m_pEnclosing( enclosing )
	{}
	bool SymbolTable::Exists( const std::string & name ) const
	{
		for( const SymbolTable* symtab = this; symtab != nullptr; symtab = symtab->Enclosing() )
		{
			if( symtab->ExistsLocally( name ) )
			{
				return true;
			}
		}

		return false;
	}
	bool SymbolTable::ExistsLocally( const std::string& name ) const
	{
		auto it = m_mapSymbols.find( name );
		return it != m_mapSymbols.end();
	}
	bool SymbolTable::AddSymbol( const std::string& name, std::unique_ptr<Symbol> sym )
	{
		if( ExistsLocally( name ) ) return false;
		m_mapSymbols[name] = std::move( sym );
		return true;
	}
	Symbol* SymbolTable::GetSymbol( const std::string& name ) const
	{
		auto it = m_mapSymbols.find( name );
		if( it != m_mapSymbols.end() )
		{
			return it->second.get();
		}

		if( m_pEnclosing )
		{
			return m_pEnclosing->GetSymbol( name );
		}

		return nullptr;
	}
}
