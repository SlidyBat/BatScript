#include "memory_stream.h"

#include <cassert>
#include <fstream>

MemoryStream::MemoryStream( const char* data, size_t size )
{
	m_Bytes.resize( size );
	memcpy( &m_Bytes[0], data, size );
}

MemoryStream::MemoryStream( std::vector<char> data )
	:
	m_Bytes( std::move( data ) )
{
}

void MemoryStream::Seek( SeekPosition where )
{
	if( where == SeekPosition::START )
	{
		m_iCurrentByte = 0;
	}
	else if( where == SeekPosition::END )
	{
		m_iCurrentByte = m_Bytes.size();
	}
}

void MemoryStream::Seek( size_t pos, SeekPosition dir )
{
	switch( dir )
	{
	case SeekPosition::START:
		m_iCurrentByte = pos;
		break;
	case SeekPosition::END:
		assert( pos < m_Bytes.size() );
		m_iCurrentByte = m_Bytes.size() - pos;
		break;
	case SeekPosition::CURRENT:
		m_iCurrentByte += pos;
		break;
	default:
		assert( false );
	}
}

size_t MemoryStream::Tell() const
{
	return m_iCurrentByte;
}

char MemoryStream::ReadByte()
{
	if( EndOfStream() )
	{
		return 0;
	}

	return m_Bytes[m_iCurrentByte++];
}

void MemoryStream::ReadBytes( char* pBytes, const size_t size )
{
	if( EndOfStream() )
	{
		return;
	}

	size_t newsize = size;
	if( m_iCurrentByte + size >= m_Bytes.size() )
	{
		newsize = m_Bytes.size() - m_iCurrentByte;
	}

	for( size_t i = 0; i < newsize; i++ )
	{
		pBytes[i] = m_Bytes[m_iCurrentByte++];
	}
}

void MemoryStream::WriteByte( const char byte )
{
	if( EndOfStream() )
	{
		m_Bytes.emplace_back( byte );
		m_iCurrentByte++;
	}

	m_Bytes[m_iCurrentByte++] = byte;
}

void MemoryStream::WriteBytes( const char* pBytes, const size_t size )
{
	if( EndOfStream() )
	{
		m_Bytes.reserve( m_Bytes.size() + size );
		for( size_t i = 0; i < size; i++ )
		{
			m_Bytes.emplace_back( pBytes[i] );
		}

		m_iCurrentByte += size;
	}
	else if( m_iCurrentByte + size > m_Bytes.size() )
	{
		const size_t total_size = m_Bytes.size();
		const size_t needed = (m_iCurrentByte + size) - total_size;
		m_Bytes.reserve( total_size + needed );

		for( size_t i = m_iCurrentByte; i < total_size; i++ )
		{
			m_Bytes[m_iCurrentByte + i] = pBytes[i];
		}

		const size_t off = total_size - m_iCurrentByte;
		for( size_t i = 0; i < needed; i++ )
		{
			m_Bytes.emplace_back( pBytes[off + i] );
		}
		
		m_iCurrentByte += size;
	}
	else
	{
		for( size_t i = 0; i < size; i++ )
		{
			m_Bytes[m_iCurrentByte++] = pBytes[i];
		}
	}
}

bool MemoryStream::EndOfStream() const
{
	return m_iCurrentByte >= m_Bytes.size();
}

size_t MemoryStream::Size() const
{
	return m_Bytes.size();
}

const char* MemoryStream::Base() const
{
	return m_Bytes.data();
}

char* MemoryStream::Base()
{
	return m_Bytes.data();
}

void MemoryStream::Clear()
{
	m_Bytes.clear();
	m_iCurrentByte = 0;
}

MemoryStream MemoryStream::FromStream( std::istream& stream )
{
	assert( stream );
	stream.seekg( 0, std::ios::end );
	size_t size = (size_t)stream.tellg();
	stream.seekg( 0, std::ios::beg );

	if( size == 0 )
	{
		return {};
	}

	MemoryStream ret;
	ret.m_Bytes.resize( size + 1 );
	stream.read( &ret.m_Bytes[0], size );
	ret.m_Bytes[size] = '\0';

	return ret;
}

