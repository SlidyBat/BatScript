#pragma once

#include <cassert>
#include "token.h"
#include "stringlib.h"
#include "sourceloc.h"
#include "type.h"

#define AST_TYPES(_) \
	/* Literals */ \
	_( IntLiteral ) \
	_( FloatLiteral ) \
	_( StringLiteral ) \
	_( TokenLiteral ) /* For any literals that can be represented by their token type (e.g. true, false, null) */ \
	/* Expressions */ \
	_( BinaryExpr ) \
	_( UnaryExpr ) \
	_( GroupExpr ) \
	_( VarExpr ) \
	_( CallExpr ) \
	/* Statements */ \
	_( ExpressionStmt ) \
	_( BlockStmt ) \
	_( PrintStmt ) \
	_( IfStmt ) \
	_( WhileStmt ) \
	_( ForStmt ) \
	_( ReturnStmt ) \
	_( ImportStmt ) \
	_( NativeStmt ) \
	/* Declarations */ \
	_( VarDecl ) \
	_( FuncDecl )

namespace Bat
{
	enum class AstType
	{
#define _(asttype) asttype,
		AST_TYPES(_)
#undef _
	};

#define _(asttype) class asttype;
	AST_TYPES(_)
#undef _

	class AstVisitor
	{
	public:
#define _(asttype) virtual void Visit##asttype( asttype* node ) = 0;
		AST_TYPES(_)
#undef _
	};

	class AstNode
	{
	public:
		AstNode() = default;
		AstNode( const SourceLoc& loc )
			:
			m_Loc( loc )
		{}

		virtual AstType Type() const = 0;
		virtual void Accept( AstVisitor* visitor ) = 0;
		virtual const char* Name() const = 0;

#define _(asttype) \
		bool Is##asttype() const { return Type() == AstType::asttype; } \
		asttype* As##asttype() { assert( Is##asttype() ); return (asttype*) this; } \
		const asttype* As##asttype() const { assert( Is##asttype() ); return (const asttype*)this; } \
		asttype* To##asttype() { if ( !Is##asttype() ) return nullptr; return (asttype*)this; } \
		const asttype* To##asttype() const { if ( !Is##asttype() ) return nullptr; return (const asttype*)this; }
		AST_TYPES(_)
#undef _


