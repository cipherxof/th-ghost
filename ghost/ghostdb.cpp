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
#include "config.h"
#include "ghostdb.h"
#include "bnet.h"

//
// CGHostDB
//

CGHostDB :: CGHostDB( CConfig *CFG ) : m_HasError( false )
{

}

CGHostDB :: ~CGHostDB( )
{

}

void CGHostDB :: RecoverCallable( CBaseCallable *callable )
{

}

bool CGHostDB :: Begin( )
{
	return true;
}

bool CGHostDB :: Commit( )
{
	return true;
}

uint32_t CGHostDB :: AdminCount( string server )
{
	return 0;
}

bool CGHostDB :: AdminCheck( string server, string user )
{
	return false;
}

bool CGHostDB :: AdminAdd( string server, string user )
{
	return false;
}

bool CGHostDB :: AdminRemove( string server, string user )
{
	return false;
}

vector<string> CGHostDB :: AdminList( string server )
{
	return vector<string>( );
}

uint32_t CGHostDB :: BanCount( string server )
{
	return 0;
}

CDBBan *CGHostDB :: BanCheck( string server, string user, string ip, string hostname, string ownername )
{
	return NULL;
}

uint32_t CGHostDB :: BanAdd( string server, string user, string ip, string gamename, string admin, string reason, uint32_t expiretime, string context )
{
	return false;
}

bool CGHostDB :: BanRemove( string server, string user, string context )
{
	return false;
}

bool CGHostDB :: BanRemove( string user, string context )
{
	return false;
}

map<string, string> CGHostDB :: SpoofList( )
{
	return map<string, string>( );
}

void CGHostDB :: ReconUpdate( uint32_t hostcounter, uint32_t seconds )
{

}

vector<string> CGHostDB :: CommandList( )
{
	return vector<string>( );
}

uint32_t CGHostDB :: GameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver, string savetype )
{
	return 0;
}

uint32_t CGHostDB :: GameUpdate( uint32_t id, string map, string gamename, string ownername, string creatorname, uint32_t players, string usernames, uint32_t slotsTotal, uint32_t totalGames, uint32_t totalPlayers, bool add )
{
	return 0;
}

uint32_t CGHostDB :: GamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour, string savetype )
{
	return 0;
}

uint32_t CGHostDB :: GamePlayerCount( string name )
{
	return 0;
}

CDBGamePlayerSummary *CGHostDB :: GamePlayerSummaryCheck( string name, string realm )
{
	return NULL;
}

CDBVampPlayerSummary *CGHostDB :: VampPlayerSummaryCheck( string name )
{
	return NULL;
}

uint32_t CGHostDB :: DotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec, string saveType )
{
	return 0;
}

uint32_t CGHostDB :: DotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills, string saveType )
{
	return 0;
}

uint32_t CGHostDB :: DotAPlayerCount( string name )
{
	return 0;
}

CDBDotAPlayerSummary *CGHostDB :: DotAPlayerSummaryCheck( string name, string realm, string saveType )
{
	return NULL;
}

CDBTreePlayerSummary *CGHostDB :: TreePlayerSummaryCheck( string name, string realm )
{
	return NULL;
}

CDBShipsPlayerSummary *CGHostDB :: ShipsPlayerSummaryCheck( string name, string realm )
{
	return NULL;
}

CDBSnipePlayerSummary *CGHostDB :: SnipePlayerSummaryCheck( string name, string realm )
{
	return NULL;
}

CDBW3MMDPlayerSummary *CGHostDB :: W3MMDPlayerSummaryCheck( string name, string realm, string category )
{
	return NULL;
}

string CGHostDB :: FromCheck( uint32_t ip )
{
	return "??";
}

bool CGHostDB :: FromAdd( uint32_t ip1, uint32_t ip2, string country )
{
	return false;
}

bool CGHostDB :: DownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime )
{
	return false;
}

uint32_t CGHostDB :: W3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing, string saveType )
{
	return 0;
}

bool CGHostDB :: W3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints, string saveType )
{
	return false;
}

bool CGHostDB :: W3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals, string saveType )
{
	return false;
}

bool CGHostDB :: W3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings, string saveType )
{
	return false;
}

void CGHostDB :: CreateThread( CBaseCallable *callable )
{
	callable->SetReady( true );
}

CCallableAdminCount *CGHostDB :: ThreadedAdminCount( string server )
{
	return NULL;
}

