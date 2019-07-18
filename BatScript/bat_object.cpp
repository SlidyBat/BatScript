#include "bat_object.h"

#include <string>
#include "bat_callable.h"
#include "interpreter.h"

namespace Bat
{
	BatObject::~BatObject()
	{
		if( type == TYPE_CALLABLE )
		{
			delete value.func;
		}
	}
	BatObject BatObject::operator+( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 + rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 + rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 + rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 + (double)rhs.value.i64 );

		throw BatObjectError();
	}
	BatObject BatObject::operator-( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 - rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 - rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 - rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 - (double)rhs.value.i64 );

		throw BatObjectError();
	}
	BatObject BatObject::operator/( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 / rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 / rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 / rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 / (double)rhs.value.i64 );

		throw BatObjectError();
	}
	BatObject BatObject::operator*( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 * rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 * rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 * rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 * (double)rhs.value.i64 );

		throw BatObjectError();
	}
	BatObject BatObject::operator%( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 % rhs.value.i64 );

		throw BatObjectError();
	}
	BatObject BatObject::operator<<( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 << rhs.value.i64 );

		throw BatObjectError();
	}
	BatObject BatObject::operator>>( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 >> rhs.value.i64 );

		throw BatObjectError();
	}
	BatObject BatObject::operator^( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 ^ rhs.value.i64 );

		throw BatObjectError();
	}
	BatObject BatObject::operator|( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 | rhs.value.i64 );

		throw BatObjectError();
	}
	BatObject BatObject::operator&( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 & rhs.value.i64 );

		throw BatObjectError();
	}
	BatObject BatObject::operator==( const BatObject& rhs )
	{
		if( type != rhs.type ) throw BatObjectError();
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

		throw BatObjectError();
	}
	BatObject BatObject::operator!=( const BatObject& rhs )
	{
		return !(*this == rhs);
	}
	BatObject BatObject::operator<( const BatObject& rhs )
	{
		if( type != rhs.type ) throw BatObjectError();
		switch( type )
		{
			case TYPE_INT:
				return value.i64 < rhs.value.i64;
			case TYPE_FLOAT:
				return value.f64 < rhs.value.f64;
		}

		throw BatObjectError();
	}
	BatObject BatObject::operator<=( const BatObject& rhs )
	{
		if( type != rhs.type ) throw BatObjectError();
		switch( type )
		{
			case TYPE_INT:
				return value.i64 <= rhs.value.i64;
			case TYPE_FLOAT:
				return value.f64 <= rhs.value.f64;
		}

		throw BatObjectError();
	}
	BatObject BatObject::operator>( const BatObject& rhs )
	{
		if( type != rhs.type ) throw BatObjectError();
		switch( type )
		{
			case TYPE_INT:
				return value.i64 > rhs.value.i64;
			case TYPE_FLOAT:
				return value.f64 > rhs.value.f64;
		}

		throw BatObjectError();
	}
	BatObject BatObject::operator>=( const BatObject& rhs )
	{
		if( type != rhs.type ) throw BatObjectError();
		switch( type )
		{
			case TYPE_INT:
				return value.i64 >= rhs.value.i64;
			case TYPE_FLOAT:
				return value.f64 >= rhs.value.f64;
		}

		throw BatObjectError();
	}
	BatObject BatObject::operator!()
	{
		switch( type )
		{
			case TYPE_BOOL:
			case TYPE_INT:
				return !value.i64;
		}

		throw BatObjectError();
	}
	BatObject BatObject::operator-()
	{
		switch( type )
		{
			case TYPE_FLOAT:
				return -value.f64;
			case TYPE_INT:
				return -value.i64;
		}

		throw BatObjectError();
	}
	BatObject BatObject::operator~()
	{
		switch( type )
		{
			case TYPE_INT:
				return ~value.i64;
		}

		throw BatObjectError();
	}
	BatObject BatObject::operator()( Interpreter& interpreter, const std::vector<BatObject>& args )
	{
		if( type != TYPE_CALLABLE ) throw BatObjectError();
		if( args.size() != value.func->Arity() ) throw BatObjectError();

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
	bool BatObject::IsTruthy()
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

		throw BatObjectError();
	}
}