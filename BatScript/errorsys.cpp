#include "errorsys.h"

#include <iostream>

namespace Bat
{
	static bool g_bHadError = false;
	static std::string g_szSource;

	void ErrorSys::Report( size_t line, size_t column, const std::string& message )
	{
		g_bHadError = true;
		if( g_szSource.empty() )
		{
			std::cerr << "[" << line << ":" << column << "] Error: " << message << "\n";
		}
		else
		{
			std::cerr << "[" << g_szSource << ":" << line << ":" << column << "] Error: " << message << "\n";
		}
	}

	bool ErrorSys::HadError()
	{
		return g_bHadError;
	}
	void ErrorSys::SetSource( const std::string& source )
	{
		g_szSource = source;
	}
}