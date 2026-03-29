#include "parser.h"

#include "errorsys.h"
#include "stringlib.h"

namespace Bat
{
	std::vector<std::unique_ptr<Statement>> Parser::Parse()
	{
		std::vector<std::unique_ptr<Statement>> statements;
		while( !AtEnd() )
		{
			auto stmt = ParseStatement();
			if( stmt != nullptr )
			{
				statements.push_back( std::move( stmt ) );
			}
		}
		return statements;
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

	void Parser::GoBack()
	{
		m_iCurrent--;
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

	TypeSpecifier Parser::ExpectType( const std::string& message )
	{
		if( Check( TOKEN_VOID ) )
		{
			// void is special case, can't have any type modifiers on void
			Token type_name = Advance();
			TypeSpecifier t( type_name );

			return t;
		}

		if( Check( TOKEN_BOOL ) ||
			Check( TOKEN_INT ) ||
			Check( TOKEN_FLOAT ) ||
			Check( TOKEN_STRING ) ||
			Check( TOKEN_IDENT ) )
		{
			Token type_name = Advance();
			TypeSpecifier t( type_name );

			size_t rank = 0;
			while( Match( TOKEN_LBRACKET ) )
			{
				rank++;
				Expect( TOKEN_RBRACKET, "Expected closing bracket in array declaration" );
			}

			t.SetRank( rank );

			return t;
		}

		Error( message );
		throw ParseError();
	}

	bool Parser::CheckTerminator()
	{
		return AtEnd() || Check( TOKEN_ENDOFLINE );
	}

	void Parser::ExpectTerminator( const std::string& msg )
	{
		if( !Match( TOKEN_ENDOFLINE ) && !AtEnd() )
		{
			Error( msg );
			throw ParseError();
		}
	}

	void Parser::Error( const std::string& message )
	{
		auto tok = Peek();
		ErrorSys::Report( tok.loc.Line(), tok.loc.Column(), message );
	}

	void Parser::Synchronize()
	{
		while( !AtEnd() )
		{
			Advance();
			switch( Peek().type )
			{
				case TOKEN_CLASS:
				case TOKEN_STRUCT:
				case TOKEN_CONST:
				case TOKEN_IF:
				case TOKEN_WHILE:
				case TOKEN_FOR:
				case TOKEN_PRINT:
				case TOKEN_RETURN:
				case TOKEN_ENUM:
				case TOKEN_IMPORT:
				case TOKEN_NATIVE:
				case TOKEN_TYPEDEF:
				case TOKEN_ALIASDEF:
					return;
				
				default:
					break;
			}
		}
	}

	std::unique_ptr<Statement> Parser::ParseStatement()
	{
		try
		{
			if( Check( TOKEN_DEF ) ||
				Check( TOKEN_IF ) ||
				Check( TOKEN_WHILE ) ||
				Check( TOKEN_FOR ) ||
				Check( TOKEN_CLASS ) )
			{
				return ParseCompoundStatement();
			}

			return ParseSimpleStatement();
		}
		catch( const ParseError& )
		{
			Synchronize();
			return nullptr;
		}
	}

	std::unique_ptr<Statement> Parser::ParseCompoundStatement()
	{
		if( Match( TOKEN_IF ) )     return ParseIf();
		if( Match( TOKEN_WHILE ) )  return ParseWhile();
		if( Match( TOKEN_FOR ) )    return ParseFor();
		if( Match( TOKEN_DEF ) )    return ParseFuncDeclaration();
		if( Match( TOKEN_CLASS ) )  return ParseClassDeclaration();

		assert( false && "Unexpected token" );
		return nullptr;
	}

	std::unique_ptr<Statement> Parser::ParseSimpleStatement()
	{
		if( Match( TOKEN_PRINT ) )    return ParsePrint();
		if( Match( TOKEN_RETURN ) )   return ParseReturn();
		if( Match( TOKEN_BREAK ) )    return ParseBreak();
		if( Match( TOKEN_CONTINUE ) ) return ParseContinue();
		if( Match( TOKEN_IMPORT ) )   return ParseImportAs();
		if( Match( TOKEN_FROM ) )     return ParseFromImport();
		if( Match( TOKEN_NATIVE ) )   return ParseNative();

		return ParseAssign();
	}

	std::unique_ptr<Statement> Parser::ParseAssign()
	{
		SourceLoc loc = Peek().loc;

		if( Match( TOKEN_IDENT ) )
		{
			if( Check( TOKEN_COLON ) || Check( TOKEN_COLON_EQUAL ) )
			{
				GoBack();
				return ParseVarDeclaration();
			}
			GoBack();
		}

		auto expr = ParseExpression();
		if( Match( TOKEN_EQUAL )
			|| Match( TOKEN_PLUS_EQUAL )
			|| Match( TOKEN_MINUS_EQUAL )
			|| Match( TOKEN_ASTERISK_EQUAL )
			|| Match( TOKEN_SLASH_EQUAL )
			|| Match( TOKEN_PERCENT_EQUAL )
			|| Match( TOKEN_AMP_EQUAL )
			|| Match( TOKEN_HAT_EQUAL )
			|| Match( TOKEN_BAR_EQUAL ) )
		{
			Token op = Previous();
			auto right = ParseExpression();
			ExpectTerminator();
			return std::make_unique<AssignStmt>( loc, std::move( expr ), op.type, std::move( right ) );
		}

		ExpectTerminator();
		return std::make_unique<ExpressionStmt>( loc, std::move( expr ) );
	}

	std::unique_ptr<PrintStmt> Parser::ParsePrint()
	{
		SourceLoc loc = Previous().loc;

		auto stmt = std::make_unique<PrintStmt>( loc, ParseExpression() );
		ExpectTerminator();
		return stmt;
	}

	std::unique_ptr<BlockStmt> Parser::ParseBlock()
	{
		SourceLoc loc = Previous().loc;

		Expect( TOKEN_INDENT, "Expected indent" );

		std::vector<std::unique_ptr<Statement>> statements;
		while( !Check( TOKEN_DEDENT ) && !AtEnd() )
		{
			statements.push_back( ParseStatement() );
		}

		if( !AtEnd() )
		{
			Expect( TOKEN_DEDENT, "Expected dedent after block" );
		}

		return std::make_unique<BlockStmt>( loc, std::move( statements ) );
	}

	std::unique_ptr<IfStmt> Parser::ParseIf()
	{
		SourceLoc loc = Previous().loc;

		auto condition = ParseExpression();
		Expect( TOKEN_COLON, "Expected ':' after if statement" );
		std::unique_ptr<Statement> then_branch;
		if( Match( TOKEN_ENDOFLINE ) )
		{
			then_branch = ParseBlock();
		}
		else
		{
			then_branch = ParseSimpleStatement();
		}

		std::unique_ptr<Statement> else_branch = nullptr;
		if( Match( TOKEN_ELSE ) )
		{
			Expect( TOKEN_COLON, "Expected ':' after else statement" );

			if( Match( TOKEN_ENDOFLINE ) )
			{
				else_branch = ParseBlock();
			}
			else
			{
				else_branch = ParseSimpleStatement();
			}
		}

		return std::make_unique<IfStmt>( loc, std::move( condition ), std::move( then_branch ), std::move( else_branch ) );
	}

	std::unique_ptr<WhileStmt> Parser::ParseWhile()
	{
		SourceLoc loc = Previous().loc;

		auto condition = ParseExpression();
		Expect( TOKEN_COLON, "Expected ':' after while statement" );
		std::unique_ptr<Statement> body;
		if( Match( TOKEN_ENDOFLINE ) )
		{
			body = ParseBlock();
		}
		else
		{
			body = ParseSimpleStatement();
		}

		return std::make_unique<WhileStmt>( loc, std::move( condition ), std::move( body ) );
	}

	std::unique_ptr<ForStmt> Parser::ParseFor()
	{
		SourceLoc loc = Previous().loc;

		Token variable = Expect( TOKEN_IDENT, "Expected identifier" );

		Expect( TOKEN_IN, "Expected 'in'" );
		
		auto iterable = ParseExpression();
		Expect( TOKEN_COLON, "Expected ':' after for statement" );

		std::unique_ptr<Statement> body;
		if( Match( TOKEN_ENDOFLINE ) )
		{
			body = ParseBlock();
		}
		else
		{
			body = ParseSimpleStatement();
		}

		return std::make_unique<ForStmt>( loc, variable, std::move( iterable ), std::move( body ) );
	}

	std::unique_ptr<ReturnStmt> Parser::ParseReturn()
	{
		SourceLoc loc = Previous().loc;

		std::unique_ptr<Expression> ret_value = nullptr;
		if( !CheckTerminator() )
		{
			ret_value = ParseExpression();
		}
		ExpectTerminator();
		return std::make_unique<ReturnStmt>( loc, std::move( ret_value ) );
	}

	std::unique_ptr<BreakStmt> Parser::ParseBreak()
	{
		SourceLoc loc = Previous().loc;

		ExpectTerminator();
		return std::make_unique<BreakStmt>( loc );
	}

	std::unique_ptr<ContinueStmt> Parser::ParseContinue()
	{
		SourceLoc loc = Previous().loc;

		ExpectTerminator();
		return std::make_unique<ContinueStmt>( loc );
	}

	std::unique_ptr<ImportStmt> Parser::ParseImportAs()
	{
		SourceLoc loc = Previous().loc;

		Token module_name = Expect( TOKEN_IDENT, "Expected module name to import" );

		Token as = module_name;
		if( Match( TOKEN_AS ) )
		{
			as = Expect( TOKEN_IDENT, "Expected identifier" );
		}

		ExpectTerminator();
		return std::make_unique<ImportStmt>( loc, std::move( module_name ), std::move( as ) );
	}

	std::unique_ptr<ImportStmt> Parser::ParseFromImport()
	{
		SourceLoc loc = Previous().loc;

		Token module_name = Expect( TOKEN_IDENT, "Expected module name to import from" );
		Token as = module_name;

		Expect( TOKEN_IMPORT, "Expected 'import' keyword" );

		std::vector<Token> import_list;
		if( Match( TOKEN_ASTERISK ) )
		{
			import_list.emplace_back( Previous() );
		}
		else
		{
			do
			{
				import_list.emplace_back( Expect( TOKEN_IDENT, "Expected identifier" ) );
			}
			while( Match( TOKEN_COMMA ) );
		}

		ExpectTerminator();
		return std::make_unique<ImportStmt>( loc, std::move( module_name ), std::move( as ), std::move( import_list ) );
	}

	std::unique_ptr<NativeStmt> Parser::ParseNative()
	{
		SourceLoc loc = Previous().loc;

		FunctionSignature sig = ParseFuncSignature();
		ExpectTerminator();
		return std::make_unique<NativeStmt>( loc, std::move( sig ) );
	}

	std::unique_ptr<VarDecl> Parser::ParseVarDeclaration()
	{
		SourceLoc loc = Peek().loc;

		Token ident = Expect( TOKEN_IDENT, "Expected variable name" );

		TypeSpecifier type_name;
		std::unique_ptr<Expression> init = nullptr;
		if( Match( TOKEN_COLON ) )
		{
			type_name = ExpectType( "Expected variable type" );
			if( Match( TOKEN_EQUAL ) )
			{
				init = ParseExpressions();
			}
		}
		else if( Match( TOKEN_COLON_EQUAL ) )
		{
			init = ParseExpressions();
		}

		ExpectTerminator();

		return std::make_unique<VarDecl>( loc, std::move( type_name ), ident, std::move( init ) );
	}

	std::unique_ptr<FuncDecl> Parser::ParseFuncDeclaration()
	{
		SourceLoc loc = Previous().loc;

		FunctionSignature sig = ParseFuncSignature();
		Expect( TOKEN_COLON, "Expected ':' after function declaration" );

		std::unique_ptr<Statement> body;
		if( Match( TOKEN_ENDOFLINE ) )
		{
			body = ParseBlock();
		}
		else
		{
			body = ParseSimpleStatement();
		}

		return std::make_unique<FuncDecl>( loc, std::move( sig ), std::move( body ) );
	}

	std::unique_ptr<ClassDecl> Parser::ParseClassDeclaration()
	{
		SourceLoc loc = Previous().loc;

		Token name = Expect( TOKEN_IDENT, "Expected class name" );
		Expect( TOKEN_COLON, "Expected ':' after class declaration" );
		Expect( TOKEN_ENDOFLINE, "Expected new line after class declaration" );
		Expect( TOKEN_INDENT, "Expected class body" );

		std::vector<std::unique_ptr<VarDecl>> fields;
		std::vector<std::unique_ptr<FuncDecl>> methods;
		std::vector<std::unique_ptr<NativeStmt>> natives;

		while( !Check( TOKEN_DEDENT ) && !AtEnd() )
		{
			if( Match( TOKEN_DEF ) )
			{
				methods.push_back( ParseFuncDeclaration() );
			}
			else if( Match( TOKEN_NATIVE ) )
			{
				natives.push_back( ParseNative() );
			}
			else if( Check( TOKEN_IDENT ) )
			{
				fields.push_back( ParseVarDeclaration() );
			}
			else
			{
				break;
			}
		}

		if( !AtEnd() )
		{
			Expect( TOKEN_DEDENT, "Expected dedent after block" );
		}

		return std::make_unique<ClassDecl>( loc, std::move( name ), std::move( fields ), std::move( methods ), std::move( natives ) );
	}

	FunctionSignature Parser::ParseFuncSignature()
	{
		Token name = Expect( TOKEN_IDENT, "Expected function name" );
		Expect( TOKEN_LPAREN, "Expected '(' after function name" );

		std::vector<TypeSpecifier> types;
		std::vector<Token> params;
		std::vector<std::unique_ptr<Expression>> defaults;
		bool varargs = false;
		bool found_def = false;
		if( !Check( TOKEN_RPAREN ) )
		{
			do
			{
				if( Match( TOKEN_ELLIPSIS ) )
				{
					varargs = true;
					break; // varargs ellipsis are last parameter, exit out and don't check for more
				}

				Token param = Expect( TOKEN_IDENT, "Expected parameter identifier" );
				params.push_back( param );
				Expect( TOKEN_COLON, "Expected ':'" );
				TypeSpecifier type = ExpectType( "Expected parameter type" );
				types.push_back( std::move( type ) );

				if( Match( TOKEN_EQUAL ) )
				{
					found_def = true;
					defaults.push_back( ParseExpression() );
				}
				else if( found_def )
				{
					Error( "Cannot have non-defaulted parameter after a defaulted parameter" );
					throw ParseError();
				}
				else
				{
					defaults.push_back( nullptr );
				}
			} while( Match( TOKEN_COMMA ) );
		}
		Expect( TOKEN_RPAREN, "Expected ')' after function parameters" );

		TypeSpecifier return_type;
		if( Match( TOKEN_ARROW ) )
		{
			return_type = ExpectType( "Expected function return type" );
		}

		return FunctionSignature( std::move( return_type ), name, std::move( types ), params, std::move( defaults ), varargs );
	}

	std::unique_ptr<Expression> Parser::ParseExpressions()
	{
		SourceLoc loc = Peek().loc;

		std::unique_ptr<Expression> expr = ParseExpression();
		if( !Match( TOKEN_COMMA ) )
			return expr;
		
		std::vector<std::unique_ptr<Expression>> elements;
		elements.push_back( std::move( expr ) );

		while( !CheckTerminator() )
		{
			elements.push_back( ParseExpression() );
			Match( TOKEN_COMMA );
		}

		return std::make_unique<TupleLiteral>( loc, std::move( elements ) );
	}

	std::unique_ptr<Expression> Parser::ParseExpression()
	{
		return ParseOr();
	}

	std::unique_ptr<Expression> Parser::ParseOr()
	{
		SourceLoc loc = Peek().loc;

		auto expr = ParseAnd();
		while( Match( TOKEN_OR ) )
		{
			Token op = Previous();
			auto right = ParseAnd();
			expr = std::make_unique<BinaryExpr>( loc, op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseAnd()
	{
		SourceLoc loc = Peek().loc;

		auto expr = ParseBitOr();
		while( Match( TOKEN_AND ) )
		{
			Token op = Previous();
			auto right = ParseBitOr();
			expr = std::make_unique<BinaryExpr>( loc, op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseBitOr()
	{
		SourceLoc loc = Peek().loc;

		auto expr = ParseBitXor();
		while( Match( TOKEN_BAR ) )
		{
			Token op = Previous();
			auto right = ParseBitXor();
			expr = std::make_unique<BinaryExpr>( loc, op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseBitXor()
	{
		SourceLoc loc = Peek().loc;

		auto expr = ParseBitAnd();
		while( Match( TOKEN_HAT ) )
		{
			Token op = Previous();
			auto right = ParseBitAnd();
			expr = std::make_unique<BinaryExpr>( loc, op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseBitAnd()
	{
		SourceLoc loc = Peek().loc;

		auto expr = ParseEqualCompare();
		while( Match( TOKEN_AMP ) )
		{
			Token op = Previous();
			auto right = ParseEqualCompare();
			expr = std::make_unique<BinaryExpr>( loc, op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseEqualCompare()
	{
		SourceLoc loc = Peek().loc;

		auto expr = ParseSizeCompare();
		while( Match( TOKEN_EQUAL_EQUAL ) || Match( TOKEN_EXCLMARK_EQUAL ) )
		{
			Token op = Previous();
			auto right = ParseSizeCompare();
			expr = std::make_unique<BinaryExpr>( loc, op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseSizeCompare()
	{
		SourceLoc loc = Peek().loc;

		auto expr = ParseBitShift();
		while( Match( TOKEN_LESS ) || Match( TOKEN_LESS_EQUAL ) ||
			Match( TOKEN_GREATER ) || Match( TOKEN_GREATER_EQUAL ) )
		{
			Token op = Previous();
			auto right = ParseBitShift();
			expr = std::make_unique<BinaryExpr>( loc, op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseBitShift()
	{
		SourceLoc loc = Peek().loc;

		auto expr = ParseAddition();
		while( Match( TOKEN_LESS_LESS ) || Match( TOKEN_GREATER_GREATER ) )
		{
			Token op = Previous();
			auto right = ParseAddition();
			expr = std::make_unique<BinaryExpr>( loc, op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseAddition()
	{
		SourceLoc loc = Peek().loc;

		auto expr = ParseMultiplication();
		while( Match( TOKEN_PLUS ) || Match( TOKEN_MINUS ) )
		{
			Token op = Previous();
			auto right = ParseMultiplication();
			expr = std::make_unique<BinaryExpr>( loc, op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseMultiplication()
	{
		SourceLoc loc = Peek().loc;

		auto expr = ParseUnary();
		while( Match( TOKEN_ASTERISK ) ||
			Match( TOKEN_SLASH ) ||
			Match( TOKEN_PERCENT ) )
		{
			Token op = Previous();
			auto right = ParseUnary();
			expr = std::make_unique<BinaryExpr>( loc, op.type, std::move( expr ), std::move( right ) );
		}

		return expr;
	}

	std::unique_ptr<Expression> Parser::ParseUnary()
	{
		SourceLoc loc = Peek().loc;

		if( Match( TOKEN_MINUS ) ||
			Match( TOKEN_EXCLMARK ) ||
			Match( TOKEN_TILDE ) ||
			Match( TOKEN_AMP ) )
		{
			Token op = Previous();
			auto right = ParseUnary();
			return std::make_unique<UnaryExpr>( loc, op.type, std::move( right ) );
		}

		return ParseCallOrIndexOrAttr();
	}

	std::unique_ptr<Expression> Parser::ParseCallOrIndexOrAttr()
	{
		SourceLoc loc = Peek().loc;

		auto left = ParsePrimary();

		while( true )
		{
			switch( Peek().type )
			{
			case TOKEN_LPAREN:   left = ParseCall( std::move( left ) ); break;
			case TOKEN_LBRACKET: left = ParseIndex( std::move( left ) ); break;
			case TOKEN_DOT:      left = ParseAttr( std::move( left ) ); break;
			default:             return std::move( left );
			}
		}
	}

	std::unique_ptr<Expression> Parser::ParseCall( std::unique_ptr<Expression> left )
	{
		SourceLoc loc = Previous().loc;

		Expect( TOKEN_LPAREN, "Expected '('" );

		std::vector<std::unique_ptr<Expression>> arguments;
		if( !Check( TOKEN_RPAREN ) )
		{
			do
			{
				auto arg = ParseExpression();
				arguments.push_back( std::move( arg ) );
			} while( Match( TOKEN_COMMA ) );
		}
		Expect( TOKEN_RPAREN, "Expected ')'" );

		return std::make_unique<CallExpr>( loc, std::move( left ), std::move( arguments ) );
	}

	std::unique_ptr<Expression> Parser::ParseIndex( std::unique_ptr<Expression> left )
	{
		SourceLoc loc = Previous().loc;

		Expect( TOKEN_LBRACKET, "Expected '['" );

		std::unique_ptr<Expression> index;

		if ( !Check( TOKEN_COLON ) )
			index = ParseExpression();
		
		if ( Match( TOKEN_COLON ) )
		{
			std::unique_ptr<Expression> end;

			if ( !Check( TOKEN_RBRACKET ) )
				end = ParseExpression();

			Expect( TOKEN_RBRACKET, "Expected ']'" );

			return std::make_unique<SliceExpr>( loc, std::move( left ), std::move( index ), std::move( end ) );
		}
		else
		{
			Expect( TOKEN_RBRACKET, "Expected ']'" );

			return std::make_unique<IndexExpr>( loc, std::move( left ), std::move( index ) );
		}
	}

	std::unique_ptr<Expression> Parser::ParseAttr( std::unique_ptr<Expression> left )
	{
		SourceLoc loc = Previous().loc;

		Expect( TOKEN_DOT, "Expected '.'" );

		Token attr = Expect( TOKEN_IDENT, "Expected identifier" );

		return std::make_unique<AttrExpr>( loc, std::move( left ), std::move( attr ) );
	}

	std::unique_ptr<Expression> Parser::ParsePrimary()
	{
		SourceLoc loc = Peek().loc;

		if( Match( TOKEN_INT_LITERAL ) )    return std::make_unique<IntLiteral>( loc, Previous().literal.i64 );
		if( Match( TOKEN_FLOAT_LITERAL ) )  return std::make_unique<FloatLiteral>( loc, Previous().literal.f64 );
		if( Match( TOKEN_STRING_LITERAL ) ) return std::make_unique<StringLiteral>( loc, Previous().literal.str );

		if( Match( TOKEN_TRUE ) || Match( TOKEN_FALSE ) || Match( TOKEN_NIL ) )
			return std::make_unique<TokenLiteral>( loc, Previous().type );

		if( Match( TOKEN_IDENT ) ) return std::make_unique<VarExpr>( loc, Previous() );

		if( Match( TOKEN_LPAREN ) )
		{
			if( Match( TOKEN_RPAREN ) )
			{
				// Empty tuple
				return std::make_unique<TupleLiteral>( loc, std::vector<std::unique_ptr<Expression>>() );
			}

			auto expr = ParseExpression();

			if( Match( TOKEN_COMMA ) )
			{
				// Tuple
				std::vector<std::unique_ptr<Expression>> elements;

				elements.push_back( std::move( expr ) );

				while( !Match( TOKEN_RPAREN ) )
				{
					elements.push_back( ParseExpression() );
					Match( TOKEN_COMMA );
				}

				return std::make_unique<TupleLiteral>( loc, std::move( elements ) );
			}

			Expect( TOKEN_RPAREN, "Expected ')' after expression." );

			return std::make_unique<GroupExpr>( loc, std::move( expr ) );
		}

		if( Match( TOKEN_LBRACKET ) )
		{
			std::vector<std::unique_ptr<Expression>> arr_values;
			if( !Check( TOKEN_RBRACKET ) )
			{
				do
				{
					arr_values.push_back( ParseExpression() );
				} while( Match( TOKEN_COMMA ) && !Check( TOKEN_RBRACKET ) );
			}
			Expect( TOKEN_RBRACKET, "Expected closing ']' for array literal" );

			return std::make_unique<ArrayLiteral>( loc, std::move( arr_values ) );
		}
		
		Error( std::string( "Unexpected '") + TokenTypeToString( Peek().type ) + "'." );
		throw ParseError();
	}
}