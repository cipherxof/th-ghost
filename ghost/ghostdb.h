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

#ifndef GHOSTDB_H
#define GHOSTDB_H

//
// CGHostDB
//

class CBaseCallable;
class CCallableAdminCount;
class CCallableAdminCheck;
class CCallableAdminAdd;
class CCallableAdminRemove;
class CCallableAdminList;
class CCallableBanCount;
class CCallableBanCheck;
class CCallableBanAdd;
class CCallableBanRemove;
class CCallableSpoofList;
class CCallableReconUpdate;
class CCallableCommandList;
class CCallableGameAdd;
class CCallableGameUpdate;
class CCallableGamePlayerAdd;
class CCallableGamePlayerSummaryCheck;
class CCallableDotAGameAdd;
class CCallableDotAPlayerAdd;
class CCallableDotAPlayerSummaryCheck;
class CCallableVampPlayerSummaryCheck;
class CCallableTreePlayerSummaryCheck;
class CCallableShipsPlayerSummaryCheck;
class CCallableSnipePlayerSummaryCheck;
class CCallableW3MMDPlayerSummaryCheck;
class CCallableDownloadAdd;
class CCallableScoreCheck;
class CCallableLeagueCheck;
class CCallableGetTournament;
class CCallableTournamentChat;
class CCallableTournamentUpdate;
class CCallableConnectCheck;
class CCallableW3MMDPlayerAdd;
class CCallableW3MMDVarAdd;
class CDBBan;
class CDBGame;
class CDBGamePlayer;
class CDBGamePlayerSummary;
class CDBDotAPlayerSummary;
class CDBVampPlayerSummary;
class CDBTreePlayerSummary;
class CDBShipsPlayerSummary;
class CDBSnipePlayerSummary;
class CDBW3MMDPlayerSummary;
class CBNET;

typedef pair<uint32_t,string> VarP;

class CGHostDB
{
protected:
	bool m_HasError;
	string m_Error;

public:
	CGHostDB( CConfig *CFG );
	virtual ~CGHostDB( );

	bool HasError( )			{ return m_HasError; }
	string GetError( )			{ return m_Error; }
	virtual string GetStatus( )	{ return "DB STATUS --- OK"; }

	virtual void RecoverCallable( CBaseCallable *callable );

	// standard (non-threaded) database functions

	virtual bool Begin( );
	virtual bool Commit( );
	virtual uint32_t AdminCount( string server );
	virtual bool AdminCheck( string server, string user );
	virtual bool AdminAdd( string server, string user );
	virtual bool AdminRemove( string server, string user );
	virtual vector<string> AdminList( string server );
	virtual uint32_t BanCount( string server );
	virtual CDBBan *BanCheck( string server, string user, string ip, string hostname, string ownername );
	virtual uint32_t BanAdd( string server, string user, string ip, string gamename, string admin, string reason, uint32_t expiretime, string context );
	virtual bool BanRemove( string server, string user, string context );
	virtual bool BanRemove( string user, string context );
	virtual map<string, string> SpoofList( );
	virtual void ReconUpdate( uint32_t hostcounter, uint32_t seconds );
	virtual vector<string> CommandList(  );
	virtual uint32_t GameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver, string savetype );
	virtual uint32_t GameUpdate( uint32_t id, string map, string gamename, string ownername, string creatorname, uint32_t players, string usernames, uint32_t slotsTotal, uint32_t totalGames, uint32_t totalPlayers, bool add );
	virtual uint32_t GamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour, string savetype );
	virtual uint32_t GamePlayerCount( string name );
	virtual CDBGamePlayerSummary *GamePlayerSummaryCheck( string name, string realm );
	virtual CDBVampPlayerSummary *VampPlayerSummaryCheck( string name );
	virtual uint32_t DotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec, string saveType );
	virtual uint32_t DotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills, string saveType );
	virtual uint32_t DotAPlayerCount( string name );
	virtual CDBDotAPlayerSummary *DotAPlayerSummaryCheck( string name, string realm, string saveType );
	virtual CDBTreePlayerSummary *TreePlayerSummaryCheck( string name, string realm );
	virtual CDBShipsPlayerSummary *ShipsPlayerSummaryCheck( string name, string realm );
	virtual CDBSnipePlayerSummary *SnipePlayerSummaryCheck( string name, string realm );
	virtual CDBW3MMDPlayerSummary *W3MMDPlayerSummaryCheck( string name, string realm, string category );
	virtual string FromCheck( uint32_t ip );
	virtual bool FromAdd( uint32_t ip1, uint32_t ip2, string country );
	virtual bool DownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime );
	virtual uint32_t W3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing, string saveType );
	virtual bool W3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints, string saveType );
	virtual bool W3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals, string saveType );
	virtual bool W3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings, string saveType );

	// threaded database functions

	virtual void CreateThread( CBaseCallable *callable );
	virtual CCallableAdminCount *ThreadedAdminCount( string server );
	virtual CCallableAdminCheck *ThreadedAdminCheck( string server, string user );
	virtual CCallableAdminAdd *ThreadedAdminAdd( string server, string user );
	virtual CCallableAdminRemove *ThreadedAdminRemove( string server, string user );
	virtual CCallableAdminList *ThreadedAdminList( string server );
	virtual CCallableBanCount *ThreadedBanCount( string server );
	virtual CCallableBanCheck *ThreadedBanCheck( string server, string user, string ip, string hostname, string ownername );
	virtual CCallableBanAdd *ThreadedBanAdd( string server, string user, string ip, string gamename, string admin, string reason, uint32_t expiretime, string context );
	virtual CCallableBanRemove *ThreadedBanRemove( string server, string user, string context );
	virtual CCallableBanRemove *ThreadedBanRemove( string user, string context );
	virtual CCallableSpoofList *ThreadedSpoofList( );
	virtual CCallableReconUpdate *ThreadedReconUpdate( uint32_t hostcounter, uint32_t seconds );
	virtual CCallableCommandList *ThreadedCommandList( );
	virtual CCallableGameAdd *ThreadedGameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver, string savetype );
	virtual CCallableGameUpdate *ThreadedGameUpdate( uint32_t id, string map, string gamename, string ownername, string creatorname, uint32_t players, string usernames, uint32_t slotsTotal, uint32_t totalGames, uint32_t totalPlayers, bool add );
	virtual CCallableGamePlayerAdd *ThreadedGamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour, string savetype );
	virtual CCallableGamePlayerSummaryCheck *ThreadedGamePlayerSummaryCheck( string name, string realm );
	virtual CCallableVampPlayerSummaryCheck *ThreadedVampPlayerSummaryCheck( string name );
	virtual CCallableDotAGameAdd *ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec, string saveType );
	virtual CCallableDotAPlayerAdd *ThreadedDotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills, string saveType );
	virtual CCallableDotAPlayerSummaryCheck *ThreadedDotAPlayerSummaryCheck( string name, string realm, string saveType );
	virtual CCallableTreePlayerSummaryCheck *ThreadedTreePlayerSummaryCheck( string name, string realm );
	virtual CCallableShipsPlayerSummaryCheck *ThreadedShipsPlayerSummaryCheck( string name, string realm );
	virtual CCallableSnipePlayerSummaryCheck *ThreadedSnipePlayerSummaryCheck( string name, string realm );
	virtual CCallableW3MMDPlayerSummaryCheck *ThreadedW3MMDPlayerSummaryCheck( string name, string realm, string category );
	virtual CCallableDownloadAdd *ThreadedDownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime );
	virtual CCallableScoreCheck *ThreadedScoreCheck( string category, string name, string server );
	virtual CCallableLeagueCheck *ThreadedLeagueCheck( string category, string name, string server, string gamename );
	virtual CCallableGetTournament *ThreadedGetTournament( string gamename );
	virtual CCallableTournamentChat *ThreadedTournamentChat( uint32_t chatid, string message );
	virtual CCallableTournamentUpdate *ThreadedTournamentUpdate( uint32_t matchid, string gamename, uint32_t status );
	virtual CCallableConnectCheck *ThreadedConnectCheck( string name, uint32_t sessionkey );
	virtual CCallableW3MMDPlayerAdd *ThreadedW3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing, string saveType );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints, string saveType );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals, string saveType );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings, string saveType );
};

