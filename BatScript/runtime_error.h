#pragma once

#include <exception>
#include <string>
#include "sourceloc.h"
#include "errorsys.h"

namespace Bat
{
	class RuntimeError : public std::exception
	{
	public:
		RuntimeError( const SourceLoc& loc, const std::string& msg )
			:
			msg( msg )
		{
			ErrorSys::Report( loc.Line(), loc.Column(), msg );
		}

		virtual const char* what() const override { return msg.c_str(); }
	private:
		std::string msg;
	};
}