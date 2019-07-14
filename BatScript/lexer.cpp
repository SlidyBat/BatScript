#include "lexer.h"

#include "stringlib.h"
#include "stringpool.h"
#include "errorsys.h"

namespace Bat
{
	Lexer::Lexer( const std::string& text, const std::string& source )
		:
		m_szText( text ),
		m_szSource( source )
	{}

	std::vector<Token> Lexer::Scan()
	{
		while( !AtEnd() )
		{
			m_iStart = m_iCurrent;
			ScanToken();
		}

		m_Tokens.emplace_back( TOKEN_ENDOFFILE, "", m_iLine, 0 );
		return m_Tokens;
	}

	std::string Lexer::GetCurrLexeme() const
	{
		return m_szText.substr( m_iStart, m_iCurrent - m_iStart );
	}

	int Lexer::GetCurrColumn() const
	{
		return m_iStart - m_iLineStart + 1;
	}

	char Lexer::Peek() const
	{
		return AtEnd() ? '\0' : m_szText[m_iCurrent];
	}

	char Lexer::PeekNext() const
	{
		if( m_iCurrent + 1 >= m_szText.length() )
		{
			return '\0';
		}
		return m_szText[m_iCurrent + 1];
	}

	char Lexer::Advance()
	{
		m_iCurrent++;
		return m_szText[m_iCurrent - 1];
	}

	bool Lexer::Match( char c )
	{
		if( Peek() == c )
		{
			Advance();
			return true;
		}

		return false;
	}

	void Lexer::ScanToken()
	{
		char c = Advance();
		switch( c )
		{
			case '(': AddToken( TOKEN_LPAREN ); break;
			case ')': AddToken( TOKEN_RPAREN ); break;
			case '{': AddToken( TOKEN_LBRACE ); break;
			case '}': AddToken( TOKEN_RBRACE ); break;
			case '[': AddToken( TOKEN_LPAREN ); break;
			case ']': AddToken( TOKEN_RPAREN ); break;
			case '.': AddToken( TOKEN_DOT ); break;
			case ',': AddToken( TOKEN_COMMA ); break;
			case '?': AddToken( TOKEN_QUESMARK ); break;
			case ';': AddToken( TOKEN_SEMICOLON ); break;
			case ':': AddToken( TOKEN_COLON ); break;
			case '@': AddToken( TOKEN_AT ); break;
			case '~': AddToken( TOKEN_TILDE ); break;
			case '\\': AddToken( TOKEN_BACKSLASH ); break;

			case '+': Match( '=' ) ? AddToken( TOKEN_PLUS_EQUAL ) : AddToken( TOKEN_PLUS ); break;
			case '-': Match( '=' ) ? AddToken( TOKEN_MINUS_EQUAL ) : AddToken( TOKEN_MINUS ); break;
			case '*': Match( '=' ) ? AddToken( TOKEN_ASTERISK_EQUAL ) : AddToken( TOKEN_ASTERISK ); break;
			case '=': Match( '=' ) ? AddToken( TOKEN_EQUAL_EQUAL ) : AddToken( TOKEN_EQUAL ); break;
			case '!': Match( '=' ) ? AddToken( TOKEN_EXCLMARK_EQUAL ) : AddToken( TOKEN_EXCLMARK ); break;
			case '>': Match( '=' ) ? AddToken( TOKEN_GREATER_EQUAL ) : AddToken( TOKEN_GREATER ); break;
			case '<': Match( '=' ) ? AddToken( TOKEN_LESS_EQUAL ) : AddToken( TOKEN_LESS ); break;
			case '%': Match( '=' ) ? AddToken( TOKEN_PERCENT_EQUAL ) : AddToken( TOKEN_PERCENT ); break;
			case '|': Match( '=' ) ? AddToken( TOKEN_BAR_EQUAL ) : AddToken( TOKEN_BAR ); break;
			case '&': Match( '=' ) ? AddToken( TOKEN_AMP_EQUAL ) : AddToken( TOKEN_AMP ); break;
			case '^': Match( '=' ) ? AddToken( TOKEN_HAT_EQUAL ) : AddToken( TOKEN_HAT ); break;

			case '/':
				if( Match( '=' ) )      AddToken( TOKEN_SLASH_EQUAL );
				else if( Match( '/' ) ) Comment();
				else                    AddToken( TOKEN_SLASH );

			case '"': String( '"' ); break;
			case '\'': String('\''); break;

			case ' ':
			case '\t':
			case '\r':
				break;

			case '\n':
				AddToken( TOKEN_ENDOFLINE );
				m_iLine++;
				m_iLineStart = m_iCurrent + 1;
				break;

			default:
				if( IsNumeric( c ) )
				{
					Number();
				}
				else if( IsIdentifier( c ) )
				{
					Identifier();
				}
				else
				{
					Error( std::string( "Unexpected character '" ) + c + "'." );
				}
		}
	}

	void Lexer::AddToken( TokenType t )
	{
		m_Tokens.emplace_back( t, GetCurrLexeme(), m_iLine, GetCurrColumn() );
	}

	bool Lexer::AtEnd() const
	{
		return m_iCurrent >= m_szText.length();
	}

	void Lexer::Error( const std::string& message )
	{
		ErrorSys::Report( m_szSource, m_iLine, GetCurrColumn(), message );
	}

	void Lexer::Comment()
	{
		while( !AtEnd() && Peek() != '\n' )
		{
			Advance();
		}

		AddToken( TOKEN_COMMENT );
	}

	void Lexer::Number()
	{
		bool is_float = false;
		while( !AtEnd() && IsNumeric( Peek() ) )
		{
			Advance();
		}

		if( Peek() == '.' && IsNumeric( PeekNext() ) )
		{
			Advance();
			is_float = true;
		}

		if( is_float )
		{
			while( !AtEnd() && IsNumeric( Peek() ) )
			{
				Advance();
			}
		}

		std::string l = GetCurrLexeme();

		if( is_float )
		{
			double d = atof( l.c_str() );
			m_Tokens.emplace_back( d, l, m_iLine, GetCurrColumn() );
		}
		else
		{
			int64_t i = _atoi64( l.c_str() );
			m_Tokens.emplace_back( i, l, m_iLine, GetCurrColumn() );
		}
	}

	void Lexer::Identifier()
	{
		while( !AtEnd() && IsIdentifier( Peek() ) )
		{
			Advance();
		}

		std::string ident = GetCurrLexeme();
		TokenType keyword_type = KeywordStringToType( ident );
		if( keyword_type != TOKEN_NONE )
		{
			AddToken( keyword_type );
		}
		else
		{
			AddToken( TOKEN_IDENT );
		}
	}

	void Lexer::String(char quote)
	{
		while( !AtEnd() && Peek() != quote )
		{
			if( Peek() == '\n' )
			{
				m_iLine++;
			}
			Advance();
		}

		if( AtEnd() )
		{
			Error( "Unterminated string." );
			return;
		}

		Advance(); // Eat ending quote

		std::string lexeme = m_szText.substr( m_iStart, m_iCurrent - m_iStart );
		const char* str = stringpool.AddString( m_szText.substr( m_iStart + 1, m_iCurrent - m_iStart - 1 ) );

		m_Tokens.emplace_back( str, lexeme, m_iLine, GetCurrColumn() );
	}
}