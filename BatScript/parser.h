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

		// Returns vector of statements
		std::vector<std::unique_ptr<Statement>> Parse();
	private:
		bool AtEnd() const;
		const Token& Peek() const;
		bool Check( TokenType type ) const;
		const Token& Advance();
		const Token& Previous() const;
		bool Match( TokenType type );

		const Token& Expect( TokenType type, const std::string& message );
		void ExpectTerminator();
		void Error( const std::string& message );
		// Skips to next statement to avoid cascading errors
		void Synchronize();
	private:
		// Statement parsing
		std::unique_ptr<Statement> ParseDeclaration();
		std::unique_ptr<Statement> ParseStatement();
		std::unique_ptr<Statement> ParseExpressionStatement();
		std::unique_ptr<Statement> ParsePrint();
		std::unique_ptr<Statement> ParseBlock();
		std::unique_ptr<Statement> ParseIf();
		std::unique_ptr<Statement> ParseWhile();
		std::unique_ptr<Statement> ParseFor();
		std::unique_ptr<Statement> ParseReturn();
		std::unique_ptr<Statement> ParseVarDeclaration();
		std::unique_ptr<Statement> ParseFuncDeclaration();

		// Expression parsing
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
		std::unique_ptr<Expression> ParseCall();
		std::unique_ptr<Expression> ParsePrimary();
	private:
		int m_iCurrent = 0;
		std::vector<Token> m_Tokens;
		std::string m_szSource;
	};
}