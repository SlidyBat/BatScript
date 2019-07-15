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
}