CCallableAdminCheck *CGHostDB :: ThreadedAdminCheck( string server, string user )
{
	return NULL;
}

CCallableAdminAdd *CGHostDB :: ThreadedAdminAdd( string server, string user )
{
	return NULL;
}

CCallableAdminRemove *CGHostDB :: ThreadedAdminRemove( string server, string user )
{
	return NULL;
}

CCallableAdminList *CGHostDB :: ThreadedAdminList( string server )
{
	return NULL;
}

CCallableBanCount *CGHostDB :: ThreadedBanCount( string server )
{
	return NULL;
}

CCallableBanCheck *CGHostDB :: ThreadedBanCheck( string server, string user, string ip, string hostname, string ownername )
{
	return NULL;
}

CCallableBanAdd *CGHostDB :: ThreadedBanAdd( string server, string user, string ip, string gamename, string admin, string reason, uint32_t expiretime, string context )
{
	return NULL;
}

CCallableBanRemove *CGHostDB :: ThreadedBanRemove( string server, string user, string context )
{
	return NULL;
}

CCallableBanRemove *CGHostDB :: ThreadedBanRemove( string user, string context )
{
	return NULL;
}

CCallableSpoofList *CGHostDB :: ThreadedSpoofList( )
{
	return NULL;
}

CCallableReconUpdate *CGHostDB :: ThreadedReconUpdate( uint32_t hostcounter, uint32_t seconds )
{
	return NULL;
}

CCallableCommandList *CGHostDB :: ThreadedCommandList( )
{
	return NULL;
}

CCallableGameAdd *CGHostDB :: ThreadedGameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver, string savetype )
{
	return NULL;
}

CCallableGameUpdate *CGHostDB :: ThreadedGameUpdate( uint32_t id, string map, string gamename, string ownername, string creatorname, uint32_t players, string usernames, uint32_t slotsTotal, uint32_t totalGames, uint32_t totalPlayers, bool add )
{
	return NULL;
}

CCallableGamePlayerAdd *CGHostDB :: ThreadedGamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour, string savetype )
{
	return NULL;
}

CCallableGamePlayerSummaryCheck *CGHostDB :: ThreadedGamePlayerSummaryCheck( string name, string realm )
{
	return NULL;
}

CCallableVampPlayerSummaryCheck *CGHostDB :: ThreadedVampPlayerSummaryCheck( string name )
{
	return NULL;
}

CCallableDotAGameAdd *CGHostDB :: ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec, string saveType )
{
	return NULL;
}

CCallableDotAPlayerAdd *CGHostDB :: ThreadedDotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, string item1, string item2, string item3, string item4, string item5, string item6, string hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills, string saveType )
{
	return NULL;
}

CCallableDotAPlayerSummaryCheck *CGHostDB :: ThreadedDotAPlayerSummaryCheck( string name, string realm, string saveType )
{
	return NULL;
}

CCallableTreePlayerSummaryCheck *CGHostDB :: ThreadedTreePlayerSummaryCheck( string name, string realm )
{
	return NULL;
}

CCallableShipsPlayerSummaryCheck *CGHostDB :: ThreadedShipsPlayerSummaryCheck( string name, string realm )
{
	return NULL;
}

CCallableSnipePlayerSummaryCheck *CGHostDB :: ThreadedSnipePlayerSummaryCheck( string name, string realm )
{
	return NULL;
}

CCallableW3MMDPlayerSummaryCheck *CGHostDB :: ThreadedW3MMDPlayerSummaryCheck( string name, string realm, string category )
{
	return NULL;
}

CCallableDownloadAdd *CGHostDB :: ThreadedDownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime )
{
	return NULL;
}

CCallableScoreCheck *CGHostDB :: ThreadedScoreCheck( string category, string name, string server )
{
	return NULL;
}

CCallableLeagueCheck *CGHostDB :: ThreadedLeagueCheck( string category, string name, string server, string gamename )
{
	return NULL;
}

CCallableGetTournament *CGHostDB :: ThreadedGetTournament( string gamename )
{
	return NULL;
}

CCallableTournamentChat *CGHostDB :: ThreadedTournamentChat( uint32_t chatid, string message )
{
	return NULL;
}

CCallableTournamentUpdate *CGHostDB :: ThreadedTournamentUpdate( uint32_t matchid, string gamename, uint32_t status )
{
	return NULL;
}

