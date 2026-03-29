#pragma once

#include <vector>
#include "token.h"
#include "ast.h"

namespace Bat
{
	class ParseError : public std::exception
	{};

	class Parser
	{
	public:
		Parser( std::vector<Token> tokens )
			:
			m_Tokens( std::move( tokens ) )
		{}

		// Returns vector of statements
		std::vector<std::unique_ptr<Statement>> Parse();
	private:
		bool AtEnd() const;
		const Token& Peek() const;
		bool Check( TokenType type ) const;
		const Token& Advance();
		const Token& Previous() const;
		bool Match( TokenType type );
		void GoBack();

		const Token& Expect( TokenType type, const std::string& message );
		TypeSpecifier ExpectType( const std::string& message );
		bool CheckTerminator();
		void ExpectTerminator( const std::string& msg = "Expected newline" );
		void Error( const std::string& message );
		// Skips to next statement to avoid cascading errors
		void Synchronize();
	private:
		// Statement parsing
		std::unique_ptr<Statement> ParseStatement();
		std::unique_ptr<Statement> ParseSimpleStatement();
		std::unique_ptr<Statement> ParseCompoundStatement();
		std::unique_ptr<Statement> ParseAssign();
		std::unique_ptr<PrintStmt> ParsePrint();
		std::unique_ptr<BlockStmt> ParseBlock();
		std::unique_ptr<IfStmt> ParseIf();
		std::unique_ptr<WhileStmt> ParseWhile();
		std::unique_ptr<ForStmt> ParseFor();
		std::unique_ptr<ReturnStmt> ParseReturn();
		std::unique_ptr<BreakStmt> ParseBreak();
		std::unique_ptr<ContinueStmt> ParseContinue();
		std::unique_ptr<ImportStmt> ParseImportAs();
		std::unique_ptr<ImportStmt> ParseFromImport();
		std::unique_ptr<NativeStmt> ParseNative();
		std::unique_ptr<VarDecl> ParseVarDeclaration();
		std::unique_ptr<FuncDecl> ParseFuncDeclaration();
		std::unique_ptr<ClassDecl> ParseClassDeclaration();

		FunctionSignature ParseFuncSignature();

		// Expression parsing
		std::unique_ptr<Expression> ParseExpressions();
		std::unique_ptr<Expression> ParseExpression();
		std::unique_ptr<Expression> ParseOr();
		std::unique_ptr<Expression> ParseAnd();
		std::unique_ptr<Expression> ParseBitOr();
		std::unique_ptr<Expression> ParseBitXor();
		std::unique_ptr<Expression> ParseBitAnd();
		std::unique_ptr<Expression> ParseEqualCompare();
		std::unique_ptr<Expression> ParseSizeCompare();
		std::unique_ptr<Expression> ParseBitShift();
		std::unique_ptr<Expression> ParseAddition();
		std::unique_ptr<Expression> ParseMultiplication();
		std::unique_ptr<Expression> ParseUnary();
		std::unique_ptr<Expression> ParseCallOrIndexOrAttr();
		std::unique_ptr<Expression> ParseCall( std::unique_ptr<Expression> left );
		std::unique_ptr<Expression> ParseIndex( std::unique_ptr<Expression> left );
		std::unique_ptr<Expression> ParseAttr( std::unique_ptr<Expression> left );
		std::unique_ptr<Expression> ParsePrimary();
	private:
		int m_iCurrent = 0;
		std::vector<Token> m_Tokens;
	};
}