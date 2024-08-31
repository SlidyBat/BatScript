#pragma once

#include <string>

namespace Bat
{
	class ErrorSys
	{
	public:
		static void Report( size_t line, size_t column, const std::string& message );
		static bool HadError();
		static void PushSource( const std::string& source );
		static void PopSource();
		static void Reset();
	};

	class ErrorSysSource
	{
	public:
		ErrorSysSource( const std::string& source )
		{
			ErrorSys::PushSource( source );
		}

		~ErrorSysSource()
		{
			ErrorSys::PopSource();
		}
	};
}