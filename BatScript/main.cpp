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
#include "compiler.h"
#include "disassembler.h"
#include "vm.h"

using namespace Bat;

Interpreter interpreter;
SemanticAnalysis sa;
Compiler compiler;
VirtualMachine vm;

void Run( const std::string& src, bool print_expression_results = false, bool use_compiler = true )
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
			else if( !use_compiler )
			{
				// AstPrinter::Print( res[i].get() );
				interpreter.Execute( std::move( res[i] ) );
			}

			if( ErrorSys::HadError() ) return;
		}

		if( use_compiler )
		{
			compiler.Compile( std::move( res ) );

			if( ErrorSys::HadError() ) return;

			auto code = compiler.Code();

			Disassembler disasm( code, src );
			disasm.Disassemble();

			vm.Run( code );
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
	interpreter.AddNative( name, callback );
	vm.AddNative( name, callback );
}

using namespace std::chrono;

int fib( int n )
{
	if( n < 2 ) return n;
	return fib( n - 2 ) + fib( n - 1 );
}

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

		//Compiler compiler;
		//auto x = compiler.EmitJump( OpCode::JZ );
		//compiler.Emit( OpCode::PUSH, 15 );
		//compiler.Emit( OpCode::PUSH, 12 );
		//compiler.Emit( OpCode::ADD );
		//compiler.Emit( OpCode::PRINT );
		//compiler.EmitF( OpCode::PUSH, 1 );
		//compiler.EmitF( OpCode::PUSH, 2 );
		//compiler.Emit( OpCode::ADDF );
		//
		//compiler.Patch( x, 1 );
		//
		//auto code = compiler.Code();
		//
		//Disassembler disasm( code );
		//disasm.Disassemble();
		//
		//VirtualMachine vm;
		//vm.Run( code );
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