#include "parser.h"

#include "errorsys.h"
#include "stringlib.h"

namespace Bat
{
	std::vector<std::unique_ptr<Statement>> Parser::Parse()
	{
		try
		{
			std::vector<std::unique_ptr<Statement>> statements;
			while( !AtEnd() )
			{
				statements.push_back( ParseDeclaration() );
			}
			return statements;
		}
		catch( const ParseError& )
		{
			return {};
		}
	}

	bool Parser::AtEnd() const
	{
		return Check( TOKEN_ENDOFFILE );
	}

	const Token& Parser::Peek() const
	{
		return m_Tokens[m_iCurrent];
	}

	bool Parser::Check( TokenType type ) const
	{
		return Peek().type == type;
	}

	const Token& Parser::Advance()
	{
		m_iCurrent++;
		return Previous();
	}

	const Token& Parser::Previous() const
	{
		return m_Tokens[m_iCurrent - 1];
	}

	bool Parser::Match( TokenType type )
	{
		if( Check( type ) )
		{
			Advance();
			return true;
		}
		return false;
	}

	const Token& Parser::Expect( TokenType type, const std::string& message )
	{
		if( Check( type ) )
		{
			return Advance();
		}

		Error( message );
		throw ParseError();
	}

	void Parser::ExpectTerminator()
	{
		if( !Match( TOKEN_SEMICOLON ) && !Match( TOKEN_ENDOFLINE ) && !AtEnd() )
		{
			Error( "Expected statement terminator (newline or ';')" );
			throw ParseError();
		}
	}

	void Parser::Error( const std::string& message )
	{
		auto tok = Peek();
		ErrorSys::Report( m_szSource, tok.line, tok.column, message );
	}

	void Parser::Synchronize()
	{
		Advance();
		while( !AtEnd() )
		{
			switch( Peek().type )
			{
				case TOKEN_CLASS:
				case TOKEN_STRUCT:
				case TOKEN_CONST:
				case TOKEN_VAR:
				case TOKEN_IF:
				case TOKEN_WHILE:
				case TOKEN_FOR:
				case TOKEN_PRINT:
				case TOKEN_RETURN:
				case TOKEN_ENUM:
				case TOKEN_IMPORT:
				case TOKEN_TYPEDEF:
				case TOKEN_ALIASDEF:
					return;
			}
		}
	}

	std::unique_ptr<Statement> Parser::ParseDeclaration()
	{
		try
		{
			if( Match( TOKEN_VAR ) || Match( TOKEN_CONST ) ) return ParseVarDeclaration();

			return ParseStatement();
		}
		catch( const ParseError& )
		{
			Synchronize();
			return nullptr;
		}
	}

	std::unique_ptr<Statement> Parser::ParseStatement()
	{
		if( Match( TOKEN_PRINT ) ) return ParsePrint();

		return ParseExpressionStatement();
	}

	std::unique_ptr<Statement> Parser::ParseExpressionStatement()
	{
		auto stmt = std::make_unique<ExpressionStmt>( ParseExpression() );
		ExpectTerminator();
		return stmt;
	}

	std::unique_ptr<Statement> Parser::ParsePrint()
	{
		auto stmt = std::make_unique<PrintStmt>( ParseExpression() );
		ExpectTerminator();
		return stmt;
	}

	std::unique_ptr<Statement> Parser::ParseVarDeclaration()
	{
		Token classifier = Previous();
		Token ident = Expect( TOKEN_IDENT, "Expected variable name." );

		std::unique_ptr<Expression> init = nullptr;
		if( Match( TOKEN_EQUAL ) )
		{
			init = ParseExpression();
		}

		ExpectTerminator();

		return std::make_unique<VarDecl>( classifier, ident, std::move( init ) );
	}

	std::unique_ptr<Expression> Parser::ParseExpression()
	{
		return ParseAssign();
	}

