#include "token.h"

#include <unordered_map>

namespace Bat
{
	static const char* g_szTokenNames[] =
	{
	#define _(name, str) str,
		TOKEN_TYPES( _ )
	#undef _
	};

	const char* TokenTypeToString( TokenType type )
	{
		return g_szTokenNames[type];
	}

	TokenType KeywordStringToType( const std::string& keyword_str )
	{
		static bool initialized = false;
		static std::unordered_map<std::string, TokenType> keyword_map;
		if( !initialized )
		{
#define _(type, name) keyword_map[name] = TOKEN_##type;
			KEYWORD_TYPES( _ );
#undef _
			initialized = true;
		}

		auto it = keyword_map.find( keyword_str );
		if( it == keyword_map.end() )
		{
			return TOKEN_NONE;
		}

		return it->second;
	}

	Token::Token( TokenType type, const std::string& lexeme, int line, int column )
		:
		type( type ),
		lexeme( lexeme ),
		loc( line, column )
	{}

	Token::Token( double f64, const std::string & lexeme, int line, int column )
		:
		type( TOKEN_FLOAT ),
		lexeme( lexeme ),
		loc( line, column )
	{
		literal.f64 = f64;
	}

	Token::Token( int64_t i64, const std::string& lexeme, int line, int column )
		:
		type( TOKEN_INT ),
		lexeme( lexeme ),
		loc( line, column )
	{
		literal.i64 = i64;
	}

	Token::Token( const char* str, const std::string& lexeme, int line, int column )
		:
		type( TOKEN_STRING ),
		lexeme( lexeme ),
		loc( line, column )
	{
		literal.str = str;
	}
}