#pragma once

#include <iostream>
#include "memory_stream.h"
#include "compiler.h"

namespace Bat
{
	class Disassembler
	{
	public:
		Disassembler( BatCode code );
		Disassembler( BatCode code, const std::string& source_code );

		void Disassemble();
		void Disassemble( std::ostream& out );
	private:
		void Operand( std::ostream& out );
	private:
		BatCode m_Code;
		std::vector<std::string> m_SourceLines;
	};
}