#pragma once

#include <cstdint>
#include <exception>
#include <string>
#include <vector>
#include <cassert>
#include "type.h"

namespace Bat
{
	class BatCallable;
	class BatObject;
	class Interpreter;
	class Environment;

	class BatObjectError : public std::exception
	{
	public:
		BatObjectError( const std::string& message )
			:
			message( message )
		{}

		virtual char const* what() const noexcept override
		{
			return message.c_str();
		}
	private:
		std::string message;
	};

	enum ObjectType
	{
		TYPE_UNDEFINED,
		TYPE_NULL,
		TYPE_INT,
		TYPE_FLOAT,
		TYPE_BOOL,
		TYPE_STR,
		TYPE_CALLABLE,
		TYPE_ARRAY,
		TYPE_MODULE
	};

	class BatObject
	{
	public:
		BatObject();
		~BatObject();
		BatObject( const BatObject& other );
		BatObject& operator=( const BatObject& rhs );
		BatObject( BatObject&& donor ) noexcept;
		BatObject& operator=( BatObject&& rhs ) noexcept;
		BatObject( int64_t i );
		BatObject( double f );
		BatObject( const char* s );
		BatObject( bool s );
		BatObject( BatCallable* func );
		BatObject( const BatObject* arr, size_t arr_size );
		BatObject( Environment* module );

		BatObject Add( const BatObject& rhs );
		BatObject Sub( const BatObject& rhs );
		BatObject Div( const BatObject& rhs );
		BatObject Mul( const BatObject& rhs );
		BatObject Mod( const BatObject& rhs );
		BatObject LShift( const BatObject& rhs );
		BatObject RShift( const BatObject& rhs );
		BatObject BitXor( const BatObject& rhs );
		BatObject BitOr( const BatObject& rhs );
		BatObject BitAnd( const BatObject& rhs );
		BatObject CmpEq( const BatObject& rhs );
		BatObject CmpNeq( const BatObject& rhs );
		BatObject CmpL( const BatObject& rhs );
		BatObject CmpLe( const BatObject& rhs );
		BatObject CmpG( const BatObject& rhs );
		BatObject CmpGe( const BatObject& rhs );
		BatObject Not();
		BatObject Neg();
		BatObject BitNeg();
		BatObject Call( Interpreter& interpreter, const std::vector<BatObject>& args );
		BatObject& Index( const BatObject& index );

		void Assign( const BatObject& other );

		bool IsTruthy() const;

		std::string ToString() const;

		bool Bool() const { assert( type == TYPE_BOOL ); return value.i64; }
		int64_t Int() const { assert( type == TYPE_INT ); return value.i64; }
		double Float() const { assert( type == TYPE_FLOAT ); return value.f64; }
		const char* String() const { assert( type == TYPE_STR ); return value.str; }
		BatCallable* Function() const { assert( type == TYPE_CALLABLE ); return value.func; }
		BatObject* Array() const { assert( type == TYPE_ARRAY ); return value.arr; }
		Environment* Module() const { assert( type == TYPE_MODULE ); return value.mod; }
	private:
		void Copy( const BatObject& other );
		void Move( const BatObject& other );
	public:
		ObjectType type = TYPE_UNDEFINED;
		union ObjectValues
		{
			ObjectValues() {}
			~ObjectValues() {}

			int64_t i64;
			double f64;
			char* str;
			BatCallable* func;
			BatObject* arr;
			Environment* mod;
		} value;
		size_t arr_size = 0;
	};

	template <typename T>
	struct AlwaysFalse
	{
		static constexpr bool value = false;
	};

	template <typename T>
	T FromBatObject( const BatObject& obj )
	{
		static_assert( AlwaysFalse<T>::value, "FromBatObject unimplemented!" );
	}

	template <typename T>
	BatObject ToBatObject( const T& val )
	{
		static_assert( AlwaysFalse<T>::value, "ToBatObject unimplemented!" );
	}

	template <> inline int FromBatObject( const BatObject& obj ) { return obj.Int(); }
	template <> inline int64_t FromBatObject( const BatObject& obj ) { return obj.Int(); }
	template <> inline uint64_t FromBatObject( const BatObject& obj ) { return obj.Int(); }
	template <> inline bool FromBatObject( const BatObject& obj ) { return obj.Bool(); }
	template <> inline float FromBatObject( const BatObject& obj ) { return obj.Float(); }
	template <> inline double FromBatObject( const BatObject& obj ) { return obj.Float(); }
	template <> inline std::string FromBatObject( const BatObject& obj ) { return obj.String(); }
	template <> inline const char* FromBatObject( const BatObject& obj ) { return obj.String(); }

	template <> inline BatObject ToBatObject( const int& val ) { return BatObject( (int64_t)val ); }
	template <> inline BatObject ToBatObject( const int64_t& val ) { return BatObject( val ); }
	template <> inline BatObject ToBatObject( const uint64_t& val ) { return BatObject( (int64_t)val ); }
	template <> inline BatObject ToBatObject( const bool& val ) { return BatObject( val ); }
	template <> inline BatObject ToBatObject( const float& val ) { return BatObject( (double)val ); }
	template <> inline BatObject ToBatObject( const double& val ) { return BatObject( val ); }
	template <> inline BatObject ToBatObject( const std::string& val ) { return BatObject( val.c_str() ); }
	template <> inline BatObject ToBatObject( const char* const& val ) { return BatObject( val ); }
}