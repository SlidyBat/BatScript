#pragma once

#include <vector>
#include "token.h"
#include "ast.h"

namespace Bat
{
	class ParseError : public std::exception
	{};

	class Parser
	{
	public:
		Parser( std::vector<Token> tokens, const std::string& source = "" )
			:
			m_Tokens( std::move( tokens ) ),
			m_szSource( source )
		{}

		std::unique_ptr<Expression> Parse();
	private:
		bool AtEnd() const;
		const Token& Peek() const;
		const Token& Advance();
		const Token& Previous() const;
		bool Match( TokenType type );

		void Error( const std::string& message );
	private:
		std::unique_ptr<Expression> ParseExpression();
		std::unique_ptr<Expression> ParseAssign();
		std::unique_ptr<Expression> ParseOr();
		std::unique_ptr<Expression> ParseAnd();
		std::unique_ptr<Expression> ParseBitOr();
		std::unique_ptr<Expression> ParseBitXor();
		std::unique_ptr<Expression> ParseBitAnd();
		std::unique_ptr<Expression> ParseEqualCompare();
		std::unique_ptr<Expression> ParseSizeCompare();
		std::unique_ptr<Expression> ParseBitShift();
		std::unique_ptr<Expression> ParseAddition();
		std::unique_ptr<Expression> ParseMultiplication();
		std::unique_ptr<Expression> ParseUnary();
		std::unique_ptr<Expression> ParsePrimary();
	private:
		int m_iCurrent = 0;
		std::vector<Token> m_Tokens;
		std::string m_szSource;
	};
}