CCallableConnectCheck *CGHostDB :: ThreadedConnectCheck( string name, uint32_t sessionkey )
{
	return NULL;
}

CCallableW3MMDPlayerAdd *CGHostDB :: ThreadedW3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing, string saveType )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints, string saveType )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals, string saveType )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings, string saveType )
{
	return NULL;
}

//
// Callables
//

void CBaseCallable :: Init( )
{
	m_StartTicks = GetTicks( );
}

void CBaseCallable :: Close( )
{
	m_EndTicks = GetTicks( );
	m_Ready = true;
}

CCallableAdminCount :: ~CCallableAdminCount( )
{

}

CCallableAdminCheck :: ~CCallableAdminCheck( )
{

}

CCallableAdminAdd :: ~CCallableAdminAdd( )
{

}

CCallableAdminRemove :: ~CCallableAdminRemove( )
{

}

CCallableAdminList :: ~CCallableAdminList( )
{

}

CCallableBanCount :: ~CCallableBanCount( )
{

}

CCallableBanCheck :: ~CCallableBanCheck( )
{
	delete m_Result;
}

CCallableBanAdd :: ~CCallableBanAdd( )
{

}

CCallableBanRemove :: ~CCallableBanRemove( )
{

}

CCallableSpoofList :: ~CCallableSpoofList( )
{
	// don't delete anything in m_Result here, it's the caller's responsibility
}

CCallableReconUpdate :: ~CCallableReconUpdate( )
{

}

CCallableCommandList :: ~CCallableCommandList( )
{
	// don't delete anything in m_Result here, it's the caller's responsibility
}

CCallableGameAdd :: ~CCallableGameAdd( )
{

}

CCallableGameUpdate :: ~CCallableGameUpdate( )
{

}

CCallableGamePlayerAdd :: ~CCallableGamePlayerAdd( )
{

}

CCallableGamePlayerSummaryCheck :: ~CCallableGamePlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableVampPlayerSummaryCheck :: ~CCallableVampPlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableDotAGameAdd :: ~CCallableDotAGameAdd( )
{

}

CCallableDotAPlayerAdd :: ~CCallableDotAPlayerAdd( )
{

}

CCallableDotAPlayerSummaryCheck :: ~CCallableDotAPlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableTreePlayerSummaryCheck :: ~CCallableTreePlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableShipsPlayerSummaryCheck :: ~CCallableShipsPlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableSnipePlayerSummaryCheck :: ~CCallableSnipePlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableW3MMDPlayerSummaryCheck :: ~CCallableW3MMDPlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableDownloadAdd :: ~CCallableDownloadAdd( )
{

}

CCallableScoreCheck :: ~CCallableScoreCheck( )
{
	delete m_Result;
}

CCallableLeagueCheck :: ~CCallableLeagueCheck( )
{

}

CCallableGetTournament :: ~CCallableGetTournament( )
{

}

CCallableTournamentChat :: ~CCallableTournamentChat( )
{

}

CCallableTournamentUpdate :: ~CCallableTournamentUpdate( )
{

}

CCallableConnectCheck :: ~CCallableConnectCheck( )
{

}

CCallableW3MMDPlayerAdd :: ~CCallableW3MMDPlayerAdd( )
{

}

CCallableW3MMDVarAdd :: ~CCallableW3MMDVarAdd( )
{

}

//
// CDBBan
//

CDBBan :: CDBBan( uint32_t nId ) : m_Id( nId ), m_Delete( true ), m_CacheTime( 0 )
{

}

CDBBan :: CDBBan( uint32_t nId, string nServer, string nName, string nIP, string nDate, string nGameName, string nAdmin, string nReason, string nExpireDate, string nContext, uint32_t nCacheTime ) : m_Id( nId ), m_Server( nServer ), m_Name( nName ), m_IP( nIP ), m_Date( nDate ), m_GameName( nGameName ), m_Admin( nAdmin ), m_Reason( nReason ), m_ExpireDate( nExpireDate ), m_Context( nContext ), m_Delete( false ), m_CacheTime( nCacheTime )
{

}

