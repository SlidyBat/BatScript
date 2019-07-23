#pragma once

#include <unordered_map>
#include "symbol.h"

namespace Bat
{
	class SymbolTable
	{
	public:
		SymbolTable() = default;
		SymbolTable( SymbolTable* enclosing );

		bool Exists( const std::string& name ) const;
		bool ExistsLocally( const std::string& name ) const;
		bool AddSymbol( const std::string& name, std::unique_ptr<Symbol> sym );
		Symbol* GetSymbol( const std::string& name ) const;
		SymbolTable* Enclosing() { return m_pEnclosing; }
		const SymbolTable* Enclosing() const { return m_pEnclosing; }
	private:
		SymbolTable* m_pEnclosing = nullptr;
		std::unordered_map<std::string, std::unique_ptr<Symbol>> m_mapSymbols;
	};
}