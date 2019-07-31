#pragma once

#include <functional>
#include "bat_object.h"
#include "ast.h"

namespace Bat
{
	class Interpreter;

	using BatNativeCallback = std::function<BatObject( const std::vector<BatObject>& )>;

	struct ReturnValue
	{
		BatObject value;
	};

	class BatCallable
	{
	public:
		virtual size_t NumDefaults() const = 0;
		virtual BatObject Call( Interpreter& interpreter, const std::vector<BatObject>& args ) = 0;
	};

	class BatFunction : public BatCallable
	{
	public:
		BatFunction( FuncDecl* declaration );

		virtual size_t NumDefaults() const override;
		virtual BatObject Call( Interpreter& interpreter, const std::vector<BatObject>& args ) override;
	private:
		FuncDecl* m_pDeclaration;
		size_t m_nDefaults;
	};

	class BatNative : public BatCallable
	{
	public:
		BatNative( BatNativeCallback callback );

		virtual size_t NumDefaults() const override { return 0; } // Default values for params not possible for natives yet
		virtual BatObject Call( Interpreter& interpreter, const std::vector<BatObject>& args ) override;
	private:
		BatNativeCallback m_Callback;
	};
}