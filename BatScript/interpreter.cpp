#include "interpreter.h"

namespace Bat
{
	BatObject Interpreter::Evaluate( Expression* e )
	{
		return Traverse( e );
	}

	BatObject Interpreter::VisitNULL_LITERAL()
	{
		return {};
	}
	BatObject Interpreter::VisitBOOL_LITERAL( bool value )
	{
		return value;
	}
	BatObject Interpreter::VisitFLOAT_LITERAL( double value )
	{
		return value;
	}
	BatObject Interpreter::VisitINT_LITERAL( int64_t value )
	{
		return value;
	}
	BatObject Interpreter::VisitSTR_LITERAL( const char* str )
	{
		return str;
	}
	Bat::BatObject Interpreter::VisitADD( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l + r;
	}
	Bat::BatObject Interpreter::VisitSUB( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l - r;
	}
	Bat::BatObject Interpreter::VisitDIV( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l / r;
	}
	Bat::BatObject Interpreter::VisitMUL( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l * r;
	}
	Bat::BatObject Interpreter::VisitMOD( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l % r;
	}
	Bat::BatObject Interpreter::VisitBITOR( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l | r;
	}
	Bat::BatObject Interpreter::VisitBITXOR( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l ^ r;
	}
	Bat::BatObject Interpreter::VisitBITAND( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l & r;
	}
	Bat::BatObject Interpreter::VisitLBITSHIFT( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l << r;
	}
	Bat::BatObject Interpreter::VisitRBITSHIFT( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l >> r;
	}
	Bat::BatObject Interpreter::VisitASSIGN( Bat::Expression* left, Bat::Expression* right )
	{
		throw BatObjectError();
	}
	Bat::BatObject Interpreter::VisitADD_ASSIGN( Bat::Expression* left, Bat::Expression* right )
	{
		throw BatObjectError();
	}
	Bat::BatObject Interpreter::VisitSUB_ASSIGN( Bat::Expression* left, Bat::Expression* right )
	{
		throw BatObjectError();
	}
	Bat::BatObject Interpreter::VisitDIV_ASSIGN( Bat::Expression* left, Bat::Expression* right )
	{
		throw BatObjectError();
	}
	Bat::BatObject Interpreter::VisitMUL_ASSIGN( Bat::Expression* left, Bat::Expression* right )
	{
		throw BatObjectError();
	}
	Bat::BatObject Interpreter::VisitMOD_ASSIGN( Bat::Expression* left, Bat::Expression* right )
	{
		throw BatObjectError();
	}
	Bat::BatObject Interpreter::VisitBITOR_ASSIGN( Bat::Expression* left, Bat::Expression* right )
	{
		throw BatObjectError();
	}
	Bat::BatObject Interpreter::VisitBITXOR_ASSIGN( Bat::Expression* left, Bat::Expression* right )
	{
		throw BatObjectError();
	}
	Bat::BatObject Interpreter::VisitBITAND_ASSIGN( Bat::Expression* left, Bat::Expression* right )
	{
		throw BatObjectError();
	}
	Bat::BatObject Interpreter::VisitAND( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		if( !l.IsTruthy() )
		{
			return false;
		}
		BatObject r = Evaluate( right );
		if( !r.IsTruthy() )
		{
			return false;
		}
		return true;
	}
	Bat::BatObject Interpreter::VisitOR( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		if( l.IsTruthy() )
		{
			return true;
		}
		BatObject r = Evaluate( right );
		if( r.IsTruthy() )
		{
			return true;
		}
		return false;
	}
	Bat::BatObject Interpreter::VisitCMPEQ( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l == r;
	}
	Bat::BatObject Interpreter::VisitCMPNEQ( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l != r;
	}
	Bat::BatObject Interpreter::VisitCMPL( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l < r;
	}
	Bat::BatObject Interpreter::VisitCMPLE( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l <= r;
	}
	Bat::BatObject Interpreter::VisitCMPG( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l > r;
	}
	Bat::BatObject Interpreter::VisitCMPGE( Bat::Expression* left, Bat::Expression* right )
	{
		BatObject l = Evaluate( left );
		BatObject r = Evaluate( right );
		return l >= r;
	}
	Bat::BatObject Interpreter::VisitNOT( Bat::Expression* expr )
	{
		BatObject e = Evaluate( expr );
		return !e;
	}
	Bat::BatObject Interpreter::VisitBITNEG( Bat::Expression* expr )
	{
		BatObject e = Evaluate( expr );
		return ~e;
	}
	Bat::BatObject Interpreter::VisitNEG( Bat::Expression* expr )
	{
		BatObject e = Evaluate( expr );
		return -e;
	}
	Bat::BatObject Interpreter::VisitGROUP( Bat::Expression* expr )
	{
		return Evaluate( expr );
	}
	Bat::BatObject Interpreter::VisitADDROF( Bat::Expression* expr )
	{
		throw BatObjectError();
	}
	Bat::BatObject Interpreter::VisitMOVE( Bat::Expression* expr )
	{
		throw BatObjectError();
	}
}
