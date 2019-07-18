#pragma once

#include <cstdint>
#include <exception>
#include <string>
#include <vector>

namespace Bat
{
	class BatCallable;
	class BatObject;
	class Interpreter;

	class BatObjectError : public std::exception
	{};

	class BatObject
	{
	public:
		BatObject() = default;
		~BatObject();
		BatObject( int64_t i )
			:
			type( TYPE_INT )
		{
			value.i64 = i;
		}
		BatObject( double f )
			:
			type( TYPE_FLOAT )
		{
			value.f64 = f;
		}
		BatObject( const char* s )
			:
			type( TYPE_STR )
		{
			value.str = s;
		}
		BatObject( bool s )
			:
			type( TYPE_BOOL )
		{
			value.i64 = s;
		}
		BatObject( BatCallable* func )
			:
			type( TYPE_CALLABLE )
		{
			value.func = func;
		}

		BatObject operator+( const BatObject& rhs );
		BatObject operator-( const BatObject& rhs );
		BatObject operator/( const BatObject& rhs );
		BatObject operator*( const BatObject& rhs );
		BatObject operator%( const BatObject& rhs );
		BatObject operator<<( const BatObject& rhs );
		BatObject operator>>( const BatObject& rhs );
		BatObject operator^( const BatObject& rhs );
		BatObject operator|( const BatObject& rhs );
		BatObject operator&( const BatObject& rhs );
		BatObject operator==( const BatObject& rhs );
		BatObject operator!=( const BatObject& rhs );
		BatObject operator<( const BatObject& rhs );
		BatObject operator<=( const BatObject& rhs );
		BatObject operator>( const BatObject& rhs );
		BatObject operator>=( const BatObject& rhs );
		BatObject operator!();
		BatObject operator-();
		BatObject operator~();
		BatObject operator()( Interpreter& interpreter, const std::vector<BatObject>& args );

		std::string ToString();
		bool IsTruthy();
	private:
		enum Type
		{
			TYPE_NULL,
			TYPE_INT,
			TYPE_FLOAT,
			TYPE_BOOL,
			TYPE_STR,
			TYPE_CALLABLE,
		};
		Type type = TYPE_NULL;
		union
		{
			int64_t i64;
			double f64;
			const char* str;
			BatCallable* func;
		} value;
	};
}