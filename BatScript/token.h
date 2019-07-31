#pragma once

#include <string>
#include "sourceloc.h"

#define KEYWORD_TYPES(_) \
	_(TRUE,           "true")                                   \
	_(FALSE,          "false")                                  \
	_(IF,             "if")                                     \
	_(ELSE,           "else")                                   \
	_(WHILE,          "while")                                  \
	_(FOR,            "for")                                    \
	_(DO,             "do")                                     \
	_(RETURN,         "return")                                 \
	_(ENUM,           "enum")                                   \
	_(IMPORT,         "import")                                 \
	_(NATIVE,         "native")                                 \
	_(CONST,          "const")                                  \
	_(VAR,            "var")                                    \
	_(TYPEDEF,        "typedef")                                \
	_(ALIASDEF,       "aliasdef")                               \
	_(ASYNC,          "async")                                  \
	_(AWAIT,          "await")                                  \
	_(PRINT,          "print")                                  \
	_(STRUCT,         "struct")                                 \
	_(CLASS,          "class")                                  \
	_(THIS,           "this")                                   \
	_(NIL,            "null")                                   \
	_(AND,            "and")                                    \
	_(OR,             "or")                                     \
	_(DEF,            "def")                                    \
	_(BOOL,           "bool")                                   \
	_(INT,            "int")                                    \
	_(FLOAT,          "float")                                  \
	_(STRING,         "string")                                 \


#define TOKEN_TYPES(_) \
	_(NONE,           "<none>")                                 \
	/* Single-char tokens */                                    \
	_(LPAREN,         "(")                                      \
	_(RPAREN,         ")")                                      \
	_(LBRACE,         "{")                                      \
	_(RBRACE,         "}")                                      \
	_(LBRACKET,       "[")                                      \
	_(RBRACKET,       "]")                                      \
	_(DOT,            ".")                                      \
	_(COMMA,          ",")                                      \
	_(BACKSLASH,      "\\")                                     \
	_(QUESMARK,       "?")                                      \
	_(SEMICOLON,      ";")                                      \
	_(COLON,          ":")                                      \
	_(AT,             "@")                                      \
	_(TILDE,          "~")                                      \
	/* One/two char */                                          \
	_(PLUS,           "+")                                      \
	_(PLUS_EQUAL,     "+=")                                     \
	_(MINUS,          "-")                                      \
	_(MINUS_EQUAL,    "-=")                                     \
	_(ASTERISK,       "*")                                      \
	_(ASTERISK_EQUAL, "*=")                                     \
	_(SLASH,          "/")                                      \
	_(SLASH_EQUAL,    "/=")                                     \
	_(EQUAL,          "=")                                      \
	_(EQUAL_EQUAL,    "==")                                     \
	_(EXCLMARK,       "!")                                      \
	_(EXCLMARK_EQUAL, "!=")                                     \
	_(GREATER,        ">")                                      \
	_(GREATER_GREATER,">>")                                     \
	_(GREATER_EQUAL,  ">=")                                     \
	_(LESS,           "<")                                      \
	_(LESS_LESS,      "<<")                                     \
	_(LESS_EQUAL,     "<=")                                     \
	_(PERCENT,        "%")                                      \
	_(PERCENT_EQUAL,  "%=")                                     \
	_(BAR,            "|")                                      \
	_(BAR_EQUAL,      "|=")                                     \
	_(AMP,            "&")                                      \
	_(AMP_EQUAL,      "&=")                                     \
	_(HAT,            "^")                                      \
	_(HAT_EQUAL,      "^=")                                     \
	/* Literals */                                              \
	_(IDENT,          "<identifier>")                           \
	_(STRING_LITERAL,         "<string>")                               \
	_(INT_LITERAL,            "<integer>")                              \
	_(FLOAT_LITERAL,          "<float>")                                \
	/* Keywords */                                              \
	KEYWORD_TYPES(_)                                            \
	/* Special */                                               \
	_(INDENT,         "<indent>")                               \
	_(DEDENT,         "<dedent>")                               \
	_(ENDOFFILE,      "<end-of-file>")                          \
	_(ENDOFLINE,      "<end-of-line>")                          \
	_(COMMENT,        "<comment>")                              \
	_(UNKNOWN,        "<unknown>")

namespace Bat
{
	enum TokenType
	{
#define _(token, tokstr) TOKEN_##token,
		TOKEN_TYPES( _ )
#undef _
	};

	// Converts a token type to it's string representation
	const char* TokenTypeToString( TokenType type );
	// Converts a keyword string to it's type (or TOKEN_NONE if it's not a keyword)
	TokenType KeywordStringToType( const std::string& keyword_str );

	struct Token
	{
		Token() = default;
		Token( TokenType type, const std::string& lexeme, int line, int column );
		Token( double f64, const std::string& lexeme, int line, int column );
		Token( int64_t i64, const std::string& lexeme, int line, int column );
		Token( const char* str, const std::string& lexeme, int line, int column );

		TokenType type;
		std::string lexeme;
		SourceLoc loc;
		union
		{
			int64_t i64;
			double f64;
			const char* str;
		} literal;
	};
}