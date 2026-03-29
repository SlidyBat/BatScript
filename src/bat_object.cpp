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
			case TYPE_TUPLE:     return "tuple";
			case TYPE_MODULE:    return "module";
			case TYPE_CLASS:     return "class";
			case TYPE_OBJECT:    return "object";
		}
		
		return "<error-type>";
	}

	BatObject::BatObject()
	{
	}

	BatObject::~BatObject()
	{
		if( impl )
		{
			impl->refcnt--;
			if (impl->refcnt == 0)
				delete impl;
		}
	}
	BatObject::BatObject( const BatObject& other )
	{
		impl = other.impl;
		if( impl )
			impl->refcnt++;
	}
	BatObject& BatObject::operator=( const BatObject& rhs )
	{
		if( impl != rhs.impl )
		{
			if( impl && --impl->refcnt == 0 )
				delete impl;
			impl = rhs.impl;
			if( impl )
				impl->refcnt++;
		}
		return *this;

		return *this;
	}
	BatObject::BatObject( BatObject&& donor ) noexcept
	{
		impl = donor.impl;
		donor.impl = nullptr;
	}
	BatObject& BatObject::operator=( BatObject&& rhs ) noexcept
	{
    	if( impl && --impl->refcnt == 0 )
			delete impl;
		impl = rhs.impl;
		rhs.impl = nullptr;

		return *this;
	}
	BatObject BatObject::Int( int64_t i )
	{
		BatObject object = BatObject::New( TYPE_INT );
		object.Value().i64 = i;
		return object;
	}
	BatObject BatObject::Float( double f )
	{
		BatObject object = BatObject::New( TYPE_FLOAT );
		object.Value().f64 = f;
		return object;
	}
	BatObject BatObject::String( const char* s )
	{
		BatObject object = BatObject::New( TYPE_STR );
		size_t len = strlen( s );
		object.Value().str = new char[len + 1];
		memcpy( object.Value().str, s, len + 1 );
		object.impl->arr_size = len;
		return object;
	}
    BatObject BatObject::Bytes(const char *s, size_t len)
    {
		BatObject object = BatObject::New( TYPE_STR );
		object.Value().str = new char[len + 1];
		memcpy( object.Value().str, s, len );
		object.Value().str[len] = '\0';
		object.impl->arr_size = len;
		return object;
    }
    BatObject BatObject::Bool(bool s)
    {
		BatObject object = BatObject::New( TYPE_BOOL );
		object.Value().i64 = s;
		return object;
	}
	BatObject BatObject::Function( BatCallable* func )
	{
		BatObject object = BatObject::New( TYPE_CALLABLE );
		object.Value().func = func;
		return object;
	}
	BatObject BatObject::Array( const BatObject* arr, size_t size )
	{
		BatObject object = BatObject::New( TYPE_ARRAY );
		object.impl->arr_size = size;
		if( size > 0 )
		{
			object.Value().arr = new BatObject[size];
			for( size_t i = 0; i < size; i++ )
			{
				object.Value().arr[i] = arr[i];
			}
		}
		else
		{
			object.Value().arr = nullptr;
		}
		return object;
	}
	BatObject BatObject::Tuple( const BatObject* elements, size_t size )
	{
		BatObject object = BatObject::New( TYPE_TUPLE );
		object.impl->arr_size = size;
		if( size > 0 )
		{
			object.Value().arr = new BatObject[size];
			for( size_t i = 0; i < size; i++ )
			{
				object.Value().arr[i] = elements[i];
			}
		}
		else
		{
			object.Value().arr = nullptr;
		}
		return object;
	}
	BatObject BatObject::Module( Environment* module )
	{
		BatObject object = BatObject::New( TYPE_MODULE );
		object.Value().env = module;
		return object;
	}
    BatObject BatObject::Class( Environment *env )
    {
		BatObject object = BatObject::New( TYPE_CLASS );
		object.Value().env = env;
		return object;
    }
    BatObject BatObject::Object( const char *classname, void *opaque )
    {
		BatObject object = BatObject::New( TYPE_OBJECT );
		object.impl->classname = classname;
		object.Value().opaque = opaque;
		return object;
    }
    BatObject BatObject::Add( const BatObject &rhs )
    {
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type == TYPE_NULL || rhs_type == TYPE_NULL )
			throw BatObjectError( std::string("Cannot add ") + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );

		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();

		if( type == TYPE_INT && rhs_type == TYPE_INT ) return BatObject::Int( value.i64 + rhs_value.i64 );
		if( type == TYPE_FLOAT && rhs_type == TYPE_FLOAT ) return BatObject::Float( value.f64 + rhs_value.f64 );
		if( type == TYPE_INT && rhs_type == TYPE_FLOAT ) return BatObject::Float( (double)value.i64 + rhs_value.f64 );
		if( type == TYPE_FLOAT && rhs_type == TYPE_INT ) return BatObject::Float( value.f64 + (double)rhs_value.i64 );

		if( type == TYPE_STR && rhs_type == TYPE_STR )
		{
			BatObject res = BatObject::New( TYPE_STR );
			res.impl->arr_size = impl->arr_size + rhs.impl->arr_size;
			char* str = new char[res.impl->arr_size + 1];
			memcpy( str, value.str, impl->arr_size );
			memcpy( str + impl->arr_size, rhs_value.str, rhs.impl->arr_size );
			str[res.impl->arr_size] = '\0';
			res.Value().str = str;
			return res;
		}

		// Adding a list and another object will append that object to the list
		if( type == TYPE_ARRAY )
		{
			// TODO: What about appending array to end of another array?
			BatObject res = BatObject::New( TYPE_ARRAY );
			res.impl->arr_size = impl->arr_size + 1;
			res.Value().arr = new BatObject[res.impl->arr_size];
			for( size_t i = 0; i < impl->arr_size; i++ )
			{
				res.Value().arr[i] = value.arr[i];
			}
			res.Value().arr[impl->arr_size] = rhs;
			return res;
		}

		throw BatObjectError( std::string("Cannot add ") + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );
	}
	BatObject BatObject::Sub( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type == TYPE_NULL || rhs_type == TYPE_NULL )
			throw BatObjectError( std::string( "Cannot subtract " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );
	
		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();

		if( type == TYPE_INT && rhs_type == TYPE_INT ) return BatObject::Int( value.i64 - rhs_value.i64 );
		if( type == TYPE_FLOAT && rhs_type == TYPE_FLOAT ) return BatObject::Float( value.f64 - rhs_value.f64 );
		if( type == TYPE_INT && rhs_type == TYPE_FLOAT ) return BatObject::Float( (double)value.i64 - rhs_value.f64 );
		if( type == TYPE_FLOAT && rhs_type == TYPE_INT ) return BatObject::Float( value.f64 - (double)rhs_value.i64 );

		throw BatObjectError( std::string( "Cannot subtract " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );
	}
	BatObject BatObject::Div( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type == TYPE_NULL || rhs_type == TYPE_NULL )
			throw BatObjectError( std::string( "Cannot divide " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );

		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();

		if( rhs_value.i64 == 0 )
			throw BatObjectError("Division by zero");

		if( type == TYPE_INT && rhs_type == TYPE_INT ) return BatObject::Int( value.i64 / rhs_value.i64 );
		if( type == TYPE_FLOAT && rhs_type == TYPE_FLOAT ) return BatObject::Float( value.f64 / rhs_value.f64 );
		if( type == TYPE_INT && rhs_type == TYPE_FLOAT ) return BatObject::Float( (double)value.i64 / rhs_value.f64 );
		if( type == TYPE_FLOAT && rhs_type == TYPE_INT ) return BatObject::Float( value.f64 / (double)rhs_value.i64 );

		throw BatObjectError( std::string( "Cannot divide " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );
	}
	BatObject BatObject::Mul( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type == TYPE_NULL || rhs_type == TYPE_NULL )
			throw BatObjectError( std::string( "Cannot multiply " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );

		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();
		
		if( type == TYPE_INT && rhs_type == TYPE_INT ) return BatObject::Int( value.i64 * rhs_value.i64 );
		if( type == TYPE_FLOAT && rhs_type == TYPE_FLOAT ) return BatObject::Float( value.f64 * rhs_value.f64 );
		if( type == TYPE_INT && rhs_type == TYPE_FLOAT ) return BatObject::Float( (double)value.i64 * rhs_value.f64 );
		if( type == TYPE_FLOAT && rhs_type == TYPE_INT ) return BatObject::Float( value.f64 * (double)rhs_value.i64 );

		throw BatObjectError( std::string( "Cannot multiply " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );
	}
	BatObject BatObject::Mod( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type != TYPE_INT || rhs_type != TYPE_INT )
			throw BatObjectError( std::string( "Cannot mod " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );

		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();

		return BatObject::Int( value.i64 % rhs_value.i64 );
	}
	BatObject BatObject::LShift( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type != TYPE_INT || rhs_type != TYPE_INT )
			throw BatObjectError( std::string( "Cannot left-shift " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );

		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();

		return BatObject::Int( value.i64 << rhs_value.i64 );
	}
	BatObject BatObject::RShift( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type != TYPE_INT || rhs_type != TYPE_INT )
			throw BatObjectError( std::string( "Cannot right-shift " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );

		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();

		return BatObject::Int( value.i64 >> rhs_value.i64 );
	}
	BatObject BatObject::BitXor( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type != TYPE_INT || rhs_type != TYPE_INT )
			throw BatObjectError( std::string( "Cannot bitwise-xor " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );

		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();
		
		return BatObject::Int( value.i64 ^ rhs_value.i64 );
	}
	BatObject BatObject::BitOr( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type != TYPE_INT || rhs_type != TYPE_INT )
			throw BatObjectError( std::string( "Cannot bitwise-or " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );

		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();
		
		return BatObject::Int( value.i64 | rhs_value.i64 );
	}
	BatObject BatObject::BitAnd( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type != TYPE_INT || rhs_type != TYPE_INT )
			throw BatObjectError( std::string( "Cannot bitwise-and " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );

		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();
		
		return BatObject::Int( value.i64 & rhs_value.i64 );
	}
	BatObject BatObject::CmpEq( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();

		if( type == rhs_type )
		{
			if( type == TYPE_UNDEFINED || type == TYPE_NULL )
				return BatObject::True();

			const ObjectValues& value = Value();
			const ObjectValues& rhs_value = rhs.Value();

			switch( type )
			{
				case TYPE_UNDEFINED:
				case TYPE_NULL:
					return BatObject::True();
				case TYPE_INT:
					return BatObject::Bool( value.i64 == rhs_value.i64 );
				case TYPE_BOOL:
					return BatObject::Bool( (value.i64 != 0) == (rhs_value.i64 != 0) );
				case TYPE_FLOAT:
					return BatObject::Bool( value.f64 == rhs_value.f64 );
				case TYPE_STR:
					if ( impl->arr_size != rhs.impl->arr_size )
						return BatObject::False();
					return BatObject::Bool( memcmp( value.str, rhs_value.str, impl->arr_size ) == 0 );
				case TYPE_ARRAY:
					if ( impl->arr_size != rhs.impl->arr_size )
						return BatObject::False();
					for ( size_t i = 0; i < impl->arr_size; i++ )
						if ( !value.arr[i].CmpEq( rhs_value.arr[i] ).IsTruthy() )
							return BatObject::False();
					return BatObject::True();
				case TYPE_TUPLE:
					if ( impl->arr_size != rhs.impl->arr_size )
						return BatObject::False();
					for ( size_t i = 0; i < impl->arr_size; i++ )
						if ( !value.arr[i].CmpEq( rhs_value.arr[i] ).IsTruthy() )
							return BatObject::False();
					return BatObject::True();
				case TYPE_CALLABLE:
					return BatObject::Bool( value.func == rhs_value.func );
				case TYPE_OBJECT:
					return BatObject::False(); // TODO
				case TYPE_MODULE:
				case TYPE_CLASS:
					return BatObject::Bool( value.env == rhs_value.env );
			}
		}

		throw BatObjectError( std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );
	}
	BatObject BatObject::CmpNeq( const BatObject& rhs )
	{
		return CmpEq( rhs ).Not();
	}
	BatObject BatObject::CmpL( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();
		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();

		if( type == rhs_type )
		{
			switch( type )
			{
				case TYPE_INT:
					return BatObject::Bool( value.i64 < rhs_value.i64 );
				case TYPE_FLOAT:
					return BatObject::Bool( value.f64 < rhs_value.f64 );
				case TYPE_STR:
					return BatObject::Bool( std::string( value.str ) < std::string( rhs_value.str ) ); // TODO: Use memcmp
				
				case TYPE_UNDEFINED:
				case TYPE_NULL:
				case TYPE_BOOL:
				case TYPE_CALLABLE:
				case TYPE_ARRAY:
				case TYPE_TUPLE:
				case TYPE_MODULE:
				case TYPE_OBJECT:
				case TYPE_CLASS:
					break;
			}
		}

		throw BatObjectError( std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );
	}
	BatObject BatObject::CmpLe( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();
		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();

		if( type == rhs_type )
		{
			switch( type )
			{
				case TYPE_INT:
					return BatObject::Bool( value.i64 <= rhs_value.i64 );
				case TYPE_FLOAT:
					return BatObject::Bool( value.f64 <= rhs_value.f64 );
				case TYPE_STR:
					return BatObject::Bool( std::string( value.str ) <= std::string( rhs_value.str ) ); // TODO: Use memcmp
				
				case TYPE_UNDEFINED:
				case TYPE_NULL:
				case TYPE_BOOL:
				case TYPE_CALLABLE:
				case TYPE_ARRAY:
				case TYPE_TUPLE:
				case TYPE_MODULE:
				case TYPE_OBJECT:
				case TYPE_CLASS:
					break;
			}
		}

		throw BatObjectError( std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );
	}
	BatObject BatObject::CmpG( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();
		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();

		if( type == rhs_type )
		{
			switch( type )
			{
				case TYPE_INT:
					return BatObject::Bool( value.i64 > rhs_value.i64 );
				case TYPE_FLOAT:
					return BatObject::Bool( value.f64 > rhs_value.f64 );
				case TYPE_STR:
					return BatObject::Bool( std::string( value.str ) > std::string( rhs_value.str ) ); // TODO: Use memcmp
				
				case TYPE_UNDEFINED:
				case TYPE_NULL:
				case TYPE_BOOL:
				case TYPE_CALLABLE:
				case TYPE_ARRAY:
				case TYPE_TUPLE:
				case TYPE_MODULE:
				case TYPE_OBJECT:
				case TYPE_CLASS:
					break;
			}
		}

		throw BatObjectError( std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );
	}
	BatObject BatObject::CmpGe( const BatObject& rhs )
	{
		const ObjectType type = Type();
		const ObjectType rhs_type = rhs.Type();
		const ObjectValues& value = Value();
		const ObjectValues& rhs_value = rhs.Value();

		if( type == rhs_type )
		{
			switch( type )
			{
				case TYPE_INT:
					return BatObject::Bool( value.i64 >= rhs_value.i64 );
				case TYPE_FLOAT:
					return BatObject::Bool( value.f64 >= rhs_value.f64 );
				case TYPE_STR:
					return BatObject::Bool( std::string( value.str ) >= std::string( rhs_value.str ) );
				
				case TYPE_UNDEFINED:
				case TYPE_NULL:
				case TYPE_BOOL:
				case TYPE_CALLABLE:
				case TYPE_ARRAY:
				case TYPE_TUPLE:
				case TYPE_MODULE:
				case TYPE_OBJECT:
				case TYPE_CLASS:
					break;
			}
		}

		throw BatObjectError( std::string( "Cannot compare " ) + TypeToStr( type ) + " and " + TypeToStr( rhs_type ) );
	}
	BatObject BatObject::Not()
	{
		const ObjectType type = Type();
		const ObjectValues& value = Value();

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
			case TYPE_TUPLE:
			case TYPE_MODULE:
			case TYPE_OBJECT:
			case TYPE_CLASS:
				break;
		}

		throw BatObjectError( std::string( "Cannot invert " ) + TypeToStr( type ) );
	}
	BatObject BatObject::Neg()
	{
		const ObjectType type = Type();
		const ObjectValues& value = Value();

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
			case TYPE_TUPLE:
			case TYPE_MODULE:
			case TYPE_OBJECT:
			case TYPE_CLASS:
				break;
		}

		throw BatObjectError( std::string( "Cannot negate " ) + TypeToStr( type ) );
	}
	BatObject BatObject::BitNeg()
	{
		const ObjectType type = Type();
		const ObjectValues& value = Value();

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
			case TYPE_TUPLE:
			case TYPE_MODULE:
			case TYPE_OBJECT:
			case TYPE_CLASS:
				break;
		}

		throw BatObjectError( std::string( "Cannot bitwise-negate " ) + TypeToStr( type ) );
	}
	BatObject BatObject::Call( Interpreter& interpreter, const std::vector<BatObject>& args ) const
	{
		const ObjectType type = Type();

		if( type != TYPE_CALLABLE )
		{
			throw BatObjectError( std::string( "Cannot call " ) + TypeToStr( type ) );
		}

		const ObjectValues& value = Value();

		return value.func->Call( interpreter, args );
	}
	BatObject BatObject::Index( const BatObject& index )
	{
		const ObjectType type = Type();

		if( type != TYPE_ARRAY && type != TYPE_TUPLE && type != TYPE_STR )
		{
			throw BatObjectError( std::string( "Cannot index " ) + TypeToStr( type ) );
		}

		if( index.Type() != TYPE_INT )
		{
			throw BatObjectError( std::string( "Index must be an integer" ) );
		}

		const ObjectValues& value = Value();

		auto i = index.GetInt();
		if( i < 0 )
		{
			i += impl->arr_size;
		}

		if( (uint64_t)i >= impl->arr_size )
		{
			throw BatObjectError( std::string( "Index " ) + std::to_string( i ) + " out of bounds" );
		}

		if ( type == TYPE_ARRAY || type == TYPE_TUPLE )
		{
			return value.arr[i];
		}
		else
		{
			return BatObject::Bytes( &value.str[i], 1 );
		}
	}
	BatObject BatObject::Slice( const BatObject& begin, const BatObject& end )
	{
		const ObjectType type = Type();

		if( type != TYPE_ARRAY && type != TYPE_TUPLE && type != TYPE_STR )
		{
			throw BatObjectError( std::string( "Cannot slice " ) + TypeToStr( type ) );
		}

		if( begin.Type() != TYPE_NULL && begin.Type() != TYPE_INT )
		{
			throw BatObjectError( std::string( "Slice begin index must be an integer" ) );
		}
		if( end.Type() != TYPE_NULL && end.Type() != TYPE_INT )
		{
			throw BatObjectError( std::string( "Slice end index must be an integer" ) );
		}

		const ObjectValues& value = Value();

		int64_t begin_i = (begin.Type() == TYPE_INT) ? begin.GetInt() : 0;
		if( begin_i < 0 )
		{
			begin_i += impl->arr_size;
			if ( begin_i < 0 )
				begin_i = 0;
		}
		if( begin_i > impl->arr_size )
			begin_i = impl->arr_size;

		int64_t end_i = (end.Type() == TYPE_INT) ? end.GetInt() : impl->arr_size;
		if( end_i < 0 )
		{
			end_i += impl->arr_size;
			if ( end_i < 0 )
				end_i = 0;
		}
		if( end_i > impl->arr_size )
			end_i = impl->arr_size;

		if ( end_i < begin_i )
			end_i = begin_i;

		if ( type == TYPE_ARRAY )
			return BatObject::Array( &value.arr[begin_i], end_i - begin_i );
		else if ( type == TYPE_TUPLE )
			return BatObject::Tuple( &value.arr[begin_i], end_i - begin_i );
		else
			return BatObject::Bytes( &value.str[begin_i], end_i - begin_i );
	}
	std::string BatObject::ToString() const
	{
		const ObjectValues& value = Value();

		switch( Type() )
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
				for( size_t i = 0; i < impl->arr_size; i++ )
				{
					s += value.arr[i].ToString();
					s += ",";
				}
				s += "]";
				return s;
			}
			case TYPE_TUPLE:
			{
				std::string s = "(";
				for( size_t i = 0; i < impl->arr_size; i++ )
				{
					if( i > 0 ) s += ", ";
					s += value.arr[i].ToString();
				}
				if( impl->arr_size == 1 ) s += ",";
				s += ")";
				return s;
			}
			case TYPE_MODULE:
				return "module";
			case TYPE_CLASS:
				return impl->classname;
			case TYPE_OBJECT:
				return "object";
		}

		return "error";
	}
    const BatObject &BatObject::GetField( const std::string &name ) const
    {
		assert( Type() == TYPE_OBJECT );
		return impl->fields.at( name );
    }
    void BatObject::SetField( const std::string &name, BatObject value )
    {
		assert( Type() == TYPE_OBJECT );
		impl->fields[name] = std::move( value );
    }
    BatObject BatObject::New( ObjectType type )
    {
		BatObject object;
		object.impl = new BatObjectImpl;
		object.impl->type = type;
		return object;
    }
	bool BatObject::IsTruthy() const
	{
		const ObjectValues& value = Value();

		switch( Type() )
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
			case TYPE_TUPLE:
			case TYPE_MODULE:
			case TYPE_CLASS:
			case TYPE_OBJECT:
				return true;
		}

		throw BatObjectError( std::string( "Cannot implicitly convert " ) + TypeToStr( Type() ) + " to bool" );
	}
	
    BatObject::BatObjectImpl::BatObjectImpl()
    {
		memset(&value, 0, sizeof(value));
    }
	BatObject::BatObjectImpl::~BatObjectImpl()
	{
		if( type == TYPE_STR )
		{
			delete[] value.str;
		}
		else if( type == TYPE_MODULE || type == TYPE_CLASS )
		{
			delete value.env;
		}
		else if( type == TYPE_ARRAY || type == TYPE_TUPLE )
		{
			delete[] value.arr;
		}
		else if( type == TYPE_CALLABLE )
		{
			delete value.func;
		}
	}
}