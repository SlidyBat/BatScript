#pragma once

#include <string>

namespace Bat
{
	class ErrorSys
	{
	public:
		static void Report( const std::string& filename, int line, int column, const std::string& message );
		static bool HadError();
	};
}