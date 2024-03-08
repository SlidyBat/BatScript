#pragma once

#include <vector>
#include <string_view>

enum class FileMode
{
	BINARY,
	TEXT
};

enum class SeekPosition
{
	START,
	END,
	CURRENT
};

class MemoryStream
{
public:
public:
	MemoryStream() = default;
	MemoryStream( const char* data, size_t size );
	MemoryStream( std::vector<char> data );

	void Seek( SeekPosition where );
	void Seek( size_t pos, SeekPosition dir );
	size_t Tell() const;

	char ReadByte();
	void ReadBytes( char* pBytes, const size_t size );
	template <typename T>
	T Read()
	{
		T val;
		char* pStart = reinterpret_cast<char*>(&val);
		constexpr size_t size = sizeof( T );
		ReadBytes( pStart, size );

		return val;
	}

	int64_t ReadInt64();
	int32_t ReadInt32();
	int16_t ReadInt16();
	uint64_t ReadUInt64();
	uint32_t ReadUInt32();
	uint16_t ReadUInt16();
	char ReadChar(); // really just an alias for ReadByte
	short ReadShort();
	int ReadInt();
	float ReadFloat();
	double ReadDouble();
	bool ReadBool();
	std::string ReadString( char delimiter = '\0' );

	void WriteByte( const char byte );
	void WriteBytes( const char* pBytes, const size_t size );
	template <typename T>
	void Write( const T& val )
	{
		const char* pStart = reinterpret_cast<const char*>(&val);
		const size_t size = sizeof( T );
		WriteBytes( pStart, size );
	}

	void WriteInt64( int64_t val );
	void WriteInt32( int32_t val );
	void WriteInt16( int16_t val );
	void WriteUInt64( uint64_t val );
	void WriteUInt32( uint32_t val );
	void WriteUInt16( uint16_t val );
	void WriteChar( char val ); // really just an alias for WriteByte
	void WriteShort( short val );
	void WriteInt( int val );
	void WriteFloat( float val );
	void WriteDouble( double val );
	void WriteBool( bool val );
	void WriteString( std::string_view str, bool terminate = true );

	bool EndOfStream() const;
	size_t Size() const;
	const char* Base() const;
	char* Base();

	void Clear();

	static MemoryStream FromStream( std::istream& stream );
	static MemoryStream FromFile( const std::string& filename, FileMode mode = FileMode::BINARY );
	static MemoryStream FromFile( const std::wstring& filename, FileMode mode = FileMode::BINARY );
	static void ToStream( const MemoryStream& ms, std::ostream& stream );
	static void ToFile( const MemoryStream& ms, const std::string& filename, FileMode mode = FileMode::BINARY );
	static void ToFile( const MemoryStream& ms, const std::wstring& filename, FileMode mode = FileMode::BINARY );
private:
	size_t m_iCurrentByte = 0;
	std::vector<char> m_Bytes;
};