#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <cassert>
#include "ast.h"
#include "type.h"

#define SYMBOL_TYPES(_) \
	_(Variable) \
	_(Function) \
	_(Type)

namespace Bat
{
#define _(symtype) class symtype##Symbol;
	SYMBOL_TYPES(_)
#undef _

	enum class SymbolKind
	{
#define _(symtype) symtype,
		SYMBOL_TYPES(_)
#undef _
	};

	class Symbol
	{
	public:
		Symbol( AstNode* node, SymbolKind kind )
			:
			m_pNode( node ),
			m_Kind( kind )
		{}
		
		AstNode* Node() { return m_pNode; }
		const AstNode* Node() const { return m_pNode; }
		SymbolKind Kind() const { return m_Kind; }

#define _(symtype) \
		virtual bool Is##symtype() const { return Kind() == SymbolKind::##symtype; } \
		virtual symtype##Symbol* As##symtype() { if( !Is##symtype() ) return nullptr; return (symtype##Symbol*)this; } \
		virtual const symtype##Symbol* As##symtype() const { if( !Is##symtype() ) return nullptr; return (const symtype##Symbol*)this; } \
		virtual symtype##Symbol* To##symtype() { assert( Is##symtype() ); return (symtype##Symbol*)this; } \
		virtual const symtype##Symbol* To##symtype() const { assert( Is##symtype() ); return (const symtype##Symbol*)this; }
		SYMBOL_TYPES(_)
#undef _
	private:
		AstNode* m_pNode;
		SymbolKind m_Kind;
	};

	class VariableSymbol : public Symbol
	{
	public:
		VariableSymbol( AstNode* node, Type* type = nullptr )
			:
			Symbol( node, SymbolKind::Variable ),
			m_pType( type )
		{}

		void SetVarType( Type* type ) { m_pType = type; }
		Type* VarType() { return m_pType; }
		const Type* VarType() const { return m_pType; }
	private:
		Type* m_pType;
	};

	class FunctionSymbol : public Symbol
	{
	public:
		FunctionSymbol( AstNode* node )
			:
			Symbol( node, SymbolKind::Function )
		{}
	};

	class TypeSymbol : public Symbol
	{
	public:
		TypeSymbol( AstNode* node, Type* type )
			:
			Symbol( node, SymbolKind::Type ),
			m_pType( type )
		{}

		void SetType( Type * type ) { m_pType = type; }
		Type* InnerType() { return m_pType; }
		const Type* InnerType() const { return m_pType; }
	private:
		Type* m_pType = nullptr;
	};
}