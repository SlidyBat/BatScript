#include "lexer.h"

namespace Bat
{
	Lexer::Lexer( const std::string& text )
	{
		SetSource( text );
	}

	void Lexer::SetSource( const std::string& text )
	{
		m_szText = text;
	}

	Token Lexer::NextToken()
	{
		Token t;
		t.lexeme = m_szText;
		t.type = TOKEN_EOF;

		return t;
	}
}