	std::unique_ptr<Expression> Parser::ParseAssign()
	{
		auto expr = ParseOr();
		while( Match( TOKEN_EQUAL ) ||
			Match( TOKEN_PLUS_EQUAL ) ||
			Match( TOKEN_MINUS_EQUAL ) ||
			Match( TOKEN_ASTERISK_EQUAL ) ||
			Match( TOKEN_SLASH_EQUAL ) ||
			Match( TOKEN_PERCENT_EQUAL ) ||
			Match( TOKEN_AMP_EQUAL ) ||
			Match( TOKEN_HAT_EQUAL ) ||
			Match( TOKEN_BAR_EQUAL ) )
		{
			Token op = Previous();
			auto right = ParseOr();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseOr()
	{
		auto expr = ParseAnd();
		while( Match( TOKEN_OR ) )
		{
			Token op = Previous();
			auto right = ParseAnd();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseAnd()
	{
		auto expr = ParseBitOr();
		while( Match( TOKEN_AND ) )
		{
			Token op = Previous();
			auto right = ParseBitOr();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseBitOr()
	{
		auto expr = ParseBitXor();
		while( Match( TOKEN_BAR ) )
		{
			Token op = Previous();
			auto right = ParseBitXor();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseBitXor()
	{
		auto expr = ParseBitAnd();
		while( Match( TOKEN_HAT ) )
		{
			Token op = Previous();
			auto right = ParseBitAnd();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseBitAnd()
	{
		auto expr = ParseEqualCompare();
		while( Match( TOKEN_AMP ) )
		{
			Token op = Previous();
			auto right = ParseEqualCompare();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseEqualCompare()
	{
		auto expr = ParseSizeCompare();
		while( Match( TOKEN_EQUAL_EQUAL ) || Match( TOKEN_EXCLMARK_EQUAL ) )
		{
			Token op = Previous();
			auto right = ParseSizeCompare();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseSizeCompare()
	{
		auto expr = ParseBitShift();
		while( Match( TOKEN_LESS ) || Match( TOKEN_LESS_EQUAL ) ||
			Match( TOKEN_GREATER ) || Match( TOKEN_GREATER_EQUAL ) )
		{
			Token op = Previous();
			auto right = ParseBitShift();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseBitShift()
	{
		auto expr = ParseAddition();
		while( Match( TOKEN_LESS_LESS ) || Match( TOKEN_GREATER_GREATER ) )
		{
			Token op = Previous();
			auto right = ParseAddition();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseAddition()
	{
		auto expr = ParseMultiplication();
		while( Match( TOKEN_PLUS ) || Match( TOKEN_MINUS ) )
		{
			Token op = Previous();
			auto right = ParseMultiplication();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseMultiplication()
	{
		auto expr = ParseUnary();
		while( Match( TOKEN_ASTERISK ) ||
			Match( TOKEN_SLASH ) ||
			Match( TOKEN_PERCENT ) )
		{
			Token op = Previous();
			auto right = ParseUnary();
			expr = std::make_unique<BinaryExpr>( op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseUnary()
	{
		if( Match( TOKEN_MINUS ) ||
			Match( TOKEN_EXCLMARK ) ||
			Match( TOKEN_TILDE ) ||
			Match( TOKEN_AMP ) )
		{
			Token op = Previous();
			auto right = ParseUnary();
			return std::make_unique<UnaryExpr>( op.type, std::move( right ) );
		}

		return ParsePrimary();
	}

	std::unique_ptr<Expression> Parser::ParsePrimary()
	{
		if( Match( TOKEN_INT ) ) return std::make_unique<IntLiteral>( Previous().literal.i64 );
		if( Match( TOKEN_FLOAT ) ) return std::make_unique<FloatLiteral>( Previous().literal.f64 );
		if( Match( TOKEN_STRING ) ) return std::make_unique<StringLiteral>( Previous().literal.str );

		if( Match( TOKEN_TRUE ) || Match( TOKEN_FALSE ) || Match( TOKEN_NIL ) )
			return std::make_unique<TokenLiteral>( Previous().type );

		if( Match( TOKEN_IDENT ) ) return std::make_unique<VarExpr>( Previous() );

		if( Match( TOKEN_LPAREN ) )
		{
			auto expr = ParseExpression();
			Expect( TOKEN_RPAREN, "Expected ')' after expression." );

			return std::make_unique<GroupExpr>( std::move( expr ) );
		}

		Error( std::string( "Unexpected '") + TokenTypeToString( Peek().type ) + "'." );
		throw ParseError();
	}
}