//
// Callables
//

// life cycle of a callable:
//  - the callable is created in one of the database's ThreadedXXX functions
//  - initially the callable is NOT ready (i.e. m_Ready = false)
//  - the ThreadedXXX function normally creates a thread to perform some query and (potentially) store some result in the callable
//  - at the time of this writing all threads are immediately detached, the code does not join any threads (the callable's "readiness" is used for this purpose instead)
//  - when the thread completes it will set m_Ready = true
//  - DO NOT DO *ANYTHING* TO THE CALLABLE UNTIL IT'S READY OR YOU WILL CREATE A CONCURRENCY MESS
//  - THE ONLY SAFE FUNCTION IN THE CALLABLE IS GetReady
//  - when the callable is ready you may access the callable's result which will have been set within the (now terminated) thread

// example usage:
//  - normally you will call a ThreadedXXX function, store the callable in a vector, and periodically check if the callable is ready
//  - when the callable is ready you will consume the result then you will pass the callable back to the database via the RecoverCallable function
//  - the RecoverCallable function allows the database to recover some of the callable's resources to be reused later (e.g. MySQL connections)
//  - note that this will NOT free the callable's memory, you must do that yourself after calling the RecoverCallable function
//  - be careful not to leak any callables, it's NOT safe to delete a callable even if you decide that you don't want the result anymore
//  - you should deliver any to-be-orphaned callables to the main vector in CGHost so they can be properly deleted when ready even if you don't care about the result anymore
//  - e.g. if a player does a stats check immediately before a game is deleted you can't just delete the callable on game deletion unless it's ready

class CBaseCallable
{
protected:
	string m_Error;
	volatile bool m_Ready;
	uint32_t m_StartTicks;
	uint32_t m_EndTicks;

public:
	CBaseCallable( ) : m_Error( ), m_Ready( false ), m_StartTicks( 0 ), m_EndTicks( 0 ) { }
	virtual ~CBaseCallable( ) { }

	virtual void operator( )( ) { }

	virtual void Init( );
	virtual void Close( );

	virtual string GetError( )				{ return m_Error; }
	virtual bool GetReady( )				{ return m_Ready; }
	virtual void SetReady( bool nReady )	{ m_Ready = nReady; }
	virtual uint32_t GetElapsed( )			{ return m_Ready ? m_EndTicks - m_StartTicks : 0; }
};