CDBBan :: CDBBan( CDBBan *copy )
{
	m_Id = copy->m_Id;
	m_Server = copy->m_Server;
	m_Name = copy->m_Name;
	m_IP = copy->m_IP;
	m_Date = copy->m_Date;
	m_GameName = copy->m_GameName;
	m_Admin = copy->m_Admin;
	m_Reason = copy->m_Reason;
	m_ExpireDate = copy->m_ExpireDate;
	m_Context = copy->m_Context;
	m_Delete = copy->m_Delete;
	m_CacheTime = copy->m_CacheTime;
}

CDBBan :: ~CDBBan( )
{

}

//
// CDBGame
//

CDBGame :: CDBGame( uint32_t nID, string nServer, string nMap, string nDateTime, string nGameName, string nOwnerName, uint32_t nDuration ) : m_ID( nID ), m_Server( nServer ), m_Map( nMap ), m_DateTime( nDateTime ), m_GameName( nGameName ), m_OwnerName( nOwnerName ), m_Duration( nDuration )
{

}

CDBGame :: ~CDBGame( )
{

}

//
// CDBGamePlayer
//

CDBGamePlayer :: CDBGamePlayer( uint32_t nID, uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour )
    : m_ID( nID ), m_GameID( nGameID ), m_Name( nName ), m_IP( nIP ), m_Spoofed( nSpoofed ), m_SpoofedRealm( nSpoofedRealm ), m_Reserved( nReserved ), m_LoadingTime( nLoadingTime ), m_Left( nLeft ), m_LeftReason( nLeftReason ), m_Team( nTeam ), m_Colour( nColour )
{

}

CDBGamePlayer :: ~CDBGamePlayer( )
{

}

//
// CDBGamePlayerSummary
//

CDBGamePlayerSummary :: CDBGamePlayerSummary( string nServer, string nName, uint32_t nTotalGames, double nLeftPercent, uint32_t nPlayingTime )
    : m_Server( nServer ), m_Name( nName ), m_TotalGames( nTotalGames ), m_LeftPercent( nLeftPercent ), m_PlayingTime( nPlayingTime )
{

}

CDBGamePlayerSummary :: ~CDBGamePlayerSummary( )
{

}

//
// CDBVampPlayerSummary
//

CDBVampPlayerSummary :: CDBVampPlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalHumanGames, uint32_t nTotalVampGames, uint32_t nTotalHumanWins, uint32_t nTotalVampWins, uint32_t nTotalHumanLosses, uint32_t nTotalVampLosses, uint32_t nTotalVampKills, double nMinCommandCenter, double nAvgCommandCenter, double nMinBase, double nAvgBase )
    : m_TotalGames( nTotalGames ), m_TotalHumanGames( nTotalHumanGames ), m_TotalVampGames( nTotalVampGames ), m_TotalHumanWins( nTotalHumanWins ), m_TotalVampWins( nTotalVampWins ), m_TotalHumanLosses( nTotalHumanLosses ), m_TotalVampLosses( nTotalVampLosses ), m_TotalVampKills( nTotalVampKills ), m_MinCommandCenter( nMinCommandCenter ), m_AvgCommandCenter( nAvgCommandCenter ), m_MinBase( nMinBase ), m_AvgBase( nAvgBase )
{

}

CDBVampPlayerSummary :: ~CDBVampPlayerSummary( )
{

}

//
// CDBDotAGame
//

CDBDotAGame :: CDBDotAGame( uint32_t nID, uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec ) : m_ID( nID ), m_GameID( nGameID ), m_Winner( nWinner ), m_Min( nMin ), m_Sec( nSec )
{

}

CDBDotAGame :: ~CDBDotAGame( )
{

}

//
// CDBDotAPlayer
//

CDBDotAPlayer :: CDBDotAPlayer( ) : m_ID( 0 ), m_GameID( 0 ), m_Colour( 0 ), m_Kills( 0 ), m_Deaths( 0 ), m_CreepKills( 0 ), m_CreepDenies( 0 ), m_Assists( 0 ), m_Gold( 0 ), m_NeutralKills( 0 ), m_NewColour( 0 ), m_TowerKills( 0 ), m_RaxKills( 0 ), m_CourierKills( 0 )
{

}

