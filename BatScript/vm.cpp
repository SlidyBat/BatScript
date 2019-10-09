#include "vm.h"

#include <cassert>
#include <iostream>
#include "errorsys.h"
#include "instructions.h"

#define BINARY_OP(op) \
	do \
	{ \
		auto a = Pop(); \
		auto b = Pop(); \
		Push( a op b ); \
	} while( false )

#define UNARY_OP(op) \
	do \
	{ \
		auto a = Pop(); \
		Push( op a ); \
	} while( false )

#define BINARY_OP_F(op) \
	do \
	{ \
		auto a = PopF(); \
		auto b = PopF(); \
		PushF( a op b ); \
	} while( false )

#define UNARY_OP_F(op) \
	do \
	{ \
		auto a = PopF(); \
		PushF( op a ); \
	} while( false )

#define TARGET(op) OpCode::op: \
	TARGET_##op

#define DISPATCH_CASE(name, operands, pushes, pops, mnemonic) case OpCode::name: goto TARGET_##name;
#define DISPATCH() \
	switch( ReadOp() ) \
	{ \
		OPCODES(DISPATCH_CASE) \
	}

namespace Bat
{
	void VirtualMachine::AddNative( const std::string& name, BatNativeCallback callback )
	{
		assert( m_Natives.count( name ) == 0 );
		m_Natives[name] = std::move( callback );
	}
	void VirtualMachine::Run( BatCode& bc )
	{
		bc.code.Seek( SeekPosition::START );

		m_pCode = bc.code.Base();
		m_iIP = (int)bc.entry_point;

		while( true )
		{
			auto op = ReadOp();
			switch( op )
			{
			case TARGET(NOP): break;

			case TARGET(PUSH):
			{
				auto val = ReadI64();
				Push( val );

				DISPATCH();
			}
			case TARGET(POP):
			{
				Pop();

				DISPATCH();
			}
			case TARGET(DUP):
			{
				auto val = Pop();
				Push( val );
				Push( val );

				DISPATCH();
			}
			case TARGET(DUPX1):
			{
				auto val1 = Pop();
				auto val2 = Pop();
				Push( val1 );
				Push( val2 );
				Push( val1 );

				DISPATCH();
			}
			case TARGET(PROC):
			{
				PushCallStack( m_iBasePointer );
				m_iBasePointer = m_iStackPointer;

				DISPATCH();
			}
			case TARGET(ENDPROC):
			{
				m_iStackPointer = m_iBasePointer;
				m_iBasePointer = PopCallStack();

				DISPATCH();
			}
			case TARGET(STACK):
			{
				m_iStackPointer += ReadI64();

				DISPATCH();
			}
			case TARGET(SSTACK):
			{
				m_iStackPointer -= ReadI64();

				DISPATCH();
			}

			case TARGET(LOAD_LOCAL):
			{
				auto addr = Pop();
				auto value = *reinterpret_cast<int64_t*>(&m_Stack[m_iBasePointer + addr]);
				Push( value );

				DISPATCH();
			}
			case TARGET(LOAD_GLOBAL):
			{
				auto addr = Pop();
				auto value = *reinterpret_cast<int64_t*>(&m_Stack[addr]);
				Push( value );

				DISPATCH();
			}
			case TARGET(STORE_LOCAL):
			{
				auto value = Pop();
				auto addr = Pop();
				*reinterpret_cast<int64_t*>(&m_Stack[m_iBasePointer + addr]) = value;

				DISPATCH();
			}
			case TARGET(STORE_GLOBAL):
			{
				auto value = Pop();
				auto addr = Pop();
				*reinterpret_cast<int64_t*>(&m_Stack[addr]) = value;

				DISPATCH();
			}
			case TARGET(SETRET):
			{
				m_RetValue = Pop();

				DISPATCH();
			}
			case TARGET(GETRET):
			{
				Push( m_RetValue );

				DISPATCH();
			}

			case TARGET(BITAND): BINARY_OP( & ); DISPATCH();
			case TARGET(BITOR):  BINARY_OP( | ); DISPATCH();
			case TARGET(BITXOR): BINARY_OP( ^ ); DISPATCH();
			case TARGET(BITNOT): UNARY_OP( ~ ); DISPATCH();
			case TARGET(SHL):    BINARY_OP( << ); DISPATCH();
			case TARGET(SHR):    BINARY_OP( >> ); DISPATCH();

			case TARGET(EQ):     BINARY_OP( == ); DISPATCH();
			case TARGET(NEQ):    BINARY_OP( != ); DISPATCH();
			case TARGET(LESS):   BINARY_OP( < ); DISPATCH();
			case TARGET(LESSE):  BINARY_OP( <= ); DISPATCH();
			case TARGET(GRT):    BINARY_OP( > ); DISPATCH();
			case TARGET(GRTE):   BINARY_OP( >= ); DISPATCH();
			case TARGET(NOT):    UNARY_OP( ! ); DISPATCH();

			case TARGET(ADD):    BINARY_OP( + ); DISPATCH();
			case TARGET(SUB):    BINARY_OP( - ); DISPATCH();
			case TARGET(DIV):    BINARY_OP( / ); DISPATCH();
			case TARGET(MUL):    BINARY_OP( * ); DISPATCH();
			case TARGET(MOD):    BINARY_OP( % ); DISPATCH();
			case TARGET(NEG):    UNARY_OP( - ); DISPATCH();

			case TARGET(ITOF):
			{
				auto a = Pop();
				PushF( (double)a );

				DISPATCH();
			}
			case TARGET(FTOI):
			{
				auto a = PopF();
				Push( (int64_t)a );

				DISPATCH();
			}
			case TARGET(ADDF):   BINARY_OP_F( + ); DISPATCH();
			case TARGET(SUBF):   BINARY_OP_F( - ); DISPATCH();
			case TARGET(DIVF):   BINARY_OP_F( / ); DISPATCH();
			case TARGET(MULF):   BINARY_OP_F( * ); DISPATCH();
			case TARGET(NEGF):   UNARY_OP_F( - ); DISPATCH();

			case TARGET(EQF):    BINARY_OP_F( == ); DISPATCH();
			case TARGET(NEQF):   BINARY_OP_F( != ); DISPATCH();
			case TARGET(LESSF):  BINARY_OP_F( < ); DISPATCH();
			case TARGET(LESSEF): BINARY_OP_F( <= ); DISPATCH();
			case TARGET(GRTF):   BINARY_OP_F( > ); DISPATCH();
			case TARGET(GRTEF):  BINARY_OP_F( >= ); DISPATCH();

			case TARGET(JMP):
			{
				GoTo( ReadI64() );

				DISPATCH();
			}
			case TARGET(JZ):
			{
				auto target = ReadI64();
				if( Pop() == 0 )
				{
					GoTo( target );
				}

				DISPATCH();
			}
			case TARGET(JNZ):
			{
				auto target = ReadI64();
				if( Pop() != 0 )
				{
					GoTo( target );
				}

				DISPATCH();
			}
			case TARGET(CALL):
			{
				auto func = Pop();
				PushCallStack( m_iIP );
				GoTo( func );

				DISPATCH();
			}
			case TARGET(RET):
			{
				auto ret_addr = PopCallStack();
				GoTo( ret_addr );

				DISPATCH();
			}

			case TARGET(PRINTB):
			{
				auto val = Pop();
				std::cout << (val ? "true" : "false") << std::endl;

				DISPATCH();
			}
			case TARGET(PRINTI):
			{
				auto val = Pop();
				std::cout << val << std::endl;

				DISPATCH();
			}
			case TARGET(PRINTF):
			{
				auto val = PopF();
				std::cout << std::to_string( val ) << std::endl;

				DISPATCH();
			}
			case TARGET(PRINTS):
			{
				auto val = Pop();
				std::cout << bc.string_literals[val] << std::endl;

				DISPATCH();
			}

			case TARGET(NATIVE):
			{
				auto native_idx = Pop();
				const BatNativeInfo& native = bc.natives[native_idx];
				HandleNative( bc, native );

				DISPATCH();
			}

			case TARGET(HALT):
			{
				return;
			}

			default:
			{
				assert( false && "Unhandled opcode" );
				return;
			}
			}
		}
	}

