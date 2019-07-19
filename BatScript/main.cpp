#include <iostream>
#include <limits>

#include "memory_stream.h"
#include "lexer.h"
#include "parser.h"
#include "errorsys.h"
#include "ast_printer.h"
#include "interpreter.h"
#include "bat_callable.h"
#include "runtime_error.h"

using namespace Bat;

Interpreter interpreter;

void Run( const std::string& src, bool print_expression_results = false )
{
	Lexer l( src );
	auto tokens = l.Scan();

	if( ErrorSys::HadError() ) return;

	Parser p( std::move( tokens ) );
	std::vector<std::unique_ptr<Statement>> res = p.Parse();

	if( ErrorSys::HadError() ) return;

	try
	{
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

			if( ErrorSys::HadError() ) return;
		}
	}
	catch( const RuntimeError& )
	{
		return;
	}
}

void RunFromFile( const std::string& filename )
{
	ErrorSys::SetSource( filename );
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
	interpreter.AddNative( "nativetest", 0, []( const std::vector<BatObject>& args ) {
		std::cout << "hehe\n";
		return BatObject();
	} );

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