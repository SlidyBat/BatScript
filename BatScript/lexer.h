#pragma once

#include <string>
#include <vector>
#include "token.h"

namespace Bat
{
	class Lexer
	{
	public:
		// `text` is the text to scan
		// `source` is the source of the text (e.g. the filename), used in error messages
		Lexer( const std::string& text, const std::string& source = "" );

		std::vector<Token> Scan();
	private:
		std::string GetCurrLexeme() const;
		int GetCurrColumn() const;

		char Peek() const;
		char PeekNext() const;
		char Advance();
		bool Match( char c );
		void ScanToken();

		void AddToken( TokenType t );

		bool AtEnd() const;

		void Error( const std::string& message );
	private:
		void Comment();
		void Number();
		void Identifier();
		void String(char quote);
	private:
		std::vector<Token> m_Tokens;
		std::string m_szSource;
		std::string m_szText;

		size_t m_iStart = 0;
		size_t m_iCurrent = 0;
		size_t m_iLine = 1;
		size_t m_iLineStart = 0;
	};
}