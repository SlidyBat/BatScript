#include "disassembler.h"

#include <cassert>
#include <iomanip>
#include <sstream>
#include "instructions.h"
#include "stringlib.h"

namespace Bat
{
	Disassembler::Disassembler( BatCode code )
		:
		m_Code( std::move( code ) )
	{
		m_Code.code.Seek( SeekPosition::START );
	}
	Disassembler::Disassembler( BatCode code, const std::string& source_code )
		:
		m_Code( std::move( code ) ),
		m_SourceLines( SplitString( source_code, '\n' ) )
	{
		m_Code.code.Seek( SeekPosition::START );
	}
	void Disassembler::Disassemble()
	{
		Disassemble( std::cout );
	}
	void Disassembler::Disassemble( std::ostream& out )
	{
		int current_op = 0;
		int current_line = -1;
		while( !m_Code.code.EndOfStream() )
		{
			if( !m_SourceLines.empty() )
			{
				int line = m_Code.debug_info.line_mapping[current_op];
				if( line != current_line )
				{
					// TODO: handle statements/expressions that span multiple lines, lines like "else:" are currently skipped in disassembler
					std::cout << "; " << m_SourceLines[line - 1] << std::endl;
					current_line = line;
				}
			}

			size_t address = m_Code.code.Tell();
			OpCode op = m_Code.code.Read<OpCode>();

			out << std::setfill( '0' ) << std::setw( 4 ) << std::hex << address << std::dec;
			out << '\t';

			switch( op )
			{
#define _(name, operands, pushes, pops, mnemonic) \
			case OpCode::name: \
			{ \
				std::stringstream ss; \
				ss << std::hex << std::setfill( '0' ) << std::setw( 2 ) << (int)op << std::dec; \
				auto old = m_Code.code.Tell(); \
				for( int i = 0; i < operands; i++ ) ss << ' ' << std::hex << std::setfill( '0' ) << std::setw( 2 ) << m_Code.code.ReadInt64() << std::dec; \
				out << std::setfill( ' ' ) << std::setw( 20 ) << ss.str() << '\t'; \
				m_Code.code.Seek( old, SeekPosition::START ); \
				out << #mnemonic; \
				for( int i = 0; i < operands; i++ ) Operand( out ); \
				break; \
			}
OPCODES( _ )
#undef _
			default:
				assert( false && "Unhandled opcode" );
			}

			out << '\n';
			
			current_op++;
		}
	}
	void Disassembler::Operand( std::ostream& out )
	{
		out << ' ';

		out << "0x" << std::hex << m_Code.code.ReadInt64() << std::dec;
	}
}
