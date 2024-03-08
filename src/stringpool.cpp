#include "stringpool.h"

namespace Bat
{
	StringPool stringpool;

	StringPool::~StringPool()
	{
		delete m_pPool;
	}

	const char* Bat::StringPool::AddString( const std::string& str )
	{
		auto it = m_StringsSet.find( str );
		if( it != m_StringsSet.end() )
		{
			return it->second;
		}

		if( m_pPool == nullptr )
		{
			m_pPool = new StringEntry( str );
			m_pTail = m_pPool;
		}
		else
		{
			m_pTail->next = new StringEntry( str );
			m_pTail = m_pTail->next;
		}

		const char* pooled_str = m_pTail->str.c_str();
		m_StringsSet[str] = pooled_str;
		return pooled_str;
	}
}