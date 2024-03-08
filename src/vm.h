#pragma once

#include "memory_stream.h"
#include "bat_callable.h"
#include "compiler.h"

namespace Bat
{
	class VirtualMachine
	{
	public:
		void AddNative( const std::string& name, BatNativeCallback callback );

		void Run( BatCode& bc );
	private:
		template <typename T>
		void PushAny( T val )
		{
			assert( m_iStackPointer + sizeof( T ) <= sizeof( m_Stack ) );
			*reinterpret_cast<T*>(&m_Stack[m_iStackPointer]) = val;
			m_iStackPointer += sizeof( T );
		}
		template <typename T>
		T PopAny()
		{
			assert( m_iStackPointer - sizeof( T ) >= 0 );
			m_iStackPointer -= sizeof( T );
			return *reinterpret_cast<T*>(&m_Stack[m_iStackPointer]);
		}
		template <typename T>
		void PushAnyCallStack( T val )
		{
			assert( m_iCallStackPointer + sizeof( T ) <= sizeof( m_CallStack ) );
			*reinterpret_cast<T*>(&m_CallStack[m_iCallStackPointer]) = val;
			m_iCallStackPointer += sizeof( T );
		}
		template <typename T>
		T PopAnyCallStack()
		{
			assert( m_iCallStackPointer - sizeof( T ) >= 0 );
			m_iCallStackPointer -= sizeof( T );
			return *reinterpret_cast<T*>(&m_CallStack[m_iCallStackPointer]);
		}
		template <typename T>
		T ReadCode()
		{
			T val = *reinterpret_cast<T*>(&m_pCode[m_iIP]);
			m_iIP += sizeof( T );
			return val;
		}
		OpCode ReadOp() { return ReadCode<OpCode>(); }
		int64_t ReadI64() { return ReadCode<int64_t>(); }

		void Push( int64_t val ) { PushAny( val ); }
		void PushCallStack( int64_t val ) { PushAnyCallStack( val ); }
		void PushF( double val ) { PushAny( val ); }
		int64_t Pop() { return PopAny<int64_t>(); }
		int64_t PopCallStack() { return PopAnyCallStack<int64_t>(); }
		double PopF() { return PopAny<double>(); }

		void GoTo( int64_t addr );

		void HandleNative( const BatCode& bc, const BatNativeInfo& native );
	private:
		char m_Stack[4096];
		char m_CallStack[4096];
		char* m_pCode = nullptr;
		int m_iIP = 0;
		int64_t m_iStackPointer = 0;
		int64_t m_iCallStackPointer = 0;
		int64_t m_iBasePointer = 0;
		std::unordered_map<std::string, BatNativeCallback> m_Natives;
	};
}