class CCallableAdminCount : virtual public CBaseCallable
{
protected:
	string m_Server;
	uint32_t m_Result;

public:
	CCallableAdminCount( string nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( 0 ) { }
	virtual ~CCallableAdminCount( );

	virtual string GetServer( )					{ return m_Server; }
	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableAdminCheck : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	bool m_Result;

public:
	CCallableAdminCheck( string nServer, string nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableAdminCheck( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableAdminAdd : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	bool m_Result;

public:
	CCallableAdminAdd( string nServer, string nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableAdminAdd( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableAdminRemove : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	bool m_Result;

public:
	CCallableAdminRemove( string nServer, string nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableAdminRemove( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableAdminList : virtual public CBaseCallable
{
protected:
	string m_Server;
	vector<string> m_Result;

public:
	CCallableAdminList( string nServer ) : CBaseCallable( ), m_Server( nServer ) { }
	virtual ~CCallableAdminList( );

	virtual vector<string> GetResult( )					{ return m_Result; }
	virtual void SetResult( vector<string> nResult )	{ m_Result = nResult; }
};

class CCallableBanCount : virtual public CBaseCallable
{
protected:
	string m_Server;
	uint32_t m_Result;

public:
	CCallableBanCount( string nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( 0 ) { }
	virtual ~CCallableBanCount( );

	virtual string GetServer( )					{ return m_Server; }
	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableBanCheck : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	string m_IP;
	string m_HostName;
	string m_OwnerName;
	CDBBan *m_Result;

public:
	CCallableBanCheck( string nServer, string nUser, string nIP, string nHostName, string nOwnerName ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_IP( nIP ), m_HostName( nHostName ), m_OwnerName( nOwnerName ), m_Result( NULL ) { }
	virtual ~CCallableBanCheck( );

	virtual string GetServer( )					{ return m_Server; }
	virtual string GetUser( )					{ return m_User; }
	virtual string GetIP( )						{ return m_IP; }
	virtual string GetHostName( )				{ return m_HostName; }
	virtual string GetOwnerName( )				{ return m_OwnerName; }
	virtual CDBBan *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBBan *nResult )	{ m_Result = nResult; }
};

class CCallableBanAdd : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	string m_IP;
	string m_GameName;
	string m_Admin;
	string m_Reason;
	uint32_t m_ExpireTime;
	string m_Context;
	uint32_t m_Result;

public:
	CCallableBanAdd( string nServer, string nUser, string nIP, string nGameName, string nAdmin, string nReason, uint32_t nExpireTime, string nContext ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_IP( nIP ), m_GameName( nGameName ), m_Admin( nAdmin ), m_Reason( nReason ), m_ExpireTime( nExpireTime ), m_Context( nContext ), m_Result( false ) { }
	virtual ~CCallableBanAdd( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual string GetIP( )					{ return m_IP; }
	virtual string GetGameName( )			{ return m_GameName; }
	virtual string GetAdmin( )				{ return m_Admin; }
	virtual string GetReason( )				{ return m_Reason; }
	virtual uint32_t GetExpireTime( )		{ return m_ExpireTime; }
	virtual string GetContext( )			{ return m_Context; }
	virtual uint32_t GetResult( )			{ return m_Result; }
	virtual void SetResult( uint32_t nResult ) { m_Result = nResult; }
};

class CCallableBanRemove : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_User;
	string m_Context;
	bool m_Result;

public:
	CCallableBanRemove( string nServer, string nUser, string nContext ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Context( nContext ), m_Result( false ) { }
	virtual ~CCallableBanRemove( );

	virtual string GetServer( )				{ return m_Server; }
	virtual string GetUser( )				{ return m_User; }
	virtual string GetContext( )			{ return m_Context; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableSpoofList : virtual public CBaseCallable
{
protected:
	map<string, string> m_Result;

public:
	CCallableSpoofList( ) : CBaseCallable( ) { }
	virtual ~CCallableSpoofList( );

	virtual map<string, string> GetResult( )				{ return m_Result; }
	virtual void SetResult( map<string, string> nResult )	{ m_Result = nResult; }
};

class CCallableCommandList : virtual public CBaseCallable
{
protected:
	vector<string> m_Result;

public:
	CCallableCommandList( ) : CBaseCallable( ) { }
	virtual ~CCallableCommandList( );

	virtual vector<string> GetResult( )				{ return m_Result; }
	virtual void SetResult( vector<string> nResult )	{ m_Result = nResult; }
};

class CCallableReconUpdate : virtual public CBaseCallable
{
protected:
	uint32_t m_HostCounter;
	uint32_t m_Seconds;

public:
	CCallableReconUpdate( uint32_t nHostCounter, uint32_t nSeconds ) : CBaseCallable( ), m_HostCounter( nHostCounter ), m_Seconds( nSeconds ) { }
	virtual ~CCallableReconUpdate( );
};

class CCallableGameAdd : virtual public CBaseCallable
{
protected:
	string m_Server;
	string m_Map;
	string m_GameName;
	string m_OwnerName;
	uint32_t m_Duration;
	uint32_t m_GameState;
	string m_CreatorName;
	string m_CreatorServer;
	string m_SaveType;
	uint32_t m_Result;

public:
	CCallableGameAdd( string nServer, string nMap, string nGameName, string nOwnerName, uint32_t nDuration, uint32_t nGameState, string nCreatorName, string nCreatorServer, string nSaveType ) : CBaseCallable( ), m_Server( nServer ), m_Map( nMap ), m_GameName( nGameName ), m_OwnerName( nOwnerName ), m_Duration( nDuration ), m_GameState( nGameState ), m_CreatorName( nCreatorName ), m_CreatorServer( nCreatorServer ), m_SaveType( nSaveType ), m_Result( 0 ) { }
	virtual ~CCallableGameAdd( );

	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableGameUpdate : virtual public CBaseCallable
{
protected:
	uint32_t m_ID;
    string m_Map;
    string m_GameName;
    string m_OwnerName;
    string m_CreatorName;
    bool m_Add;
    uint32_t m_Players;
    string m_Usernames;
    uint32_t m_SlotsTotal;
    uint32_t m_TotalGames;
    uint32_t m_TotalPlayers;
    uint32_t m_Result;
public:
 CCallableGameUpdate( uint32_t id, string map, string gamename, string ownername, string creatorname, uint32_t players, string usernames, uint32_t slotsTotal, uint32_t totalGames, uint32_t totalPlayers, bool add ) : CBaseCallable( ), m_ID( id ), m_Map(map), m_GameName(gamename), m_OwnerName(ownername), m_CreatorName(creatorname), m_Add(add), m_Players(players), m_Usernames(usernames), m_SlotsTotal(slotsTotal), m_TotalGames(totalGames), m_TotalPlayers(totalPlayers) { }
	virtual ~CCallableGameUpdate( );

	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableGamePlayerAdd : virtual public CBaseCallable
{
protected:
	uint32_t m_GameID;
	string m_Name;
	string m_IP;
	uint32_t m_Spoofed;
	string m_SpoofedRealm;
	uint32_t m_Reserved;
	uint32_t m_LoadingTime;
	uint32_t m_Left;
	string m_LeftReason;
	uint32_t m_Team;
	uint32_t m_Colour;
	string m_SaveType;
	uint32_t m_Result;

public:
	CCallableGamePlayerAdd( uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour, string nSaveType ) : CBaseCallable( ), m_GameID( nGameID ), m_Name( nName ), m_IP( nIP ), m_Spoofed( nSpoofed ), m_SpoofedRealm( nSpoofedRealm ), m_Reserved( nReserved ), m_LoadingTime( nLoadingTime ), m_Left( nLeft ), m_LeftReason( nLeftReason ), m_Team( nTeam ), m_Colour( nColour ), m_SaveType( nSaveType ), m_Result( 0 ) { }
	virtual ~CCallableGamePlayerAdd( );

	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableGamePlayerSummaryCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	string m_Realm;
	CDBGamePlayerSummary *m_Result;

public:
	CCallableGamePlayerSummaryCheck( string nName, string nRealm ) : CBaseCallable( ), m_Name( nName ), m_Realm( nRealm ), m_Result( NULL ) { }
	virtual ~CCallableGamePlayerSummaryCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual string GetRealm( )								{ return m_Realm; }
	virtual CDBGamePlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBGamePlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableVampPlayerSummaryCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	CDBVampPlayerSummary *m_Result;

public:
	CCallableVampPlayerSummaryCheck( string nName ) : CBaseCallable( ), m_Name( nName ), m_Result( NULL ) { }
	virtual ~CCallableVampPlayerSummaryCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual CDBVampPlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBVampPlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableDotAGameAdd : virtual public CBaseCallable
{
protected:
	uint32_t m_GameID;
	uint32_t m_Winner;
	uint32_t m_Min;
	uint32_t m_Sec;
	uint32_t m_Result;
	string m_SaveType;

public:
	CCallableDotAGameAdd( uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec, string nSaveType ) : CBaseCallable( ), m_GameID( nGameID ), m_Winner( nWinner ), m_Min( nMin ), m_Sec( nSec ), m_SaveType( nSaveType ), m_Result( 0 ) { }
	virtual ~CCallableDotAGameAdd( );

	virtual string GetSaveType( )				{ return m_SaveType; }
	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableDotAPlayerAdd : virtual public CBaseCallable
{
protected:
	uint32_t m_GameID;
	uint32_t m_Colour;
	uint32_t m_Kills;
	uint32_t m_Deaths;
	uint32_t m_CreepKills;
	uint32_t m_CreepDenies;
	uint32_t m_Assists;
	uint32_t m_Gold;
	uint32_t m_NeutralKills;
	string m_Item1;
	string m_Item2;
	string m_Item3;
	string m_Item4;
	string m_Item5;
	string m_Item6;
	string m_Hero;
	uint32_t m_NewColour;
	uint32_t m_TowerKills;
	uint32_t m_RaxKills;
	uint32_t m_CourierKills;
	uint32_t m_Result;
	string m_SaveType;

public:
	CCallableDotAPlayerAdd( uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, string nItem1, string nItem2, string nItem3, string nItem4, string nItem5, string nItem6, string nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills, string nSaveType ) : CBaseCallable( ), m_GameID( nGameID ), m_Colour( nColour ), m_Kills( nKills ), m_Deaths( nDeaths ), m_CreepKills( nCreepKills ), m_CreepDenies( nCreepDenies ), m_Assists( nAssists ), m_Gold( nGold ), m_NeutralKills( nNeutralKills ), m_Item1( nItem1 ), m_Item2( nItem2 ), m_Item3( nItem3 ), m_Item4( nItem4 ), m_Item5( nItem5 ), m_Item6( nItem6 ), m_Hero( nHero ), m_NewColour( nNewColour ), m_TowerKills( nTowerKills ), m_RaxKills( nRaxKills ), m_CourierKills( nCourierKills ), m_SaveType( nSaveType ), m_Result( 0 ) { }
	virtual ~CCallableDotAPlayerAdd( );

	virtual string GetSaveType( )				{ return m_SaveType; }
	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableDotAPlayerSummaryCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	string m_Realm;
	CDBDotAPlayerSummary *m_Result;
	string m_SaveType;

public:
	CCallableDotAPlayerSummaryCheck( string nName, string nRealm, string nSaveType ) : CBaseCallable( ), m_Name( nName ), m_Realm( nRealm ), m_SaveType( nSaveType ), m_Result( NULL ) { }
	virtual ~CCallableDotAPlayerSummaryCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual string GetRealm( )								{ return m_Realm; }
	virtual string GetSaveType( )							{ return m_SaveType; }
	virtual CDBDotAPlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBDotAPlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableTreePlayerSummaryCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	string m_Realm;
	CDBTreePlayerSummary *m_Result;

public:
	CCallableTreePlayerSummaryCheck( string nName, string nRealm ) : CBaseCallable( ), m_Name( nName ), m_Realm( nRealm ), m_Result( NULL ) { }
	virtual ~CCallableTreePlayerSummaryCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual string GetRealm( )								{ return m_Realm; }
	virtual CDBTreePlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBTreePlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableShipsPlayerSummaryCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	string m_Realm;
	CDBShipsPlayerSummary *m_Result;

public:
	CCallableShipsPlayerSummaryCheck( string nName, string nRealm ) : CBaseCallable( ), m_Name( nName ), m_Realm( nRealm ), m_Result( NULL ) { }
	virtual ~CCallableShipsPlayerSummaryCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual string GetRealm( )								{ return m_Realm; }
	virtual CDBShipsPlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBShipsPlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableSnipePlayerSummaryCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	string m_Realm;
	CDBSnipePlayerSummary *m_Result;

public:
	CCallableSnipePlayerSummaryCheck( string nName, string nRealm ) : CBaseCallable( ), m_Name( nName ), m_Realm( nRealm ), m_Result( NULL ) { }
	virtual ~CCallableSnipePlayerSummaryCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual string GetRealm( )								{ return m_Realm; }
	virtual CDBSnipePlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBSnipePlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableW3MMDPlayerSummaryCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	string m_Realm;
	string m_Category;
	CDBW3MMDPlayerSummary *m_Result;

public:
	CCallableW3MMDPlayerSummaryCheck( string nName, string nRealm, string nCategory ) : CBaseCallable( ), m_Name( nName ), m_Realm( nRealm ), m_Category( nCategory ), m_Result( NULL ) { }
	virtual ~CCallableW3MMDPlayerSummaryCheck( );

	virtual string GetName( )								{ return m_Name; }
	virtual string GetRealm( )								{ return m_Realm; }
	virtual string GetCategory( )							{ return m_Category; }
	virtual CDBW3MMDPlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBW3MMDPlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableDownloadAdd : virtual public CBaseCallable
{
protected:
	string m_Map;
	uint32_t m_MapSize;
	string m_Name;
	string m_IP;
	uint32_t m_Spoofed;
	string m_SpoofedRealm;
	uint32_t m_DownloadTime;
	bool m_Result;

public:
	CCallableDownloadAdd( string nMap, uint32_t nMapSize, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nDownloadTime ) : CBaseCallable( ), m_Map( nMap ), m_MapSize( nMapSize ), m_Name( nName ), m_IP( nIP ), m_Spoofed( nSpoofed ), m_SpoofedRealm( nSpoofedRealm ), m_DownloadTime( nDownloadTime ), m_Result( false ) { }
	virtual ~CCallableDownloadAdd( );

	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableScoreCheck : virtual public CBaseCallable
{
protected:
	string m_Category;
	string m_Name;
	string m_Server;
	double *m_Result;

public:
	CCallableScoreCheck( string nCategory, string nName, string nServer ) : CBaseCallable( ), m_Category( nCategory ), m_Name( nName ), m_Server( nServer ), m_Result( NULL ) { }
	virtual ~CCallableScoreCheck( );

	virtual string GetName( )					{ return m_Name; }
	virtual double *GetResult( )					{ return m_Result; }
	virtual void SetResult( double *nResult )	{ m_Result = nResult; }
};

class CCallableLeagueCheck : virtual public CBaseCallable
{
protected:
	string m_Category;
	string m_Name;
	string m_Server;
	string m_GameName;
	uint32_t m_Result;

public:
	CCallableLeagueCheck( string nCategory, string nName, string nServer, string nGameName ) : CBaseCallable( ), m_Category( nCategory ), m_Name( nName ), m_Server( nServer ), m_GameName( nGameName ), m_Result( 255 ) { }
	virtual ~CCallableLeagueCheck( );

	virtual string GetName( )					{ return m_Name; }
	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableGetTournament : virtual public CBaseCallable
{
protected:
	string m_GameName;
	vector<string> m_Result;

public:
	CCallableGetTournament( string nGameName ) : CBaseCallable( ), m_GameName( nGameName ) { }
	virtual ~CCallableGetTournament( );

	virtual vector<string> GetResult( )			{ return m_Result; }
	virtual void SetResult( vector<string> nResult ) { m_Result = nResult; }
};

class CCallableTournamentChat : virtual public CBaseCallable
{
protected:
	uint32_t m_ChatID;
	string m_Message;

public:
	CCallableTournamentChat( uint32_t nChatID, string nMessage ) : CBaseCallable( ), m_ChatID( nChatID ), m_Message( nMessage ) { }
	virtual ~CCallableTournamentChat( );
};

class CCallableTournamentUpdate : virtual public CBaseCallable
{
protected:
	uint32_t m_MatchID;
	string m_GameName;
	uint32_t m_Status;

public:
	CCallableTournamentUpdate( uint32_t nMatchID, string nGameName, uint32_t nStatus ) : CBaseCallable( ), m_MatchID( nMatchID ), m_GameName( nGameName), m_Status( nStatus ) { }
	virtual ~CCallableTournamentUpdate( );
};

class CCallableConnectCheck : virtual public CBaseCallable
{
protected:
	string m_Name;
	uint32_t m_SessionKey;
	bool m_Result;

public:
	CCallableConnectCheck( string nName, uint32_t nSessionKey ) : CBaseCallable( ), m_Name( nName ), m_SessionKey( nSessionKey ), m_Result( false ) { }
	virtual ~CCallableConnectCheck( );

	virtual string GetName( )					{ return m_Name; }
	virtual bool GetResult( )					{ return m_Result; }
	virtual void SetResult( bool nResult )		{ m_Result = nResult; }
};

class CCallableW3MMDPlayerAdd : virtual public CBaseCallable
{
protected:
	string m_Category;
	uint32_t m_GameID;
	uint32_t m_PID;
	string m_Name;
	string m_Flag;
	uint32_t m_Leaver;
	uint32_t m_Practicing;
	string m_SaveType;
	uint32_t m_Result;

public:
	CCallableW3MMDPlayerAdd( string nCategory, uint32_t nGameID, uint32_t nPID, string nName, string nFlag, uint32_t nLeaver, uint32_t nPracticing, string nSaveType ) : CBaseCallable( ), m_Category( nCategory ), m_GameID( nGameID ), m_PID( nPID ), m_Name( nName ), m_Flag( nFlag ), m_Leaver( nLeaver ), m_Practicing( nPracticing ), m_SaveType( nSaveType ), m_Result( 0 ) { }
	virtual ~CCallableW3MMDPlayerAdd( );

	virtual uint32_t GetResult( )				{ return m_Result; }
	virtual void SetResult( uint32_t nResult )	{ m_Result = nResult; }
};

class CCallableW3MMDVarAdd : virtual public CBaseCallable
{
protected:
	uint32_t m_GameID;
	map<VarP,int32_t> m_VarInts;
	map<VarP,double> m_VarReals;
	map<VarP,string> m_VarStrings;
	string m_SaveType;

	enum ValueType {
		VALUETYPE_INT = 1,
		VALUETYPE_REAL = 2,
		VALUETYPE_STRING = 3
	};

	ValueType m_ValueType;
	bool m_Result;

public:
	CCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,int32_t> nVarInts, string nSaveType ) : CBaseCallable( ), m_GameID( nGameID ), m_VarInts( nVarInts ), m_ValueType( VALUETYPE_INT ), m_SaveType( nSaveType ), m_Result( false ) { }
	CCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,double> nVarReals, string nSaveType ) : CBaseCallable( ), m_GameID( nGameID ), m_VarReals( nVarReals ), m_ValueType( VALUETYPE_REAL ), m_SaveType( nSaveType ), m_Result( false ) { }
	CCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,string> nVarStrings, string nSaveType ) : CBaseCallable( ), m_GameID( nGameID ), m_VarStrings( nVarStrings ), m_ValueType( VALUETYPE_STRING ), m_SaveType( nSaveType ), m_Result( false ) { }
	virtual ~CCallableW3MMDVarAdd( );

	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

//
// CDBBan
//

class CDBBan
{
private:
	uint32_t m_Id;
	string m_Server;
	string m_Name;
	string m_IP;
	string m_Date;
	string m_GameName;
	string m_Admin;
	string m_Reason;
	string m_ExpireDate;
	string m_Context;
	bool m_Delete; //whether this represents a ban that should be deleted from the ban list (ban list fast only)
	uint32_t m_CacheTime; //0 unless ban list fast

public:
	CDBBan( uint32_t nId ); //sets delete = true
	CDBBan( uint32_t nId, string nServer, string nName, string nIP, string nDate, string nGameName, string nAdmin, string nReason, string nExpireDate, string nContext, uint32_t nCacheTime ); //sets delete = false
	CDBBan( CDBBan *copy );
	~CDBBan( );

	uint32_t GetId( )		{ return m_Id; }
	string GetServer( )		{ return m_Server; }
	string GetName( )		{ return m_Name; }
	string GetIP( )			{ return m_IP; }
	string GetDate( )		{ return m_Date; }
	string GetGameName( )	{ return m_GameName; }
	string GetAdmin( )		{ return m_Admin; }
	string GetReason( )		{ return m_Reason; }
	string GetExpireDate( ) { return m_ExpireDate; }
	string GetContext( )	{ return m_Context; }
	bool GetDelete( )		{ return m_Delete; }
	uint32_t GetCacheTime( ){ return m_CacheTime; }
};

//
// CDBGame
//

class CDBGame
{
private:
	uint32_t m_ID;
	string m_Server;
	string m_Map;
	string m_DateTime;
	string m_GameName;
	string m_OwnerName;
	uint32_t m_Duration;

public:
	CDBGame( uint32_t nID, string nServer, string nMap, string nDateTime, string nGameName, string nOwnerName, uint32_t nDuration );
	~CDBGame( );

	uint32_t GetID( )		{ return m_ID; }
	string GetServer( )		{ return m_Server; }
	string GetMap( )		{ return m_Map; }
	string GetDateTime( )	{ return m_DateTime; }
	string GetGameName( )	{ return m_GameName; }
	string GetOwnerName( )	{ return m_OwnerName; }
	uint32_t GetDuration( )	{ return m_Duration; }

	void SetDuration( uint32_t nDuration )	{ m_Duration = nDuration; }
};

//
// CDBGamePlayer
//

class CDBGamePlayer
{
private:
	uint32_t m_ID;
	uint32_t m_GameID;
	string m_Name;
	string m_IP;
	uint32_t m_Spoofed;
	string m_SpoofedRealm;
	uint32_t m_Reserved;
	uint32_t m_LoadingTime;
	uint32_t m_Left;
	string m_LeftReason;
	uint32_t m_Team;
	uint32_t m_Colour;

public:
	CDBGamePlayer( uint32_t nID, uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour );
	~CDBGamePlayer( );

	uint32_t GetID( )			{ return m_ID; }
	uint32_t GetGameID( )		{ return m_GameID; }
	string GetName( )			{ return m_Name; }
	string GetIP( )				{ return m_IP; }
	uint32_t GetSpoofed( )		{ return m_Spoofed; }
	string GetSpoofedRealm( )	{ return m_SpoofedRealm; }
	uint32_t GetReserved( )		{ return m_Reserved; }
	uint32_t GetLoadingTime( )	{ return m_LoadingTime; }
	uint32_t GetLeft( )			{ return m_Left; }
	string GetLeftReason( )		{ return m_LeftReason; }
	uint32_t GetTeam( )			{ return m_Team; }
	uint32_t GetColour( )		{ return m_Colour; }

	void SetLoadingTime( uint32_t nLoadingTime )	{ m_LoadingTime = nLoadingTime; }
	void SetLeft( uint32_t nLeft )					{ m_Left = nLeft; }
	void SetLeftReason( string nLeftReason )		{ m_LeftReason = nLeftReason; }
};

//
// CDBGamePlayerSummary
//

class CDBGamePlayerSummary
{
private:
	string m_Server;
	string m_Name;
	uint32_t m_TotalGames;			// total number of games played
	double m_LeftPercent;
	uint32_t m_PlayingTime;

public:
	CDBGamePlayerSummary( string nServer, string nName, uint32_t nTotalGames, double nLeftPercent, uint32_t nPlayingTime );
	~CDBGamePlayerSummary( );

	string GetServer( )					{ return m_Server; }
	string GetName( )					{ return m_Name; }
	uint32_t GetTotalGames( )			{ return m_TotalGames; }
	double GetLeftPercent( )			{ return m_LeftPercent; }
	uint32_t GetPlayingTime( )			{ return m_PlayingTime; }
};

//
// CDBVampPlayerSummary
//

class CDBVampPlayerSummary
{
private:
	string m_Server;
	string m_Name;
	uint32_t m_TotalGames;
	uint32_t m_TotalHumanGames;
	uint32_t m_TotalVampGames;
	uint32_t m_TotalHumanWins;
	uint32_t m_TotalVampWins;
	uint32_t m_TotalHumanLosses;
	uint32_t m_TotalVampLosses;
	uint32_t m_TotalVampKills;
	double m_MinCommandCenter;
	double m_AvgCommandCenter;
	double m_MinBase;
	double m_AvgBase;

public:
	CDBVampPlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalHumanGames, uint32_t nTotalVampGames, uint32_t nTotalHumanWins, uint32_t nTotalVampWins, uint32_t nTotalHumanLosses, uint32_t nTotalVampLosses, uint32_t nTotalVampKills, double nMinCommandCenter, double nAvgCommandCenter, double nMinBase, double nAvgBase );
	~CDBVampPlayerSummary( );

	string GetServer( )					{ return m_Server; }
	string GetName( )					{ return m_Name; }
	uint32_t GetTotalGames( )			{ return m_TotalGames; }
	uint32_t GetTotalHumanGames( )		{ return m_TotalHumanGames; }
	uint32_t GetTotalVampGames( )		{ return m_TotalVampGames; }
	uint32_t GetTotalHumanWins( )		{ return m_TotalHumanWins; }
	uint32_t GetTotalVampWins( )		{ return m_TotalVampWins; }
	uint32_t GetTotalHumanLosses( )		{ return m_TotalHumanLosses; }
	uint32_t GetTotalVampLosses( )		{ return m_TotalVampLosses; }
	uint32_t GetTotalVampKills( )		{ return m_TotalVampKills; }
	double GetMinCommandCenter( )		{ return m_MinCommandCenter; }
	double GetAvgCommandCenter( )		{ return m_AvgCommandCenter; }
	double GetMinBase( )				{ return m_MinBase; }
	double GetAvgBase( )				{ return m_AvgBase; }
};

//
// CDBDotAGame
//

class CDBDotAGame
{
private:
	uint32_t m_ID;
	uint32_t m_GameID;
	uint32_t m_Winner;
	uint32_t m_Min;
	uint32_t m_Sec;

public:
	CDBDotAGame( uint32_t nID, uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec );
	~CDBDotAGame( );

	uint32_t GetID( )		{ return m_ID; }
	uint32_t GetGameID( )	{ return m_GameID; }
	uint32_t GetWinner( )	{ return m_Winner; }
	uint32_t GetMin( )		{ return m_Min; }
	uint32_t GetSec( )		{ return m_Sec; }
};

//
// CDBDotAPlayer
//

class CDBDotAPlayer
{
private:
	uint32_t m_ID;
	uint32_t m_GameID;
	uint32_t m_Colour;
	uint32_t m_Kills;
	uint32_t m_Deaths;
	uint32_t m_CreepKills;
	uint32_t m_CreepDenies;
	uint32_t m_Assists;
	uint32_t m_Gold;
	uint32_t m_NeutralKills;
	string m_Items[6];
	string m_Hero;
	uint32_t m_NewColour;
	uint32_t m_TowerKills;
	uint32_t m_RaxKills;
	uint32_t m_CourierKills;

public:
	CDBDotAPlayer( );
	CDBDotAPlayer( uint32_t nID, uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, string nItem1, string nItem2, string nItem3, string nItem4, string nItem5, string nItem6, string nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills );
	~CDBDotAPlayer( );

	uint32_t GetID( )			{ return m_ID; }
	uint32_t GetGameID( )		{ return m_GameID; }
	uint32_t GetColour( )		{ return m_Colour; }
	uint32_t GetKills( )		{ return m_Kills; }
	uint32_t GetDeaths( )		{ return m_Deaths; }
	uint32_t GetCreepKills( )	{ return m_CreepKills; }
	uint32_t GetCreepDenies( )	{ return m_CreepDenies; }
	uint32_t GetAssists( )		{ return m_Assists; }
	uint32_t GetGold( )			{ return m_Gold; }
	uint32_t GetNeutralKills( )	{ return m_NeutralKills; }
	string GetItem( unsigned int i );
	string GetHero( )			{ return m_Hero; }
	uint32_t GetNewColour( )	{ return m_NewColour; }
	uint32_t GetTowerKills( )	{ return m_TowerKills; }
	uint32_t GetRaxKills( )		{ return m_RaxKills; }
	uint32_t GetCourierKills( )	{ return m_CourierKills; }

	void SetColour( uint32_t nColour )				{ m_Colour = nColour; }
	void SetKills( uint32_t nKills )				{ m_Kills = nKills; }
	void SetDeaths( uint32_t nDeaths )				{ m_Deaths = nDeaths; }
	void SetCreepKills( uint32_t nCreepKills )		{ m_CreepKills = nCreepKills; }
	void SetCreepDenies( uint32_t nCreepDenies )	{ m_CreepDenies = nCreepDenies; }
	void SetAssists( uint32_t nAssists )			{ m_Assists = nAssists; }
	void SetGold( uint32_t nGold )					{ m_Gold = nGold; }
	void SetNeutralKills( uint32_t nNeutralKills )	{ m_NeutralKills = nNeutralKills; }
	void SetItem( unsigned int i, string item );
	void SetHero( string nHero )					{ m_Hero = nHero; }
	void SetNewColour( uint32_t nNewColour )		{ m_NewColour = nNewColour; }
	void SetTowerKills( uint32_t nTowerKills )		{ m_TowerKills = nTowerKills; }
	void SetRaxKills( uint32_t nRaxKills )			{ m_RaxKills = nRaxKills; }
	void SetCourierKills( uint32_t nCourierKills )	{ m_CourierKills = nCourierKills; }
};

//
// CDBDotAPlayerSummary
//

class CDBDotAPlayerSummary
{
private:
	string m_Server;
	string m_Name;
	uint32_t m_TotalGames;			// total number of dota games played
	uint32_t m_TotalWins;			// total number of dota games won
	uint32_t m_TotalLosses;			// total number of dota games lost
	uint32_t m_TotalKills;			// total number of hero kills
	uint32_t m_TotalDeaths;			// total number of deaths
	uint32_t m_TotalCreepKills;		// total number of creep kills
	uint32_t m_TotalCreepDenies;	// total number of creep denies
	uint32_t m_TotalAssists;		// total number of assists
	uint32_t m_TotalNeutralKills;	// total number of neutral kills
	uint32_t m_TotalTowerKills;		// total number of tower kills
	uint32_t m_TotalRaxKills;		// total number of rax kills
	uint32_t m_TotalCourierKills;	// total number of courier kills
	double m_Score;

public:
	CDBDotAPlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, uint32_t nTotalCreepKills, uint32_t nTotalCreepDenies, uint32_t nTotalAssists, uint32_t nTotalNeutralKills, uint32_t nTotalTowerKills, uint32_t nTotalRaxKills, uint32_t nTotalCourierKills, double nScore );
	~CDBDotAPlayerSummary( );

	string GetServer( )					{ return m_Server; }
	string GetName( )					{ return m_Name; }
	uint32_t GetTotalGames( )			{ return m_TotalGames; }
	uint32_t GetTotalWins( )			{ return m_TotalWins; }
	uint32_t GetTotalLosses( )			{ return m_TotalLosses; }
	uint32_t GetTotalKills( )			{ return m_TotalKills; }
	uint32_t GetTotalDeaths( )			{ return m_TotalDeaths; }
	uint32_t GetTotalCreepKills( )		{ return m_TotalCreepKills; }
	uint32_t GetTotalCreepDenies( )		{ return m_TotalCreepDenies; }
	uint32_t GetTotalAssists( )			{ return m_TotalAssists; }
	uint32_t GetTotalNeutralKills( )	{ return m_TotalNeutralKills; }
	uint32_t GetTotalTowerKills( )		{ return m_TotalTowerKills; }
	uint32_t GetTotalRaxKills( )		{ return m_TotalRaxKills; }
	uint32_t GetTotalCourierKills( )	{ return m_TotalCourierKills; }
	double GetScore( )					{ return m_Score; }

	float GetAvgKills( )				{ return m_TotalGames > 0 ? (float)m_TotalKills / m_TotalGames : 0; }
	float GetAvgDeaths( )				{ return m_TotalGames > 0 ? (float)m_TotalDeaths / m_TotalGames : 0; }
	float GetAvgCreepKills( )			{ return m_TotalGames > 0 ? (float)m_TotalCreepKills / m_TotalGames : 0; }
	float GetAvgCreepDenies( )			{ return m_TotalGames > 0 ? (float)m_TotalCreepDenies / m_TotalGames : 0; }
	float GetAvgAssists( )				{ return m_TotalGames > 0 ? (float)m_TotalAssists / m_TotalGames : 0; }
	float GetAvgNeutralKills( )			{ return m_TotalGames > 0 ? (float)m_TotalNeutralKills / m_TotalGames : 0; }
	float GetAvgTowerKills( )			{ return m_TotalGames > 0 ? (float)m_TotalTowerKills / m_TotalGames : 0; }
	float GetAvgRaxKills( )				{ return m_TotalGames > 0 ? (float)m_TotalRaxKills / m_TotalGames : 0; }
	float GetAvgCourierKills( )			{ return m_TotalGames > 0 ? (float)m_TotalCourierKills / m_TotalGames : 0; }
};

//
// CDBTreePlayerSummary
//

class CDBTreePlayerSummary
{
private:
	string m_Server;
	string m_Name;
	uint32_t m_TotalGames;			// total number of dota games played
	uint32_t m_TotalWins;			// total number of dota games won
	uint32_t m_TotalLosses;			// total number of dota games lost
	uint32_t m_TotalKills;			// total number of hero kills
	uint32_t m_TotalTKs;			// total number of team kills
	uint32_t m_TotalDeaths;			// total number of deaths
	uint32_t m_TotalSaves;		// total number of saves
    uint32_t m_TotalEntGames;       //total number of ent games
    uint32_t m_TotalInfernalGames;  //total number of infernal games
    double m_Score;

public:
	CDBTreePlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalTKs, uint32_t nTotalDeaths, uint32_t nTotalSaves, uint32_t nTotalEntGames, uint32_t nTotalInfernalGames, double nScore );
	~CDBTreePlayerSummary( );

	string GetServer( )					{ return m_Server; }
	string GetName( )					{ return m_Name; }
	uint32_t GetTotalGames( )			{ return m_TotalGames; }
	uint32_t GetTotalWins( )			{ return m_TotalWins; }
	uint32_t GetTotalLosses( )			{ return m_TotalLosses; }
	uint32_t GetTotalKills( )			{ return m_TotalKills; }
	uint32_t GetTotalTKs( )				{ return m_TotalTKs; }
	uint32_t GetTotalDeaths( )			{ return m_TotalDeaths; }
	uint32_t GetTotalSaves( )			{ return m_TotalSaves; }
    uint32_t GetTotalEntGames( )    	{ return m_TotalEntGames; }
    uint32_t GetTotalInfernalGames() 	{return m_TotalInfernalGames; }
    double GetScore( )					{ return m_Score; }

	float GetAvgKills( )				{ return m_TotalInfernalGames > 0 ? (float)m_TotalKills / m_TotalInfernalGames : 0; }
	float GetAvgTKs( )					{ return m_TotalEntGames > 0 ? (float)m_TotalTKs / m_TotalEntGames : 0; }
	float GetAvgDeaths( )				{ return m_TotalEntGames > 0 ? (float)m_TotalDeaths / m_TotalEntGames : 0; }
	float GetAvgSaves( )				{ return m_TotalEntGames > 0 ? (float)m_TotalSaves / m_TotalEntGames : 0; }
};

//
// CDBShipsPlayerSummary
//

class CDBShipsPlayerSummary
{
private:
	string m_Server;
	string m_Name;
	uint32_t m_TotalGames;			// total number of dota games played
	uint32_t m_TotalWins;			// total number of dota games won
	uint32_t m_TotalLosses;			// total number of dota games lost
	uint32_t m_TotalKills;			// total number of hero kills
	uint32_t m_TotalDeaths;			// total number of deaths
	double m_Score;

public:
	CDBShipsPlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, double nScore );
	~CDBShipsPlayerSummary( );

	string GetServer( )					{ return m_Server; }
	string GetName( )					{ return m_Name; }
	uint32_t GetTotalGames( )			{ return m_TotalGames; }
	uint32_t GetTotalWins( )			{ return m_TotalWins; }
	uint32_t GetTotalLosses( )			{ return m_TotalLosses; }
	uint32_t GetTotalKills( )			{ return m_TotalKills; }
	uint32_t GetTotalDeaths( )			{ return m_TotalDeaths; }
	double GetScore( )					{ return m_Score; }

	float GetAvgKills( )				{ return m_TotalGames > 0 ? (float)m_TotalKills / m_TotalGames : 0; }
	float GetAvgDeaths( )				{ return m_TotalGames > 0 ? (float)m_TotalDeaths / m_TotalGames : 0; }
};

//
// CDBSnipePlayerSummary
//

class CDBSnipePlayerSummary
{
private:
	string m_Server;
	string m_Name;
	uint32_t m_TotalGames;			// total number of dota games played
	uint32_t m_TotalWins;			// total number of dota games won
	uint32_t m_TotalLosses;			// total number of dota games lost
	uint32_t m_TotalKills;			// total number of hero kills
	uint32_t m_TotalDeaths;			// total number of deaths
	double m_Score;

public:
	CDBSnipePlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, double nScore );
	~CDBSnipePlayerSummary( );

	string GetServer( )					{ return m_Server; }
	string GetName( )					{ return m_Name; }
	uint32_t GetTotalGames( )			{ return m_TotalGames; }
	uint32_t GetTotalWins( )			{ return m_TotalWins; }
	uint32_t GetTotalLosses( )			{ return m_TotalLosses; }
	uint32_t GetTotalKills( )			{ return m_TotalKills; }
	uint32_t GetTotalDeaths( )			{ return m_TotalDeaths; }
	double GetScore( )					{ return m_Score; }

	float GetAvgKills( )				{ return m_TotalGames > 0 ? (float)m_TotalKills / m_TotalGames : 0; }
	float GetAvgDeaths( )				{ return m_TotalGames > 0 ? (float)m_TotalDeaths / m_TotalGames : 0; }
};

//
// CDBW3MMDlayerSummary
//

class CDBW3MMDPlayerSummary
{
private:
	string m_Server;
	string m_Name;
	string m_Category;
	uint32_t m_TotalGames;			// total number of dota games played
	uint32_t m_TotalWins;			// total number of dota games won
	uint32_t m_TotalLosses;			// total number of dota games lost
	double m_Score;
	uint32_t m_Rank;

public:
	CDBW3MMDPlayerSummary( string nServer, string nName, string nCategory, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, double nScore, uint32_t m_Rank );
	~CDBW3MMDPlayerSummary( );

	string GetServer( )					{ return m_Server; }
	string GetName( )					{ return m_Name; }
	string GetCategory( )				{ return m_Category; }
	uint32_t GetTotalGames( )			{ return m_TotalGames; }
	uint32_t GetTotalWins( )			{ return m_TotalWins; }
	uint32_t GetTotalLosses( )			{ return m_TotalLosses; }
	double GetScore( )					{ return m_Score; }
	int GetRank( )						{ return m_Rank; }
};

#endif
