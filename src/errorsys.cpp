#include "errorsys.h"

#include <iostream>
#include <vector>

namespace Bat
{
	static bool g_bHadError = false;
	static std::vector<std::string> g_SourceStack;

	void ErrorSys::Report( size_t line, size_t column, const std::string& message )
	{
		g_bHadError = true;
		if( g_SourceStack.empty() )
		{
			std::cerr << "[" << line << ":" << column << "] Error: " << message << "\n";
		}
		else
		{
			std::cerr << "[" << g_SourceStack.back() << ":" << line << ":" << column << "] Error: " << message << "\n";
		}
	}

	bool ErrorSys::HadError()
	{
		return g_bHadError;
	}
	void ErrorSys::PushSource( const std::string& source )
	{
		g_SourceStack.push_back( source );
	}
	void ErrorSys::PopSource()
	{
		g_SourceStack.pop_back();
	}
	void ErrorSys::Reset()
	{
		g_bHadError = false;
	}
}