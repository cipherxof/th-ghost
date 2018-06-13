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

#include "ghost.h"
#include "util.h"
#include "bnlsprotocol.h"

CBNLSProtocol :: CBNLSProtocol( )
{

}

CBNLSProtocol :: ~CBNLSProtocol( )
{

}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////

BYTEARRAY CBNLSProtocol :: RECEIVE_BNLS_WARDEN( BYTEARRAY data )
{
	// 2 bytes					-> Length
	// 1 byte					-> ID
	// (BYTE)					-> Usage
	// (DWORD)					-> Cookie
	// (BYTE)					-> Result
	// (WORD)					-> Length of data
	// (VOID)					-> Data

	if( ValidateLength( data ) && data.size( ) >= 11 )
	{
		unsigned char Usage = data[3];
		uint32_t Cookie = UTIL_ByteArrayToUInt32( data, false, 4 );
		unsigned char Result = data[8];
		uint16_t Length = UTIL_ByteArrayToUInt16( data, false, 10 );

		if( Result == 0x00 )
			return BYTEARRAY( data.begin( ) + 11, data.end( ) );
		else
			CONSOLE_Print( "[BNLSPROTO] received error code " + UTIL_ToString( data[8] ) );
	}

	return BYTEARRAY( );
}

////////////////////
// SEND FUNCTIONS //
////////////////////

BYTEARRAY CBNLSProtocol :: SEND_BNLS_NULL( )
{
	BYTEARRAY packet;
	packet.push_back( 0 );							// packet length will be assigned later
	packet.push_back( 0 );							// packet length will be assigned later
	packet.push_back( BNLS_NULL );					// BNLS_NULL
	AssignLength( packet );
	return packet;
}

BYTEARRAY CBNLSProtocol :: SEND_BNLS_WARDEN_SEED( uint32_t cookie, uint32_t seed )
{
	unsigned char Client[] = {  80,  88,  51,  87 };	// "W3XP"

	BYTEARRAY packet;
	packet.push_back( 0 );								// packet length will be assigned later
	packet.push_back( 0 );								// packet length will be assigned later
	packet.push_back( BNLS_WARDEN );					// BNLS_WARDEN
	packet.push_back( 0 );								// BNLS_WARDEN_SEED
	UTIL_AppendByteArray( packet, cookie, false );		// cookie
	UTIL_AppendByteArray( packet, Client, 4 );			// Client
	UTIL_AppendByteArray( packet, (uint16_t)4, false );	// length of seed
	UTIL_AppendByteArray( packet, seed, false );		// seed
	packet.push_back( 0 );								// username is blank
	UTIL_AppendByteArray( packet, (uint16_t)0, false );	// password length
														// password
	AssignLength( packet );
	return packet;
}

BYTEARRAY CBNLSProtocol :: SEND_BNLS_WARDEN_RAW( uint32_t cookie, BYTEARRAY raw )
{
	BYTEARRAY packet;
	packet.push_back( 0 );											// packet length will be assigned later
	packet.push_back( 0 );											// packet length will be assigned later
	packet.push_back( BNLS_WARDEN );								// BNLS_WARDEN
	packet.push_back( 1 );											// BNLS_WARDEN_RAW
	UTIL_AppendByteArray( packet, cookie, false );					// cookie
	UTIL_AppendByteArray( packet, (uint16_t)raw.size( ), false );	// raw length
	UTIL_AppendByteArray( packet, raw );							// raw
	AssignLength( packet );
	return packet;
}

BYTEARRAY CBNLSProtocol :: SEND_BNLS_WARDEN_RUNMODULE( uint32_t cookie )
{
	return BYTEARRAY( );
}

/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool CBNLSProtocol :: AssignLength( BYTEARRAY &content )
{
	// insert the actual length of the content array into bytes 1 and 2 (indices 0 and 1)

	BYTEARRAY LengthBytes;

	if( content.size( ) >= 2 && content.size( ) <= 65535 )
	{
		LengthBytes = UTIL_CreateByteArray( (uint16_t)content.size( ), false );
		content[0] = LengthBytes[0];
		content[1] = LengthBytes[1];
		return true;
	}

	return false;
}

bool CBNLSProtocol :: ValidateLength( BYTEARRAY &content )
{
	// verify that bytes 1 and 2 (indices 0 and 1) of the content array describe the length

	uint16_t Length;
	BYTEARRAY LengthBytes;

	if( content.size( ) >= 2 && content.size( ) <= 65535 )
	{
		LengthBytes.push_back( content[0] );
		LengthBytes.push_back( content[1] );
		Length = UTIL_ByteArrayToUInt16( LengthBytes, false );

		if( Length == content.size( ) )
			return true;
	}

	return false;
}
