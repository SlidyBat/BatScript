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
		Environment() = default;
		Environment( Environment* enclosing );

		bool Exists( const std::string& name ) const;
		bool ExistsLocally( const std::string& name ) const;
		bool AddVar( const std::string& name, const BatObject& value );
		const BatObject* GetVar( const std::string& name ) const;
		bool SetVar( const std::string& name, const BatObject& value );
		Environment* Enclosing() { return m_pEnclosing; }
	private:
		Environment* m_pEnclosing = nullptr;
		std::unordered_map<std::string, BatObject> m_mapVariables;
	};
}