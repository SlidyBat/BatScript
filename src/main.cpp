#include <iostream>
#include <limits>
#include <chrono>
#include <filesystem>
#include <thread>

#include "stringlib.h"
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
#include "optparse.h"

using namespace Bat;

Interpreter interpreter;
SemanticAnalysis sa;
Compiler compiler;
VirtualMachine vm;

// Options
enum class ExecuteMethod
{
	NONE,
	INTERPRETER,
	VM
};

bool print_ast = false;
bool disassemble = false;
ExecuteMethod exec_method = ExecuteMethod::INTERPRETER;

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
			if( print_ast )
			{
				AstPrinter::Print( res[i].get() );
			}

			if( print_expression_results && res[i]->IsExpressionStmt() )
			{
				auto expr_res = interpreter.Evaluate( res[i]->AsExpressionStmt()->Expr() );
				std::cout << expr_res.ToString() << std::endl;
			}
			else if( exec_method == ExecuteMethod::INTERPRETER )
			{
				interpreter.Execute( std::move( res[i] ) );
			}

			if( ErrorSys::HadError() ) return;
		}

		if( exec_method != ExecuteMethod::INTERPRETER )
		{
			compiler.Compile( std::move( res ) );

			if( ErrorSys::HadError() ) return;

			auto code = compiler.Code();

			if( disassemble )
			{
				Disassembler disasm( code, src );
				disasm.Disassemble();
			}

			if( exec_method == ExecuteMethod::VM )
			{
				vm.Run( code );
			}
		}
	}
	catch( const RuntimeError& )
	{
		return;
	}
}

void RunFromFile( const std::string& filename )
{
	if( !std::filesystem::exists( filename ) )
	{
		std::cerr << "File '" << filename << "' does not exist" << std::endl;
		return;
	}

	ErrorSysSource err_source( filename );
	auto source = MemoryStream::FromFile( filename, FileMode::TEXT );

	std::filesystem::path path = std::filesystem::absolute( filename );
	std::filesystem::current_path( path.parent_path() );

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

void AddModule( const std::string& name, BatModule module )
{
	interpreter.AddModule( name, module );
	vm.AddModule( name, std::move( module ) );
}

using namespace std::chrono;

int fib( int n )
{
	if( n < 2 ) return n;
	return fib( n - 2 ) + fib( n - 1 );
}

int main( int argc, char** argv )
{
	BatModule file;
	file.AddNative( "open", []( std::string filename, std::string mode ) {
		FILE* f = fopen( filename.c_str(), mode.c_str() );
		return BatObject::Object( "File", f );
	} );
	file.AddNative( "close", []( void* file ) {
		fclose( (FILE*)file );
	} );
	file.AddNative( "File.read", []( void* file, int size ) {
		auto* f = (FILE*)file;
		if ( size < 0 )
		{
			long pos = ftell( f );
			fseek( f, 0, SEEK_END );
			long end = ftell( f );
			fseek( f, pos, SEEK_SET );
			size = (end - pos);
		}

		auto data = std::make_unique<char[]>( size + 1 );
		fread( data.get(), size, 1, f );
		return BatObject::String( data.get() );
	} );
	file.AddNative( "File.write", []( void* file, std::string data ) {
		auto* f = (FILE*)file;
		fwrite( data.data(), data.size(), 1, f );
	} );
	file.AddNative( "File.test", []( void* file ) {
		printf("ASADSDASDAD\n");
	} );

	BatModule time;
	time.AddNative( "now", []() {
		return duration_cast<milliseconds>( system_clock::now().time_since_epoch() ).count();
	} );
	time.AddNative( "sleep", []( int duration ) {
		std::this_thread::sleep_for( milliseconds( duration ) );
	} );

	BatModule core;
	core.AddNativeRaw( "format", []( const std::vector<BatObject>& args ) {
		std::stringstream ss;

		const char* fmt = args[0].GetString();
		size_t curr_arg = 1;
		for ( const char* p = fmt; *p != '\0'; p++ )
		{
			if ( *p == '%' )
			{
				char type = p[1];
				if ( type == '\0' )
				{
					ss << *p;
					continue;
				}

				if ( curr_arg >= args.size() )
					return BatObject::String( "ERROR" );
				const BatObject& arg = args[curr_arg++];

				// TODO: Type checking
				switch ( type )
				{
					case 's':
						ss << arg.ToString();
						break;
					case 'c':
						ss << arg.ToString();
						break;
					case 'i':
					case 'd':
						ss << arg.GetInt();
						break;
					case 'f':
						ss << arg.GetFloat();
						break;
				}
			}
			else
			{
				ss << *p;
			}
		}

		return BatObject::String( ss.str().c_str() );
	} );
	
	AddModule( "file", std::move( file ) );
	AddModule( "time", std::move( time ) );
	AddModule( "core", std::move( core ) );

	if( argc >= 2 )
	{
		OptParse optparse;
		optparse.AddFlagOption( "disasm", 'd' )
			.AddFlagOption( "ast", 'a' )
			.AddArgOption( "method", 'm' );
		optparse.Process( argc, argv );

		if( optparse["disasm"] )
		{
			disassemble = true;
		}

		if( optparse["ast"] )
		{
			print_ast = true;
		}

		if( optparse["method"] )
		{
			if( optparse["method"] == "vm"s )
			{
				exec_method = ExecuteMethod::VM;
			}
			else if( optparse["method"] == "interpreter"s )
			{
				exec_method = ExecuteMethod::INTERPRETER;
			}
			else if( optparse["method"] == "none"s )
			{
				exec_method = ExecuteMethod::NONE;
			}
			else
			{
				std::cerr << "Method must be one of: none, vm, interpreter\n";
				return -1;
			}
		}

		RunFromFile( optparse.GetArg( 0 ) );
	}
	else
	{
		RunFromPrompt();
		disassemble = true;
	}

	if( argc < 2 )
	{
		system( "pause" );
	}

	if( ErrorSys::HadError() )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}