#include "lexer.h"

#include "stringlib.h"
#include "stringpool.h"
#include "errorsys.h"

namespace Bat
{
	Lexer::Lexer( const std::string& text )
		:
		m_szText( text )
	{}

	std::vector<Token> Lexer::Scan()
	{
		while( !AtEnd() )
		{
			m_iStart = m_iCurrent;
			ScanToken();
		}

		m_Tokens.emplace_back( TOKEN_ENDOFFILE, "", m_iLine, 0 );

		if( m_iParenLevel != 0 )
		{
			ErrorSys::Report( /*m_iParenLine[m_iParenLevel], m_iParenCol[m_iParenLevel],*/ 0, 0, std::string( "No closing parentheses found for '" ) + m_iParenStack[m_iParenLevel] + "'" );
		}

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

	void Lexer::GoBack()
	{
		m_iCurrent--;
	}

	void Lexer::ScanToken()
	{
		char c = 0;
		bool blankline = false;
		if( m_bBeginningOfLine )
		{
			int col = 0;
			m_bBeginningOfLine = false;

			while( true )
			{
				c = Advance();
				if( c == ' ' )
				{
					col++;
				}
				else if( c == '\t' )
				{
					col = (col / TAB_SIZE + 1) * TAB_SIZE;
				}
				else
				{
					break;
				}
			}

			GoBack();

			if( (c == '/' && Match( '/' )) )
			{
				Comment();
				c = Peek();
			}
			if( c == '\n' )
			{
				m_bBeginningOfLine = true;
				m_iLine++;
				m_iLineStart = m_iCurrent + 1;

				blankline = true;
			}

			if( !blankline && m_iParenLevel == 0 )
			{
				if( col > m_iIndentStack[m_iCurrentIndent] )
				{
					m_iCurrentIndent++;
					m_iIndentStack[m_iCurrentIndent] = col;
					AddToken( TOKEN_INDENT );
				}
				else if( col < m_iIndentStack[m_iCurrentIndent] )
				{
					while( m_iCurrentIndent > 0 &&
						col < m_iIndentStack[m_iCurrentIndent] )
					{
						AddToken( TOKEN_DEDENT );
						m_iCurrentIndent--;
					}

					if( col != m_iIndentStack[m_iCurrentIndent] )
					{
						Error( "Inconsistent indentation" );
						return;
					}
				}
			}

			if( blankline || AtEnd() )
			{
				if( blankline ) Advance();
				return;
			}
		}

		m_iStart = m_iCurrent;

		c = Advance();

		switch( c )
		{
			case '(':
			case '[':
			case '{':
				OpeningParen( c );
				break;
			case ')':
			case ']':
			case '}':
				ClosingParen( c );
				break;

			case ',':  AddToken( TOKEN_COMMA ); break;
			case '?':  AddToken( TOKEN_QUESMARK ); break;
			case ';':  AddToken( TOKEN_SEMICOLON ); break;
			case ':':  AddToken( TOKEN_COLON ); break;
			case '@':  AddToken( TOKEN_AT ); break;
			case '~':  AddToken( TOKEN_TILDE ); break;
			case '\\': AddToken( TOKEN_BACKSLASH ); break;

			case '+': Match( '=' ) ? AddToken( TOKEN_PLUS_EQUAL )     : AddToken( TOKEN_PLUS ); break;
			case '-': Match( '=' ) ? AddToken( TOKEN_MINUS_EQUAL )    : AddToken( TOKEN_MINUS ); break;
			case '*': Match( '=' ) ? AddToken( TOKEN_ASTERISK_EQUAL ) : AddToken( TOKEN_ASTERISK ); break;
			case '=': Match( '=' ) ? AddToken( TOKEN_EQUAL_EQUAL )    : AddToken( TOKEN_EQUAL ); break;
			case '!': Match( '=' ) ? AddToken( TOKEN_EXCLMARK_EQUAL ) : AddToken( TOKEN_EXCLMARK ); break;
			case '%': Match( '=' ) ? AddToken( TOKEN_PERCENT_EQUAL )  : AddToken( TOKEN_PERCENT ); break;
			case '|': Match( '=' ) ? AddToken( TOKEN_BAR_EQUAL )      : AddToken( TOKEN_BAR ); break;
			case '&': Match( '=' ) ? AddToken( TOKEN_AMP_EQUAL )      : AddToken( TOKEN_AMP ); break;
			case '^': Match( '=' ) ? AddToken( TOKEN_HAT_EQUAL )      : AddToken( TOKEN_HAT ); break;

			case '>':
				if( Match( '=' ) )      AddToken( TOKEN_GREATER_EQUAL );
				else if( Match( '>' ) ) AddToken( TOKEN_GREATER_GREATER );
				else                    AddToken( TOKEN_GREATER );
				break;
			case '<':
				if( Match( '=' ) )      AddToken( TOKEN_LESS_EQUAL );
				else if( Match( '<' ) ) AddToken( TOKEN_LESS_LESS );
				else                    AddToken( TOKEN_LESS );
				break;
			case '/':
				if( Match( '=' ) )      AddToken( TOKEN_SLASH_EQUAL );
				else if( Match( '/' ) ) Comment();
				else                    AddToken( TOKEN_SLASH );
				break;

			case '.':
				if( Match( '.' ) )
				{
					if( Match( '.' ) )
					{
						AddToken( TOKEN_ELLIPSIS );
					}
					else
					{
						AddToken( TOKEN_DOT_DOT );
					}
				}
				else
				{
					AddToken( TOKEN_DOT );
				}
				break;

			case '"':  String( '"' ); break;
			case '\'': String('\''); break;

			case ' ':
			case '\t':
			case '\r':
				break;

			case '\n':
				if( m_iParenLevel == 0 )
				{
					AddToken( TOKEN_ENDOFLINE );
				}
				m_bBeginningOfLine = true;
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
		ErrorSys::Report( m_iLine, GetCurrColumn(), message );
	}

	void Lexer::OpeningParen( char paren )
	{
		if( m_iParenLevel >= MAX_PAREN_LEVEL )
		{
			Error( "Too many nested parentheses" );
			return;
		}

		m_iParenLevel++;
		m_iParenStack[m_iParenLevel] = paren;

		switch( paren )
		{
			case '(': AddToken( TOKEN_LPAREN ); break;
			case '[': AddToken( TOKEN_LBRACKET ); break;
			case '{': AddToken( TOKEN_LBRACE ); break;
		}
	}

	void Lexer::ClosingParen( char paren )
	{
		if( m_iParenLevel == 0 )
		{
			Error( std::string( "Unmatched closing parentheses '" ) + paren + "'" );
			return;
		}
		if( (paren == ')' && m_iParenStack[m_iParenLevel] != '(') ||
			(paren == ']' && m_iParenStack[m_iParenLevel] != '[') ||
			(paren == '}' && m_iParenStack[m_iParenLevel] != '{') )
		{
			Error( std::string( "Closing parentheses '" ) + paren + " do not match opening parentheses '" + m_iParenStack[m_iParenLevel] + "'" );
			return;
		}

		m_iParenLevel--;

		switch( paren )
		{
			case ')': AddToken( TOKEN_RPAREN ); break;
			case ']': AddToken( TOKEN_RBRACKET ); break;
			case '}': AddToken( TOKEN_RBRACE ); break;
		}
	}

	void Lexer::Comment()
	{
		while( !AtEnd() && Peek() != '\n' )
		{
			Advance();
		}

		// AddToken( TOKEN_COMMENT );
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
				m_iLineStart = m_iCurrent + 1;
			}
			Advance();
		}

		if( AtEnd() )
		{
			Error( "Unterminated string." );
			return;
		}

		Advance(); // Eat ending quote

		std::string lexeme = GetCurrLexeme();
		const char* str = stringpool.AddString( m_szText.substr( m_iStart + 1, m_iCurrent - m_iStart - 2 ) );

		m_Tokens.emplace_back( str, lexeme, m_iLine, GetCurrColumn() );
	}
}