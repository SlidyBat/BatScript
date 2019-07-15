#pragma once

#include "ast.h"
#include "bat_object.h"

namespace Bat
{
	class Interpreter : public ASTVisitor<BatObject>
	{
	public:
		Interpreter() = default;

		BatObject Evaluate( Expression* e );
	private:
		virtual BatObject VisitNULL_LITERAL() override;
		virtual BatObject VisitBOOL_LITERAL( bool value ) override;
		virtual BatObject VisitFLOAT_LITERAL( double value ) override;
		virtual BatObject VisitINT_LITERAL( int64_t value ) override;
		virtual BatObject VisitSTR_LITERAL( const char* str ) override;
		virtual BatObject VisitADD( Expression* left, Expression* right );
		virtual BatObject VisitSUB( Expression* left, Expression* right );
		virtual BatObject VisitDIV( Expression* left, Expression* right );
		virtual BatObject VisitMUL( Expression* left, Expression* right );
		virtual BatObject VisitMOD( Expression* left, Expression* right );
		virtual BatObject VisitBITOR( Expression* left, Expression* right );
		virtual BatObject VisitBITXOR( Expression* left, Expression* right );
		virtual BatObject VisitBITAND( Expression* left, Expression* right );
		virtual BatObject VisitLBITSHIFT( Expression* left, Expression* right );
		virtual BatObject VisitRBITSHIFT( Expression* left, Expression* right );
		virtual BatObject VisitASSIGN( Expression* left, Expression* right );
		virtual BatObject VisitADD_ASSIGN( Expression* left, Expression* right );
		virtual BatObject VisitSUB_ASSIGN( Expression* left, Expression* right );
		virtual BatObject VisitDIV_ASSIGN( Expression* left, Expression* right );
		virtual BatObject VisitMUL_ASSIGN( Expression* left, Expression* right );
		virtual BatObject VisitMOD_ASSIGN( Expression* left, Expression* right );
		virtual BatObject VisitBITOR_ASSIGN( Expression* left, Expression* right );
		virtual BatObject VisitBITXOR_ASSIGN( Expression* left, Expression* right );
		virtual BatObject VisitBITAND_ASSIGN( Expression* left, Expression* right );
		virtual BatObject VisitAND( Expression* left, Expression* right );
		virtual BatObject VisitOR( Expression* left, Expression* right );
		virtual BatObject VisitCMPEQ( Expression* left, Expression* right );
		virtual BatObject VisitCMPNEQ( Expression* left, Expression* right );
		virtual BatObject VisitCMPL( Expression* left, Expression* right );
		virtual BatObject VisitCMPLE( Expression* left, Expression* right );
		virtual BatObject VisitCMPG( Expression* left, Expression* right );
		virtual BatObject VisitCMPGE( Expression* left, Expression* right );
		virtual BatObject VisitNOT( Expression* expr );
		virtual BatObject VisitBITNEG( Expression* expr );
		virtual BatObject VisitNEG( Expression* expr );
		virtual BatObject VisitGROUP( Expression* expr );
		virtual BatObject VisitADDROF( Expression* expr );
		virtual BatObject VisitMOVE( Expression* expr );
	};
}