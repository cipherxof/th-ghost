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

#ifndef BNETPROTOCOL_H
#define BNETPROTOCOL_H

//
// CBNETProtocol
//

#define BNET_HEADER_CONSTANT 255

class CIncomingGameHost;
class CIncomingChatEvent;
class CIncomingFriendList;
class CIncomingClanList;

class CBNETProtocol
{
public:
	enum Protocol {
		SID_NULL					= 0,	// 0x0
		SID_STOPADV					= 2,	// 0x2
		SID_GETADVLISTEX			= 9,	// 0x9
		SID_ENTERCHAT				= 10,	// 0xA
		SID_JOINCHANNEL				= 12,	// 0xC
		SID_CHATCOMMAND				= 14,	// 0xE
		SID_CHATEVENT				= 15,	// 0xF
		SID_CHECKAD					= 21,	// 0x15
		SID_STARTADVEX3				= 28,	// 0x1C
		SID_DISPLAYAD				= 33,	// 0x21
		SID_NOTIFYJOIN				= 34,	// 0x22
		SID_PING					= 37,	// 0x25
		SID_LOGONRESPONSE			= 41,	// 0x29
		SID_NETGAMEPORT				= 69,	// 0x45
		SID_AUTH_INFO				= 80,	// 0x50
		SID_AUTH_CHECK				= 81,	// 0x51
		SID_AUTH_ACCOUNTLOGON		= 83,	// 0x53
		SID_AUTH_ACCOUNTLOGONPROOF	= 84,	// 0x54
		SID_WARDEN					= 94,	// 0x5E
		SID_FRIENDSLIST				= 101,	// 0x65
		SID_FRIENDSUPDATE			= 102,	// 0x66
		SID_CLANCREATIONINVITATION	= 114,	// 0x72
		SID_CLANINVITATION			= 119,  // 0x77
		SID_CLANREMOVEMEMBER    	= 120,  // 0x78
		SID_CLANINVITATIONRESPONSE	= 121,	// 0x79
		SID_CLANCHANGERANK			= 122,	// 0x7A
		SID_CLANSETMOTD 			= 123,	// 0x7B
		SID_CLANMEMBERLIST			= 125,	// 0x7D
		SID_CLANMEMBERSTATUSCHANGE	= 127	// 0x7F
	};

	enum KeyResult {
		KR_GOOD				= 0,
		KR_OLD_GAME_VERSION	= 256,
		KR_INVALID_VERSION	= 257,
		KR_ROC_KEY_IN_USE	= 513,
		KR_TFT_KEY_IN_USE	= 529
	};

	enum IncomingChatEvent {
		EID_SHOWUSER			= 1,	// received when you join a channel (includes users in the channel and their information)
		EID_JOIN				= 2,	// received when someone joins the channel you're currently in
		EID_LEAVE				= 3,	// received when someone leaves the channel you're currently in
		EID_WHISPER				= 4,	// received a whisper message
		EID_TALK				= 5,	// received when someone talks in the channel you're currently in
		EID_BROADCAST			= 6,	// server broadcast
		EID_CHANNEL				= 7,	// received when you join a channel (includes the channel's name, flags)
		EID_USERFLAGS			= 9,	// user flags updates
		EID_WHISPERSENT			= 10,	// sent a whisper message
		EID_CHANNELFULL			= 13,	// channel is full
		EID_CHANNELDOESNOTEXIST	= 14,	// channel does not exist
		EID_CHANNELRESTRICTED	= 15,	// channel is restricted
		EID_INFO				= 18,	// broadcast/information message
		EID_ERROR				= 19,	// error message
		EID_EMOTE				= 23	// emote
	};

	enum RankCode {
		CLAN_INITIATE = 0,		// 0x00 First week member
		CLAN_PARTIAL_MEMBER = 1,	// 0x01 Peon
		CLAN_MEMBER = 2,		// 0x02 Grunt
		CLAN_OFFICER = 3,		// 0x03 Shaman
		CLAN_LEADER = 4			// 0x04 Chieftain
	};

private:
	BYTEARRAY m_ClientToken;			// set in constructor
	BYTEARRAY m_LogonType;				// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_ServerToken;			// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_MPQFileTime;			// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_IX86VerFileName;		// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_ValueStringFormula;		// set in RECEIVE_SID_AUTH_INFO
	BYTEARRAY m_KeyState;				// set in RECEIVE_SID_AUTH_CHECK
	BYTEARRAY m_KeyStateDescription;	// set in RECEIVE_SID_AUTH_CHECK
	BYTEARRAY m_Salt;					// set in RECEIVE_SID_AUTH_ACCOUNTLOGON
	BYTEARRAY m_ServerPublicKey;		// set in RECEIVE_SID_AUTH_ACCOUNTLOGON
	BYTEARRAY m_UniqueName;				// set in RECEIVE_SID_ENTERCHAT
	BYTEARRAY m_ClanLastInviteTag;		// set in RECEIVE_SID_CLANCREATIONINVITATION, SID_CLANINVITATIONRESPONSE
	BYTEARRAY m_ClanLastInviteName;		// set in RECEIVE_SID_CLANCREATIONINVITATION, SID_CLANINVITATIONRESPONSE

public:
	CBNETProtocol( );
	~CBNETProtocol( );