	void VirtualMachine::GoTo( int64_t addr )
	{
		m_iIP = (int)addr;
	}
	void VirtualMachine::HandleNative( const BatCode& bc, const BatNativeInfo& native )
	{
		auto it = m_Natives.find( native.name );
		if( it == m_Natives.end() )
		{
			// TODO: proper line/column report
			ErrorSys::Report( 0, 0, std::string( "Native '" ) + native.name + "' not bound" );
			return;
		}

		std::vector<BatObject> params;
		auto& callback = it->second;

		const auto& param_types = native.desc.param_types;
		for( size_t param_idx = 0; param_idx < param_types.size(); param_idx++ )
		{
			switch( param_types[param_idx] )
			{
			case TYPE_BOOL:
				params.emplace_back( (bool)Pop() );
				break;
			case TYPE_INT:
				params.emplace_back( (int64_t)Pop() );
				break;
			case TYPE_FLOAT:
				params.emplace_back( (double)Pop() );
				break;
			case TYPE_STR:
				params.emplace_back( bc.string_literals[Pop()].c_str() );
				break;
			default:
				assert( false );
			}
		}

		BatObject result = callback( params );

		switch( result.type )
		{
		case TYPE_BOOL:
			Push( result.Bool() );
			break;
		case TYPE_INT:
			Push( result.Int() );
			break;
		case TYPE_FLOAT:
			PushF( result.Float() );
			break;
		default:
			assert( false );
		}
	}
}