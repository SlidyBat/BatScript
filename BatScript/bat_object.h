#pragma once

#include <cstdint>
#include <exception>
#include <string>

namespace Bat
{
	class BatObjectError : public std::exception
	{};

	class BatObject
	{
	public:
		BatObject() = default;
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

		std::string ToString();
		bool IsTruthy();
	private:
		enum Type
		{
			TYPE_NULL,
			TYPE_INT,
			TYPE_FLOAT,
			TYPE_BOOL,
			TYPE_STR
		};
		Type type;
		union
		{
			int64_t i64;
			double f64;
			const char* str;
		} value;
	};
}