	BYTEARRAY GetClientToken( )				{ return m_ClientToken; }
	BYTEARRAY GetLogonType( )				{ return m_LogonType; }
	BYTEARRAY GetServerToken( )				{ return m_ServerToken; }
	BYTEARRAY GetMPQFileTime( )				{ return m_MPQFileTime; }
	BYTEARRAY GetIX86VerFileName( )			{ return m_IX86VerFileName; }
	string GetIX86VerFileNameString( )		{ return string( m_IX86VerFileName.begin( ), m_IX86VerFileName.end( ) ); }
	BYTEARRAY GetValueStringFormula( )		{ return m_ValueStringFormula; }
	string GetValueStringFormulaString( )	{ return string( m_ValueStringFormula.begin( ), m_ValueStringFormula.end( ) ); }
	BYTEARRAY GetKeyState( )				{ return m_KeyState; }
	string GetKeyStateDescription( )		{ return string( m_KeyStateDescription.begin( ), m_KeyStateDescription.end( ) ); }
	BYTEARRAY GetSalt( )					{ return m_Salt; }
	BYTEARRAY GetServerPublicKey( )			{ return m_ServerPublicKey; }
	BYTEARRAY GetUniqueName( )				{ return m_UniqueName; }

	// receive functions

	bool RECEIVE_SID_NULL( BYTEARRAY data );
	CIncomingGameHost *RECEIVE_SID_GETADVLISTEX( BYTEARRAY data );
	bool RECEIVE_SID_ENTERCHAT( BYTEARRAY data );
	CIncomingChatEvent *RECEIVE_SID_CHATEVENT( BYTEARRAY data );
	bool RECEIVE_SID_CHECKAD( BYTEARRAY data );
	bool RECEIVE_SID_STARTADVEX3( BYTEARRAY data );
	BYTEARRAY RECEIVE_SID_PING( BYTEARRAY data );
	bool RECEIVE_SID_LOGONRESPONSE( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_INFO( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_CHECK( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_ACCOUNTLOGON( BYTEARRAY data );
	bool RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY data );
	BYTEARRAY RECEIVE_SID_WARDEN( BYTEARRAY data );
	vector<CIncomingFriendList *> RECEIVE_SID_FRIENDSLIST( BYTEARRAY data );
	vector<CIncomingClanList *> RECEIVE_SID_CLANMEMBERLIST( BYTEARRAY data );
	CIncomingClanList *RECEIVE_SID_CLANMEMBERSTATUSCHANGE( BYTEARRAY data );
	string RECEIVE_SID_CLANCREATIONINVITATION( BYTEARRAY data );
	string RECEIVE_SID_CLANINVITATIONRESPONSE( BYTEARRAY data );

	// send functions

	BYTEARRAY SEND_PROTOCOL_INITIALIZE_SELECTOR( );
	BYTEARRAY SEND_SID_NULL( );
	BYTEARRAY SEND_SID_STOPADV( );
	BYTEARRAY SEND_SID_GETADVLISTEX( string gameName );
	BYTEARRAY SEND_SID_ENTERCHAT( );
	BYTEARRAY SEND_SID_JOINCHANNEL( string channel );
	BYTEARRAY SEND_SID_CHATCOMMAND( string command );
	BYTEARRAY SEND_SID_CHECKAD( );
	BYTEARRAY SEND_SID_STARTADVEX3( unsigned char state, BYTEARRAY mapGameType, BYTEARRAY mapFlags, BYTEARRAY mapWidth, BYTEARRAY mapHeight, string gameName, string hostName, uint32_t upTime, string mapPath, BYTEARRAY mapCRC, BYTEARRAY mapSHA1, uint32_t hostCounter );
	BYTEARRAY SEND_SID_NOTIFYJOIN( string gameName );
	BYTEARRAY SEND_SID_PING( BYTEARRAY pingValue );
	BYTEARRAY SEND_SID_LOGONRESPONSE( BYTEARRAY clientToken, BYTEARRAY serverToken, BYTEARRAY passwordHash, string accountName );
	BYTEARRAY SEND_SID_NETGAMEPORT( uint16_t serverPort );
	BYTEARRAY SEND_SID_AUTH_INFO( unsigned char ver, bool TFT, uint32_t localeID, string countryAbbrev, string country );
	BYTEARRAY SEND_SID_AUTH_CHECK( bool TFT, BYTEARRAY clientToken, BYTEARRAY exeVersion, BYTEARRAY exeVersionHash, BYTEARRAY keyInfoROC, BYTEARRAY keyInfoTFT, string exeInfo, string keyOwnerName );
	BYTEARRAY SEND_SID_AUTH_ACCOUNTLOGON( BYTEARRAY clientPublicKey, string accountName );
	BYTEARRAY SEND_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY clientPasswordProof );
	BYTEARRAY SEND_SID_WARDEN( BYTEARRAY wardenResponse );
	BYTEARRAY SEND_SID_FRIENDSLIST( );
	BYTEARRAY SEND_SID_CLANMEMBERLIST( );
	BYTEARRAY SEND_SID_CLANINVITATION( string accountName );
	BYTEARRAY SEND_SID_CLANREMOVEMEMBER( string accountName );
	BYTEARRAY SEND_SID_CLANCHANGERANK( string accountName, CBNETProtocol :: RankCode rank );
	BYTEARRAY SEND_SID_CLANSETMOTD( string motd );
	BYTEARRAY SEND_SID_CLANCREATIONINVITATION( bool accept );
	BYTEARRAY SEND_SID_CLANINVITATIONRESPONSE( bool accept );

	// other functions

private:
	bool AssignLength( BYTEARRAY &content );
	bool ValidateLength( BYTEARRAY &content );
};

//
// CIncomingGameHost
//

class CIncomingGameHost
{
private:
	BYTEARRAY m_IP;
	uint16_t m_Port;
	string m_GameName;
	BYTEARRAY m_HostCounter;

public:
	CIncomingGameHost( BYTEARRAY &nIP, uint16_t nPort, string nGameName, BYTEARRAY &nHostCounter );
	~CIncomingGameHost( );

