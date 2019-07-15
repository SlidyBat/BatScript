#include <iostream>
#include <limits>

#include "memory_stream.h"
#include "lexer.h"
#include "parser.h"
#include "errorsys.h"

using namespace Bat;

void Print( Expression* e, int level = 0 )
{
	for( int i = 0; i < level; i++ )
	{
		std::cout << "  ";
	}
	switch( e->type )
	{
		case EXPR_ADD: std::cout << "+"; break;
		case EXPR_SUB: std::cout << "-"; break;
		case EXPR_DIV: std::cout << "/"; break;
		case EXPR_MUL: std::cout << "*"; break;
		case EXPR_MOD: std::cout << "%"; break;

		case EXPR_BITOR: std::cout << "|"; break;
		case EXPR_BITXOR: std::cout << "^"; break;
		case EXPR_BITAND: std::cout << "&"; break;
		case EXPR_LBITSHIFT: std::cout << "<<"; break;
		case EXPR_RBITSHIFT: std::cout << ">>"; break;

		case EXPR_ASSIGN: std::cout << "="; break;
		case EXPR_ADD_ASSIGN: std::cout << "+="; break;
		case EXPR_SUB_ASSIGN: std::cout << "-="; break;
		case EXPR_DIV_ASSIGN: std::cout << "/="; break;
		case EXPR_MUL_ASSIGN: std::cout << "*="; break;
		case EXPR_MOD_ASSIGN: std::cout << "%="; break;
		case EXPR_BITOR_ASSIGN: std::cout << "|="; break;
		case EXPR_BITXOR_ASSIGN: std::cout << "^="; break;
		case EXPR_BITAND_ASSIGN: std::cout << "&="; break;

		case EXPR_AND: std::cout << "and"; break;
		case EXPR_OR: std::cout << "or"; break;
		case EXPR_CMPEQ: std::cout << "=="; break;
		case EXPR_CMPNEQ: std::cout << "!="; break;
		case EXPR_CMPL: std::cout << "<"; break;
		case EXPR_CMPLE: std::cout << "<="; break;
		case EXPR_CMPG: std::cout << ">"; break;
		case EXPR_CMPGE: std::cout << ">="; break;

		case EXPR_NOT: std::cout << "!"; break;
		case EXPR_BITNEG: std::cout << "~"; break;
		case EXPR_NEG: std::cout << "-"; break;
		case EXPR_GROUP: std::cout << "()"; break;
		case EXPR_ADDROF: std::cout << "&"; break;
		case EXPR_MOVE: std::cout << "&"; break;
		case EXPR_PRINT: std::cout << "print"; break;

		case EXPR_NULL_LITERAL: std::cout << "null"; break;
		case EXPR_BOOL_LITERAL: std::cout << (e->value.i64 ? "true" : "false"); break;
		case EXPR_INT_LITERAL: std::cout << e->value.i64; break;
		case EXPR_FLOAT_LITERAL: std::cout << e->value.f64; break;
		case EXPR_STR_LITERAL: std::cout << e->value.str; break;

		default: std::cout << "<unknown> (" << e->type << ")"; break;
	}

	std::cout << std::endl;

	for( int i = 0; i < Expression::GetNumParams( e->type ); i++ )
	{
		Print( e->params[i].get(), level + 1 );
	}
}

void Run( const std::string& src )
{
	Lexer l( src );
	std::vector<Token> tokens = l.Scan();
	Parser p( std::move( tokens ) );
	std::unique_ptr<Expression> e = p.Parse();
	if( e != nullptr )
	{
		Print( e.get() );
	}
}

void RunFromFile( const std::string& filename )
{
	auto source = MemoryStream::FromFile( filename, FileMode::TEXT );
	Run( source.Base() );
}

void RunFromPrompt()
{
	std::string input;
	std::cout << "> ";
	std::getline( std::cin, input );
	while( input != "quit" )
	{
		Run( input );

		std::cout << "> ";
		std::getline( std::cin, input );
	}
}

int main( int argc, char** argv )
{
	if( argc >= 2 )
	{
		RunFromFile( argv[1] );
	}
	else
	{
		RunFromPrompt();
	}

	system( "pause" );

	if( ErrorSys::HadError() )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}