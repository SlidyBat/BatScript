#pragma once

#include <cassert>
#include "token.h"
#include "stringlib.h"

#define BINARY_EXPR_TYPES(_) \
	/* Arithmetic operators */ \
	_(2, ADD) \
	_(2, SUB) \
	_(2, DIV) \
	_(2, MUL) \
	_(2, MOD) \
	/* Bit operators */ \
	_(2, BITOR) \
	_(2, BITXOR) \
	_(2, BITAND) \
	_(2, LBITSHIFT) \
	_(2, RBITSHIFT) \
	/* Assignment operators */ \
	_(2, ASSIGN) \
	_(2, ADD_ASSIGN) \
	_(2, SUB_ASSIGN) \
	_(2, DIV_ASSIGN) \
	_(2, MUL_ASSIGN) \
	_(2, MOD_ASSIGN) \
	_(2, BITOR_ASSIGN) \
	_(2, BITXOR_ASSIGN) \
	_(2, BITAND_ASSIGN) \
	/* Logical operators */ \
	_(2, AND) \
	_(2, OR) \
	_(2, CMPEQ) \
	_(2, CMPNEQ) \
	_(2, CMPL) \
	_(2, CMPLE) \
	_(2, CMPG) \
	_(2, CMPGE)

#define UNARY_EXPR_TYPES(_) \
	/* Unary operators */ \
	_(1, NOT) \
	_(1, BITNEG) \
	_(1, NEG) \
	_(1, GROUP) \
	_(1, ADDROF) \
	_(1, MOVE) \

#define LITERAL_EXPR_TYPES(_) \
	/* Literals */ \
	_(0, BOOL_LITERAL) \
	_(0, INT_LITERAL) \
	_(0, FLOAT_LITERAL) \
	_(0, STR_LITERAL) \
	_(0, NULL_LITERAL)

#define EXPRESSION_TYPES(_) \
	BINARY_EXPR_TYPES(_) \
	UNARY_EXPR_TYPES(_) \
	LITERAL_EXPR_TYPES(_) \

namespace Bat
{
	enum ExpressionType
	{
#define _(num_params, exprtype) EXPR_##exprtype,
		EXPRESSION_TYPES(_)
#undef _
	};

	struct Expression
	{
		ExpressionType type;
		std::vector<std::unique_ptr<Expression>> params;
		union
		{
			int64_t i64;
			double f64;
			const char* str;
		} value;

		static bool IsLiteral( ExpressionType type );
		static int GetNumParams( ExpressionType type );
	};

	template <typename Arg>
	void AddParams( Expression* e, Arg arg )
	{
		e->params.push_back( std::move( arg ) );
	}

	template <typename Arg, typename... Rest>
	void AddParams( Expression* e, Arg arg, Rest... rest )
	{
		e->params.push_back( std::move( arg ) );
		AddParams( e, std::move( rest )... );
	}

#define _(num_ops, exprtype) \
	template <typename... Args> \
	std::unique_ptr<Expression> CreateExpr_##exprtype( Args... args ) \
	{ \
		auto e = std::make_unique<Expression>(); \
		e->type = EXPR_##exprtype; \
		AddParams( e.get(), std::move( args )... ); \
		return std::move( e ); \
	}
	EXPRESSION_TYPES(_)
#undef _

	template <typename T>
	class ASTVisitor
	{
	public:
		T Traverse( Expression* e )
		{
			switch( e->type )
			{
#define _(num_params, exprtype) case EXPR_##exprtype: return Visit##exprtype( e->params[0].get(), e->params[1].get() );
				BINARY_EXPR_TYPES( _ )
#undef _
#define _(num_params, exprtype) case EXPR_##exprtype: return Visit##exprtype( e->params[0].get() );
				UNARY_EXPR_TYPES( _ )
#undef _
				case EXPR_NULL_LITERAL:  return VisitNULL_LITERAL();
				case EXPR_BOOL_LITERAL:  return VisitBOOL_LITERAL( e->value.i64 );
				case EXPR_FLOAT_LITERAL: return VisitFLOAT_LITERAL( e->value.f64 );
				case EXPR_INT_LITERAL:   return VisitINT_LITERAL( e->value.i64 );
				case EXPR_STR_LITERAL:   return VisitSTR_LITERAL( e->value.str );
				default: assert( false && "Unhandled expression type" ); return T{};
			}
		}

#define _(num_ops, exprtype) virtual T Visit##exprtype( Expression* left, Expression* right ) = 0;
		BINARY_EXPR_TYPES(_)
#undef _

#define _(num_ops, exprtype) virtual T Visit##exprtype( Expression* expr ) = 0;
		UNARY_EXPR_TYPES(_)
#undef _

		virtual T VisitNULL_LITERAL() = 0;
		virtual T VisitBOOL_LITERAL( bool value ) = 0;
		virtual T VisitFLOAT_LITERAL( double value ) = 0;
		virtual T VisitINT_LITERAL( int64_t value ) = 0;
		virtual T VisitSTR_LITERAL( const char* str ) = 0;
	};

	class LiteralExpr
	{
	public:
		static std::unique_ptr<Expression> Null()
		{
			auto e = std::make_unique<Expression>();
			e->type = EXPR_NULL_LITERAL;
			return e;
		}

		static std::unique_ptr<Expression> Bool( bool bool_literal )
		{
			auto e = std::make_unique<Expression>();
			e->type = EXPR_BOOL_LITERAL;
			e->value.i64 = bool_literal ? 1 : 0;
			return e;
		}

		static std::unique_ptr<Expression> Float( double value )
		{
			auto e = std::make_unique<Expression>();
			e->type = EXPR_FLOAT_LITERAL;
			e->value.f64 = value;
			return e;
		}

		static std::unique_ptr<Expression> Int( int64_t value )
		{
			auto e = std::make_unique<Expression>();
			e->type = EXPR_INT_LITERAL;
			e->value.i64 = value;
			return e;
		}

		static std::unique_ptr<Expression> String( const char* value )
		{
			auto e = std::make_unique<Expression>();
			e->type = EXPR_STR_LITERAL;
			e->value.str = value;
			return e;
		}
	};
}