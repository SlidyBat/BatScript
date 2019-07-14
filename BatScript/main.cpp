#include <iostream>
#include <limits>

#include "memory_stream.h"
#include "lexer.h"

using namespace Bat;

void Run( const std::string& src )
{
	Lexer l( src );
	
	Token t;
	do
	{
		t = l.NextToken();
		std::cout << t.lexeme << '\n';
	} while( t.type != TOKEN_EOF );
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
	return 0;
}