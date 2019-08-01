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
SemanticAnalysis sa;

void Run( const std::string& src, bool print_expression_results = false )
{
	Lexer l( src );
	auto tokens = l.Scan();

	if( ErrorSys::HadError() ) return;

	Parser p( std::move( tokens ) );
	std::vector<std::unique_ptr<Statement>> res = p.Parse();

	if( ErrorSys::HadError() ) return;

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
				interpreter.Execute( std::move( res[i] )  );
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
		ErrorSys::Reset();
		Run( input, true );

		std::cout << "> ";
		std::getline( std::cin, input );
	}
}

void AddNative( const std::string& name, BatNativeCallback callback )
{
	interpreter.AddNative( name, std::move( callback ) );
}

using namespace std::chrono;

int main( int argc, char** argv )
{
	AddNative( "time", []( const std::vector<BatObject>& args ) {
		return BatObject( duration_cast<milliseconds>( system_clock::now().time_since_epoch() ).count() );
	} );

	// YUCK! Should make my own format func in the future, this is leaky and disgusting
	AddNative( "format", []( const std::vector<BatObject>& args ) {
		auto buffer = new char[4096];
#define B(obj) obj.value
		switch( args.size() )
		{
		case 1:  return BatObject( args[0].String() );
		case 2:  sprintf_s( buffer, 4096, args[0].String(), B( args[1] ) ); break;
		case 3:  sprintf_s( buffer, 4096, args[0].String(), B( args[1] ), B( args[2] ) ); break;
		case 4:  sprintf_s( buffer, 4096, args[0].String(), B( args[1] ), B( args[2] ), B( args[3] ) ); break;
		case 5:  sprintf_s( buffer, 4096, args[0].String(), B( args[1] ), B( args[2] ), B( args[3] ), B( args[4] ) ); break;
		case 6:  sprintf_s( buffer, 4096, args[0].String(), B( args[1] ), B( args[2] ), B( args[3] ), B( args[4] ), B( args[5] ) ); break;
		case 7:  sprintf_s( buffer, 4096, args[0].String(), B( args[1] ), B( args[2] ), B( args[3] ), B( args[4] ), B( args[5] ), B( args[6] ) ); break;
		case 8:  sprintf_s( buffer, 4096, args[0].String(), B( args[1] ), B( args[2] ), B( args[3] ), B( args[4] ), B( args[5] ), B( args[6] ), B( args[7] ) ); break;
		case 9:  sprintf_s( buffer, 4096, args[0].String(), B( args[1] ), B( args[2] ), B( args[3] ), B( args[4] ), B( args[5] ), B( args[6] ), B( args[7] ), B( args[8] ) ); break;
		case 10: sprintf_s( buffer, 4096, args[0].String(), B( args[1] ), B( args[2] ), B( args[3] ), B( args[4] ), B( args[5] ), B( args[6] ), B( args[7] ), B( args[8] ), B( args[9] ) ); break;
		default:
			ErrorSys::Report( 0, 0, "Too many arguments to format (max 10 + format string)" ); // TODO: Get lines/stack trace here
			return BatObject( "ERROR" );
		}

		
		return BatObject( buffer );
#undef B
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