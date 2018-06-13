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

#ifndef BNLSCLIENT_H
#define BNLSCLIENT_H

//
// CBNLSClient
//

class CTCPClient;
class CBNLSProtocol;
class CCommandPacket;

class CBNLSClient
{
private:
	CTCPClient *m_Socket;							// the connection to the BNLS server
	CBNLSProtocol *m_Protocol;						// battle.net protocol
	queue<CCommandPacket *> m_Packets;				// queue of incoming packets
	bool m_WasConnected;
	string m_Server;
	uint16_t m_Port;
	uint32_t m_LastNullTime;
	uint32_t m_WardenCookie;						// the warden cookie
	queue<BYTEARRAY> m_OutPackets;					// queue of outgoing packets to be sent
	queue<BYTEARRAY> m_WardenResponses;				// the warden responses to be sent to battle.net
	uint32_t m_TotalWardenIn;
	uint32_t m_TotalWardenOut;

public:
	CBNLSClient( string nServer, uint16_t nPort, uint32_t nWardenCookie );
	~CBNLSClient( );

	BYTEARRAY GetWardenResponse( );
	uint32_t GetTotalWardenIn( )		{ return m_TotalWardenIn; }
	uint32_t GetTotalWardenOut( )		{ return m_TotalWardenOut; }

	// processing functions

	unsigned int SetFD( void *fd, void *send_fd, int *nfds );
	bool Update( void *fd, void *send_fd );
	void ExtractPackets( );
	void ProcessPackets( );

	// other functions

	void QueueWardenSeed( uint32_t seed );
	void QueueWardenRaw( BYTEARRAY wardenRaw );
};

#endif
