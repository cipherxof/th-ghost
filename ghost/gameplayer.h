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

#ifndef GAMEPLAYER_H
#define GAMEPLAYER_H

class CTCPSocket;
class CCommandPacket;
class CGameProtocol;
class CGame;
class CIncomingJoinPlayer;
class CIncomingGarenaUser;
class CCallableBanCheck;
class CDBBan;

//
// CPotentialPlayer
//

class CPotentialPlayer
{
public:
	CGameProtocol *m_Protocol;
	CBaseGame *m_Game;

protected:
	// note: we permit m_Socket to be NULL in this class to allow for the virtual host player which doesn't really exist
	// it also allows us to convert CPotentialPlayers to CGamePlayers without the CPotentialPlayer's destructor closing the socket

	CTCPSocket *m_Socket;
	queue<CCommandPacket *> m_Packets;
	bool m_DeleteMe;
	bool m_Error;
	string m_ErrorString;
	CIncomingJoinPlayer *m_IncomingJoinPlayer;
	CIncomingGarenaUser *m_IncomingGarenaUser;
	
	bool m_Banned;
	CCallableBanCheck *m_CallableBanCheck;

    uint32_t m_ConnectionState; // zero if no packets received (wait REQJOIN), one if only REQJOIN received (wait MAPSIZE), two otherwise
    uint32_t m_ConnectionTime;  // last time the player did something relating to connection state
	string m_CachedIP;							// cached external IP string

public:
	CPotentialPlayer( CGameProtocol *nProtocol, CBaseGame *nGame, CTCPSocket *nSocket );
	virtual ~CPotentialPlayer( );

	virtual CTCPSocket *GetSocket( )				{ return m_Socket; }
	virtual BYTEARRAY GetExternalIP( );
	virtual string GetExternalIPString( );
	virtual queue<CCommandPacket *> GetPackets( )	{ return m_Packets; }
	virtual bool GetDeleteMe( )						{ return m_DeleteMe; }
	virtual bool GetError( )						{ return m_Error; }
	virtual string GetErrorString( )				{ return m_ErrorString; }
	virtual CIncomingJoinPlayer *GetJoinPlayer( )	{ return m_IncomingJoinPlayer; }
	virtual BYTEARRAY GetGarenaIP( );
	virtual CIncomingGarenaUser *GetGarenaUser( )	{ return m_IncomingGarenaUser; }

	virtual void SetSocket( CTCPSocket *nSocket )	{ m_Socket = nSocket; }
	virtual void SetDeleteMe( bool nDeleteMe )		{ m_DeleteMe = nDeleteMe; }
	virtual void SetGarenaUser( CIncomingGarenaUser *nIncomingGarenaUser ) { m_IncomingGarenaUser = nIncomingGarenaUser; }
	virtual void SetBanned( )						{ m_Banned = true; }

	// processing functions

	virtual bool Update( void *fd );
	virtual void ExtractPackets( );
	virtual void ProcessPackets( );

	// other functions

	virtual void Send( BYTEARRAY data );
	virtual void SendBannedInfo( CDBBan *Ban, string type );
};

//
// CGamePlayer
//

