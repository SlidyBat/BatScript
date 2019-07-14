#pragma once

#include <string>
#include <unordered_map>

namespace Bat
{
	// Implemented as a linked list to ensure no reallocations so pointer references are always valid
	class StringPool
	{
	public:
		~StringPool();

		const char* AddString( const std::string& str );
	private:
		class StringEntry
		{
		public:
			StringEntry( const std::string& str )
				:
				str( str )
			{}
			~StringEntry()
			{
				delete next;
			}

			std::string str;
			StringEntry* next = nullptr;
		};
		StringEntry* m_pPool = nullptr;
		StringEntry* m_pTail = nullptr;
		std::unordered_map<std::string, const char*> m_StringsSet;
	};

	extern StringPool stringpool;
}