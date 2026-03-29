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
		TYPE_TUPLE,
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
		static BatObject Bytes( const char* s, size_t len );
		static BatObject Bool( bool s );
		static BatObject Function( BatCallable* func );
		static BatObject Array( const BatObject* arr, size_t arr_size );
		static BatObject Tuple( const BatObject* elements, size_t size );
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
		BatObject Call( Interpreter& interpreter, const std::vector<BatObject>& args ) const;
		BatObject Index( const BatObject& index );
		BatObject Slice( const BatObject& begin, const BatObject& end );
		size_t Size() const { return impl ? impl->arr_size : 0; }

		bool IsTruthy() const;

		std::string ToString() const;

		ObjectType Type() const { return impl ? impl->type : TYPE_NULL; }

		bool GetBool() const { assert( Type() == TYPE_BOOL ); return Value().i64; }
		int64_t GetInt() const { assert( Type() == TYPE_INT ); return Value().i64; }
		double GetFloat() const { assert( Type() == TYPE_FLOAT ); return Value().f64; }
		const char* GetString() const { assert( Type() == TYPE_STR ); return Value().str; }
		BatCallable* GetFunction() const { assert( Type() == TYPE_CALLABLE ); return Value().func; }
		BatObject* GetElements() const { assert( Type() == TYPE_ARRAY || Type() == TYPE_TUPLE ); return Value().arr; }
		Environment* GetModule() const { assert( Type() == TYPE_MODULE ); return Value().env; }
		Environment* GetClass() const { assert( Type() == TYPE_CLASS ); return Value().env; }

		void* GetOpaque() const { assert( Type() == TYPE_OBJECT ); return Value().opaque; }
		void SetOpaque( void* opaque ) { assert( Type() == TYPE_OBJECT ); Value().opaque = opaque; }
		const char* Classname() const { assert( Type() == TYPE_OBJECT ); return impl->classname; }
		const BatObject& GetField( const std::string& name ) const;
		void SetField( const std::string& name, BatObject value );
	private:
		static BatObject New( ObjectType type );
	public:
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
		};

		const ObjectValues& Value() const { return impl->value; }
		ObjectValues& Value() { return impl->value; }
	private:
		struct BatObjectImpl
		{
			BatObjectImpl();
			~BatObjectImpl();

			ObjectType type = TYPE_UNDEFINED;
			int refcnt = 1;
			ObjectValues value;
			size_t arr_size = 0;
			const char* classname = nullptr;
			std::unordered_map<std::string, BatObject> fields;
		};

		BatObjectImpl* impl = nullptr;
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