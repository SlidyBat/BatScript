#pragma once

#include <cstdint>
#include <exception>
#include <string>
#include <vector>
#include <cassert>
#include "sourceloc.h"

namespace Bat
{
	class BatCallable;
	class BatObject;
	class Interpreter;

	enum ObjectType
	{
		TYPE_NULL,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_BOOL,
		TYPE_STR,
		TYPE_CALLABLE,
	};

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

		BatObject Add( const BatObject& rhs, const SourceLoc& loc );
		BatObject Sub( const BatObject& rhs, const SourceLoc& loc );
		BatObject Div( const BatObject& rhs, const SourceLoc& loc );
		BatObject Mul( const BatObject& rhs, const SourceLoc& loc );
		BatObject Mod( const BatObject& rhs, const SourceLoc& loc );
		BatObject LShift( const BatObject& rhs, const SourceLoc& loc );
		BatObject RShift( const BatObject& rhs, const SourceLoc& loc );
		BatObject BitXor( const BatObject& rhs, const SourceLoc& loc );
		BatObject BitOr( const BatObject& rhs, const SourceLoc& loc );
		BatObject BitAnd( const BatObject& rhs, const SourceLoc& loc );
		BatObject CmpEq( const BatObject& rhs, const SourceLoc& loc );
		BatObject CmpNeq( const BatObject& rhs, const SourceLoc& loc );
		BatObject CmpL( const BatObject& rhs, const SourceLoc& loc );
		BatObject CmpLe( const BatObject& rhs, const SourceLoc& loc );
		BatObject CmpG( const BatObject& rhs, const SourceLoc& loc );
		BatObject CmpGe( const BatObject& rhs, const SourceLoc& loc );
		BatObject Not( const SourceLoc& loc );
		BatObject Neg( const SourceLoc& loc );
		BatObject BitNeg( const SourceLoc& loc );
		BatObject Call( Interpreter& interpreter, const std::vector<BatObject>& args, const SourceLoc& loc );

		bool IsTruthy( const SourceLoc& loc );

		std::string ToString();

		int64_t Int() const { assert( type == TYPE_INT ); return value.i64; }
		double Float() const { assert( type == TYPE_FLOAT ); return value.f64; }
		const char* String() const { assert( type == TYPE_STR ); return value.str; }
		BatCallable* Function() const { assert( type == TYPE_CALLABLE ); return value.func; }
	public:
	public:
		ObjectType type = TYPE_NULL;
		union
		{
			int64_t i64;
			double f64;
			const char* str;
			BatCallable* func;
		} value;
	};
}