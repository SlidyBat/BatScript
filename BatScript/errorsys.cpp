#include "errorsys.h"

#include <iostream>

namespace Bat
{
	static bool g_bHadError = false;

	void ErrorSys::Report( const std::string& filename, int line, int column, const std::string& message )
	{
		g_bHadError = true;
		std::cerr << "[" << filename << ":" << line << ":" << column << "] Error: " << message << "\n";
	}

	bool ErrorSys::HadError()
	{
		return g_bHadError;
	}
}