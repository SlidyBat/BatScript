#pragma once

#include <cstdint>
#include <exception>
#include <string>
#include <vector>
#include <unordered_map>
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
		TYPE_OBJECT,
		TYPE_CLASS,
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

		static BatObject Int( int64_t i );
		static BatObject Float( double f );
		static BatObject String( const char* s );
		static BatObject Bool( bool s );
		static BatObject Function( BatCallable* func );
		static BatObject Array( const BatObject* arr, size_t arr_size );
		static BatObject Module( Environment* module );
		static BatObject Class( Environment* env );
		static BatObject Object( const char* classname, void* opaque = nullptr );

		static BatObject True() { return Bool( true ); }
		static BatObject False() { return Bool( false ); }

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

		bool GetBool() const { assert( type == TYPE_BOOL ); return value.i64; }
		int64_t GetInt() const { assert( type == TYPE_INT ); return value.i64; }
		double GetFloat() const { assert( type == TYPE_FLOAT ); return value.f64; }
		const char* GetString() const { assert( type == TYPE_STR ); return value.str; }
		BatCallable* GetFunction() const { assert( type == TYPE_CALLABLE ); return value.func; }
		BatObject* GetArray() const { assert( type == TYPE_ARRAY ); return value.arr; }
		Environment* GetModule() const { assert( type == TYPE_MODULE ); return value.env; }
		Environment* GetClass() const { assert( type == TYPE_CLASS ); return value.env; }

		void* GetOpaque() const { assert( type == TYPE_OBJECT ); return value.opaque; }
		void SetOpaque( void* opaque ) { assert( type == TYPE_OBJECT ); value.opaque = opaque; }

		const BatObject& GetField( const std::string& name ) const;
		void SetField( const std::string& name, BatObject value );
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
			Environment* env;
			void* opaque;
		} value;
		size_t arr_size = 0;
		const char* classname;
		std::unordered_map<std::string, BatObject> fields;
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

	template <> inline BatObject FromBatObject( const BatObject& obj ) { return obj; }
	template <> inline int FromBatObject( const BatObject& obj ) { return obj.GetInt(); }
	template <> inline int64_t FromBatObject( const BatObject& obj ) { return obj.GetInt(); }
	template <> inline uint64_t FromBatObject( const BatObject& obj ) { return obj.GetInt(); }
	template <> inline bool FromBatObject( const BatObject& obj ) { return obj.GetBool(); }
	template <> inline float FromBatObject( const BatObject& obj ) { return obj.GetFloat(); }
	template <> inline double FromBatObject( const BatObject& obj ) { return obj.GetFloat(); }
	template <> inline std::string FromBatObject( const BatObject& obj ) { return obj.GetString(); }
	template <> inline const char* FromBatObject( const BatObject& obj ) { return obj.GetString(); }
	template <> inline void* FromBatObject( const BatObject& obj ) { return obj.GetOpaque(); }

	template <> inline BatObject ToBatObject( const BatObject& val ) { return val; }
	template <> inline BatObject ToBatObject( const int& val ) { return BatObject::Int( (int64_t)val ); }
	template <> inline BatObject ToBatObject( const int64_t& val ) { return BatObject::Int( val ); }
	template <> inline BatObject ToBatObject( const uint64_t& val ) { return BatObject::Int( (int64_t)val ); }
	template <> inline BatObject ToBatObject( const bool& val ) { return BatObject::Bool( val ); }
	template <> inline BatObject ToBatObject( const float& val ) { return BatObject::Float( (double)val ); }
	template <> inline BatObject ToBatObject( const double& val ) { return BatObject::Float( val ); }
	template <> inline BatObject ToBatObject( const std::string& val ) { return BatObject::String( val.c_str() ); }
	template <> inline BatObject ToBatObject( const char* const& val ) { return BatObject::String( val ); }
}