class CGamePlayer : public CPotentialPlayer
{
private:
	unsigned char m_PID;
	string m_Name;								// the player's name
	string m_FriendlyName;						// the player's name to other players (in case spoofed with donate)
	BYTEARRAY m_InternalIP;						// the player's internal IP address as reported by the player when connecting
	vector<uint32_t> m_Pings;					// store the last few (20) pings received so we can take an average
	queue<uint32_t> m_CheckSums;				// the last few checksums the player has sent (for detecting desyncs)
	string m_LeftReason;						// the reason the player left the game
	string m_SpoofedRealm;						// the realm the player last spoof checked on
	string m_JoinedRealm;						// the realm the player joined on (probable, can be spoofed)
	uint32_t m_TotalPacketsSent;
	uint32_t m_TotalPacketsReceived;
	uint32_t m_LeftCode;						// the code to be sent in W3GS_PLAYERLEAVE_OTHERS for why this player left the game
	uint32_t m_Cookies;					// the number of cookies this user has, for the !cookie and !eat command
	uint32_t m_SyncCounter;						// the number of keepalive packets received from this player
	uint32_t m_JoinTime;						// GetTime when the player joined the game (used to delay sending the /whois a few seconds to allow for some lag)
	uint32_t m_LastMapPartSent;					// the last mappart sent to the player (for sending more than one part at a time)
	uint32_t m_LastMapPartAcked;				// the last mappart acknowledged by the player
	uint32_t m_StartedDownloadingTicks;			// GetTicks when the player started downloading the map
	uint32_t m_FinishedDownloadingTime;			// GetTime when the player finished downloading the map
	uint32_t m_FinishedLoadingTicks;			// GetTicks when the player finished loading the game
	uint32_t m_StartedLaggingTicks;				// GetTicks when the player started lagging
	uint32_t m_TotalLaggingTicks;				// total ticks that the player has been lagging in the game
	uint32_t m_StatsSentTime;					// GetTime when we sent this player's stats to the chat (to prevent players from spamming !stats)
	uint32_t m_StatsDotASentTime;				// GetTime when we sent this player's dota stats to the chat (to prevent players from spamming !statsdota)
	uint32_t m_LastGProxyWaitNoticeSentTime;
	queue<BYTEARRAY> m_LoadInGameData;			// queued data to be sent when the player finishes loading when using "load in game"
	double m_Score;								// the player's generic "score" for the matchmaking algorithm
	bool m_Spoofed;								// if the player has spoof checked or not
	bool m_Reserved;							// if the player is reserved (VIP) or not
	bool m_WhoisShouldBeSent;					// if a battle.net /whois should be sent for this player or not
	bool m_WhoisSent;							// if we've sent a battle.net /whois for this player yet (for spoof checking)
	bool m_DownloadAllowed;						// if we're allowed to download the map or not (used with permission based map downloads)
	bool m_DownloadStarted;						// if we've started downloading the map or not
	bool m_DownloadFinished;					// if we've finished downloading the map or not
	bool m_FinishedLoading;						// if the player has finished loading or not
	bool m_Lagging;								// if the player is lagging or not (on the lag screen)
	bool m_DropVote;							// if the player voted to drop the laggers or not (on the lag screen)
	bool m_KickVote;							// if the player voted to kick a player or not
	bool m_ForfeitVote;
	bool m_DrawVote;
	bool m_StartVote;							// if the player voted to start or not
	bool m_Muted;								// if the player is muted or not
	bool m_Autoban;								// whether or not this player should be consider for autobanning
	vector<uint32_t> m_MuteMessages;			// times player sent messages to determine if we should automute
	uint32_t m_MutedTicks;
	vector<uint32_t> m_FlameMessages;			// times player sent messages to determine flame level
	bool m_MutedAuto;							// whether or not mute was automatic mute
	bool m_LeftMessageSent;						// if the playerleave message has been sent or not
	bool m_GProxy;								// if the player is using GProxy++
	bool m_GProxyDisconnectNoticeSent;			// if a disconnection notice has been sent or not when using GProxy++
	queue<BYTEARRAY> m_GProxyBuffer;
	uint32_t m_GProxyReconnectKey;
	uint32_t m_LastGProxyAckTime;
	vector<string> m_IgnoreList;				// list of usernames this player is ignoring
	bool m_NoLag;								// if the player wants to ignore lag screen

public:
	CGamePlayer( CGameProtocol *nProtocol, CBaseGame *nGame, CTCPSocket *nSocket, unsigned char nPID, string nJoinedRealm, string nName, BYTEARRAY nInternalIP, bool nReserved );
	CGamePlayer( CPotentialPlayer *potential, unsigned char nPID, string nJoinedRealm, string nName, BYTEARRAY nInternalIP, bool nReserved );
	virtual ~CGamePlayer( );

