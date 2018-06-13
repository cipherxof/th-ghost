/*

	ent-ghost
	Copyright [2011-2013] [Jack Lu]

	This file is part of the ent-ghost source code.

	ent-ghost is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	ent-ghost source code is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with ent-ghost source code. If not, see <http://www.gnu.org/licenses/>.

	ent-ghost is modified from GHost++ (http://ghostplusplus.googlecode.com/)
	GHost++ is Copyright [2008] [Trevor Hogan]

*/

#ifndef UTIL_H
#define UTIL_H

// byte arrays

BYTEARRAY UTIL_CreateByteArray( unsigned char *a, int size );
BYTEARRAY UTIL_CreateByteArray( unsigned char c );
BYTEARRAY UTIL_CreateByteArray( uint16_t i, bool reverse );
BYTEARRAY UTIL_CreateByteArray( uint32_t i, bool reverse );
uint16_t UTIL_ByteArrayToUInt16( BYTEARRAY b, bool reverse, unsigned int start = 0 );
uint32_t UTIL_ByteArrayToUInt32( BYTEARRAY b, bool reverse, unsigned int start = 0 );
string UTIL_ByteArrayToDecString( BYTEARRAY b );
string UTIL_ByteArrayToHexString( BYTEARRAY b );
void UTIL_AppendByteArray( BYTEARRAY &b, BYTEARRAY append );
void UTIL_AppendByteArrayFast( BYTEARRAY &b, BYTEARRAY &append );
void UTIL_AppendByteArray( BYTEARRAY &b, unsigned char *a, int size );
void UTIL_AppendByteArray( BYTEARRAY &b, string append, bool terminator = true );
void UTIL_AppendByteArrayFast( BYTEARRAY &b, string &append, bool terminator = true );
void UTIL_AppendByteArray( BYTEARRAY &b, uint16_t i, bool reverse );
void UTIL_AppendByteArray( BYTEARRAY &b, uint32_t i, bool reverse );
BYTEARRAY UTIL_ExtractCString( BYTEARRAY &b, unsigned int start );
unsigned char UTIL_ExtractHex( BYTEARRAY &b, unsigned int start, bool reverse );
BYTEARRAY UTIL_ExtractNumbers( string s, unsigned int count );
BYTEARRAY UTIL_ExtractHexNumbers( string s );

// conversions

string UTIL_ToString( unsigned long i );
string UTIL_ToString( unsigned short i );
string UTIL_ToString( unsigned int i );
string UTIL_ToString( long i );
string UTIL_ToString( short i );
string UTIL_ToString( int i );
string UTIL_ToString( float f, int digits );
string UTIL_ToString( double d, int digits );
string UTIL_ToHexString( uint32_t i );
uint16_t UTIL_ToUInt16( string &s );
uint32_t UTIL_ToUInt32( string &s );
uint64_t UTIL_ToUInt64( string &s );
int16_t UTIL_ToInt16( string &s );
int32_t UTIL_ToInt32( string &s );
double UTIL_ToDouble( string &s );
string UTIL_MSToString( uint32_t ms );

// files

bool UTIL_FileExists( string file );
string UTIL_FileRead( string file, uint32_t start, uint32_t length );
string UTIL_FileRead( string file );
bool UTIL_FileWrite( string file, unsigned char *data, uint32_t length );
string UTIL_FileSafeName( string fileName );
string UTIL_AddPathSeperator( string path );

// stat strings

BYTEARRAY UTIL_EncodeStatString( BYTEARRAY &data );
BYTEARRAY UTIL_DecodeStatString( BYTEARRAY &data );

// other

bool UTIL_IsLanIP( BYTEARRAY ip );
bool UTIL_IsLocalIP( BYTEARRAY ip, vector<BYTEARRAY> &localIPs );
void UTIL_Replace( string &Text, string Key, string Value );
vector<string> UTIL_Tokenize( string s, char delim );

// math

uint32_t UTIL_Factorial( uint32_t x );

#define nCr(n, r) (UTIL_Factorial(n) / UTIL_Factorial((n)-(r)) / UTIL_Factorial(r))
#define nPr(n, r) (UTIL_Factorial(n) / UTIL_Factorial((n)-(r)))

#endif
