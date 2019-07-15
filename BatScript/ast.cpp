#include "ast.h"

namespace Bat
{
	static bool g_bIsLiteral[] =
	{
#define _(num_params, exprtype) false,
		BINARY_EXPR_TYPES(_)
		UNARY_EXPR_TYPES(_)
#undef _
#define _(num_params, exprtype) true,
		LITERAL_EXPR_TYPES(_)
#undef _
	};

	static int g_iNumParams[] = 
	{
#define _(num_params, exprtype) num_params,
		EXPRESSION_TYPES(_)
#undef _
	};

	bool Bat::Expression::IsLiteral( ExpressionType type )
	{
		return g_bIsLiteral[type];
	}
	int Expression::GetNumParams( ExpressionType type )
	{
		return g_iNumParams[type];
	}

	void ASTVisitor::Traverse( Expression* e )
	{
		switch( e->type )
		{
#define _(num_params, exprtype) case EXPR_##exprtype: Visit##exprtype( e->params[0].get(), e->params[1].get() ); Traverse( e->params[0].get() ); Traverse( e->params[1].get() ); break;
			BINARY_EXPR_TYPES(_)
#undef _
#define _(num_params, exprtype) case EXPR_##exprtype: Visit##exprtype( e->params[0].get() ); Traverse( e->params[0].get() ); break;
			UNARY_EXPR_TYPES(_)
#undef _
			case EXPR_NULL_LITERAL:  VisitNULL_LITERAL(); break;
			case EXPR_BOOL_LITERAL:  VisitBOOL_LITERAL( e->value.i64 ); break;
			case EXPR_FLOAT_LITERAL: VisitFLOAT_LITERAL( e->value.f64 ); break;
			case EXPR_INT_LITERAL:   VisitINT_LITERAL( e->value.i64 ); break;
			case EXPR_STR_LITERAL:   VisitSTR_LITERAL( e->value.str ); break;
		}
	}
}