		const SourceLoc& Location() const { return m_Loc; }
	private:
		SourceLoc m_Loc;
	};

#define DECLARE_AST_NODE(asttype) \
	virtual AstType Type() const override { return AstType::##asttype; } \
	virtual void Accept( AstVisitor* visitor ) override { visitor->Visit##asttype( this ); } \
	virtual const char* Name() const override { return #asttype; }

	class Expression : public AstNode
	{
	public:
		Expression( const SourceLoc& loc )
			:
			AstNode( loc )
		{}
	};
	class Statement : public AstNode
	{
	public:
		Statement( const SourceLoc& loc )
			:
			AstNode( loc )
		{}
	};

	class IntLiteral : public Expression
	{
	public:
		DECLARE_AST_NODE( IntLiteral );

		IntLiteral( const SourceLoc& loc, int64_t value ) : Expression( loc ), value( value ) {}

		int64_t value;
	};

	class FloatLiteral : public Expression
	{
	public:
		DECLARE_AST_NODE( FloatLiteral );

		FloatLiteral( const SourceLoc& loc, double value ) : Expression( loc ), value( value ) {}

		double value;
	};

	class StringLiteral : public Expression
	{
	public:
		DECLARE_AST_NODE( StringLiteral );

		StringLiteral( const SourceLoc& loc, const char* value ) : Expression( loc ), value( value ) {}

		const char* value;
	};

	class TokenLiteral : public Expression
	{
	public:
		DECLARE_AST_NODE( TokenLiteral );

		TokenLiteral( const SourceLoc& loc, TokenType value ) : Expression( loc ), value( value ) {}

		TokenType value;
	};

	class BinaryExpr : public Expression
	{
	public:
		DECLARE_AST_NODE( BinaryExpr );

		BinaryExpr( const SourceLoc& loc, TokenType op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right )
			:
			Expression( loc ),
			m_Op( op ),
			m_pLeft( std::move( left ) ),
			m_pRight( std::move( right ) )
		{}

		TokenType Op() const { return m_Op; }
		Expression* Left() { return m_pLeft.get(); }
		Expression* Right() { return m_pRight.get(); }
	private:
		TokenType m_Op;
		std::unique_ptr<Expression> m_pLeft;
		std::unique_ptr<Expression> m_pRight;
	};

	class UnaryExpr : public Expression
	{
	public:
		DECLARE_AST_NODE( UnaryExpr );

		UnaryExpr( const SourceLoc& loc, TokenType op, std::unique_ptr<Expression> right )
			:
			Expression( loc ),
			m_Op( op ),
			m_pRight( std::move( right ) )
		{}

		TokenType Op() const { return m_Op; }
		Expression* Right() { return m_pRight.get(); }
	private:
		TokenType m_Op;
		std::unique_ptr<Expression> m_pRight;
	};

	class GroupExpr : public Expression
	{
	public:
		DECLARE_AST_NODE( GroupExpr );

		GroupExpr( const SourceLoc& loc, std::unique_ptr<Expression> expression )
			:
			Expression( loc ),
			m_pExpression( std::move( expression ) )
		{}

		Expression* Expr() { return m_pExpression.get(); }
	private:
		std::unique_ptr<Expression> m_pExpression;
	};

	class VarExpr : public Expression
	{
	public:
		DECLARE_AST_NODE( VarExpr );

		VarExpr( const SourceLoc& loc, Token name ) : Expression( loc ), name( name ) {}

		Token name;
	};

	class CallExpr : public Expression
	{
	public:
		DECLARE_AST_NODE( CallExpr );

		CallExpr( const SourceLoc& loc, std::unique_ptr<Expression> func, std::vector<std::unique_ptr<Expression>> arguments )
			:
			Expression( loc ),
			m_pFunc( std::move( func ) ),
			m_pArguments( std::move( arguments ) )
		{}

		Expression* Function() { return m_pFunc.get(); }
		size_t NumArgs() const { return m_pArguments.size(); }
		Expression* Arg( size_t index ) const { return m_pArguments[index].get(); }
	private:
		std::unique_ptr<Expression> m_pFunc;
		std::vector<std::unique_ptr<Expression>> m_pArguments;
	};

	class ExpressionStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( ExpressionStmt );

		ExpressionStmt( const SourceLoc& loc, std::unique_ptr<Expression> expression )
			:
			Statement( loc ),
			m_pExpression( std::move( expression ) ) {}

		Expression* Expr() { return m_pExpression.get(); }
	private:
		std::unique_ptr<Expression> m_pExpression;
	};

	class BlockStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( BlockStmt );

		BlockStmt( const SourceLoc& loc, std::vector<std::unique_ptr<Statement>> statements )
			:
			Statement( loc ),
			m_Statements( std::move( statements ) ) {}

		size_t NumStatements() const { return m_Statements.size(); }
		Statement* Stmt(size_t index) const { return m_Statements[index].get(); }
	private:
		std::vector<std::unique_ptr<Statement>> m_Statements;
	};

	class PrintStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( PrintStmt );

		PrintStmt( const SourceLoc& loc, std::unique_ptr<Expression> expression )
			:
			Statement( loc ),
			m_pExpression( std::move( expression ) ) {}

		Expression* Expr() { return m_pExpression.get(); }
	private:
		std::unique_ptr<Expression> m_pExpression;
	};

	class IfStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( IfStmt );

		IfStmt( const SourceLoc& loc, std::unique_ptr<Expression> condition, std::unique_ptr<Statement> then_branch, std::unique_ptr<Statement> else_branch )
			:
			Statement( loc ),
			m_pCondition( std::move( condition ) ),
			m_pThen( std::move( then_branch ) ),
			m_pElse( std::move( else_branch ) )
		{}
		
