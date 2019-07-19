#pragma once

#include <string>
#include <vector>
#include "token.h"

namespace Bat
{
	class Lexer
	{
	public:
		Lexer( const std::string& text );

		std::vector<Token> Scan();
	private:
		std::string GetCurrLexeme() const;
		int GetCurrColumn() const;

		char Peek() const;
		char PeekNext() const;
		char Advance();
		bool Match( char c );
		void GoBack();
		void ScanToken();

		void AddToken( TokenType t );

		bool AtEnd() const;

		void Error( const std::string& message );
	private:
		void OpeningParen( char paren );
		void ClosingParen( char paren );

		void Comment();
		void Number();
		void Identifier();
		void String(char quote);
	private:
		std::vector<Token> m_Tokens;
		std::string m_szText;

		int m_iStart = 0;
		int m_iCurrent = 0;
		int m_iLine = 1;
		int m_iLineStart = 0;

		static constexpr int MAX_INDENT_LEVEL = 1024;
		static constexpr int MAX_PAREN_LEVEL = 256;
		static constexpr int TAB_SIZE = 8;
		bool m_bBeginningOfLine = true;
		int m_iIndentStack[MAX_INDENT_LEVEL];
		int m_iCurrentIndent = 0;
		char m_iParenStack[MAX_PAREN_LEVEL];
		int m_iParenLevel = 0;
	};
}