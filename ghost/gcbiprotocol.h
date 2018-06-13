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

#ifndef GCBIPROTOCOL_H
#define GCBIPROTOCOL_H

//
// CGCBIProtocol
//

#define GCBI_HEADER_CONSTANT		249

#define REJECTGCBI_INVALID			1
#define REJECTGCBI_NOTFOUND			2

class CIncomingGarenaUser;

class CGCBIProtocol
{
public:
	enum Protocol {
		GCBI_INIT				= 1
	};

	CGCBIProtocol( );
	~CGCBIProtocol( );

	// receive functions
	CIncomingGarenaUser *RECEIVE_GCBI_INIT( BYTEARRAY data );

	// send functions

	// other functions

private:
	bool AssignLength( BYTEARRAY &content );
	bool ValidateLength( BYTEARRAY &content );
};

//
// CIncomingGarenaUser
//
	
class CIncomingGarenaUser
{
private:
	uint32_t m_IP;
	uint32_t m_UserID;
	uint32_t m_RoomID;
	uint32_t m_UserExp;
	string m_CountryCode;

public:
	CIncomingGarenaUser( uint32_t nIP, uint32_t nUserID, uint32_t nRoomID, uint32_t nUserExp, string nCountryCode );
	~CIncomingGarenaUser( );

	uint32_t GetIP( ) { return m_IP; }
	uint32_t GetUserID( ) { return m_UserID; }
	uint32_t GetRoomID( ) { return m_RoomID; }
	uint32_t GetUserExp( ) { return m_UserExp; }
	string GetCountryCode( ) { return m_CountryCode; }
};

#endif
