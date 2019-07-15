#include "parser.h"

#include "errorsys.h"
#include "stringlib.h"

namespace Bat
{
	std::unique_ptr<Expression> Parser::Parse()
	{
		try
		{
			return ParseExpression();
		}
		catch( const ParseError& )
		{
			return nullptr;
		}
	}

	bool Parser::AtEnd() const
	{
		return Peek().type == TOKEN_ENDOFFILE;
	}

	const Token& Parser::Peek() const
	{
		return m_Tokens[m_iCurrent];
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
		if( Peek().type == type )
		{
			Advance();
			return true;
		}
		return false;
	}

	void Parser::Error( const std::string& message )
	{
		auto tok = Peek();
		ErrorSys::Report( m_szSource, tok.line, tok.column, message );
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
			switch( op.type )
			{
				case TOKEN_EQUAL:          expr = CreateExpr_ASSIGN( std::move( expr ), std::move( right ) ); break;
				case TOKEN_PLUS_EQUAL:     expr = CreateExpr_ADD_ASSIGN( std::move( expr ), std::move( right ) ); break;
				case TOKEN_MINUS_EQUAL:    expr = CreateExpr_SUB_ASSIGN( std::move( expr ), std::move( right ) ); break;
				case TOKEN_ASTERISK_EQUAL: expr = CreateExpr_MUL_ASSIGN( std::move( expr ), std::move( right ) ); break;
				case TOKEN_SLASH_EQUAL:    expr = CreateExpr_DIV_ASSIGN( std::move( expr ), std::move( right ) ); break;
				case TOKEN_PERCENT_EQUAL:  expr = CreateExpr_MOD_ASSIGN( std::move( expr ), std::move( right ) ); break;
				case TOKEN_AMP_EQUAL:      expr = CreateExpr_BITAND_ASSIGN( std::move( expr ), std::move( right ) ); break;
				case TOKEN_HAT_EQUAL:      expr = CreateExpr_BITXOR_ASSIGN( std::move( expr ), std::move( right ) ); break;
				case TOKEN_BAR_EQUAL:      expr = CreateExpr_BITOR_ASSIGN( std::move( expr ), std::move( right ) ); break;
			}
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
			expr = CreateExpr_OR( std::move( expr ), std::move( right ) );
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
			expr = CreateExpr_AND( std::move( expr ), std::move( right ) );
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
			expr = CreateExpr_BITOR( std::move( expr ), std::move( right ) );
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
			expr = CreateExpr_BITXOR( std::move( expr ), std::move( right ) );
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
			expr = CreateExpr_BITAND( std::move( expr ), std::move( right ) );
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
			switch( op.type )
			{
				case TOKEN_EQUAL_EQUAL:    expr = CreateExpr_CMPEQ( std::move( expr ), std::move( right ) ); break;
				case TOKEN_EXCLMARK_EQUAL: expr = CreateExpr_CMPNEQ( std::move( expr ), std::move( right ) ); break;
			}
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
			switch( op.type )
			{
				case TOKEN_LESS:          expr = CreateExpr_CMPL( std::move( expr ), std::move( right ) ); break;
				case TOKEN_LESS_EQUAL:    expr = CreateExpr_CMPLE( std::move( expr ), std::move( right ) ); break;
				case TOKEN_GREATER:       expr = CreateExpr_CMPG( std::move( expr ), std::move( right ) ); break;
				case TOKEN_GREATER_EQUAL: expr = CreateExpr_CMPGE( std::move( expr ), std::move( right ) ); break;
			}
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
			switch( op.type )
			{
				case TOKEN_LESS_LESS:       expr = CreateExpr_LBITSHIFT( std::move( expr ), std::move( right ) ); break;
				case TOKEN_GREATER_GREATER: expr = CreateExpr_RBITSHIFT( std::move( expr ), std::move( right ) ); break;
			}
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
			switch( op.type )
			{
				case TOKEN_PLUS:  expr = CreateExpr_ADD( std::move( expr ), std::move( right ) ); break;
				case TOKEN_MINUS: expr = CreateExpr_SUB( std::move( expr ), std::move( right ) ); break;
			}
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
			switch( op.type )
			{
				case TOKEN_ASTERISK:  expr = CreateExpr_MUL( std::move( expr ), std::move( right ) ); break;
				case TOKEN_SLASH:     expr = CreateExpr_DIV( std::move( expr ), std::move( right ) ); break;
				case TOKEN_PERCENT:   expr = CreateExpr_MOD( std::move( expr ), std::move( right ) ); break;
			}
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseUnary()
	{
		if( Match( TOKEN_PLUS ) ||
			Match( TOKEN_MINUS ) ||
			Match( TOKEN_EXCLMARK ) ||
			Match( TOKEN_TILDE ) ||
			Match( TOKEN_AMP ) ||
			Match( TOKEN_PRINT ) )
		{
			Token op = Previous();
			auto right = ParseUnary();
			switch( op.type )
			{
				case TOKEN_PLUS:     return std::move( right ); // TODO: Support this? Error?
				case TOKEN_MINUS:    return CreateExpr_NEG( std::move( right ) );
				case TOKEN_EXCLMARK: return CreateExpr_NOT( std::move( right ) );
				case TOKEN_TILDE:    return CreateExpr_BITNEG( std::move( right ) );
				case TOKEN_AMP:      return CreateExpr_MOVE( std::move( right ) );
				case TOKEN_PRINT:    return CreateExpr_PRINT( std::move( right ) );
			}
		}

		return ParsePrimary();
	}

	std::unique_ptr<Expression> Parser::ParsePrimary()
	{
		if( Match( TOKEN_INT ) ) return LiteralExpr::Int( Previous().literal.i64 );
		if( Match( TOKEN_FLOAT ) ) return LiteralExpr::Float( Previous().literal.f64 );
		if( Match( TOKEN_STRING ) ) return LiteralExpr::String( Previous().literal.str );
		if( Match( TOKEN_TRUE ) ) return LiteralExpr::Bool( true );
		if( Match( TOKEN_FALSE ) ) return LiteralExpr::Bool( false );
		if( Match( TOKEN_NIL ) ) return LiteralExpr::Null();

		if( Match( TOKEN_LPAREN ) )
		{
			auto expr = ParseExpression();
			if( !Match( TOKEN_RPAREN ) )
			{
				Error( "Expected ')' after expression." );
			}

			return CreateExpr_GROUP( std::move( expr ) );
		}

		Error( std::string( "Unexpected '") + TokenTypeToString( Peek().type ) + "'." );
		throw ParseError();
	}
}