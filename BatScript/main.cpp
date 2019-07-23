#include <iostream>
#include <limits>
#include <chrono>

#include "memory_stream.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analysis.h"
#include "errorsys.h"
#include "ast_printer.h"
#include "interpreter.h"
#include "bat_callable.h"
#include "runtime_error.h"

using namespace Bat;

Interpreter interpreter;
// Stores all parsed statements, needs to be global so that lifetime lasts across REPL commands
std::vector<std::unique_ptr<Statement>> statements;

void Run( const std::string& src, bool print_expression_results = false )
{
	Lexer l( src );
	auto tokens = l.Scan();

	if( ErrorSys::HadError() ) return;

	Parser p( std::move( tokens ) );
	std::vector<std::unique_ptr<Statement>> res = p.Parse();

	if( ErrorSys::HadError() ) return;

	SemanticAnalysis sa;

	for( size_t i = 0; i < res.size(); i++ )
	{
		sa.Analyze( res[i].get() );
	}

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

			// Transfer ownership
			statements.push_back( std::move( res[i] ) );

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
		ErrorSys::Reset();
		Run( input, true );

		std::cout << "> ";
		std::getline( std::cin, input );
	}
}

using namespace std::chrono;

int main( int argc, char** argv )
{
	interpreter.AddNative( "time", 0, []( const std::vector<BatObject>& args ) {
		return BatObject( duration_cast<milliseconds>( system_clock::now().time_since_epoch() ).count() );
	} );

	if( argc >= 2 )
	{
		RunFromFile( argv[1] );
	}
	else
	{
		//RunFromPrompt();
		RunFromFile( "test2.bat" );
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