		Expression* Condition() { return m_pCondition.get(); }
		Statement* Then() { return m_pThen.get(); }
		Statement* Else() { return m_pElse.get(); }
	private:
		std::unique_ptr<Expression> m_pCondition;
		std::unique_ptr<Statement> m_pThen;
		std::unique_ptr<Statement> m_pElse;
	};

	class WhileStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( WhileStmt );

		WhileStmt( const SourceLoc& loc, std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body )
			:
			Statement( loc ),
			m_pCondition( std::move( condition ) ),
			m_pBody( std::move( body ) )
		{}

		Expression* Condition() { return m_pCondition.get(); }
		Statement* Body() { return m_pBody.get(); }
	private:
		std::unique_ptr<Expression> m_pCondition;
		std::unique_ptr<Statement> m_pBody;
	};

	class ForStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( ForStmt );

		ForStmt( const SourceLoc& loc, std::unique_ptr<Expression> initializer, std::unique_ptr<Expression> condition, std::unique_ptr<Expression> increment, std::unique_ptr<Statement> body )
			:
			Statement( loc ),
			m_pCondition( std::move( condition ) ),
			m_pInitializer( std::move( initializer ) ),
			m_pIncrement( std::move( increment ) ),
			m_pBody( std::move( body ) )
		{}

		Expression* Condition() { return m_pCondition.get(); }
		Expression* Initializer() { return m_pInitializer.get(); }
		Expression* Increment() { return m_pIncrement.get(); }
		Statement* Body() { return m_pBody.get(); }
	private:
		std::unique_ptr<Expression> m_pCondition;
		std::unique_ptr<Expression> m_pInitializer;
		std::unique_ptr<Expression> m_pIncrement;
		std::unique_ptr<Statement> m_pBody;
	};

	class ReturnStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( ReturnStmt );

		ReturnStmt( const SourceLoc& loc, std::unique_ptr<Expression> ret_value )
			:
			Statement( loc ),
			m_pRetValue( std::move( ret_value ) )
		{}

		Expression* RetValue() { return m_pRetValue.get(); }
	private:
		std::unique_ptr<Expression> m_pRetValue;
	};

	class ImportStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( ImportStmt );

		ImportStmt( const SourceLoc& loc, Token module_name )
			:
			Statement( loc ),
			m_Module( module_name )
		{}

		const std::string& ModuleName() const { return m_Module.lexeme; }
	private:
		Token m_Module;
	};

	class FunctionSignature
	{
	public:
		FunctionSignature( Token return_identifier, Token identifier, std::vector<Token> types, std::vector<Token> parameters, std::vector<std::unique_ptr<Expression>> defaults, bool varargs )
			:
			m_ReturnIdentifier( return_identifier ),
			m_Identifier( identifier ),
			m_Types( std::move( types ) ),
			m_Parameters( std::move( parameters ) ),
			m_pDefaults( std::move( defaults ) ),
			m_bVarArgs( varargs )
		{}

		const Token& ReturnIdentifier() const { return m_ReturnIdentifier; }
		const Token& Identifier() const { return m_Identifier; }
		// Number of parameters, not including vararg ellipsis as one
		size_t NumParams() const { return m_Parameters.size(); }
		const Token& ParamType( size_t index ) const { return m_Types[index]; }
		const Token& ParamIdent( size_t index ) const { return m_Parameters[index]; }
		Expression* ParamDefault( size_t index ) const { return m_pDefaults[index].get(); }
		void SetReturnType( Type* rettype ) { m_pReturnType = rettype; }
		Type* ReturnType() { return m_pReturnType; }
		const Type* ReturnType() const { return m_pReturnType; }
		bool VarArgs() const { return m_bVarArgs; }
	private:
		Token m_ReturnIdentifier;
		Token m_Identifier;
		std::vector<Token> m_Types;
		std::vector<Token> m_Parameters;
		std::vector<std::unique_ptr<Expression>> m_pDefaults;
		bool m_bVarArgs;

		Type* m_pReturnType = nullptr;
	};

	class NativeStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( NativeStmt );

		NativeStmt( const SourceLoc& loc, FunctionSignature sig )
			:
			Statement( loc ),
			m_Signature( std::move( sig ) )
		{}

		const FunctionSignature& Signature() const { return m_Signature; }
		FunctionSignature& Signature() { return m_Signature; }
	private:
		FunctionSignature m_Signature;
	};

	class VarDecl : public Statement
	{
	public:
		DECLARE_AST_NODE( VarDecl );

		VarDecl( const SourceLoc& loc, Token classifier, Token identifier, std::unique_ptr<Expression> initializer )
			:
			Statement( loc ),
			m_Classifier( classifier ),
			m_Identifier( identifier ),
			m_pInitializer( std::move( initializer ) )
		{}

		const Token& Classifier() const { return m_Classifier; }
		const Token& Identifier() const { return m_Identifier; }
		Expression* Initializer() { return m_pInitializer.get(); }
		void SetNext( std::unique_ptr<VarDecl> next ) { m_pNext = std::move( next ); }
		VarDecl* Next() { return m_pNext.get(); }
	private:
		Token m_Classifier;
		Token m_Identifier;
		std::unique_ptr<Expression> m_pInitializer;
		std::unique_ptr<VarDecl> m_pNext;
	};

	class FuncDecl : public Statement
	{
	public:
		DECLARE_AST_NODE( FuncDecl );

		FuncDecl( const SourceLoc& loc, FunctionSignature sig, std::unique_ptr<Statement> body )
			:
			Statement( loc ),
			m_Signature( std::move( sig ) ),
			m_pBody( std::move( body ) )
		{}

		FunctionSignature& Signature() { return m_Signature; }
		Statement* Body() { return m_pBody.get(); }
	private:
		FunctionSignature m_Signature;
		std::unique_ptr<Statement> m_pBody;
	};
}