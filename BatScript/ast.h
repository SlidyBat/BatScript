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
	_( ArrayLiteral ) /* List of values that evaluate to an array. E.g. "[1, 2, 3]" */ \
	/* Expressions */ \
	_( BinaryExpr ) \
	_( UnaryExpr ) \
	_( GroupExpr ) \
	_( VarExpr ) \
	_( CallExpr ) \
	_( IndexExpr ) \
	_( CastExpr ) \
	/* Statements */ \
	_( ExpressionStmt ) \
	_( AssignStmt ) \
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

	class LValueExpr;
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

		virtual AstType Kind() const = 0;
		virtual void Accept( AstVisitor* visitor ) = 0;
		virtual const char* Name() const = 0;

#define _(asttype) \
		bool Is##asttype() const { return Kind() == AstType::asttype; } \
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
	virtual AstType Kind() const override { return AstType::##asttype; } \
	virtual void Accept( AstVisitor* visitor ) override { visitor->Visit##asttype( this ); } \
	virtual const char* Name() const override { return #asttype; }

	class Expression : public AstNode
	{
	public:
		Expression( const SourceLoc& loc )
			:
			AstNode( loc )
		{}

		void SetType( Type* type ) { m_pType = type; }
		const Bat::Type* Type() const { return m_pType; }
		Bat::Type* Type() { return m_pType; }
		virtual bool IsLValue() const { return false; }
	private:
		Bat::Type* m_pType = nullptr; // Type gets filled in semantic analysis pass
	};

	class LValueExpr : public Expression
	{
	public:
		LValueExpr( const SourceLoc& loc )
			:
			Expression( loc )
		{}

		virtual bool IsLValue() const override { return true; }
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

	class ArrayLiteral : public Expression
	{
	public:
		DECLARE_AST_NODE( ArrayLiteral );

		ArrayLiteral( const SourceLoc& loc, std::vector<std::unique_ptr<Expression>> values ) : Expression( loc ), m_pValues( std::move( values ) ) {}

		size_t NumValues() const { return m_pValues.size(); }
		Expression* ValueAt( size_t index ) const { return m_pValues[index].get(); }
	private:
		std::vector<std::unique_ptr<Expression>> m_pValues;
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
		std::unique_ptr<Expression> TakeLeft() { return std::move( m_pLeft ); }
		void SetLeft( std::unique_ptr<Expression> expr ) { m_pLeft = std::move( expr ); }
		Expression* Right() { return m_pRight.get(); }
		void SetRight( std::unique_ptr<Expression> expr ) { m_pRight = std::move( expr ); }
		std::unique_ptr<Expression> TakeRight() { return std::move( m_pRight ); }
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

	class VarExpr : public LValueExpr
	{
	public:
		DECLARE_AST_NODE( VarExpr );

		VarExpr( const SourceLoc& loc, Token name ) : LValueExpr( loc ), m_tokName( name ) {}

		const Token& Identifier() const { return m_tokName; }
	private:
		Token m_tokName;
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
		std::unique_ptr<Expression> TakeArg( size_t index ) { return std::move( m_pArguments[index] ); }
		void SetArg( size_t index, std::unique_ptr<Expression> expr ) { m_pArguments[index] = std::move( expr ); }
	private:
		std::unique_ptr<Expression> m_pFunc;
		std::vector<std::unique_ptr<Expression>> m_pArguments;
	};

	class IndexExpr : public LValueExpr
	{
	public:
		DECLARE_AST_NODE( IndexExpr );

		IndexExpr( const SourceLoc& loc, std::unique_ptr<Expression> arr, std::unique_ptr<Expression> index )
			:
			LValueExpr( loc ),
			m_pArray( std::move( arr ) ),
			m_pIndex( std::move( index ) )
		{}

		Expression* Array() { return m_pArray.get(); }
		Expression* Index() { return m_pIndex.get(); }
		std::unique_ptr<Expression> TakeIndex() { return std::move( m_pIndex ); }
		void SetIndex( std::unique_ptr<Expression> expr ) { m_pIndex = std::move( expr ); }
	private:
		std::unique_ptr<Expression> m_pArray;
		std::unique_ptr<Expression> m_pIndex;
	};

	class CastExpr : public Expression
	{
	public:
		DECLARE_AST_NODE( CastExpr );

		CastExpr( std::unique_ptr<Expression> expr, Bat::Type* target )
			:
			Expression( expr->Location() ),
			m_pExpr( std::move( expr ) ),
			m_pTargetType( target )
		{
			SetType( target );
		}

		Expression* Expr() { return m_pExpr.get(); }
		Bat::Type* TargetType() { return m_pTargetType; }
	private:
		std::unique_ptr<Expression> m_pExpr;
		Bat::Type* m_pTargetType;
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

	class AssignStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( AssignStmt );

		AssignStmt( const SourceLoc& loc, std::unique_ptr<Expression> lhs, TokenType op, std::unique_ptr<Expression> rhs )
			:
			Statement( loc ),
			m_pLeft( std::move( lhs ) ),
			m_Op( op ),
			m_pRight( std::move( rhs ) )
		{}

		TokenType Op() const { return m_Op; }
		Expression* Left() { return m_pLeft.get(); }
		std::unique_ptr<Expression> TakeLeft() { return std::move( m_pLeft ); }
		void SetLeft( std::unique_ptr<Expression> expr ) { m_pLeft = std::move( expr ); }
		Expression* Right() { return m_pRight.get(); }
		void SetRight( std::unique_ptr<Expression> expr ) { m_pRight = std::move( expr ); }
		std::unique_ptr<Expression> TakeRight() { return std::move( m_pRight ); }
	private:
		TokenType m_Op;
		std::unique_ptr<Expression> m_pLeft;
		std::unique_ptr<Expression> m_pRight;
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
			m_pRetExpr( std::move( ret_value ) )
		{}

		Expression* RetExpr() { return m_pRetExpr.get(); }
		std::unique_ptr<Expression> TakeRetExpr() { return std::move( m_pRetExpr ); }
		void SetRetExpr( std::unique_ptr<Expression> expr ) { m_pRetExpr = std::move( m_pRetExpr ); }
	private:
		std::unique_ptr<Expression> m_pRetExpr;
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

	class TypeSpecifier
	{
	public:
		TypeSpecifier( const Token& type_name )
			:
			m_tokTypeName( type_name )
		{}

		const Token& TypeName() const { return m_tokTypeName; }
		void SetDimensions( std::vector<std::unique_ptr<Expression>> dims ) { m_Dimensions = std::move( dims ); }
		size_t Rank() const { return m_Dimensions.size(); }
		bool IsArray() const { return Rank() > 0; }
		// Gets expression that evaluates to dimensions of specified rank, or nullptr if dimensions were not specified
		Expression* Dimensions( size_t rank ) const { assert( rank < Rank() ); return m_Dimensions[rank].get(); }
	private:
		Token m_tokTypeName;
		std::vector<std::unique_ptr<Expression>> m_Dimensions;
	};

	class FunctionSignature
	{
	public:
		FunctionSignature( TypeSpecifier return_type_name,
			const Token& identifier,
			std::vector<TypeSpecifier> types,
			std::vector<Token> parameters,
			std::vector<std::unique_ptr<Expression>> defaults,
			bool varargs )
			:
			m_ReturnTypeName( std::move( return_type_name ) ),
			m_Identifier( identifier ),
			m_Types( std::move( types ) ),
			m_Parameters( std::move( parameters ) ),
			m_pDefaults( std::move( defaults ) ),
			m_bVarArgs( varargs )
		{}

		const TypeSpecifier& ReturnTypeSpec() const { return m_ReturnTypeName; }
		const Token& Identifier() const { return m_Identifier; }
		// Number of parameters, not including vararg ellipsis as one
		size_t NumParams() const { return m_Parameters.size(); }
		const TypeSpecifier& ParamType( size_t index ) const { return m_Types[index]; }
		const Token& ParamIdent( size_t index ) const { return m_Parameters[index]; }
		Expression* ParamDefault( size_t index ) const { return m_pDefaults[index].get(); }
		std::unique_ptr<Expression> TakeParamDefault( size_t index ) { return std::move( m_pDefaults[index] ); }
		void SetParamDefault( size_t index, std::unique_ptr<Expression> expr ) { m_pDefaults[index] = std::move( expr ); }
		void SetReturnType( Type* rettype ) { m_pReturnType = rettype; }
		Type* ReturnType() { return m_pReturnType; }
		const Type* ReturnType() const { return m_pReturnType; }
		bool VarArgs() const { return m_bVarArgs; }
	private:
		TypeSpecifier m_ReturnTypeName;
		Token m_Identifier;
		std::vector<TypeSpecifier> m_Types;
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

		VarDecl( const SourceLoc& loc, TypeSpecifier type_name, Token identifier, std::unique_ptr<Expression> initializer )
			:
			Statement( loc ),
			m_TypeName( std::move( type_name ) ),
			m_Identifier( identifier ),
			m_pInitializer( std::move( initializer ) )
		{}

		const TypeSpecifier& TypeSpec() const { return m_TypeName; }
		const Token& Identifier() const { return m_Identifier; }
		Expression* Initializer() { return m_pInitializer.get(); }
		std::unique_ptr<Expression> TakeInitializer() { return std::move( m_pInitializer ); }
		void SetInitializer( std::unique_ptr<Expression> expr ) { m_pInitializer = std::move( expr ); }
		void SetType( Type* type ) { assert( m_pType == nullptr ); m_pType = type; }
		Bat::Type* Type() { return m_pType; }
	private:
		TypeSpecifier m_TypeName;
		Bat::Type* m_pType = nullptr;
		Token m_Identifier;
		std::unique_ptr<Expression> m_pInitializer;
		bool m_bIsLValue = false;
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