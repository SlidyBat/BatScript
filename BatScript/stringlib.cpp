#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "stringlib.h"

#include <locale>
#include <codecvt>
#include <cassert>

namespace Bat
{
	std::wstring StringToWide( std::string_view str )
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes( &*str.cend(), &*str.cend() );
	}
	std::string WideToString( std::wstring_view wstr )
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes( &*wstr.cend(), &*wstr.cend() );
	}
	std::vector<std::string> SplitString( std::string_view str, const char delim )
	{
		std::vector<std::string> elems;
		size_t start = 0;
		size_t count = 0;
		for( size_t i = 0; i < str.size(); i++ )
		{
			if( str[i] == delim )
			{
				elems.emplace_back( str.substr( start, count ) );
				start = i + 1;
				count = 0;
			}
			else
			{
				count++;
			}
		}

		elems.emplace_back( str.substr( start, count ) );

		return elems;
	}

	std::string JoinStrings( const std::vector<std::string>& strings, const std::string & delim )
	{
		std::string res;

		for( size_t i = 0; i < strings.size(); i++ )
		{
			res += strings[i];
			if( i != strings.size() - 1 )
			{
				res += delim;
			}
		}

		return res;
	}

	std::string JoinStrings( const std::vector<std::string_view>& strings, const std::string & delim )
	{
		std::string res;

		for( size_t i = 0; i < strings.size(); i++ )
		{
			res += std::string( strings[i] );
			if( i != strings.size() - 1 )
			{
				res += delim;
			}
		}

		return res;
	}

	std::string_view GetFileExtension( std::string_view filename )
	{
		size_t off = filename.find_last_of( '.' );
		if( off == std::string::npos )
		{
			return {};
		}

		return filename.substr( off + 1 );
	}
	std::wstring_view GetFileExtension( std::wstring_view filename )
	{
		size_t off = filename.find_last_of( '.' );
		if( off == std::string::npos )
		{
			return {};
		}

		return filename.substr( off + 1 );
	}

	bool IsWhitespace( const char c )
	{
		return isspace( c );
	}
	bool IsLowercase( const char c )
	{
		return islower( c );
	}
	bool IsUppercase( const char c )
	{
		return isupper( c );
	}
	bool IsNumeric( const char c )
	{
		return c >= '0' && c <= '9';
	}
	bool IsNumeric( std::string_view str )
	{
		for( char c : str )
		{
			if( !IsNumeric( c ) )
			{
				return false;
			}
		}

		return true;
	}
	bool IsAlphabetic( const char c )
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}
	bool IsAlphabetic( std::string_view str )
	{
		for( char c : str )
		{
			if( !IsAlphabetic( c ) )
			{
				return false;
			}
		}

		return true;
	}
	bool IsAlphanumeric( const char c )
	{
		return IsNumeric( c ) || IsAlphabetic( c );
	}
	bool IsAlphanumeric( std::string_view str )
	{
		for( char c : str )
		{
			if( !IsAlphanumeric( c ) )
			{
				return false;
			}
		}

		return true;
	}
	bool IsIdentifier( const char c )
	{
		return IsNumeric( c ) || IsAlphabetic( c ) || c == '_';
	}
	bool IsIdentifier( std::string_view str )
	{
		for( char c : str )
		{
			if( !IsIdentifier( c ) )
			{
				return false;
			}
		}

		return true;
	}
	bool IsInteger( std::string_view str )
	{
		if( !IsNumeric( str[0] ) && str[0] != '-' )
		{
			return false;
		}

		for( size_t i = 1; i < str.size(); i++ )
		{
			if( !IsNumeric( str[i] ) )
			{
				return false;
			}
		}

		return true;
	}
	bool IsFloat( std::string_view str )
	{
		bool found_dot = (str[0] == '.');
		if( !IsNumeric( str[0] ) && str[0] != '-' && !found_dot )
		{
			return false;
		}

		for( size_t i = 1; i < str.size(); i++ )
		{
			if( !IsNumeric( str[i] ) )
			{
				if( str[i] == '.' )
				{
					// can't have multiple decimal points
					if( found_dot )
					{
						return false;
					}

					found_dot = true;
				}
				else
				{
					return false;
				}
			}
		}

		return true;
	}

	std::string Trim( std::string_view str )
	{
		if( str.empty() )
		{
			return "";
		}

		const size_t size = str.size();

		size_t start = 0;
		while( IsWhitespace( str[start] ) && start < size )
		{
			start++;
		}

		// the whole thing was whitespace
		if( start == str.size() )
		{
			return "";
		}

		size_t end = size - 1;
		while( IsWhitespace( str[end] ) )
		{
			end--;
		}

		return std::string( str.substr( start, end - start + 1 ) );
	}
	std::string ToLower( std::string str )
	{
		for( size_t i = 0; i < str.size(); i++ )
		{
			str[i] = tolower( str[i] );
		}

		return str;
	}
	std::string ToUpper( std::string str )
	{
		for( size_t i = 0; i < str.size(); i++ )
		{
			str[i] = toupper( str[i] );
		}

		return str;
	}
}