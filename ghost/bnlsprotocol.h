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

#ifndef BNLSPROTOCOL_H
#define BNLSPROTOCOL_H

//
// CBNLSProtocol
//

class CBNLSProtocol
{
public:
	enum Protocol {
		BNLS_NULL					= 0x00,
		BNLS_CDKEY					= 0x01,
		BNLS_LOGONCHALLENGE			= 0x02,
		BNLS_LOGONPROOF				= 0x03,
		BNLS_CREATEACCOUNT			= 0x04,
		BNLS_CHANGECHALLENGE		= 0x05,
		BNLS_CHANGEPROOF			= 0x06,
		BNLS_UPGRADECHALLENGE		= 0x07,
		BNLS_UPGRADEPROOF			= 0x08,
		BNLS_VERSIONCHECK			= 0x09,
		BNLS_CONFIRMLOGON			= 0x0a,
		BNLS_HASHDATA				= 0x0b,
		BNLS_CDKEY_EX				= 0x0c,
		BNLS_CHOOSENLSREVISION		= 0x0d,
		BNLS_AUTHORIZE				= 0x0e,
		BNLS_AUTHORIZEPROOF			= 0x0f,
		BNLS_REQUESTVERSIONBYTE		= 0x10,
		BNLS_VERIFYSERVER			= 0x11,
		BNLS_RESERVESERVERSLOTS		= 0x12,
		BNLS_SERVERLOGONCHALLENGE	= 0x13,
		BNLS_SERVERLOGONPROOF		= 0x14,
		BNLS_RESERVED0				= 0x15,
		BNLS_RESERVED1				= 0x16,
		BNLS_RESERVED2				= 0x17,
		BNLS_VERSIONCHECKEX			= 0x18,
		BNLS_RESERVED3				= 0x19,
		BNLS_VERSIONCHECKEX2		= 0x1a,
		BNLS_WARDEN					= 0x7d
	};

public:
	CBNLSProtocol( );
	~CBNLSProtocol( );

	// receive functions

	BYTEARRAY RECEIVE_BNLS_WARDEN( BYTEARRAY data );

	// send functions

	BYTEARRAY SEND_BNLS_NULL( );
	BYTEARRAY SEND_BNLS_WARDEN_SEED( uint32_t cookie, uint32_t seed );
	BYTEARRAY SEND_BNLS_WARDEN_RAW( uint32_t cookie, BYTEARRAY raw );
	BYTEARRAY SEND_BNLS_WARDEN_RUNMODULE( uint32_t cookie );

	// other functions

private:
	bool AssignLength( BYTEARRAY &content );
	bool ValidateLength( BYTEARRAY &content );
};

#endif
