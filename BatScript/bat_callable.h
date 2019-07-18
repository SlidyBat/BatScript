#pragma once

#include <functional>
#include "bat_object.h"
#include "interpreter.h"

namespace Bat
{
	using BatNativeCallback = std::function<BatObject( const std::vector<BatObject>& )>;

	struct ReturnValue
	{
		BatObject value;
	};

	class BatCallable
	{
	public:
		virtual size_t Arity() const = 0;
		virtual BatObject Call( Interpreter& interpreter, const std::vector<BatObject>& args ) = 0;
	};

	class BatFunction : public BatCallable
	{
	public:
		BatFunction( FuncDecl* declaration );

		virtual size_t Arity() const override;
		virtual BatObject Call( Interpreter& interpreter, const std::vector<BatObject>& args ) override;
	private:
		FuncDecl* m_pDeclaration;
	};

	class BatNative : public BatCallable
	{
	public:
		BatNative( size_t arity, BatNativeCallback callback );

		virtual size_t Arity() const override;
		virtual BatObject Call( Interpreter& interpreter, const std::vector<BatObject>& args ) override;
	private:
		BatNativeCallback m_Callback;
		size_t m_iArity;
	};
}