#pragma once

#include <string>

namespace Bat
{
	class ErrorSys
	{
	public:
		static void Report( size_t line, size_t column, const std::string& message );
		static bool HadError();
		static void SetSource( const std::string& source );
		static void Reset();
	};
}