	BYTEARRAY GetIP( )			{ return m_IP; }
	string GetIPString( );
	uint16_t GetPort( )			{ return m_Port; }
	string GetGameName( )		{ return m_GameName; }
	BYTEARRAY GetHostCounter( )	{ return m_HostCounter; }
};

//
// CIncomingChatEvent
//

class CIncomingChatEvent
{
private:
	CBNETProtocol :: IncomingChatEvent m_ChatEvent;
        int32_t m_Ping;
	string m_User;
	string m_Message;

public:
        CIncomingChatEvent( CBNETProtocol :: IncomingChatEvent nChatEvent, int32_t nPing, string nUser, string nMessage );
	~CIncomingChatEvent( );

	CBNETProtocol :: IncomingChatEvent GetChatEvent( )	{ return m_ChatEvent; }
        int32_t GetPing( )									{ return m_Ping; }
	string GetUser( )									{ return m_User; }
	string GetMessage( )								{ return m_Message; }
};

//
// CIncomingFriendList
//

class CIncomingFriendList
{
private:
	string m_Account;
	unsigned char m_Status;
	unsigned char m_Area;
	string m_Location;

public:
	CIncomingFriendList( string nAccount, unsigned char nStatus, unsigned char nArea, string nLocation );
	~CIncomingFriendList( );

	string GetAccount( )		{ return m_Account; }
	unsigned char GetStatus( )	{ return m_Status; }
	unsigned char GetArea( )	{ return m_Area; }
	string GetLocation( )		{ return m_Location; }
	string GetDescription( );

private:
	string ExtractStatus( unsigned char status );
	string ExtractArea( unsigned char area );
	string ExtractLocation( string location );
};

//
// CIncomingClanList
//

class CIncomingClanList
{
private:
	string m_Name;
	unsigned char m_Rank;
	unsigned char m_Status;

public:
	CIncomingClanList( string nName, unsigned char nRank, unsigned char nStatus );
	~CIncomingClanList( );

	string GetName( )			{ return m_Name; }
	string GetRank( );
	string GetStatus( );
	string GetDescription( );
};

#endif
