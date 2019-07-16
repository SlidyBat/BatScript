#include <iostream>
#include <limits>

#include "memory_stream.h"
#include "lexer.h"
#include "parser.h"
#include "errorsys.h"
#include "ast_printer.h"
#include "interpreter.h"

using namespace Bat;

Interpreter interpreter;

void Run( const std::string& src, bool print_expression_results = false )
{
	Lexer l( src );
	std::vector<Token> tokens = l.Scan();
	Parser p( std::move( tokens ) );
	std::vector<std::unique_ptr<Statement>> res = p.Parse();
	for( size_t i = 0; i < res.size(); i++ )
	{
		if( print_expression_results && res[i]->IsExpressionStmt() )
		{
			auto expr_res = interpreter.Evaluate( res[i]->AsExpressionStmt()->Expr() );
			std::cout << expr_res.ToString() << std::endl;
		}
		else
		{
			interpreter.Execute( res[i].get() );
		}
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
		Run( input, true );

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