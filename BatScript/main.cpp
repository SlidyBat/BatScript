#include <iostream>
#include <limits>

#include "memory_stream.h"
#include "lexer.h"
#include "errorsys.h"

using namespace Bat;

void Run( const std::string& src )
{
	Lexer l( src );
	std::vector<Token> tokens = l.Scan();
	
	for( const auto& t : tokens )
	{
		std::cout << TokenTypeToString( t.type ) << '\n';
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