	unsigned char GetPID( )						{ return m_PID; }
	string GetName( )							{ return m_Name; }
	string GetFriendlyName( )					{ return m_FriendlyName; }
	BYTEARRAY GetInternalIP( )					{ return m_InternalIP; }
	unsigned int GetNumPings( )					{ return m_Pings.size( ); }
	unsigned int GetNumCheckSums( )				{ return m_CheckSums.size( ); }
	queue<uint32_t> *GetCheckSums( )			{ return &m_CheckSums; }
	string GetLeftReason( )						{ return m_LeftReason; }
	string GetSpoofedRealm( )					{ return m_SpoofedRealm; }
	string GetJoinedRealm( )					{ return m_JoinedRealm; }
	uint32_t GetCookies( )						{ return m_Cookies; }
	uint32_t GetLeftCode( )						{ return m_LeftCode; }
	uint32_t GetSyncCounter( )					{ return m_SyncCounter; }
	uint32_t GetJoinTime( )						{ return m_JoinTime; }
	uint32_t GetLastMapPartSent( )				{ return m_LastMapPartSent; }
	uint32_t GetLastMapPartAcked( )				{ return m_LastMapPartAcked; }
	uint32_t GetStartedDownloadingTicks( )		{ return m_StartedDownloadingTicks; }
	uint32_t GetFinishedDownloadingTime( )		{ return m_FinishedDownloadingTime; }
	uint32_t GetFinishedLoadingTicks( )			{ return m_FinishedLoadingTicks; }
	uint32_t GetStartedLaggingTicks( )			{ return m_StartedLaggingTicks; }
	uint32_t GetTotalLaggingTicks( )			{ return m_TotalLaggingTicks; }
	uint32_t GetStatsSentTime( )				{ return m_StatsSentTime; }
	uint32_t GetStatsDotASentTime( )			{ return m_StatsDotASentTime; }
	uint32_t GetLastGProxyWaitNoticeSentTime( )	{ return m_LastGProxyWaitNoticeSentTime; }
	queue<BYTEARRAY> *GetLoadInGameData( )		{ return &m_LoadInGameData; }
	double GetScore( )							{ return m_Score; }
	bool GetSpoofed( )							{ return m_Spoofed; }
	bool GetReserved( )							{ return m_Reserved; }
	bool GetWhoisShouldBeSent( )				{ return m_WhoisShouldBeSent; }
	bool GetWhoisSent( )						{ return m_WhoisSent; }
	bool GetDownloadAllowed( )					{ return m_DownloadAllowed; }
	bool GetDownloadStarted( )					{ return m_DownloadStarted; }
	bool GetDownloadFinished( )					{ return m_DownloadFinished; }
	bool GetFinishedLoading( )					{ return m_FinishedLoading; }
	bool GetLagging( )							{ return m_Lagging; }
	bool GetDropVote( )							{ return m_DropVote; }
	bool GetKickVote( )							{ return m_KickVote; }
	bool GetStartVote( )						{ return m_StartVote; }
	bool GetForfeitVote( )						{ return m_ForfeitVote; }
	bool GetDrawVote( )							{ return m_DrawVote; }
	bool GetMuted( )							{ return m_Muted; }
	bool GetAutoban( )							{ return m_Autoban; }
	bool GetLeftMessageSent( )					{ return m_LeftMessageSent; }
	bool GetGProxy( )							{ return m_GProxy; }
	bool GetGProxyDisconnectNoticeSent( )		{ return m_GProxyDisconnectNoticeSent; }
	uint32_t GetGProxyReconnectKey( )			{ return m_GProxyReconnectKey; }
	bool GetNoLag( )							{ return m_NoLag; }

