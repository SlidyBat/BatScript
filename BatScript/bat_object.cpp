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
			case TYPE_NULL:     return "null";
			case TYPE_INT:      return "int";
			case TYPE_FLOAT:    return "float";
			case TYPE_BOOL:     return "bool";
			case TYPE_STR:      return "string";
			case TYPE_CALLABLE: return "function";
			case TYPE_ARRAY:    return "array";
			default: return "<error-type>";
		}
	}

	BatObject::BatObject()
	{
		memset( &value, 0, sizeof( value ) );
	}

	BatObject::~BatObject()
	{
		if( type == TYPE_STR )
		{
			delete[] value.str;
		}
		// TODO: make arrays not so leaky
	}
	BatObject::BatObject( const BatObject& other )
	{
		type = other.type;
		arr_size = other.arr_size;
		fixed_arr = other.fixed_arr;
		memset( &value, 0, sizeof( value ) );
		Copy( other );
	}
	BatObject& BatObject::operator=( const BatObject& rhs )
	{
		type = rhs.type;
		arr_size = rhs.arr_size;
		fixed_arr = rhs.fixed_arr;
		memset( &value, 0, sizeof( value ) );
		Copy( rhs );

		return *this;
	}
	BatObject::BatObject( BatObject&& donor ) noexcept
	{
		type = donor.type;
		arr_size = donor.arr_size;
		fixed_arr = donor.fixed_arr;
		Move( donor );
		memset( &donor.value, 0, sizeof( donor.value ) );
	}
	BatObject& BatObject::operator=( BatObject&& rhs ) noexcept
	{
		type = rhs.type;
		arr_size = rhs.arr_size;
		fixed_arr = rhs.fixed_arr;
		Move( rhs );
		memset( &rhs.value, 0, sizeof( rhs.value ) );

		return *this;
	}
	BatObject::BatObject( int64_t i )
		:
		type( TYPE_INT )
	{
		value.i64 = i;
	}
	BatObject::BatObject( double f )
		:
		type( TYPE_FLOAT )
	{
		value.f64 = f;
	}
	BatObject::BatObject( const char* s )
		:
		type( TYPE_STR )
	{
		size_t len = strlen( s );
		value.str = new char[len];
		memcpy( value.str, s, len );
	}
	BatObject::BatObject( bool s )
		:
		type( TYPE_BOOL )
	{
		value.i64 = s;
	}
	BatObject::BatObject( BatCallable* func )
		:
		type( TYPE_CALLABLE )
	{
		value.func = func;
	}
	BatObject::BatObject( const BatObject* arr, size_t size, bool fixed_size )
		:
		type( TYPE_ARRAY ),
		arr_size( size ),
		fixed_arr( fixed_size )
	{
		value.arr = new BatObject[size];
		for( size_t i = 0; i < size; i++ )
		{
			value.arr[i].Assign( arr[i] );
		}
	}
	BatObject BatObject::Add( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 + rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 + rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 + rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 + (double)rhs.value.i64 );

		// Adding a list and another object will 
		if( type == TYPE_ARRAY )
		{
			assert( !fixed_arr );
			// TODO: What about appending array to end of another array?
			BatObject res;
			res.type = TYPE_ARRAY;
			res.arr_size = arr_size + 1;
			res.value.arr = new BatObject[res.arr_size];
			for( size_t i = 0; i < arr_size; i++ )
			{
				res.value.arr[i] = value.arr[i];
			}
			res.value.arr[arr_size] = rhs;
			return res;
		}

		throw BatObjectError( std::string("Cannot add ") + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Sub( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 - rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 - rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 - rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 - (double)rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot subtract " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Div( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 / rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 / rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 / rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 / (double)rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot divide " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Mul( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 * rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject( value.f64 * rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject( (double)value.i64 * rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject( value.f64 * (double)rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot multiply " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Mod( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 % rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot mod " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::LShift( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 << rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot left-shift " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::RShift( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 >> rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot right-shift " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::BitXor( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 ^ rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot bitwise-xor " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::BitOr( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 | rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot bitwise-or " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::BitAnd( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject( value.i64 & rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot bitwise-and " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpEq( const BatObject& rhs )
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

		throw BatObjectError( std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpNeq( const BatObject& rhs )
	{
		return CmpEq( rhs ).Not();
	}
	BatObject BatObject::CmpL( const BatObject& rhs )
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

		throw BatObjectError( std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpLe( const BatObject& rhs )
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

		throw BatObjectError( std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpG( const BatObject& rhs )
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

		throw BatObjectError( std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpGe( const BatObject& rhs )
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

		throw BatObjectError( std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Not()
	{
		switch( type )
		{
			case TYPE_BOOL:
			case TYPE_INT:
				return !value.i64;
		}

		throw BatObjectError( std::string( "Cannot invert " ) + TypeToStr( type ) );
	}
	BatObject BatObject::Neg()
	{
		switch( type )
		{
			case TYPE_FLOAT:
				return -value.f64;
			case TYPE_INT:
				return -value.i64;
		}

		throw BatObjectError( std::string( "Cannot netage " ) + TypeToStr( type ) );
	}
	BatObject BatObject::BitNeg()
	{
		switch( type )
		{
			case TYPE_INT:
				return ~value.i64;
		}

		throw BatObjectError( std::string( "Cannot bitwise-negate " ) + TypeToStr( type ) );
	}
	BatObject BatObject::Call( Interpreter& interpreter, const std::vector<BatObject>& args )
	{
		if( type != TYPE_CALLABLE )
		{
			throw BatObjectError( std::string( "Cannot call " ) + TypeToStr( type ) );
		}

		return value.func->Call( interpreter, args );
	}
	BatObject& BatObject::Index( const BatObject& index )
	{
		if( type != TYPE_ARRAY )
		{
			throw BatObjectError( std::string( "Cannot index " ) + TypeToStr( type ) );
		}

		if( index.type != TYPE_INT )
		{
			throw BatObjectError( std::string( "Array index must be an integer" ) );
		}

		auto i = index.Int();
		if( i < 0 )
		{
			i += arr_size;
		}
#ifdef _DEBUG
		if( i >= (int64_t)arr_size )
		{
			throw BatObjectError( std::string( "Array index " ) + std::to_string( i ) + " out of bounds" );
		}
#endif

		return value.arr[i];
	}
	void BatObject::Assign( const BatObject& other )
	{
		if( type == TYPE_UNDEFINED )
		{
			type = other.type;
		}
		else if( type != other.type )
		{
			throw BatObjectError( std::string( "Cannot assign object of type " ) + TypeToStr( other.type ) + " to object of type " + TypeToStr( type ) );
		}

		Copy( other );
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
			case TYPE_CALLABLE:
				return "function";
			case TYPE_ARRAY:
			{
				std::string s = "[";
				for( size_t i = 0; i < arr_size; i++ )
				{
					s += value.arr[i].ToString();
					s += ",";
				}
				s += "]";
			}
			default:
				return "<error>";
		}
	}
	void BatObject::Copy( const BatObject& other )
	{
		switch( type )
		{
			case TYPE_NULL:
			{
				break;
			}
			case TYPE_BOOL:
			case TYPE_INT:
			{
				value.i64 = other.value.i64;
				break;
			}
			case TYPE_FLOAT:
			{
				value.f64 = other.value.f64;
				break;
			}
			case TYPE_STR:
			{
				delete[] value.str;

				size_t len = strlen( other.value.str );
				value.str = new char[len];
				memcpy( value.str, other.value.str, len );
			}
			case TYPE_CALLABLE:
			{
				value.func = other.value.func;
				break;
			}
			case TYPE_ARRAY:
			{
				// Can only assign array if it's a dynamic array (in which case it can be resized) or the sizes of the arrays match
				if( fixed_arr == false || other.arr_size == arr_size )
				{
					value.arr = other.value.arr;
					ref_count = other.ref_count + 1;
				}
				else
				{
					throw BatObjectError( "Cannot assign a dynamic array to a fixed-length array" );
				}
			}
		}
	}
	void BatObject::Move( const BatObject& other )
	{
		switch( type )
		{
			case TYPE_NULL:
			{
				break;
			}
			case TYPE_BOOL:
			case TYPE_INT:
			{
				value.i64 = other.value.i64;
				break;
			}
			case TYPE_FLOAT:
			{
				value.f64 = other.value.f64;
				break;
			}
			case TYPE_STR:
			{
				value.str = other.value.str;
				break;
			}
			case TYPE_CALLABLE:
			{
				value.func = other.value.func;
				break;
			}
			case TYPE_ARRAY:
			{
				value.arr = other.value.arr;
			}
		}
	}
	bool BatObject::IsTruthy() const
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
			case TYPE_STR:
			case TYPE_CALLABLE:
			case TYPE_ARRAY:
				return true;
		}

		throw BatObjectError( std::string( "Cannot implicitly convert " ) + TypeToStr( type ) + " to bool" );
	}
}