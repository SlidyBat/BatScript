#pragma once

#include <string>
#include <unordered_map>
#include "bat_object.h"
#include "type.h"

namespace Bat
{
	class Environment
	{
	public:
		Environment( const std::string& module );
		Environment( const std::string& classname, Environment* enclosing );
		Environment( Environment* enclosing );

		const std::string& Module() const;
		const std::string& Class() const;

		bool Exists( const std::string& name ) const;
		bool ExistsLocally( const std::string& name ) const;
		bool AddVar( const std::string& name, BatObject value );
		const BatObject* GetVar( const std::string& name ) const;
		bool SetVar( const std::string& name, const BatObject& value );
		bool StealVarsFrom( Environment& from );
		Environment* Enclosing() { return m_pEnclosing; }
	private:
		std::string m_Module;
		std::string m_Class;
		Environment* m_pEnclosing = nullptr;
		std::unordered_map<std::string, BatObject> m_mapVariables;
	};
}