MemoryStream MemoryStream::FromFile( const std::string& filename, FileMode mode )
{
	std::ifstream file( filename, (mode == FileMode::BINARY) ? std::ios::binary : 0 );
	assert( file && "Could not open file" );
	return FromStream( file );
}

MemoryStream MemoryStream::FromFile( const std::wstring& filename, FileMode mode )
{
	std::ifstream file( filename, (mode == FileMode::BINARY) ? std::ios::binary : 0 );
	assert( file && "Could not open file" );
	return FromStream( file );
}

void MemoryStream::ToStream( const MemoryStream& ms, std::ostream& stream )
{
	stream.write( ms.m_Bytes.data(), ms.m_Bytes.size() );
}

void MemoryStream::ToFile( const MemoryStream& ms, const std::string& filename, FileMode mode )
{
	std::ofstream file( filename, (mode == FileMode::BINARY) ? std::ios::binary : 0 );
	assert( file && "Could not open file" );
	ToStream( ms, file );
}

void MemoryStream::ToFile( const MemoryStream& ms, const std::wstring& filename, FileMode mode )
{
	std::ofstream file( filename, (mode == FileMode::BINARY) ? std::ios::binary : 0 );
	assert( file && "Could not open file" );
	ToStream( ms, file );
}

int64_t MemoryStream::ReadInt64()
{
	return Read<int64_t>();
}

int32_t MemoryStream::ReadInt32()
{
	return Read<int32_t>();
}

int16_t MemoryStream::ReadInt16()
{
	return Read<int16_t>();
}

uint64_t MemoryStream::ReadUInt64()
{
	return Read<uint64_t>();
}

uint32_t MemoryStream::ReadUInt32()
{
	return Read<uint32_t>();
}

uint16_t MemoryStream::ReadUInt16()
{
	return Read<uint16_t>();
}

char MemoryStream::ReadChar()
{
	return ReadByte();
}

short MemoryStream::ReadShort()
{
	return Read<short>();
}

int MemoryStream::ReadInt()
{
	return Read<int>();
}

float MemoryStream::ReadFloat()
{
	return Read<float>();
}

double MemoryStream::ReadDouble()
{
	return Read<double>();
}

bool MemoryStream::ReadBool()
{
	return Read<bool>();
}

std::string MemoryStream::ReadString( char delimiter/* = '\0'*/ )
{
	std::string str;
	while( char c = ReadChar() )
	{
		if( c == delimiter )
		{
			break;
		}
		str += c;
	}

	return str;
}

void MemoryStream::WriteInt64( int64_t val )
{
	Write( val );
}

void MemoryStream::WriteInt32( int32_t val )
{
	Write( val );
}

void MemoryStream::WriteInt16( int16_t val )
{
	Write( val );
}

void MemoryStream::WriteUInt64( uint64_t val )
{
	Write( val );
}

void MemoryStream::WriteUInt32( uint32_t val )
{
	Write( val );
}

void MemoryStream::WriteUInt16( uint16_t val )
{
	Write( val );
}

void MemoryStream::WriteChar( char val )
{
	Write( val );
}

void MemoryStream::WriteShort( short val )
{
	Write( val );
}

void MemoryStream::WriteInt( int val )
{
	Write( val );
}

void MemoryStream::WriteFloat( float val )
{
	Write( val );
}

void MemoryStream::WriteDouble( double val )
{
	Write( val );
}

void MemoryStream::WriteBool( bool val )
{
	Write( val );
}

void MemoryStream::WriteString( std::string_view str, bool terminate/* = true */ )
{
	WriteBytes( str.data(), str.size() );
	if( terminate )
	{
		WriteChar( '\0' );
	}
}
