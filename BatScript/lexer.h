#pragma once

#include <string>
#include "token.h"

namespace Bat
{
	class Lexer
	{
	public:
		Lexer() = default;
		Lexer( const std::string& text );

		void SetSource( const std::string& text );

		Token NextToken();
	private:
		std::string m_szText;
	};
}