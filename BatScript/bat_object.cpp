#include "bat_object.h"

#include <string>
#include "bat_callable.h"
#include "interpreter.h"
#include "runtime_error.h"

namespace Bat
{
	static const char* TypeToStr( ObjectType type )
	{
		switch( type )
		{
			case TYPE_NULL: return "null";
			case TYPE_INT: return "int";
			case TYPE_FLOAT: return "float";
			case TYPE_BOOL: return "bool";
			case TYPE_STR: return "string";
			case TYPE_CALLABLE: return "function";
			default: return "<error-type>";
		}
	}

	BatObject::~BatObject()
	{
	}
	BatObject BatObject::Add( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 + rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 + rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 + rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 + (double)rhs.value.i64 );

		throw RuntimeError( loc, std::string("Cannot add ") + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Sub( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 - rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 - rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 - rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 - (double)rhs.value.i64 );

		throw RuntimeError( loc, std::string( "Cannot subtract " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Div( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 / rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 / rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 / rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 / (double)rhs.value.i64 );

		throw RuntimeError( loc, std::string( "Cannot divide " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Mul( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 * rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 * rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 * rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 * (double)rhs.value.i64 );

		throw RuntimeError( loc, std::string( "Cannot multiply " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Mod( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 % rhs.value.i64 );

		throw RuntimeError( loc, std::string( "Cannot mod " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::LShift( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 << rhs.value.i64 );

		throw RuntimeError( loc, std::string( "Cannot left-shift " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::RShift( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 >> rhs.value.i64 );

		throw RuntimeError( loc, std::string( "Cannot right-shift " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::BitXor( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 ^ rhs.value.i64 );

		throw RuntimeError( loc, std::string( "Cannot bitwise-xor " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::BitOr( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 | rhs.value.i64 );

		throw RuntimeError( loc, std::string( "Cannot bitwise-or " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::BitAnd( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 & rhs.value.i64 );

		throw RuntimeError( loc, std::string( "Cannot bitwise-and " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpEq( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == rhs.type )
		{
			switch( type )
			{
				case TYPE_NULL:
					return true;
				case TYPE_INT:
					return value.i64 == rhs.value.i64;
				case TYPE_BOOL:
					return (value.i64 != 0) == (rhs.value.i64 != 0);
				case TYPE_FLOAT:
					return value.f64 == rhs.value.f64;
				case TYPE_STR:
					return std::string( value.str ) == rhs.value.str;
			}
		}

		throw RuntimeError( loc, std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpNeq( const BatObject& rhs, const SourceLoc& loc )
	{
		return CmpEq( rhs, loc ).Not( loc );
	}
	BatObject BatObject::CmpL( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == rhs.type )
		{
			switch( type )
			{
				case TYPE_INT:
					return value.i64 < rhs.value.i64;
				case TYPE_FLOAT:
					return value.f64 < rhs.value.f64;
			}
		}

		throw RuntimeError( loc, std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpLe( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == rhs.type )
		{
			switch( type )
			{
				case TYPE_INT:
					return value.i64 <= rhs.value.i64;
				case TYPE_FLOAT:
					return value.f64 <= rhs.value.f64;
			}
		}

		throw RuntimeError( loc, std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpG( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == rhs.type )
		{
			switch( type )
			{
				case TYPE_INT:
					return value.i64 > rhs.value.i64;
				case TYPE_FLOAT:
					return value.f64 > rhs.value.f64;
			}
		}

		throw RuntimeError( loc, std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpGe( const BatObject& rhs, const SourceLoc& loc )
	{
		if( type == rhs.type )
		{
			switch( type )
			{
				case TYPE_INT:
					return value.i64 >= rhs.value.i64;
				case TYPE_FLOAT:
					return value.f64 >= rhs.value.f64;
			}
		}

		throw RuntimeError( loc, std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Not( const SourceLoc& loc )
	{
		switch( type )
		{
			case TYPE_BOOL:
			case TYPE_INT:
				return !value.i64;
		}

		throw RuntimeError( loc, std::string( "Cannot invert " ) + TypeToStr( type ) );
	}
	BatObject BatObject::Neg( const SourceLoc& loc )
	{
		switch( type )
		{
			case TYPE_FLOAT:
				return -value.f64;
			case TYPE_INT:
				return -value.i64;
		}

		throw RuntimeError( loc, std::string( "Cannot netage " ) + TypeToStr( type ) );
	}
	BatObject BatObject::BitNeg( const SourceLoc& loc )
	{
		switch( type )
		{
			case TYPE_INT:
				return ~value.i64;
		}

		throw RuntimeError( loc, std::string( "Cannot bitwise-negate " ) + TypeToStr( type ) );
	}
	BatObject BatObject::Call( Interpreter& interpreter, const std::vector<BatObject>& args, const SourceLoc& loc )
	{
		if( type != TYPE_CALLABLE )
		{
			throw RuntimeError( loc, std::string( "Cannot call " ) + TypeToStr( type ) );
		}

		return value.func->Call( interpreter, args );
	}
	std::string BatObject::ToString()
	{
		switch( type )
		{
			case TYPE_NULL:
				return "null";
			case TYPE_BOOL:
				return value.i64 ? "true" : "false";
			case TYPE_INT:
				return std::to_string( value.i64 );
			case TYPE_FLOAT:
				return std::to_string( value.f64 );
			case TYPE_STR:
				return value.str;
			default:
				return "<error>";
		}
	}
	bool BatObject::IsTruthy( const SourceLoc& loc )
	{
		switch( type )
		{
			case TYPE_NULL:
				return false;
			case TYPE_BOOL:
			case TYPE_INT:
				return value.i64;
			case TYPE_FLOAT:
				return value.f64;
		}

		throw RuntimeError( loc, std::string( "Cannot implicitly convert " ) + TypeToStr( type ) + " to bool" );
	}
}