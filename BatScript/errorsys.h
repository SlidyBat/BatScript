#pragma once

#include <string>

namespace Bat
{
	class ErrorSys
	{
	public:
		static void Report( int line, int column, const std::string& message );
		static bool HadError();
		static void SetSource( const std::string& source );
	};
}