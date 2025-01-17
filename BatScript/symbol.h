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

		virtual int64_t Address() const { assert( false ); return 0; }

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

	enum class StorageClass
	{
		UNKNOWN,
		LOCAL,
		GLOBAL,
		ARGUMENT
	};

	class VariableSymbol : public Symbol
	{
	public:
		VariableSymbol( AstNode* node, Type* type = nullptr )
			:
			Symbol( node, SymbolKind::Variable ),
			m_pType( type )
		{}

		void SetVarType( Type* type ) { assert( m_pType == nullptr ); m_pType = type; }
		Type* VarType() { return m_pType; }
		const Type* VarType() const { return m_pType; }

		// TODO: Store scope as well so we know how to interpret address (globals handled incorrectly atm)
		virtual int64_t Address() const override { return m_iAddress; }
		void SetAddress( int64_t addr ) { m_iAddress = addr; }

		StorageClass Storage() const { return m_Storage; }
		void SetStorage( StorageClass sc ) { m_Storage = sc; }
	private:
		Type* m_pType;
		int64_t m_iAddress = 0;
		StorageClass m_Storage = StorageClass::UNKNOWN;
	};

	enum class FunctionKind
	{
		Script, // Function defined in script
		Native, // Function defined as native
	};

	class FunctionSymbol : public Symbol
	{
	public:
		FunctionSymbol( AstNode* node, FunctionKind kind )
			:
			Symbol( node, SymbolKind::Function ),
			m_FuncKind( kind )
		{}

		FunctionSignature& Signature()
		{
			if( FuncKind() == FunctionKind::Script )
			{
				return Node()->ToFuncDecl()->Signature();
			}
			else if( FuncKind() == FunctionKind::Native )
			{
				return Node()->ToNativeStmt()->Signature();
			}

			assert( false );
			return Node()->ToFuncDecl()->Signature();
		}
		FunctionKind FuncKind() const { return m_FuncKind; }

		virtual int64_t Address() const override { return m_iAddress; }
		void SetAddress( int64_t addr ) { m_iAddress = addr; }
	private:
		FunctionKind m_FuncKind;
		bool m_bDeclaredInScript = false;
		int64_t m_iAddress = 0;
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