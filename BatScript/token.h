#pragma once

#include <string>

#define TOKEN_TYPES(_) \
	_(TOKEN_IDENT) \
	_(TOKEN_EOF) \

namespace Bat
{
	enum TokenType
	{
#define _(token) token,
		TOKEN_TYPES( _ )
#undef _
	};

	struct Token
	{
		TokenType type;
		std::string lexeme;
		union
		{
			int64_t i64;
			double f64;
		};
	};
}