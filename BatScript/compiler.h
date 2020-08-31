#pragma once

#include "ast.h"
#include "instructions.h"
#include "memory_stream.h"
#include "symbol_table.h"
#include "bat_callable.h"

namespace Bat
{
	using CodeLoc_t = int64_t;

	struct BatDebugInfo
	{
		// Maps from instruction to corresponding line
		// e.g. first entry would be the line that the first instruction corresponds to
		std::vector<int> line_mapping;
	};

	struct BatNativeInfo
	{
		std::string name;
		BatNativeDesc desc;
	};

	struct BatCode
	{
		MemoryStream code;
		CodeLoc_t entry_point;
		std::vector<std::string> string_literals;
		std::vector<BatNativeInfo> natives;
		BatDebugInfo debug_info;
	};

	class Compiler : public AstVisitor
	{
	public:
		Compiler();
		~Compiler();

		void Compile( std::vector<std::unique_ptr<Statement>> statements );

		BatCode Code() const;
	private:
		CodeLoc_t Emit( OpCode op );
		CodeLoc_t Emit( OpCode op, int64_t param1 );
		CodeLoc_t EmitF( OpCode op, double param1 );
		CodeLoc_t EmitByte( char byte );
		CodeLoc_t EmitI16( int16_t i16 );
		CodeLoc_t EmitI32( int32_t i32 );
		CodeLoc_t EmitI64( int64_t i64 );
		CodeLoc_t EmitU16( uint16_t u16 );
		CodeLoc_t EmitU32( uint32_t u32 );
		CodeLoc_t EmitU64( uint64_t u64 );
		CodeLoc_t EmitFloat( float f );
		CodeLoc_t EmitDouble( double d );

		CodeLoc_t EmitToPatch( OpCode op );
		// Patches the first operand of the opcode at a given address to contain the given value
		void Patch( CodeLoc_t addr, int64_t value );
		// Patches the jump target of a jump opcode at a given address to point to current instruction pointer
		void PatchJump( CodeLoc_t addr );

		void EmitReturn();
		void PatchReturns();

		void EmitLoad( Symbol* sym );
		void EmitStore( Symbol* sym );

		void Compile( std::unique_ptr<Statement> s );
		void Compile( Statement* s );
		void CompileLValue( Expression* e );
		void CompileRValue( Expression* e );

		void CompileBinaryExpr( BinaryExpr* node );
		void CompileAssign( AssignStmt* node );

		VariableSymbol* AddVariable( AstNode* node, const std::string& name, StorageClass storage, Type* type );
		Symbol* GetSymbol( const std::string& name ) const;
		Symbol* GetSymbol( Expression* node ) const;
		FunctionSymbol* AddFunction( AstNode* node, const std::string& name );
		FunctionSymbol* AddNative( NativeStmt* node, const std::string& name );

		bool InGlobalScope() const { return m_pSymTab->Enclosing() == nullptr; }
		void AllocateGlobalVariable( VarDecl* decl );
		void PushScope();
		void PopScope();

		int64_t AddStringLiteral( const std::string& literal );
		void UpdateCurrLine( AstNode* node );

		// Returns address of current instruction
		CodeLoc_t IP() const { return code.Size(); }
	private:
		virtual void VisitIntLiteral( IntLiteral* node ) override;
		virtual void VisitFloatLiteral( FloatLiteral* node ) override;
		virtual void VisitStringLiteral( StringLiteral* node ) override;
		virtual void VisitTokenLiteral( TokenLiteral* node ) override;
		virtual void VisitArrayLiteral( ArrayLiteral* node ) override;
		virtual void VisitBinaryExpr( BinaryExpr* node ) override;
		virtual void VisitUnaryExpr( UnaryExpr* node ) override;
		virtual void VisitCallExpr( CallExpr* node ) override;
		virtual void VisitIndexExpr( IndexExpr* node ) override;
		virtual void VisitCastExpr( CastExpr* node ) override;
		virtual void VisitGroupExpr( GroupExpr* node ) override;
		virtual void VisitVarExpr( VarExpr* node ) override;
		virtual void VisitExpressionStmt( ExpressionStmt* node ) override;
		virtual void VisitAssignStmt( AssignStmt* node ) override;
		virtual void VisitBlockStmt( BlockStmt* node ) override;
		virtual void VisitPrintStmt( PrintStmt* node ) override;
		virtual void VisitIfStmt( IfStmt* node ) override;
		virtual void VisitWhileStmt( WhileStmt* node ) override;
		virtual void VisitForStmt( ForStmt* node ) override;
		virtual void VisitReturnStmt( ReturnStmt* node ) override;
		virtual void VisitImportStmt( ImportStmt* node ) override;
		virtual void VisitNativeStmt( NativeStmt* node ) override;
		virtual void VisitVarDecl( VarDecl* node ) override;
		virtual void VisitFuncDecl( FuncDecl* node ) override;
	private:
		enum class ExprType
		{
			UNKNOWN,
			LVALUE,
			RVALUE
		};

		MemoryStream code;
		std::vector<std::string> m_StringLiterals;
		std::vector<std::string> m_Natives;
		int m_iCurrentLine = 1;
		std::vector<int> m_LineMapping;
		std::vector<std::unique_ptr<Statement>> m_pStatements;
		SymbolTable* m_pSymTab;
		int m_iStackSize = 0;
		int m_iArgumentsStackSize = 0;
		ExprType m_CompileType = ExprType::UNKNOWN;
		CodeLoc_t m_iEntryPoint = 0;

		int64_t m_iRetAddr = 0;
		std::vector<CodeLoc_t> m_ReturnsToPatch;
	};
}