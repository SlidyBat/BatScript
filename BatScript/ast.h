#pragma once

#include <cassert>
#include "token.h"
#include "stringlib.h"

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
	/* Statements */ \
	_( ExpressionStmt ) \
	_( PrintStmt ) \
	/* Declarations */ \
	_( VarDecl )

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
	};

#define DECLARE_AST_NODE(asttype) \
	virtual AstType Type() const override { return AstType::##asttype; } \
	virtual void Accept( AstVisitor* visitor ) override { visitor->Visit##asttype( this ); } \
	virtual const char* Name() const override { return #asttype; }

	class Expression : public AstNode {};
	class Statement : public AstNode {};

	class IntLiteral : public Expression
	{
	public:
		DECLARE_AST_NODE( IntLiteral );

		IntLiteral( int64_t value ) : value( value ) {}

		int64_t value;
	};

	class FloatLiteral : public Expression
	{
	public:
		DECLARE_AST_NODE( FloatLiteral );

		FloatLiteral( double value ) : value( value ) {}

		double value;
	};

	class StringLiteral : public Expression
	{
	public:
		DECLARE_AST_NODE( StringLiteral );

		StringLiteral( const char* value ) : value( value ) {}

		const char* value;
	};

	class TokenLiteral : public Expression
	{
	public:
		DECLARE_AST_NODE( TokenLiteral );

		TokenLiteral( TokenType value ) : value( value ) {}

		TokenType value;
	};

	class BinaryExpr : public Expression
	{
	public:
		DECLARE_AST_NODE( BinaryExpr );

		BinaryExpr( TokenType op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right )
			:
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

		UnaryExpr( TokenType op, std::unique_ptr<Expression> right )
			:
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

		GroupExpr( std::unique_ptr<Expression> expression ) : m_pExpression( std::move( expression ) ) {}

		Expression* Expr() { return m_pExpression.get(); }
	private:
		std::unique_ptr<Expression> m_pExpression;
	};

	class VarExpr : public Expression
	{
	public:
		DECLARE_AST_NODE( VarExpr );

		VarExpr( Token name ) : name( name ) {}

		Token name;
	};

	class ExpressionStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( ExpressionStmt );

		ExpressionStmt( std::unique_ptr<Expression> expression ) : m_pExpression( std::move( expression ) ) {}

		Expression* Expr() { return m_pExpression.get(); }
	private:
		std::unique_ptr<Expression> m_pExpression;
	};

	class PrintStmt : public Statement
	{
	public:
		DECLARE_AST_NODE( PrintStmt );

		PrintStmt( std::unique_ptr<Expression> expression ) : m_pExpression( std::move( expression ) ) {}

		Expression* Expr() { return m_pExpression.get(); }
	private:
		std::unique_ptr<Expression> m_pExpression;
	};

	class VarDecl : public Statement
	{
	public:
		DECLARE_AST_NODE( VarDecl );

		VarDecl( Token classifier, Token identifier, std::unique_ptr<Expression> initializer )
			:
			m_Classifier( classifier ),
			m_Identifier( identifier ),
			m_pInitializer( std::move( initializer ) )
		{}

		const Token& Classifier() const { return m_Classifier; }
		const Token& Identifier() const { return m_Identifier; }
		Expression* Initializer() { return m_pInitializer.get(); }
	private:
		Token m_Classifier;
		Token m_Identifier;
		std::unique_ptr<Expression> m_pInitializer;
	};
}