CDBDotAPlayer :: CDBDotAPlayer( uint32_t nID, uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, string nItem1, string nItem2, string nItem3, string nItem4, string nItem5, string nItem6, string nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills )
    : m_ID( nID ), m_GameID( nGameID ), m_Colour( nColour ), m_Kills( nKills ), m_Deaths( nDeaths ), m_CreepKills( nCreepKills ), m_CreepDenies( nCreepDenies ), m_Assists( nAssists ), m_Gold( nGold ), m_NeutralKills( nNeutralKills ), m_Hero( nHero ), m_NewColour( nNewColour ), m_TowerKills( nTowerKills ), m_RaxKills( nRaxKills ), m_CourierKills( nCourierKills )
{
	m_Items[0] = nItem1;
	m_Items[1] = nItem2;
	m_Items[2] = nItem3;
	m_Items[3] = nItem4;
	m_Items[4] = nItem5;
	m_Items[5] = nItem6;
}

CDBDotAPlayer :: ~CDBDotAPlayer( )
{

}

string CDBDotAPlayer :: GetItem( unsigned int i )
{
	if( i < 6 )
		return m_Items[i];

	return string( );
}

void CDBDotAPlayer :: SetItem( unsigned int i, string item )
{
	if( i < 6 )
		m_Items[i] = item;
}

//
// CDBDotAPlayerSummary
//

CDBDotAPlayerSummary :: CDBDotAPlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, uint32_t nTotalCreepKills, uint32_t nTotalCreepDenies, uint32_t nTotalAssists, uint32_t nTotalNeutralKills, uint32_t nTotalTowerKills, uint32_t nTotalRaxKills, uint32_t nTotalCourierKills, double nScore ) : m_Server( nServer ), m_Name( nName ), m_TotalGames( nTotalGames ), m_TotalWins( nTotalWins ), m_TotalLosses( nTotalLosses ), m_TotalKills( nTotalKills ), m_TotalDeaths( nTotalDeaths ), m_TotalCreepKills( nTotalCreepKills ), m_TotalCreepDenies( nTotalCreepDenies ), m_TotalAssists( nTotalAssists ), m_TotalNeutralKills( nTotalNeutralKills ), m_TotalTowerKills( nTotalTowerKills ), m_TotalRaxKills( nTotalRaxKills ), m_TotalCourierKills( nTotalCourierKills ), m_Score( nScore )
{

}

CDBDotAPlayerSummary :: ~CDBDotAPlayerSummary( )
{

}

CDBTreePlayerSummary :: CDBTreePlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalTKs, uint32_t nTotalDeaths, uint32_t nTotalSaves, uint32_t nTotalEntGames, uint32_t nTotalInfernalGames, double nScore ) : m_Server( nServer ), m_Name( nName ), m_TotalGames( nTotalGames ), m_TotalWins( nTotalWins ), m_TotalLosses( nTotalLosses ), m_TotalKills( nTotalKills ), m_TotalTKs( nTotalTKs ), m_TotalDeaths( nTotalDeaths ), m_TotalSaves( nTotalSaves ), m_TotalEntGames( nTotalEntGames), m_TotalInfernalGames( nTotalInfernalGames), m_Score( nScore )
{

}

CDBTreePlayerSummary :: ~CDBTreePlayerSummary( )
{

}

CDBShipsPlayerSummary :: CDBShipsPlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, double nScore ) : m_Server( nServer ), m_Name( nName ), m_TotalGames( nTotalGames ), m_TotalWins( nTotalWins ), m_TotalLosses( nTotalLosses ), m_TotalKills( nTotalKills ), m_TotalDeaths( nTotalDeaths ), m_Score( nScore )
{

}

CDBShipsPlayerSummary :: ~CDBShipsPlayerSummary( )
{

}

CDBSnipePlayerSummary :: CDBSnipePlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, double nScore ) : m_Server( nServer ), m_Name( nName ), m_TotalGames( nTotalGames ), m_TotalWins( nTotalWins ), m_TotalLosses( nTotalLosses ), m_TotalKills( nTotalKills ), m_TotalDeaths( nTotalDeaths ), m_Score( nScore )
{

}

CDBSnipePlayerSummary :: ~CDBSnipePlayerSummary( )
{

}

CDBW3MMDPlayerSummary :: CDBW3MMDPlayerSummary( string nServer, string nName, string nCategory, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, double nScore, uint32_t nRank ) : m_Server( nServer ), m_Name( nName ), m_Category( nCategory ), m_TotalGames( nTotalGames ), m_TotalWins( nTotalWins ), m_TotalLosses( nTotalLosses ), m_Score( nScore ), m_Rank( nRank )
{

}

CDBW3MMDPlayerSummary :: ~CDBW3MMDPlayerSummary( )
{

}
