#pragma once

namespace Bat
{
	class SourceLoc
	{
	public:
		SourceLoc( int line, int column )
			:
			m_iLine( line ),
			m_iColumn( column )
		{}

		int Line() const { return m_iLine; }
		int Column() const { return m_iColumn; }
	private:
		int m_iLine;
		int m_iColumn;
	};
}