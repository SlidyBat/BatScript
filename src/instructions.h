#pragma once

#include <cstdint>
#include "util.h"

// Opcode name, no. of operands, no. of pushes, no. of pops, mnemonic
// For total stack effect of an instruction do: (# of pushes - # of pops)
#define OPCODES(_) \
	_(NOP,         0, 0, 0, nop)                \
	/* Stack manipulation */                    \
	_(PUSH,         1, 1, 0, push)              \
	_(POP,          0, 0, 1, pop)               \
	_(DUP,          0, 1, 0, dup)               \
	_(DUPX1,        0, 1, 0, dupx1)             \
	_(PROC,         0, 0, 0, proc)              \
	_(STACK,        1, 0, 0, stack)             \
	/* Memory operations */                     \
	_(LOAD_LOCAL,   0, 1, 1, local.load)        \
	_(LOAD_GLOBAL,  0, 1, 1, global.load)       \
	_(STORE_LOCAL,  0, 0, 2, local.store)       \
	_(STORE_GLOBAL, 0, 0, 2, global.store)      \
	/* Bitwise operations */                    \
	_(BITOR,        0, 1, 2, bitor)             \
	_(BITAND,       0, 1, 2, bitand)            \
	_(BITXOR,       0, 1, 2, bitxor)            \
	_(BITNOT,       0, 1, 1, bitnot)            \
	_(SHL,          0, 1, 1, shl)               \
	_(SHR,          0, 1, 1, shr)               \
	/* Logical operations */                    \
	_(EQ,           0, 1, 1, eq)                \
	_(NEQ,          0, 1, 1, neq)               \
	_(LESS,         0, 1, 1, less)              \
	_(LESSE,        0, 1, 1, lesse)             \
	_(GRT,          0, 1, 1, grt)               \
	_(GRTE,         0, 1, 1, grte)              \
	_(NOT,          0, 1, 1, not)               \
	/* Jump instructions */                     \
	_(JMP,          1, 0, 1, jmp)               \
	_(JZ,           1, 0, 2, jz)                \
	_(JNZ,          1, 0, 2, jnz)               \
	_(CALL,         0, 1, 1, call)              \
	_(RET,          1, 1, 1, ret)               \
	/* Integer arithmetic */                    \
	_(ADD,          0, 1, 2, add)               \
	_(SUB,          0, 1, 2, sub)               \
	_(DIV,          0, 1, 2, div)               \
	_(MUL,          0, 1, 2, mul)               \
	_(MOD,          0, 1, 2, mod)               \
	_(NEG,          0, 1, 1, neg)               \
	/* Floating point arithmetic */             \
	_(ITOF,         0, 1, 1, itof)              \
	_(FTOI,         0, 1, 1, ftoi)              \
	_(ADDF,         0, 1, 2, addf)              \
	_(SUBF,         0, 1, 2, subf)              \
	_(DIVF,         0, 1, 2, divf)              \
	_(MULF,         0, 1, 2, mulf)              \
	_(NEGF,         0, 1, 1, negf)              \
	/* Floating point logical operations */     \
	_(EQF,          0, 1, 2, eqf)               \
	_(NEQF,         0, 1, 2, neqf)              \
	_(LESSF,        0, 1, 2, lessf)             \
	_(LESSEF,       0, 1, 2, lessef)            \
	_(GRTF,         0, 1, 2, grtf)              \
	_(GRTEF,        0, 1, 2, grtef)             \
	/* Special */                               \
	_(HALT,         0, 0, 0, halt)              \
	_(PRINTI,       0, 0, 1, int.print)         \
	_(PRINTF,       0, 0, 1, float.print)       \
	_(PRINTS,       0, 0, 1, str.print)         \
	_(PRINTB,       0, 0, 1, bool.print)        \
	_(NATIVE,       0, 1, 1, native)

namespace Bat
{
	enum class OpCode : char
	{
#define _(name, operands, pushes, pops, mnemonic) name,
		OPCODES( _ )
#undef _
	};
}