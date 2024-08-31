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
			case TYPE_UNDEFINED: return "undefined";
			case TYPE_NULL:      return "null";
			case TYPE_INT:       return "int";
			case TYPE_FLOAT:     return "float";
			case TYPE_BOOL:      return "bool";
			case TYPE_STR:       return "string";
			case TYPE_CALLABLE:  return "function";
			case TYPE_ARRAY:     return "array";
			case TYPE_MODULE:    return "module";
			case TYPE_CLASS:     return "class";
			case TYPE_OBJECT:    return "object";
		}
		
		return "<error-type>";
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
		else if( type == TYPE_MODULE )
		{
			//delete value.mod;
		}
		// TODO: make arrays/modules not so leaky
	}
	BatObject::BatObject( const BatObject& other )
	{
		type = other.type;
		arr_size = other.arr_size;
		memset( &value, 0, sizeof( value ) );
		Copy( other );
	}
	BatObject& BatObject::operator=( const BatObject& rhs )
	{
		type = rhs.type;
		arr_size = rhs.arr_size;
		memset( &value, 0, sizeof( value ) );
		Copy( rhs );

		return *this;
	}
	BatObject::BatObject( BatObject&& donor ) noexcept
	{
		type = donor.type;
		arr_size = donor.arr_size;
		Move( donor );
		memset( &donor.value, 0, sizeof( donor.value ) );
	}
	BatObject& BatObject::operator=( BatObject&& rhs ) noexcept
	{
		type = rhs.type;
		arr_size = rhs.arr_size;
		Move( rhs );
		memset( &rhs.value, 0, sizeof( rhs.value ) );

		return *this;
	}
	BatObject BatObject::Int( int64_t i )
	{
		BatObject object;
		object.type = TYPE_INT;
		object.value.i64 = i;
		return object;
	}
	BatObject BatObject::Float( double f )
	{
		BatObject object;
		object.type = TYPE_FLOAT;
		object.value.f64 = f;
		return object;
	}
	BatObject BatObject::String( const char* s )
	{
		BatObject object;
		object.type = TYPE_STR;
		size_t len = strlen( s );
		object.value.str = new char[len + 1];
		memcpy( object.value.str, s, len + 1 );
		return object;
	}
	BatObject BatObject::Bool( bool s )
	{
		BatObject object;
		object.type = TYPE_BOOL;
		object.value.i64 = s;
		return object;
	}
	BatObject BatObject::Function( BatCallable* func )
	{
		BatObject object;
		object.type = TYPE_CALLABLE;
		object.value.func = func;
		return object;
	}
	BatObject BatObject::Array( const BatObject* arr, size_t size )
	{
		BatObject object;
		object.type = TYPE_ARRAY;
		object.arr_size = size;
		object.value.arr = new BatObject[size];
		for( size_t i = 0; i < size; i++ )
		{
			object.value.arr[i].Assign( arr[i] );
		}
		return object;
	}
	BatObject BatObject::Module( Environment* module )
	{
		BatObject object;
		object.type = TYPE_MODULE;
		object.value.env = module;
		return object;
	}
    BatObject BatObject::Class( Environment *env )
    {
		BatObject object;
		object.type = TYPE_CLASS;
		object.value.env = env;
		return object;
    }
    BatObject BatObject::Object(const char *classname, void *opaque)
    {
		BatObject object;
		object.type = TYPE_OBJECT;
		object.classname = classname;
		object.value.opaque = opaque;
		return object;
    }
    BatObject BatObject::Add(const BatObject &rhs)
    {
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject::Int( value.i64 + rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject::Float( value.f64 + rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject::Float( (double)value.i64 + rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject::Float( value.f64 + (double)rhs.value.i64 );

		// Adding a list and another object will 
		if( type == TYPE_ARRAY )
		{
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
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject::Int( value.i64 - rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject::Float( value.f64 - rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject::Float( (double)value.i64 - rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject::Float( value.f64 - (double)rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot subtract " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Div( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject::Int( value.i64 / rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject::Float( value.f64 / rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject::Float( (double)value.i64 / rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject::Float( value.f64 / (double)rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot divide " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Mul( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject::Int( value.i64 * rhs.value.i64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_FLOAT ) return BatObject::Float( value.f64 * rhs.value.f64 );
		if( type == TYPE_INT && rhs.type == TYPE_FLOAT ) return BatObject::Float( (double)value.i64 * rhs.value.f64 );
		if( type == TYPE_FLOAT && rhs.type == TYPE_INT ) return BatObject::Float( value.f64 * (double)rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot multiply " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::Mod( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject::Int( value.i64 % rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot mod " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::LShift( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject::Int( value.i64 << rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot left-shift " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::RShift( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject::Int( value.i64 >> rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot right-shift " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::BitXor( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject::Int( value.i64 ^ rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot bitwise-xor " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::BitOr( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject::Int( value.i64 | rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot bitwise-or " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::BitAnd( const BatObject& rhs )
	{
		if( type == TYPE_INT && rhs.type == TYPE_INT ) return BatObject::Int( value.i64 & rhs.value.i64 );

		throw BatObjectError( std::string( "Cannot bitwise-and " ) + TypeToStr( type ) + " and " + TypeToStr( rhs.type ) );
	}
	BatObject BatObject::CmpEq( const BatObject& rhs )
	{
		if( type == rhs.type )
		{
			switch( type )
			{
				case TYPE_UNDEFINED:
				case TYPE_NULL:
					return BatObject::True();
				case TYPE_INT:
					return BatObject::Bool( value.i64 == rhs.value.i64 );
				case TYPE_BOOL:
					return BatObject::Bool( (value.i64 != 0) == (rhs.value.i64 != 0) );
				case TYPE_FLOAT:
					return BatObject::Bool( value.f64 == rhs.value.f64 );
				case TYPE_STR:
					return BatObject::Bool( std::string( value.str ) == rhs.value.str );
				case TYPE_ARRAY:
					if ( arr_size != rhs.arr_size )
						return BatObject::False();
					for ( size_t i = 0; i < arr_size; i++ )
						if ( !value.arr[i].CmpEq( rhs.value.arr[i] ).IsTruthy() )
							return BatObject::False();
					return BatObject::True();
				case TYPE_CALLABLE:
					return BatObject::Bool( value.func == rhs.value.func );
				case TYPE_OBJECT:
					return BatObject::False(); // TODO
				case TYPE_MODULE:
				case TYPE_CLASS:
					return BatObject::Bool( value.env == rhs.value.env );
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
					return BatObject::Bool( value.i64 < rhs.value.i64 );
				case TYPE_FLOAT:
					return BatObject::Bool( value.f64 < rhs.value.f64 );
				case TYPE_STR:
					return BatObject::Bool( std::string( value.str ) < std::string( rhs.value.str ) );
				
				case TYPE_UNDEFINED:
				case TYPE_NULL:
				case TYPE_BOOL:
				case TYPE_CALLABLE:
				case TYPE_ARRAY:
				case TYPE_MODULE:
				case TYPE_OBJECT:
				case TYPE_CLASS:
					break;
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
					return BatObject::Bool( value.i64 <= rhs.value.i64 );
				case TYPE_FLOAT:
					return BatObject::Bool( value.f64 <= rhs.value.f64 );
				case TYPE_STR:
					return BatObject::Bool( std::string( value.str ) <= std::string( rhs.value.str ) );
				
				case TYPE_UNDEFINED:
				case TYPE_NULL:
				case TYPE_BOOL:
				case TYPE_CALLABLE:
				case TYPE_ARRAY:
				case TYPE_MODULE:
				case TYPE_OBJECT:
				case TYPE_CLASS:
					break;
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
					return BatObject::Bool( value.i64 > rhs.value.i64 );
				case TYPE_FLOAT:
					return BatObject::Bool( value.f64 > rhs.value.f64 );
				case TYPE_STR:
					return BatObject::Bool( std::string( value.str ) > std::string( rhs.value.str ) );
				
				case TYPE_UNDEFINED:
				case TYPE_NULL:
				case TYPE_BOOL:
				case TYPE_CALLABLE:
				case TYPE_ARRAY:
				case TYPE_MODULE:
				case TYPE_OBJECT:
				case TYPE_CLASS:
					break;
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
					return BatObject::Bool( value.i64 >= rhs.value.i64 );
				case TYPE_FLOAT:
					return BatObject::Bool( value.f64 >= rhs.value.f64 );
				case TYPE_STR:
					return BatObject::Bool( std::string( value.str ) >= std::string( rhs.value.str ) );
				
				case TYPE_UNDEFINED:
				case TYPE_NULL:
				case TYPE_BOOL:
				case TYPE_CALLABLE:
				case TYPE_ARRAY:
				case TYPE_MODULE:
				case TYPE_OBJECT:
				case TYPE_CLASS:
					break;
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
				return BatObject::Bool( !value.i64 );
			
			case TYPE_UNDEFINED:
			case TYPE_NULL:
			case TYPE_FLOAT:
			case TYPE_STR:
			case TYPE_CALLABLE:
			case TYPE_ARRAY:
			case TYPE_MODULE:
			case TYPE_OBJECT:
			case TYPE_CLASS:
				break;
		}

		throw BatObjectError( std::string( "Cannot invert " ) + TypeToStr( type ) );
	}
	BatObject BatObject::Neg()
	{
		switch( type )
		{
			case TYPE_FLOAT:
				return BatObject::Float( -value.f64 );
			case TYPE_INT:
				return BatObject::Int( -value.i64 );
			
			case TYPE_UNDEFINED:
			case TYPE_NULL:
			case TYPE_BOOL:
			case TYPE_STR:
			case TYPE_CALLABLE:
			case TYPE_ARRAY:
			case TYPE_MODULE:
			case TYPE_OBJECT:
			case TYPE_CLASS:
				break;
		}

		throw BatObjectError( std::string( "Cannot negate " ) + TypeToStr( type ) );
	}
	BatObject BatObject::BitNeg()
	{
		switch( type )
		{
			case TYPE_INT:
				return BatObject::Int( ~value.i64 );
			
			case TYPE_UNDEFINED:
			case TYPE_NULL:
			case TYPE_BOOL:
			case TYPE_FLOAT:
			case TYPE_STR:
			case TYPE_CALLABLE:
			case TYPE_ARRAY:
			case TYPE_MODULE:
			case TYPE_OBJECT:
			case TYPE_CLASS:
				break;
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

		auto i = index.GetInt();
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
	std::string BatObject::ToString() const
	{
		switch( type )
		{
			case TYPE_UNDEFINED:
				return "undefined";
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
			case TYPE_MODULE:
				return "module";
			case TYPE_CLASS:
				return classname;
			case TYPE_OBJECT:
				return "object";
		}

		return "error";
	}
    const BatObject &BatObject::GetField( const std::string &name ) const
    {
		assert( type == TYPE_OBJECT );
		return fields.at( name );
    }
    void BatObject::SetField(const std::string &name, BatObject value)
    {
		assert( type == TYPE_OBJECT );
		fields[name] = std::move( value );
    }
    void BatObject::Copy(const BatObject &other)
    {
		switch( type )
		{
			case TYPE_UNDEFINED:
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
				value.str = new char[len + 1];
				memcpy( value.str, other.value.str, len + 1 );
				break;
			}
			case TYPE_CALLABLE:
			{
				value.func = other.value.func;
				break;
			}
			case TYPE_ARRAY:
			{
				// delete[] value.arr;
				assert( false );
			}
			case TYPE_CLASS:
			{
				value.env = other.value.env; // BAD
				break;
			}
			case TYPE_MODULE:
			{
				value.env = other.value.env; // BAD
				break;
			}
			case TYPE_OBJECT:
			{
				classname = other.classname;
				fields = other.fields;
				value.opaque = other.value.opaque;
				break;
			}
		}
	}
	void BatObject::Move( const BatObject& other )
	{
		switch( type )
		{
			case TYPE_UNDEFINED:
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
				break;
			}
			case TYPE_CLASS:
			{
				value.env = other.value.env;
				break;
			}
			case TYPE_MODULE:
			{
				value.env = other.value.env;
				break;
			}
			case TYPE_OBJECT:
			{
				classname = other.classname;
				fields = std::move( other.fields );
				value.opaque = other.value.opaque;
				break;
			}
		}
	}
	bool BatObject::IsTruthy() const
	{
		switch( type )
		{
			case TYPE_UNDEFINED:
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
			case TYPE_MODULE:
			case TYPE_CLASS:
			case TYPE_OBJECT:
				return true;
		}

		throw BatObjectError( std::string( "Cannot implicitly convert " ) + TypeToStr( type ) + " to bool" );
	}
}