	void SetFriendlyName( string nFriendlyName )									{ m_FriendlyName = nFriendlyName; }
	void SetLeftReason( string nLeftReason )										{ m_LeftReason = nLeftReason; }
	void SetSpoofedRealm( string nSpoofedRealm )									{ m_SpoofedRealm = nSpoofedRealm; }
	void SetLeftCode( uint32_t nLeftCode )											{ m_LeftCode = nLeftCode; }
	void SetCookies( uint32_t nCookies )											{ m_Cookies = nCookies; }
	void SetSyncCounter( uint32_t nSyncCounter )									{ m_SyncCounter = nSyncCounter; }
	void SetLastMapPartSent( uint32_t nLastMapPartSent )							{ m_LastMapPartSent = nLastMapPartSent; }
	void SetLastMapPartAcked( uint32_t nLastMapPartAcked )							{ m_LastMapPartAcked = nLastMapPartAcked; }
	void SetStartedDownloadingTicks( uint32_t nStartedDownloadingTicks )			{ m_StartedDownloadingTicks = nStartedDownloadingTicks; }
	void SetFinishedDownloadingTime( uint32_t nFinishedDownloadingTime )			{ m_FinishedDownloadingTime = nFinishedDownloadingTime; }
	void SetStartedLaggingTicks( uint32_t nStartedLaggingTicks )					{ m_StartedLaggingTicks = nStartedLaggingTicks; }
	void SetTotalLaggingTicks( uint32_t nTotalLaggingTicks )						{ m_TotalLaggingTicks = nTotalLaggingTicks; }
	void SetStatsSentTime( uint32_t nStatsSentTime )								{ m_StatsSentTime = nStatsSentTime; }
	void SetStatsDotASentTime( uint32_t nStatsDotASentTime )						{ m_StatsDotASentTime = nStatsDotASentTime; }
	void SetLastGProxyWaitNoticeSentTime( uint32_t nLastGProxyWaitNoticeSentTime )	{ m_LastGProxyWaitNoticeSentTime = nLastGProxyWaitNoticeSentTime; }
	void SetScore( double nScore )													{ m_Score = nScore; }
	void SetSpoofed( bool nSpoofed )												{ m_Spoofed = nSpoofed; }
	void SetReserved( bool nReserved )												{ m_Reserved = nReserved; }
	void SetWhoisShouldBeSent( bool nWhoisShouldBeSent )							{ m_WhoisShouldBeSent = nWhoisShouldBeSent; }
	void SetDownloadAllowed( bool nDownloadAllowed )								{ m_DownloadAllowed = nDownloadAllowed; }
	void SetDownloadStarted( bool nDownloadStarted )								{ m_DownloadStarted = nDownloadStarted; }
	void SetDownloadFinished( bool nDownloadFinished )								{ m_DownloadFinished = nDownloadFinished; }
	void SetLagging( bool nLagging )												{ m_Lagging = nLagging; }
	void SetDropVote( bool nDropVote )												{ m_DropVote = nDropVote; }
	void SetKickVote( bool nKickVote )												{ m_KickVote = nKickVote; }
	void SetForfeitVote( bool nForfeitVote )										{ m_ForfeitVote = nForfeitVote; }
	void SetDrawVote( bool nDrawVote )												{ m_DrawVote = nDrawVote; }
	void SetStartVote( bool nStartVote )											{ m_StartVote = nStartVote; }
	void SetMuted( bool nMuted )													{ m_Muted = nMuted; m_MutedTicks = GetTicks( ); m_MutedAuto = false; }
	void SetAutoban( bool nAutoban )												{ m_Autoban = nAutoban; }
	void SetLeftMessageSent( bool nLeftMessageSent )								{ m_LeftMessageSent = nLeftMessageSent; }
	void SetGProxyDisconnectNoticeSent( bool nGProxyDisconnectNoticeSent )			{ m_GProxyDisconnectNoticeSent = nGProxyDisconnectNoticeSent; }
	void SetNoLag( bool nNoLag )													{ m_NoLag = nNoLag; }

	string GetNameTerminated( );
	uint32_t GetPing( bool LCPing );
	
	bool GetIsIgnoring( string username );
	void Ignore( string username );
	void UnIgnore( string username );

	void AddLoadInGameData( BYTEARRAY nLoadInGameData )								{ m_LoadInGameData.push( nLoadInGameData ); }

	// processing functions

	virtual bool Update( void *fd );
	virtual void ExtractPackets( );
	virtual void ProcessPackets( );

	// other functions

	virtual void Send( BYTEARRAY data );
	virtual void EventGProxyReconnect( CTCPSocket *NewSocket, uint32_t LastPacket );
};

#endif
