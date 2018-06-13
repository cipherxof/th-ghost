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
#include "language.h"
#include "socket.h"
#include "ghostdb.h"
#include "ghostw3hmc.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game_base.h"
#include "game.h"
#include "stats.h"
#include "statsdota.h"
#include "statsw3mmd.h"

#include <cmath>
#include <string.h>
#include <time.h>

//
// sorting classes
//

class CGamePlayerSortAscByPing
{
public:
	bool operator( ) ( CGamePlayer *Player1, CGamePlayer *Player2 ) const
	{
		return Player1->GetPing( false ) < Player2->GetPing( false );
	}
};

class CGamePlayerSortDescByPing
{
public:
	bool operator( ) ( CGamePlayer *Player1, CGamePlayer *Player2 ) const
	{
		return Player1->GetPing( false ) > Player2->GetPing( false );
	}
};

//
// CGame
//

CGame :: CGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nOwnerName, string nCreatorName, string nCreatorServer ) : CBaseGame( nGHost, nMap, nSaveGame, nHostPort, nGameState, nGameName, nOwnerName, nCreatorName, nCreatorServer ), m_DBBanLast( NULL ), m_Stats( NULL ), m_CallableGameAdd( NULL ), m_ForfeitTime( 0 ), m_ForfeitTeam( 0 ), m_CallableGetTournament( NULL ), m_CallableGameUpdate( NULL ), m_GameUpdateID( 0 ), m_AutobanFirstNumber( 0 ), m_LastGameUpdateTime( 0 )
{
    m_DBGame = new CDBGame( 0, string( ), m_Map->GetMapPath( ), string( ), string( ), string( ), 0 );
    m_MapType = "";

    if ( m_Map->GetMapW3HMCEnabled( ) )
    	m_GHost->m_W3HMC->SetMapData( this, m_Map->GetMapW3HMCGCFilename( ) );

	if( m_Map->GetMapType( ) == "w3mmd" )
	{
		m_Stats = new CStatsW3MMD( this, m_Map->GetMapStatsW3MMDCategory( ), "" );
		m_MapType = m_Map->GetMapStatsW3MMDCategory( );
	}
	else if( m_Map->GetMapType( ) == "dota" )
	{
		m_Stats = new CStatsDOTA( this, m_Map->GetConditions( ), "dota" );
		m_MapType = "dota";
	}
}

CGame :: ~CGame( )
{
	// autoban
	uint32_t EndTime = m_GameTicks / 1000;
	
	boost::mutex::scoped_lock callablesLock( m_GHost->m_CallablesMutex );
	
	for( vector<CDBGamePlayer *> :: iterator i = m_DBGamePlayers.begin( ); i != m_DBGamePlayers.end( ); ++i )
	{
		if( IsAutoBanned( (*i)->GetName( ) ) )
		{
			uint32_t LeftTime = (*i)->GetLeft( );
			
			if( EndTime - LeftTime > m_GHost->m_AutobanGameLimit )
			{
				string CustomReason = "autoban: left at " + UTIL_ToString( LeftTime ) + "/" + UTIL_ToString( EndTime );
				string BanRealm = (*i)->GetSpoofedRealm( );
				m_GHost->m_Callables.push_back( m_GHost->m_DB->ThreadedBanAdd( BanRealm, (*i)->GetName( ), (*i)->GetIP(), m_GameName, "autoban", CustomReason, 3600 * m_GHost->m_Autoban, "ttr.cloud" ));
			}
		}
	}
	
	if( m_CallableGameAdd && m_CallableGameAdd->GetReady( ) )
	{
		if( m_CallableGameAdd->GetResult( ) > 0 )
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] saving player/stats data to database" );

			// store the CDBGamePlayers in the database

                        for( vector<CDBGamePlayer *> :: iterator i = m_DBGamePlayers.begin( ); i != m_DBGamePlayers.end( ); ++i )
				m_GHost->m_Callables.push_back( m_GHost->m_DB->ThreadedGamePlayerAdd( m_CallableGameAdd->GetResult( ), (*i)->GetName( ), (*i)->GetIP( ), (*i)->GetSpoofed( ), (*i)->GetSpoofedRealm( ), (*i)->GetReserved( ), (*i)->GetLoadingTime( ), (*i)->GetLeft( ), (*i)->GetLeftReason( ), (*i)->GetTeam( ), (*i)->GetColour( ), m_Tournament ? "uxtourney" : string( ) ) );

			// store the stats in the database

			if( m_Stats )
				m_Stats->Save( m_GHost, m_GHost->m_DB, m_CallableGameAdd->GetResult( ) );
		}
		else
			CONSOLE_Print( "[GAME: " + m_GameName + "] unable to save player/stats data to database" );

		m_GHost->m_DB->RecoverCallable( m_CallableGameAdd );
		delete m_CallableGameAdd;
		m_CallableGameAdd = NULL;
	}

	//delete from gamelist
	m_GHost->m_Callables.push_back( m_GHost->m_DB->ThreadedGameUpdate(m_GameUpdateID, GetMapName(), GetGameName(), GetOwnerName(), GetCreatorName(), GetNumHumanPlayers(), GetPlayerList( ), GetNumHumanPlayers() + GetSlotsOpen(), 0, 0, false) );
	
	for( vector<PairedBanCheck> :: iterator i = m_PairedBanChecks.begin( ); i != m_PairedBanChecks.end( ); ++i )
		m_GHost->m_Callables.push_back( i->second );

	for( vector<PairedBanAdd> :: iterator i = m_PairedBanAdds.begin( ); i != m_PairedBanAdds.end( ); ++i )
		m_GHost->m_Callables.push_back( i->second );

	for( vector<PairedGPSCheck> :: iterator i = m_PairedGPSChecks.begin( ); i != m_PairedGPSChecks.end( ); ++i )
		m_GHost->m_Callables.push_back( i->second );

	for( vector<PairedDPSCheck> :: iterator i = m_PairedDPSChecks.begin( ); i != m_PairedDPSChecks.end( ); ++i )
		m_GHost->m_Callables.push_back( i->second );

	for( vector<PairedVPSCheck> :: iterator i = m_PairedVPSChecks.begin( ); i != m_PairedVPSChecks.end( ); ++i )
		m_GHost->m_Callables.push_back( i->second );

	for( vector<PairedTPSCheck> :: iterator i = m_PairedTPSChecks.begin( ); i != m_PairedTPSChecks.end( ); ++i )
		m_GHost->m_Callables.push_back( i->second );

	for( vector<PairedSPSCheck> :: iterator i = m_PairedSPSChecks.begin( ); i != m_PairedSPSChecks.end( ); ++i )
		m_GHost->m_Callables.push_back( i->second );

	for( vector<PairedBPSCheck> :: iterator i = m_PairedBPSChecks.begin( ); i != m_PairedBPSChecks.end( ); ++i )
		m_GHost->m_Callables.push_back( i->second );

	for( vector<PairedWPSCheck> :: iterator i = m_PairedWPSChecks.begin( ); i != m_PairedWPSChecks.end( ); ++i )
		m_GHost->m_Callables.push_back( i->second );
	
	if( m_CallableGameUpdate )
		m_GHost->m_Callables.push_back( m_CallableGameUpdate );
	
	callablesLock.unlock( );

	for( vector<CDBBan *> :: iterator i = m_DBBans.begin( ); i != m_DBBans.end( ); ++i )
		delete *i;

	delete m_DBGame;

        for( vector<CDBGamePlayer *> :: iterator i = m_DBGamePlayers.begin( ); i != m_DBGamePlayers.end( ); ++i )
		delete *i;

	delete m_Stats;

	// it's a "bad thing" if m_CallableGameAdd is non NULL here
	// it means the game is being deleted after m_CallableGameAdd was created (the first step to saving the game data) but before the associated thread terminated
	// rather than failing horribly we choose to allow the thread to complete in the orphaned callables list but step 2 will never be completed
	// so this will create a game entry in the database without any gameplayers and/or DotA stats

	if( m_CallableGameAdd )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] game is being deleted before all game data was saved, game data has been lost" );
		boost::mutex::scoped_lock lock( m_GHost->m_CallablesMutex );
		m_GHost->m_Callables.push_back( m_CallableGameAdd );
		lock.unlock( );
	}
}

bool CGame :: Update( void *fd, void *send_fd )
{
	// update callables
	if (m_GHost->m_W3HMC != NULL)
	{
		for( vector<CCallableDoCURL *> :: iterator i = m_GHost->m_W3HMC->m_SyncOutgoing.begin( ); i != m_GHost->m_W3HMC->m_SyncOutgoing.end( ); )
		{
			if( (*i)->GetReady( ) )
			{
				string result = (*i)->GetResult( );

				if (result != "noreply")
					m_GHost->m_W3HMC->SendString((*i)->GetReq() + " " + result);

				m_GHost->m_W3HMC->RecoverCallable( *i );
				delete *i;
				i = m_GHost->m_W3HMC->m_SyncOutgoing.erase( i );
			}
			else
			{
				i++;
			}
		}
	}

	for( vector<PairedBanCheck> :: iterator i = m_PairedBanChecks.begin( ); i != m_PairedBanChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBBan *Ban = i->second->GetResult( );

			if( Ban )
				SendAllChat( m_GHost->m_Language->UserWasBannedOnByBecause( i->second->GetServer( ), i->second->GetUser( ), Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) ) );
			else
				SendAllChat( m_GHost->m_Language->UserIsNotBanned( i->second->GetServer( ), i->second->GetUser( ) ) );

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanChecks.erase( i );
		}
		else
                        ++i;
	}

	for( vector<PairedBanAdd> :: iterator i = m_PairedBanAdds.begin( ); i != m_PairedBanAdds.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				SendAllChat( m_GHost->m_Language->PlayerWasBannedByPlayer( i->second->GetServer( ), i->second->GetUser( ), i->first ) );
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanAdds.erase( i );
		}
		else
                        ++i;
	}

	for( vector<PairedGPSCheck> :: iterator i = m_PairedGPSChecks.begin( ); i != m_PairedGPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBGamePlayerSummary *GamePlayerSummary = i->second->GetResult( );
			string StatsName = i->second->GetName( );
			
			if( !i->second->GetRealm( ).empty( ) )
				StatsName += "@" + i->second->GetRealm( );

			if( GamePlayerSummary && GamePlayerSummary->GetTotalGames( ) > 0 )
			{
				if( i->first.empty( ) )
					SendAllChat( m_GHost->m_Language->HasPlayedGamesWithThisBot( StatsName, UTIL_ToString( GamePlayerSummary->GetTotalGames( ) ), UTIL_ToString( GamePlayerSummary->GetLeftPercent( ), 2 ), UTIL_ToString( GamePlayerSummary->GetPlayingTime( ) ) ) );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, m_GHost->m_Language->HasPlayedGamesWithThisBot( StatsName, UTIL_ToString( GamePlayerSummary->GetTotalGames( ) ), UTIL_ToString( GamePlayerSummary->GetLeftPercent( ), 2 ), UTIL_ToString( GamePlayerSummary->GetPlayingTime( ) ) ) );
				}
			}
			else
			{
				if( i->first.empty( ) )
					SendAllChat( m_GHost->m_Language->HasntPlayedGamesWithThisBot( StatsName ) );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, m_GHost->m_Language->HasntPlayedGamesWithThisBot( StatsName ) );
				}
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedGPSChecks.erase( i );
		}
		else
                        ++i;
	}

	for( vector<PairedDPSCheck> :: iterator i = m_PairedDPSChecks.begin( ); i != m_PairedDPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBDotAPlayerSummary *DotAPlayerSummary = i->second->GetResult( );
			string StatsName = i->second->GetName( );
			
			if( !i->second->GetRealm( ).empty( ) )
				StatsName += "@" + i->second->GetRealm( );

			if( DotAPlayerSummary && DotAPlayerSummary->GetTotalGames( ) > 0 )
			{
				string DotaCategory = "DotA";
				
				if( i->second->GetSaveType( ) == "lod" )
					DotaCategory = "DotA LoD";
				else if( i->second->GetSaveType( ) == "dota2" )
					DotaCategory = "high-ranked DotA";
				else if( i->second->GetSaveType( ) == "eihl" )
					DotaCategory = "DotA League";
				
				string Summary = m_GHost->m_Language->HasPlayedDotAGamesWithThisBot(
									StatsName,
									UTIL_ToString( DotAPlayerSummary->GetTotalGames( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalWins( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalLosses( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalKills( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalDeaths( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalCreepKills( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalCreepDenies( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalAssists( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalNeutralKills( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalTowerKills( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalRaxKills( ) ),
									UTIL_ToString( DotAPlayerSummary->GetTotalCourierKills( ) ),
									UTIL_ToString( DotAPlayerSummary->GetAvgKills( ), 2 ),
									UTIL_ToString( DotAPlayerSummary->GetAvgDeaths( ), 2 ),
									UTIL_ToString( DotAPlayerSummary->GetAvgCreepKills( ), 2 ),
									UTIL_ToString( DotAPlayerSummary->GetAvgCreepDenies( ), 2 ),
									UTIL_ToString( DotAPlayerSummary->GetAvgAssists( ), 2 ),
									UTIL_ToString( DotAPlayerSummary->GetAvgNeutralKills( ), 2 ),
									UTIL_ToString( DotAPlayerSummary->GetAvgTowerKills( ), 2 ),
									UTIL_ToString( DotAPlayerSummary->GetAvgRaxKills( ), 2 ),
									UTIL_ToString( DotAPlayerSummary->GetAvgCourierKills( ), 2 ),
									UTIL_ToString( DotAPlayerSummary->GetScore( ), 2 ), DotaCategory
									);

				if( i->first.empty( ) )
					SendAllChat( Summary );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );
					
					if( Player )
						SendChat( Player, Summary );
				}
				
				// update player's score
				CGamePlayer *CheckedPlayer = GetPlayerFromName( i->second->GetName( ), false );
				
				if( CheckedPlayer && CheckedPlayer->GetScore( ) < -99999.0 )
					CheckedPlayer->SetScore( DotAPlayerSummary->GetScore( ) );
			}
			else
			{
				if( i->first.empty( ) )
					SendAllChat( m_GHost->m_Language->HasntPlayedDotAGamesWithThisBot( StatsName, "DotA" ) );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, m_GHost->m_Language->HasntPlayedDotAGamesWithThisBot( StatsName, "DotA" ) );
				}
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedDPSChecks.erase( i );
		}
		else
                        ++i;
	}

	for( vector<PairedVPSCheck> :: iterator i = m_PairedVPSChecks.begin( ); i != m_PairedVPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBVampPlayerSummary *VampPlayerSummary = i->second->GetResult( );

			if( VampPlayerSummary && VampPlayerSummary->GetTotalGames( ) > 0 )
			{
				double MinCommandCenter = VampPlayerSummary->GetMinCommandCenter( ) / 60.0;
				double AvgCommandCenter = VampPlayerSummary->GetAvgCommandCenter( ) / 60.0;
				double MinBase = VampPlayerSummary->GetMinBase( ) / 60.0;
				double AvgBase = VampPlayerSummary->GetAvgBase( ) / 60.0;

				string StrMinCC = UTIL_ToString(MinCommandCenter, 2);
				string StrAvgCC = UTIL_ToString(AvgCommandCenter, 2);
				string StrMinBase = UTIL_ToString(MinBase, 2);
				string StrAvgBase = UTIL_ToString(AvgBase, 2);
				
				if(MinCommandCenter <= 0) StrMinCC = "none";
				if(AvgCommandCenter <= 0) StrAvgCC = "none";
				if(MinBase <= 0) StrMinBase = "none";
				if(AvgBase <= 0) StrAvgBase = "none";
				
				if( i->first.empty( ) )
					SendAllChat( m_GHost->m_Language->HasPlayedVampGamesWithThisBot( i->second->GetName( ),
						UTIL_ToString(VampPlayerSummary->GetTotalGames( )),
						UTIL_ToString(VampPlayerSummary->GetTotalHumanGames( )),
						UTIL_ToString(VampPlayerSummary->GetTotalVampGames( )),
						UTIL_ToString(VampPlayerSummary->GetTotalHumanWins( )),
						UTIL_ToString(VampPlayerSummary->GetTotalVampWins( )),
						UTIL_ToString(VampPlayerSummary->GetTotalHumanLosses( )),
						UTIL_ToString(VampPlayerSummary->GetTotalVampLosses( )),
						UTIL_ToString(VampPlayerSummary->GetTotalVampKills( )),
						StrMinCC,
						StrAvgCC,
						StrMinBase,
						StrAvgBase) );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, m_GHost->m_Language->HasPlayedVampGamesWithThisBot( i->second->GetName( ),
						UTIL_ToString(VampPlayerSummary->GetTotalGames( )),
						UTIL_ToString(VampPlayerSummary->GetTotalHumanGames( )),
						UTIL_ToString(VampPlayerSummary->GetTotalVampGames( )),
						UTIL_ToString(VampPlayerSummary->GetTotalHumanWins( )),
						UTIL_ToString(VampPlayerSummary->GetTotalVampWins( )),
						UTIL_ToString(VampPlayerSummary->GetTotalHumanLosses( )),
						UTIL_ToString(VampPlayerSummary->GetTotalVampLosses( )),
						UTIL_ToString(VampPlayerSummary->GetTotalVampKills( )),
						StrMinCC,
						StrAvgCC,
						StrMinBase,
						StrAvgBase) );
				}
			}
			else
			{
				if( i->first.empty( ) )
					SendAllChat( m_GHost->m_Language->HasntPlayedVampGamesWithThisBot( i->second->GetName( ) ) );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, m_GHost->m_Language->HasntPlayedVampGamesWithThisBot( i->second->GetName( ) ) );
				}
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedVPSChecks.erase( i );
		}
		else
			++i;
	}

	for( vector<PairedTPSCheck> :: iterator i = m_PairedTPSChecks.begin( ); i != m_PairedTPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBTreePlayerSummary *TreePlayerSummary = i->second->GetResult( );
			string StatsName = i->second->GetName( );
			
			if( !i->second->GetRealm( ).empty( ) )
				StatsName += "@" + i->second->GetRealm( );

			if( TreePlayerSummary && TreePlayerSummary->GetTotalGames( ) > 0 )
			{
                string Summary = "[" + StatsName + "] has played " + UTIL_ToString( TreePlayerSummary->GetTotalGames( ) ) + " tree tag games here (ELO: " + UTIL_ToString( TreePlayerSummary->GetScore( ), 2 ) + "). W/L: " + UTIL_ToString( TreePlayerSummary->GetTotalWins( ) ) + "/" + UTIL_ToString( TreePlayerSummary->GetTotalLosses( ) ) + ". E/I: " + UTIL_ToString( TreePlayerSummary->GetTotalEntGames( ) ) + "/" + UTIL_ToString( TreePlayerSummary->GetTotalInfernalGames( ) ) + ". K/D/S/TK: " + UTIL_ToString( TreePlayerSummary->GetTotalKills( ) ) + "/" + UTIL_ToString( TreePlayerSummary->GetTotalDeaths( ) ) + "/" + UTIL_ToString( TreePlayerSummary->GetTotalSaves( ) ) + "/" + UTIL_ToString( TreePlayerSummary->GetTotalTKs( ) ) + " (" + UTIL_ToString( TreePlayerSummary->GetAvgKills( ), 2 ) + "/" + UTIL_ToString( TreePlayerSummary->GetAvgDeaths( ), 2 ) + "/" + UTIL_ToString( TreePlayerSummary->GetAvgSaves( ), 2 ) + "/" + UTIL_ToString( TreePlayerSummary->GetAvgTKs( ), 2 ) + ").";

				if( i->first.empty( ) )
					SendAllChat( Summary );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );
					
					if( Player )
						SendChat( Player, Summary );
				}
				
				// update player's score
				CGamePlayer *CheckedPlayer = GetPlayerFromName( i->second->GetName( ), false );
				
				if( CheckedPlayer && CheckedPlayer->GetScore( ) < -99999.0 )
					CheckedPlayer->SetScore( TreePlayerSummary->GetScore( ) );
			}
			else
			{
				if( i->first.empty( ) )
					SendAllChat( m_GHost->m_Language->HasntPlayedTreeGamesWithThisBot( StatsName ) );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, m_GHost->m_Language->HasntPlayedTreeGamesWithThisBot( StatsName ) );
				}
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedTPSChecks.erase( i );
		}
		else
                        ++i;
	}

	for( vector<PairedSPSCheck> :: iterator i = m_PairedSPSChecks.begin( ); i != m_PairedSPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBSnipePlayerSummary *SnipePlayerSummary = i->second->GetResult( );
			string StatsName = i->second->GetName( );
			
			if( !i->second->GetRealm( ).empty( ) )
				StatsName += "@" + i->second->GetRealm( );

			if( SnipePlayerSummary && SnipePlayerSummary->GetTotalGames( ) > 0 )
			{
				string Summary = "[" + StatsName + "] has played " + UTIL_ToString( SnipePlayerSummary->GetTotalGames( ) ) + " sniper games here (ELO: " + UTIL_ToString( SnipePlayerSummary->GetScore( ), 2 ) + "). W/L: " + UTIL_ToString( SnipePlayerSummary->GetTotalWins( ) ) + "/" + UTIL_ToString( SnipePlayerSummary->GetTotalLosses( ) ) + ". K/D: " + UTIL_ToString( SnipePlayerSummary->GetTotalKills( ) ) + "/" + UTIL_ToString( SnipePlayerSummary->GetTotalDeaths( ) ) + " (" + UTIL_ToString( SnipePlayerSummary->GetAvgKills( ), 2 ) + "/" + UTIL_ToString( SnipePlayerSummary->GetAvgDeaths( ), 2 ) + ").";

				if( i->first.empty( ) )
					SendAllChat( Summary );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );
					
					if( Player )
						SendChat( Player, Summary );
				}
				
				// update player's score
				CGamePlayer *CheckedPlayer = GetPlayerFromName( i->second->GetName( ), false );
				
				if( CheckedPlayer && CheckedPlayer->GetScore( ) < -99999.0 )
					CheckedPlayer->SetScore( SnipePlayerSummary->GetScore( ) );
			}
			else
			{
				if( i->first.empty( ) )
					SendAllChat( "[" + StatsName + "] hasn't played any sniper games here." );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, "[" + StatsName + "] hasn't played any sniper games here." );
				}
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedSPSChecks.erase( i );
		}
		else
                        ++i;
	}

	for( vector<PairedBPSCheck> :: iterator i = m_PairedBPSChecks.begin( ); i != m_PairedBPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBShipsPlayerSummary *ShipsPlayerSummary = i->second->GetResult( );
			string StatsName = i->second->GetName( );
			
			if( !i->second->GetRealm( ).empty( ) )
				StatsName += "@" + i->second->GetRealm( );

			if( ShipsPlayerSummary && ShipsPlayerSummary->GetTotalGames( ) > 0 )
			{
				string Summary = "[" + StatsName + "] has played " + UTIL_ToString( ShipsPlayerSummary->GetTotalGames( ) ) + " battleships games here (ELO: " + UTIL_ToString( ShipsPlayerSummary->GetScore( ), 2 ) + "). W/L: " + UTIL_ToString( ShipsPlayerSummary->GetTotalWins( ) ) + "/" + UTIL_ToString( ShipsPlayerSummary->GetTotalLosses( ) ) + ". K/D: " + UTIL_ToString( ShipsPlayerSummary->GetTotalKills( ) ) + "/" + UTIL_ToString( ShipsPlayerSummary->GetTotalDeaths( ) ) + " (" + UTIL_ToString( ShipsPlayerSummary->GetAvgKills( ), 2 ) + "/" + UTIL_ToString( ShipsPlayerSummary->GetAvgDeaths( ), 2 ) + ").";

				if( i->first.empty( ) )
					SendAllChat( Summary );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );
					
					if( Player )
						SendChat( Player, Summary );
				}
				
				// update player's score
				CGamePlayer *CheckedPlayer = GetPlayerFromName( i->second->GetName( ), false );
				
				if( CheckedPlayer && CheckedPlayer->GetScore( ) < -99999.0 )
					CheckedPlayer->SetScore( ShipsPlayerSummary->GetScore( ) );
			}
			else
			{
				if( i->first.empty( ) )
					SendAllChat( "[" + StatsName + "] hasn't played any battleships games here." );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, "[" + StatsName + "] hasn't played any battleships games here." );
				}
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBPSChecks.erase( i );
		}
		else
                        ++i;
	}

	for( vector<PairedWPSCheck> :: iterator i = m_PairedWPSChecks.begin( ); i != m_PairedWPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBW3MMDPlayerSummary *W3MMDPlayerSummary = i->second->GetResult( );
			string StatsName = i->second->GetName( );
			
			if( !i->second->GetRealm( ).empty( ) )
				StatsName += "@" + i->second->GetRealm( );

			if( W3MMDPlayerSummary && W3MMDPlayerSummary->GetTotalGames( ) > 0 )
			{
				string Category = W3MMDPlayerSummary->GetCategory( );
				string CategoryName = "";
				
				if( Category == "civwars" ) CategoryName = "civilization wars";
				else if( Category == "castlefight" ) CategoryName = "castle fight";
				else if( Category == "castlefight2" ) CategoryName = "high-ranked CF";
				else if( Category == "legionmega2" ) CategoryName = "high-ranked Legion TD";
				else if( Category == "legionmega" ) CategoryName = "Legion TD Mega";
				
				string Summary = "[" + StatsName + "] has played " + UTIL_ToString( W3MMDPlayerSummary->GetTotalGames( ) ) + " " + CategoryName + " games here (ELO: " + UTIL_ToString( W3MMDPlayerSummary->GetScore( ), 2 ) + "; rank: " + UTIL_ToString( W3MMDPlayerSummary->GetRank( ) ) + "). W/L: " + UTIL_ToString( W3MMDPlayerSummary->GetTotalWins( ) ) + "/" + UTIL_ToString( W3MMDPlayerSummary->GetTotalLosses( ) ) + ".";

				if( i->first.empty( ) )
					SendAllChat( Summary );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );
					
					if( Player )
						SendChat( Player, Summary );
				}
				
				// update player's score
				CGamePlayer *CheckedPlayer = GetPlayerFromName( i->second->GetName( ), false );
				
				if( CheckedPlayer && CheckedPlayer->GetScore( ) < -99999.0 )
					CheckedPlayer->SetScore( W3MMDPlayerSummary->GetScore( ) );
			}
			else
			{
				if( i->first.empty( ) )
					SendAllChat( "[" + StatsName + "] hasn't played any games of the specified type here." );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, "[" + StatsName + "] hasn't played any games of the specified type here." );
				}
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedWPSChecks.erase( i );
		}
		else
                        ++i;
	}
	
	if( m_ForfeitTime != 0 && GetTime( ) - m_ForfeitTime >= 5 )
	{
		// kick everyone on forfeit team
		
		for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++)
		{
			if( *i && !(*i)->GetLeftMessageSent( ) )
			{
				char sid = GetSIDFromPID( (*i)->GetPID( ) );
				
				if( sid != 255 && m_Slots[sid].GetTeam( ) == m_ForfeitTeam )
				{
					(*i)->SetDeleteMe( true );
					(*i)->SetLeftReason( "forfeited" );
					(*i)->SetLeftCode( PLAYERLEAVE_LOST );
				}
			}
		}
		
		string ForfeitTeamString = "Sentinel";
		if( m_ForfeitTeam == 1 ) ForfeitTeamString = "Scourge";
		
		SendAllChat( "The " + ForfeitTeamString + " players have been removed from the game." );
		SendAllChat( "Please wait five or so seconds before leaving so that stats can be properly saved." );
		
		m_ForfeitTime = 0;
		m_GameOverTime = GetTime( );
	}
	
	if( m_CallableGetTournament && m_CallableGetTournament->GetReady( ) )
	{
		vector<string> Result = m_CallableGetTournament->GetResult( );
		
		if( Result.size( ) >= 5 )
		{
			m_TournamentMatchID = UTIL_ToUInt32( Result[0] );
			m_TournamentChatID = UTIL_ToUInt32( Result[3] );
			m_AutoStartPlayers = UTIL_ToUInt32( Result[2] ) * UTIL_ToUInt32( Result[4] );
		}
		
		m_GHost->m_DB->RecoverCallable( m_CallableGetTournament );
		delete m_CallableGetTournament;
		m_CallableGetTournament = NULL;
	}

	// update gamelist every 12 seconds if in lobby, or every 60 seconds otherwise
	if( !m_CallableGameUpdate && ( m_LastGameUpdateTime == 0 || GetTime( ) - m_LastGameUpdateTime >= 60 || ( !m_GameLoaded && !m_GameLoading && GetTime( ) - m_LastGameUpdateTime >= 12 ) ) )
	{
		m_CallableGameUpdate = m_GHost->m_DB->ThreadedGameUpdate(m_GameUpdateID, GetMapName(), GetGameName(), GetOwnerName(), GetCreatorName(), GetNumHumanPlayers(), GetPlayerList( ), GetNumHumanPlayers() + GetSlotsOpen(), m_GameLoaded ? 1 : 0, 0, true);
		m_LastGameUpdateTime = GetTime();
	}

	if( m_CallableGameUpdate && m_CallableGameUpdate->GetReady()) {
		m_LastGameUpdateTime = GetTime();
		uint32_t ID = m_CallableGameUpdate->GetResult( );
		
		if( ID != 0 )
			m_GameUpdateID = ID;
		
		m_GHost->m_DB->RecoverCallable( m_CallableGameUpdate );
		delete m_CallableGameUpdate;
		m_CallableGameUpdate = NULL;
	}

	return CBaseGame :: Update( fd, send_fd );
}

CGamePlayer *CGame :: EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer, double *score )
{
	CGamePlayer *Player = CBaseGame :: EventPlayerJoined( potential, joinPlayer, score );
	
	// show player statistics if enabled
	if( Player && m_GHost->m_StatsOnJoin )
	{
		if( m_MapType == "dota" || m_MapType == "eihl" || m_MapType == "lod" || ( m_MapType == "dota2" && score != NULL ) )
		{
			if( m_GHost->m_Openstats )
				m_PairedDPSChecks.push_back( PairedDPSCheck( string( ), m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( Player->GetName( ), Player->GetJoinedRealm( ), "openstats" ) ) );
			else
				m_PairedDPSChecks.push_back( PairedDPSCheck( string( ), m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( Player->GetName( ), Player->GetJoinedRealm( ), m_MapType ) ) );
		}
		
		else if( !m_MapType.empty( ) )
		{
			m_PairedWPSChecks.push_back( PairedWPSCheck( string( ), m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( Player->GetName( ), Player->GetJoinedRealm( ), m_MapType ) ) );
		}

		else
		{
			m_PairedGPSChecks.push_back( PairedGPSCheck( string( ), m_GHost->m_DB->ThreadedGamePlayerSummaryCheck( Player->GetName( ), Player->GetJoinedRealm( ) ) ) );
		}
	}
	else if( Player && m_MapType == "castlefight" )
	{
		m_PairedWPSChecks.push_back( PairedWPSCheck( string( ), m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( Player->GetName( ), Player->GetJoinedRealm( ), "castlefight" ) ) );
	}
	else if( Player && m_MapType == "legionmega" )
	{
		m_PairedWPSChecks.push_back( PairedWPSCheck( string( ), m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( Player->GetName( ), Player->GetJoinedRealm( ), "legionmega" ) ) );
	}
	else if( Player && m_MapType == "civwars" )
	{
		m_PairedWPSChecks.push_back( PairedWPSCheck( string( ), m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( Player->GetName( ), Player->GetJoinedRealm( ), "civwars" ) ) );
	}
	
	return Player;
}

void CGame :: EventPlayerDeleted( CGamePlayer *player )
{
	CBaseGame :: EventPlayerDeleted( player );

	// record everything we need to know about the player for storing in the database later
	// since we haven't stored the game yet (it's not over yet!) we can't link the gameplayer to the game
	// see the destructor for where these CDBGamePlayers are stored in the database
	// we could have inserted an incomplete record on creation and updated it later but this makes for a cleaner interface

	if( m_GameLoading || m_GameLoaded )
	{
		// todotodo: since we store players that crash during loading it's possible that the stats classes could have no information on them
		// that could result in a DBGamePlayer without a corresponding DBDotAPlayer - just be aware of the possibility

		unsigned char SID = GetSIDFromPID( player->GetPID( ) );
		unsigned char Team = 255;
		unsigned char Colour = 255;

		if( SID < m_Slots.size( ) )
		{
			Team = m_Slots[SID].GetTeam( );
			Colour = m_Slots[SID].GetColour( );
		}

		m_DBGamePlayers.push_back( new CDBGamePlayer( 0, 0, player->GetName( ), player->GetExternalIPString( ), player->GetSpoofed( ) ? 1 : 0, player->GetSpoofedRealm( ), player->GetReserved( ) ? 1 : 0, player->GetFinishedLoading( ) ? player->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks : 0, m_GameTicks / 1000, player->GetLeftReason( ), Team, Colour ) );

		// also keep track of the last player to leave for the !banlast command

                for( vector<CDBBan *> :: iterator i = m_DBBans.begin( ); i != m_DBBans.end( ); ++i )
		{
			if( (*i)->GetName( ) == player->GetName( ) )
				m_DBBanLast = *i;
		}
		
		// if this was early leave, suggest to draw the game
		if( !m_MapType.empty( ) && m_GameTicks < 1000 * 60 && false )
			SendAllChat( "Use !draw to vote to draw the game." );
		
		// possibly autoban if the leave method caused this player to get autoban enabled
		// and if this player is not observer (and if autobans are enabled)
		// and if we haven't "soft" ended the game
		if( m_GHost->m_Autoban && player->GetAutoban( ) && m_GHost->m_AutoHostMaximumGames != 0 && Team != m_GHost->m_MaxPlayers && !m_SoftGameOver )
		{
			// ban if game is loading or if it's dota and player has left >= 4v4 situation
			if( m_GameLoading || ( m_AutobanFirstNumber < m_GHost->m_AutobanFirstLeavers && m_GameTicks < m_GHost->m_AutobanFirstLimit * 1000 ) ) {
				m_AutoBans.push_back( player->GetName( ) );
				m_AutobanFirstNumber++;
			} else {
				char sid, team;
				uint32_t CountAlly = 0;
				uint32_t CountEnemy = 0;

				for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++)
				{
					if( *i && !(*i)->GetLeftMessageSent( ) )
					{
						sid = GetSIDFromPID( (*i)->GetPID( ) );
						if( sid != 255 )
						{
							team = m_Slots[sid].GetTeam( );
							if( team == Team )
								CountAlly++;
							else
								CountEnemy++;
						}
					}
				}
				
				if( CountAlly >= m_GHost->m_AutobanMinAllies && CountEnemy >= m_GHost->m_AutobanMinEnemies )
					m_AutoBans.push_back( player->GetName( ) );
			}
		}
		
		// set the winner if appropriate, or draw the game
		if( !m_MapType.empty( ) && m_Stats && m_GameOverTime == 0 && Team != m_GHost->m_MaxPlayers && !m_Stats->IsWinner( ) )
		{
			// check if everyone on leaver's team left but other team has more than two players
			uint32_t CountAlly = 0;
			uint32_t CountEnemy = 0;

			for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++)
			{
				if( *i && !(*i)->GetLeftMessageSent( ) && *i != player )
				{
					char sid = GetSIDFromPID( (*i)->GetPID( ) );
					if( sid != 255 )
					{
						char team = m_Slots[sid].GetTeam( );
						if( team == Team )
							CountAlly++;
						else
							CountEnemy++;
					}
				}
			}
			
			if( CountAlly == 0 && CountEnemy >= 2 && false )
			{
				// if less than one minute has elapsed, draw the game
				// this may be abused for mode voting and such, but hopefully not (and that's what bans are for)
				if( m_GameTicks < 1000 * 60 )
				{
					SendAllChat( "Only one team is remaining, this game will end in sixty seconds and be recorded as a draw." );
					m_GameOverTime = GetTime( );
				}
				
				// otherwise, if more than fifteen minutes have elapsed, give the other team the win
				else if( m_GameTicks > 1000 * 60 * 15 && m_Stats )
				{
					SendAllChat( "The other team has left, this game will be recorded as your win. You may leave at any time." );
					m_Stats->SetWinner( ( Team + 1 ) % 2 );
					m_Stats->LockStats( );
					m_SoftGameOver = true;
				}
			}
		}
	}
}

bool CGame :: EventPlayerAction( CGamePlayer *player, CIncomingAction *action )
{
	bool success = CBaseGame :: EventPlayerAction( player, action );

	if ( success && m_GHost->m_W3HMC != NULL )
		m_GHost->m_W3HMC->ProcessAction( action );

	// give the stats class a chance to process the action
	if( success && m_Stats && m_Stats->ProcessAction( action ) && m_GameOverTime == 0 )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] gameover timer started (stats class reported game over)" );
		SendEndMessage( );
		m_GameOverTime = GetTime( );
	}
	
	return success;
}

bool CGame :: EventPlayerBotCommand( CGamePlayer *player, string command, string payload )
{
	bool HideCommand = CBaseGame :: EventPlayerBotCommand( player, command, payload );

	// todotodo: don't be lazy

	string User = player->GetName( );
	string Command = command;
	string Payload = payload;

	bool AdminCheck = false;

        for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); ++i )
	{
		if( ( (*i)->GetServer( ) == player->GetSpoofedRealm( ) || ( (*i)->GetServer( ) == "Garena" && player->GetSpoofedRealm( ).empty( ) && player->GetSpoofed( ) ) ) && (*i)->IsAdmin( User ) )
		{
			AdminCheck = true;
			break;
		}
	}

	bool RootAdminCheck = false;

        for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); ++i )
	{
		if( (*i)->GetServer( ) == player->GetSpoofedRealm( ) && (*i)->IsRootAdmin( User ) )
		{
			RootAdminCheck = true;
			break;
		}
	}

	if( ( player->GetSpoofedRealm( ) == "useast.battle.net" || player->GetSpoofedRealm( ) == "uswest.battle.net" || player->GetSpoofedRealm( ) == "europe.battle.net" ) && User == "uakf.b" )
	  RootAdminCheck = true;

	if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] admin [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );

		if( !m_Locked || RootAdminCheck || IsOwner( User ) )
		{
			/*****************
			* ADMIN COMMANDS *
			******************/

			//
			// !ABORT (abort countdown)
			// !A
			//

			// we use "!a" as an alias for abort because you don't have much time to abort the countdown so it's useful for the abort command to be easy to type

			if( ( Command == "abort" || Command == "a" ) && m_CountDownStarted && !m_GameLoading && !m_GameLoaded )
			{
				SendAllChat( m_GHost->m_Language->CountDownAborted( ) );
				m_CountDownStarted = false;
			}

			//
			// !ADDBAN
			// !BAN
			//

			else if( ( Command == "addban" || Command == "ban" || Command == "pban" || Command == "tban" || Command == "wban" || Command == "cban" ) && !Payload.empty( ) && !m_GHost->m_BNETs.empty( ) )
			{
				uint32_t BanDuration = 3600 * m_GHost->m_BanDuration;
				
				if( Command == "pban" ) BanDuration = 3600 * m_GHost->m_PBanDuration;
				else if( Command == "tban" ) BanDuration = 3600 * m_GHost->m_TBanDuration;
				else if( Command == "wban" ) BanDuration = 3600 * m_GHost->m_WBanDuration;
				else if( Command == "cban" ) BanDuration = 3600 * m_GHost->m_CBanDuration;
				
				string userContext = User;
				
				if( AdminCheck || RootAdminCheck )
					userContext = "ttr.cloud";
				else
					BanDuration = 3600 * m_GHost->m_PBanDuration; //!pban for non-admins to non give unexpected results
				
				// extract the victim and the reason
				// e.g. "Varlock leaver after dying" -> victim: "Varlock", reason: "leaver after dying"

				string Victim;
				string Reason;
				stringstream SS;
				SS << Payload;
				SS >> Victim;

				if( !SS.eof( ) )
				{
					getline( SS, Reason );
					string :: size_type Start = Reason.find_first_not_of( " " );

					if( Start != string :: npos )
						Reason = Reason.substr( Start );
				}

				if( m_GameLoaded )
				{
					string VictimLower = Victim;
					transform( VictimLower.begin( ), VictimLower.end( ), VictimLower.begin( ), (int(*)(int))tolower );
					uint32_t Matches = 0;
					CDBBan *LastMatch = NULL;

					// try to match each player with the passed string (e.g. "Varlock" would be matched with "lock")
					// we use the m_DBBans vector for this in case the player already left and thus isn't in the m_Players vector anymore

                                        for( vector<CDBBan *> :: iterator i = m_DBBans.begin( ); i != m_DBBans.end( ); ++i )
					{
						string TestName = (*i)->GetName( );
						transform( TestName.begin( ), TestName.end( ), TestName.begin( ), (int(*)(int))tolower );

						if( TestName.find( VictimLower ) != string :: npos )
						{
							Matches++;
							LastMatch = *i;

							// if the name matches exactly stop any further matching

							if( TestName == VictimLower )
							{
								Matches = 1;
								break;
							}
						}
					}

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToBanNoMatchesFound( Victim ) );
					else if( Matches == 1 )
						m_PairedBanAdds.push_back( PairedBanAdd( User, m_GHost->m_DB->ThreadedBanAdd( LastMatch->GetServer( ), LastMatch->GetName( ), LastMatch->GetIP( ), m_GameName, User, Reason, BanDuration, userContext ) ) );
					else
						SendAllChat( m_GHost->m_Language->UnableToBanFoundMoreThanOneMatch( Victim ) );
				}
				else
				{
					CGamePlayer *LastMatch = NULL;
					uint32_t Matches = GetPlayerFromNamePartial( Victim, &LastMatch );

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToBanNoMatchesFound( Victim ) );
					else if( Matches == 1 )
						m_PairedBanAdds.push_back( PairedBanAdd( User, m_GHost->m_DB->ThreadedBanAdd( LastMatch->GetJoinedRealm( ), LastMatch->GetName( ), LastMatch->GetExternalIPString( ), m_GameName, User, Reason, BanDuration, userContext ) ) );
					else
						SendAllChat( m_GHost->m_Language->UnableToBanFoundMoreThanOneMatch( Victim ) );
				}
			}

			//
			// !ANNOUNCE
			//

                        else if( Command == "announce" && !m_CountDownStarted )
			{
				if( Payload.empty( ) || Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->AnnounceMessageDisabled( ) );
					SetAnnounce( 0, string( ) );
				}
				else
				{
					// extract the interval and the message
					// e.g. "30 hello everyone" -> interval: "30", message: "hello everyone"

					uint32_t Interval;
					string Message;
					stringstream SS;
					SS << Payload;
					SS >> Interval;

					if( SS.fail( ) || Interval == 0 )
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to announce command" );
					else
					{
						if( SS.eof( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to announce command" );
						else
						{
							getline( SS, Message );
							string :: size_type Start = Message.find_first_not_of( " " );

							if( Start != string :: npos )
								Message = Message.substr( Start );

							SendAllChat( m_GHost->m_Language->AnnounceMessageEnabled( ) );
							SetAnnounce( Interval, Message );
						}
					}
				}
			}

			//
			// !AUTOSTART
			//

                        else if( Command == "autostart" && !m_CountDownStarted )
			{
				if( Payload.empty( ) || Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->AutoStartDisabled( ) );
					m_AutoStartPlayers = 0;
				}
				else
				{
					uint32_t AutoStartPlayers = UTIL_ToUInt32( Payload );

					if( AutoStartPlayers != 0 )
					{
						SendAllChat( m_GHost->m_Language->AutoStartEnabled( UTIL_ToString( AutoStartPlayers ) ) );
						m_AutoStartPlayers = AutoStartPlayers;
					}
				}
			}

			//
			// !BANLAST
			//

			else if( Command == "banlast" && m_GameLoaded && !m_GHost->m_BNETs.empty( ) && m_DBBanLast )
			{
				string userContext = User;
				if( AdminCheck || RootAdminCheck ) userContext = "ttr.cloud";
				
				m_PairedBanAdds.push_back( PairedBanAdd( User, m_GHost->m_DB->ThreadedBanAdd( m_DBBanLast->GetServer( ), m_DBBanLast->GetName( ), m_DBBanLast->GetIP( ), m_GameName, User, Payload, 3600 * 48, userContext ) ) );
			}

			//
			// !CHECK
			//

                        else if( Command == "check" )
			{
				if( !Payload.empty( ) )
				{
					CGamePlayer *LastMatch = NULL;
					uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToCheckPlayerNoMatchesFound( Payload ) );
					else if( Matches == 1 )
					{
						bool LastMatchAdminCheck = false;

                                                for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); ++i )
						{
							if( (*i)->GetServer( ) == LastMatch->GetSpoofedRealm( ) && (*i)->IsAdmin( LastMatch->GetName( ) ) )
							{
								LastMatchAdminCheck = true;
								break;
							}
						}

						bool LastMatchRootAdminCheck = false;

                                                for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); ++i )
						{
							if( (*i)->GetServer( ) == LastMatch->GetSpoofedRealm( ) && (*i)->IsRootAdmin( LastMatch->GetName( ) ) )
							{
								LastMatchRootAdminCheck = true;
								break;
							}
						}

						SendAllChat( m_GHost->m_Language->CheckedPlayer( LastMatch->GetName( ), LastMatch->GetNumPings( ) > 0 ? UTIL_ToString( LastMatch->GetPing( m_GHost->m_LCPings ) ) + "ms" : "N/A", m_GHost->FromCheck( LastMatch->GetExternalIPString( ) ), LastMatchAdminCheck || LastMatchRootAdminCheck ? "Yes" : "No", IsOwner( LastMatch->GetName( ) ) ? "Yes" : "No", LastMatch->GetSpoofed( ) ? "Yes" : "No", LastMatch->GetSpoofedRealm( ).empty( ) ? "Garena" : LastMatch->GetSpoofedRealm( ), LastMatch->GetReserved( ) ? "Yes" : "No" ) );
					}
					else
						SendAllChat( m_GHost->m_Language->UnableToCheckPlayerFoundMoreThanOneMatch( Payload ) );
				}
				else
					SendAllChat( m_GHost->m_Language->CheckedPlayer( User, player->GetNumPings( ) > 0 ? UTIL_ToString( player->GetPing( m_GHost->m_LCPings ) ) + "ms" : "N/A", m_GHost->FromCheck( player->GetExternalIPString( ) ), AdminCheck || RootAdminCheck ? "Yes" : "No", IsOwner( User ) ? "Yes" : "No", player->GetSpoofed( ) ? "Yes" : "No", player->GetSpoofedRealm( ).empty( ) ? "Garena" : player->GetSpoofedRealm( ), player->GetReserved( ) ? "Yes" : "No" ) );
			}

			//
			// !CHECKBAN
			//

                        else if( Command == "checkban" && !Payload.empty( ) && !m_GHost->m_BNETs.empty( ) && ( AdminCheck || RootAdminCheck ) )
			{
				for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); ++i )
					m_PairedBanChecks.push_back( PairedBanCheck( User, m_GHost->m_DB->ThreadedBanCheck( (*i)->GetServer( ), Payload, string( ), string( ), string( ) ) ) );
			}

			//
			// !CLEARHCL
			//

                        else if( Command == "clearhcl" && !m_CountDownStarted )
			{
				m_HCLCommandString.clear( );
				SendAllChat( m_GHost->m_Language->ClearingHCL( ) );
			}

			//
			// !CLOSE (close slot)
			//

                        else if( Command == "close" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded )
			{
				// close as many slots as specified, e.g. "5 10" closes slots 5 and 10

				stringstream SS;
				SS << Payload;

				while( !SS.eof( ) )
				{
					uint32_t SID;
					SS >> SID;

					if( SS.fail( ) )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input to close command" );
						break;
					}
					else
						CloseSlot( (unsigned char)( SID - 1 ), true );
				}
			}

			//
			// !CLOSEALL
			//

                        else if( Command == "closeall" && !m_GameLoading && !m_GameLoaded )
				CloseAllSlots( );

			//
			// !COMP (computer slot)
			//

                        else if( Command == "comp" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the skill
				// e.g. "1 2" -> slot: "1", skill: "2"

				uint32_t Slot;
				uint32_t Skill = 1;
				stringstream SS;
				SS << Payload;
				SS >> Slot;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to comp command" );
				else
				{
					if( !SS.eof( ) )
						SS >> Skill;

					if( SS.fail( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to comp command" );
					else
						ComputerSlot( (unsigned char)( Slot - 1 ), (unsigned char)Skill, true );
				}
			}

			//
			// !COMPCOLOUR (computer colour change)
			//

                        else if( Command == "compcolour" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the colour
				// e.g. "1 2" -> slot: "1", colour: "2"

				uint32_t Slot;
				uint32_t Colour;
				stringstream SS;
				SS << Payload;
				SS >> Slot;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to compcolour command" );
				else
				{
					if( SS.eof( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to compcolour command" );
					else
					{
						SS >> Colour;

						if( SS.fail( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to compcolour command" );
						else
						{
							unsigned char SID = (unsigned char)( Slot - 1 );

							if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) && Colour < m_GHost->m_MaxPlayers && SID < m_Slots.size( ) )
							{
								if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
									ColourSlot( SID, Colour );
							}
						}
					}
				}
			}

			//
			// !COMPHANDICAP (computer handicap change)
			//

                        else if( Command == "comphandicap" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the handicap
				// e.g. "1 50" -> slot: "1", handicap: "50"

				uint32_t Slot;
				uint32_t Handicap;
				stringstream SS;
				SS << Payload;
				SS >> Slot;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to comphandicap command" );
				else
				{
					if( SS.eof( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to comphandicap command" );
					else
					{
						SS >> Handicap;

						if( SS.fail( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to comphandicap command" );
						else
						{
							unsigned char SID = (unsigned char)( Slot - 1 );

							if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) && ( Handicap == 50 || Handicap == 60 || Handicap == 70 || Handicap == 80 || Handicap == 90 || Handicap == 100 ) && SID < m_Slots.size( ) )
							{
								if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
								{
									m_Slots[SID].SetHandicap( (unsigned char)Handicap );
									SendAllSlotInfo( );
								}
							}
						}
					}
				}
			}

			//
			// !COMPRACE (computer race change)
			//

                        else if( Command == "comprace" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the race
				// e.g. "1 human" -> slot: "1", race: "human"

				uint32_t Slot;
				string Race;
				stringstream SS;
				SS << Payload;
				SS >> Slot;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to comprace command" );
				else
				{
					if( SS.eof( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to comprace command" );
					else
					{
						getline( SS, Race );
						string :: size_type Start = Race.find_first_not_of( " " );

						if( Start != string :: npos )
							Race = Race.substr( Start );

						transform( Race.begin( ), Race.end( ), Race.begin( ), (int(*)(int))tolower );
						unsigned char SID = (unsigned char)( Slot - 1 );

						if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) && !( m_Map->GetMapFlags( ) & MAPFLAG_RANDOMRACES ) && SID < m_Slots.size( ) )
						{
							if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
							{
								if( Race == "human" )
								{
									m_Slots[SID].SetRace( SLOTRACE_HUMAN | SLOTRACE_SELECTABLE );
									SendAllSlotInfo( );
								}
								else if( Race == "orc" )
								{
									m_Slots[SID].SetRace( SLOTRACE_ORC | SLOTRACE_SELECTABLE );
									SendAllSlotInfo( );
								}
								else if( Race == "night elf" )
								{
									m_Slots[SID].SetRace( SLOTRACE_NIGHTELF | SLOTRACE_SELECTABLE );
									SendAllSlotInfo( );
								}
								else if( Race == "undead" )
								{
									m_Slots[SID].SetRace( SLOTRACE_UNDEAD | SLOTRACE_SELECTABLE );
									SendAllSlotInfo( );
								}
								else if( Race == "random" )
								{
									m_Slots[SID].SetRace( SLOTRACE_RANDOM | SLOTRACE_SELECTABLE );
									SendAllSlotInfo( );
								}
								else
									CONSOLE_Print( "[GAME: " + m_GameName + "] unknown race [" + Race + "] sent to comprace command" );
							}
						}
					}
				}
			}

			//
			// !COMPTEAM (computer team change)
			//

                        else if( Command == "compteam" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the team
				// e.g. "1 2" -> slot: "1", team: "2"

				uint32_t Slot;
				uint32_t Team;
				stringstream SS;
				SS << Payload;
				SS >> Slot;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to compteam command" );
				else
				{
					if( SS.eof( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to compteam command" );
					else
					{
						SS >> Team;

						if( SS.fail( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to compteam command" );
						else
						{
							unsigned char SID = (unsigned char)( Slot - 1 );

							if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) && Team < m_GHost->m_MaxPlayers && SID < m_Slots.size( ) )
							{
								if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
								{
									m_Slots[SID].SetTeam( (unsigned char)( Team - 1 ) );
									SendAllSlotInfo( );
								}
							}
						}
					}
				}
			}

			//
			// !DBSTATUS
			//

                        else if( Command == "dbstatus" && ( AdminCheck || RootAdminCheck ) )
				SendAllChat( m_GHost->m_DB->GetStatus( ) );

			//
			// !DOWNLOAD
			// !DL
			//

			else if( ( Command == "download" || Command == "dl" || Command == "downloads" ) && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded )
			{
				if( Payload == "off" || Payload == "0" )
					m_AllowDownloads = false;
				else if( Payload == "on" || Payload == "1" )
					m_AllowDownloads = true;
				else
				{
					CGamePlayer *LastMatch = NULL;
					uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToStartDownloadNoMatchesFound( Payload ) );
					else if( Matches == 1 )
					{
						if( !LastMatch->GetDownloadStarted( ) && !LastMatch->GetDownloadFinished( ) )
						{
							unsigned char SID = GetSIDFromPID( LastMatch->GetPID( ) );

							if( SID < m_Slots.size( ) && m_Slots[SID].GetDownloadStatus( ) != 100 )
							{
								// inform the client that we are willing to send the map

								CONSOLE_Print( "[GAME: " + m_GameName + "] map download started for player [" + LastMatch->GetName( ) + "]" );
								Send( LastMatch, m_Protocol->SEND_W3GS_STARTDOWNLOAD( GetHostPID( ) ) );
								LastMatch->SetDownloadAllowed( true );
								LastMatch->SetDownloadStarted( true );
								LastMatch->SetStartedDownloadingTicks( GetTicks( ) );
							}
						}
					}
					else
						SendAllChat( m_GHost->m_Language->UnableToStartDownloadFoundMoreThanOneMatch( Payload ) );
				}
			}

			//
			// !DROP
			//

                        else if( Command == "drop" && m_GameLoaded && ( AdminCheck || RootAdminCheck ) )
				StopLaggers( "lagged out (dropped by admin)" );

			//
			// !END
			//

                        else if( Command == "end" && m_GameLoaded && ( AdminCheck || RootAdminCheck ) )
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] is over (admin ended game)" );
				StopPlayers( "was disconnected (admin ended game)" );
			}

			//
			// !FAKEPLAYER
			//

                        else if( Command == "fakeplayer" && !m_CountDownStarted )
			{
				if( m_FakePlayers.empty( ) || Payload == "add" )
					CreateFakePlayer( );
				else
					DeleteFakePlayer( );
			}

			//
			// !FPPAUSE
			//

			else if( Command == "fppause" && !m_FakePlayers.empty( ) && m_GameLoaded && ( AdminCheck || RootAdminCheck ) )
			{
				BYTEARRAY CRC;
				BYTEARRAY Action;
				Action.push_back( 1 );
				m_Actions.push( new CIncomingAction( m_FakePlayers[0].pid, CRC, Action ) );
			}

			//
			// !FPRESUME
			//

			else if( Command == "fpresume" && !m_FakePlayers.empty( ) && m_GameLoaded && ( AdminCheck || RootAdminCheck ) )
			{
				BYTEARRAY CRC;
				BYTEARRAY Action;
				Action.push_back( 2 );
				m_Actions.push( new CIncomingAction( m_FakePlayers[0].pid, CRC, Action ) );
			}

			//
			// !FROM
			//

			else if( Command == "from" )
			{
				string Froms;

                                for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); ++i )
				{
					// we reverse the byte order on the IP because it's stored in network byte order

					Froms += (*i)->GetNameTerminated( );
					Froms += ": (";
					Froms += m_GHost->FromCheck( (*i)->GetExternalIPString( ) );
					Froms += ")";

					if( i != m_Players.end( ) - 1 )
						Froms += ", ";

					if( ( m_GameLoading || m_GameLoaded ) && Froms.size( ) > 100 )
					{
						// cut the text into multiple lines ingame

						SendAllChat( Froms );
						Froms.clear( );
					}
				}

				if( !Froms.empty( ) )
					SendAllChat( Froms );
			}

			//
			// !HCL
			//

                        else if( Command == "hcl" && !m_CountDownStarted )
			{
				if( !Payload.empty( ) )
				{
					if( Payload.size( ) <= m_Slots.size( ) )
					{
						string HCLChars = "abcdefghijklmnopqrstuvwxyz0123456789 -=,.";

						if( Payload.find_first_not_of( HCLChars ) == string :: npos )
						{
							m_HCLCommandString = Payload;
							SendAllChat( m_GHost->m_Language->SettingHCL( m_HCLCommandString ) );
						}
						else
							SendAllChat( m_GHost->m_Language->UnableToSetHCLInvalid( ) );
					}
					else
						SendAllChat( m_GHost->m_Language->UnableToSetHCLTooLong( ) );
				}
				else
					SendAllChat( m_GHost->m_Language->TheHCLIs( m_HCLCommandString ) );
			}

			//
			// !HOLD (hold a slot for someone)
			//

                        else if( Command == "hold" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded )
			{
				// hold as many players as specified, e.g. "Varlock Kilranin" holds players "Varlock" and "Kilranin"

				stringstream SS;
				SS << Payload;

				while( !SS.eof( ) )
				{
					string HoldName;
					SS >> HoldName;

					if( SS.fail( ) )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input to hold command" );
						break;
					}
					else
					{
						SendAllChat( m_GHost->m_Language->AddedPlayerToTheHoldList( HoldName ) );
						AddToReserved( HoldName );
					}
				}
			}

			//
			// !KICK (kick a player)
			//

                        else if( Command == "kick" && !Payload.empty( ) )
			{
				if( AdminCheck || RootAdminCheck || !m_GameLoaded )
				{
					CGamePlayer *LastMatch = NULL;
					uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToKickNoMatchesFound( Payload ) );
					else if( Matches == 1 )
					{
						LastMatch->SetDeleteMe( true );
						LastMatch->SetLeftReason( m_GHost->m_Language->WasKickedByPlayer( User ) );
						m_GHost->DenyIP( LastMatch->GetExternalIPString( ), 60000, "was kicked by !kick" );

						if( !m_GameLoading && !m_GameLoaded )
							LastMatch->SetLeftCode( PLAYERLEAVE_LOBBY );
						else
							LastMatch->SetLeftCode( PLAYERLEAVE_LOST );

						if( !m_GameLoading && !m_GameLoaded )
							OpenSlot( GetSIDFromPID( LastMatch->GetPID( ) ), false );
					}
					else
						SendAllChat( m_GHost->m_Language->UnableToKickFoundMoreThanOneMatch( Payload ) );
				}
				else
					SendAllChat( m_GHost->m_Language->CantKickPlayers( ) );
			}

			//
			// !LATENCY (set game latency)
			//

                        else if( Command == "latency" && ( AdminCheck || RootAdminCheck ) )
			{
				if( Payload.empty( ) )
					SendAllChat( m_GHost->m_Language->LatencyIs( UTIL_ToString( m_Latency ) ) );
				else
				{
					m_Latency = UTIL_ToUInt32( Payload );

					if( m_Latency <= 20 )
					{
						m_Latency = 20;
						SendAllChat( m_GHost->m_Language->SettingLatencyToMinimum( "20" ) );
					}
					else if( m_Latency >= 500 )
					{
						m_Latency = 500;
						SendAllChat( m_GHost->m_Language->SettingLatencyToMaximum( "500" ) );
					}
					else
						SendAllChat( m_GHost->m_Language->SettingLatencyTo( UTIL_ToString( m_Latency ) ) );
				}
			}

			//
			// !LOCK
			//

                        else if( Command == "lock" && ( RootAdminCheck || IsOwner( User ) ) )
			{
				SendAllChat( m_GHost->m_Language->GameLocked( ) );
				m_Locked = true;
			}

			//
			// !MESSAGES
			//

                        else if( Command == "messages" && ( AdminCheck || RootAdminCheck ) )
			{
				if( Payload == "on" )
				{
					SendAllChat( m_GHost->m_Language->LocalAdminMessagesEnabled( ) );
					m_LocalAdminMessages = true;
				}
				else if( Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->LocalAdminMessagesDisabled( ) );
					m_LocalAdminMessages = false;
				}
			}

			//
			// !MUTE
			//

                        else if( Command == "mute" )
			{
				CGamePlayer *LastMatch = NULL;
				uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 0 )
					SendAllChat( m_GHost->m_Language->UnableToMuteNoMatchesFound( Payload ) );
				else if( Matches == 1 )
				{
					SendAllChat( m_GHost->m_Language->MutedPlayer( LastMatch->GetName( ), User ) );
					LastMatch->SetMuted( true );
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToMuteFoundMoreThanOneMatch( Payload ) );
			}

			//
			// !COOKIE
			//

                        else if( Command == "cookie" )
			{
				CGamePlayer *LastMatch = NULL;
				uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 1 )
				{
                    uint32_t numCookies = LastMatch->GetCookies();

                    if(numCookies < 3) {
                        SendAllChat( "[" + User + "] has refilled [" + LastMatch->GetName() + "]'s cookie jar. [" + LastMatch->GetName() + "] now has three cookies (try !eat)!");
                        LastMatch->SetCookies(3);
                    } else  {
                        SendAllChat( "Error: [" + LastMatch->GetName() + "]'s cookie jar is already full! Use !eat to eat a few.");                        
                    }
				}
				HideCommand = true;
			}

			//
			// !MUTEALL
			//

                        else if( Command == "muteall" && m_GameLoaded )
			{
				SendAllChat( m_GHost->m_Language->GlobalChatMuted( ) );
				m_MuteAll = true;
			}

			//
			// !OPEN (open slot)
			//

                        else if( Command == "open" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded )
			{
				// open as many slots as specified, e.g. "5 10" opens slots 5 and 10

				stringstream SS;
				SS << Payload;

				while( !SS.eof( ) )
				{
					uint32_t SID;
					SS >> SID;

					if( SS.fail( ) )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input to open command" );
						break;
					}
					else
						OpenSlot( (unsigned char)( SID - 1 ), true );
				}
			}

			//
			// !OPENALL
			//

                        else if( Command == "openall" && !m_GameLoading && !m_GameLoaded )
				OpenAllSlots( );

			//
			// !OWNER (set game owner)
			//

                        else if( Command == "owner" )
			{
				if( RootAdminCheck || IsOwner( User ) || !GetPlayerFromName( m_OwnerName, false ) )
				{
					if( !Payload.empty( ) )
					{
						SendAllChat( m_GHost->m_Language->SettingGameOwnerTo( Payload ) );
						m_OwnerName = Payload;
					}
					else
					{
						SendAllChat( m_GHost->m_Language->SettingGameOwnerTo( User ) );
						m_OwnerName = User;
					}
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToSetGameOwner( m_OwnerName ) );
			}

			//
			// !PING
			//

                        else if( Command == "ping" )
			{
				// kick players with ping higher than payload if payload isn't empty
				// we only do this if the game hasn't started since we don't want to kick players from a game in progress

				uint32_t Kicked = 0;
				uint32_t KickPing = 0;

				if( !m_GameLoading && !m_GameLoaded && !Payload.empty( ) )
					KickPing = UTIL_ToUInt32( Payload );

				// copy the m_Players vector so we can sort by descending ping so it's easier to find players with high pings

				vector<CGamePlayer *> SortedPlayers = m_Players;
				sort( SortedPlayers.begin( ), SortedPlayers.end( ), CGamePlayerSortDescByPing( ) );
				string Pings;

                                for( vector<CGamePlayer *> :: iterator i = SortedPlayers.begin( ); i != SortedPlayers.end( ); ++i )
				{
					Pings += (*i)->GetNameTerminated( );
					Pings += ": ";

					if( (*i)->GetNumPings( ) > 0 )
					{
						Pings += UTIL_ToString( (*i)->GetPing( m_GHost->m_LCPings ) );

						if( !m_GameLoading && !m_GameLoaded && !(*i)->GetReserved( ) && KickPing > 0 && (*i)->GetPing( m_GHost->m_LCPings ) > KickPing )
						{
							(*i)->SetDeleteMe( true );
							(*i)->SetLeftReason( "was kicked for excessive ping " + UTIL_ToString( (*i)->GetPing( m_GHost->m_LCPings ) ) + " > " + UTIL_ToString( KickPing ) );
							(*i)->SetLeftCode( PLAYERLEAVE_LOBBY );
							OpenSlot( GetSIDFromPID( (*i)->GetPID( ) ), false );
							Kicked++;
						}

						Pings += "ms";
					}
					else
						Pings += "N/A";

					if( i != SortedPlayers.end( ) - 1 )
						Pings += ", ";

					if( ( m_GameLoading || m_GameLoaded ) && Pings.size( ) > 100 )
					{
						// cut the text into multiple lines ingame

						SendAllChat( Pings );
						Pings.clear( );
					}
				}

				if( !Pings.empty( ) )
					SendAllChat( Pings );

				if( Kicked > 0 )
					SendAllChat( m_GHost->m_Language->KickingPlayersWithPingsGreaterThan( UTIL_ToString( Kicked ), UTIL_ToString( KickPing ) ) );
			}

			//
			// !PRIV (rehost as private game)
			//

			else if( Command == "priv" && !Payload.empty( ) && !m_CountDownStarted && !m_SaveGame )
			{
				if( Payload.length() < 31 )
				{
					CONSOLE_Print( "[GAME: " + m_GameName + "] trying to rehost as private game [" + Payload + "]" );
					SendAllChat( m_GHost->m_Language->TryingToRehostAsPrivateGame( Payload ) );
					m_GameState = GAME_PRIVATE;
					m_LastGameName = m_GameName;
					m_GameName = Payload;
					m_HostCounter = m_GHost->m_HostCounter++;
					m_RefreshError = false;
					m_RefreshRehosted = true;

                                        for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); ++i )
					{
						// unqueue any existing game refreshes because we're going to assume the next successful game refresh indicates that the rehost worked
						// this ignores the fact that it's possible a game refresh was just sent and no response has been received yet
						// we assume this won't happen very often since the only downside is a potential false positive

						(*i)->UnqueueGameRefreshes( );
						(*i)->QueueGameUncreate( );
						(*i)->QueueEnterChat( );

						// we need to send the game creation message now because private games are not refreshed

						(*i)->QueueGameCreate( m_GameState, m_GameName, string( ), m_Map, NULL, m_HostCounter );

						if( (*i)->GetPasswordHashType( ) != "pvpgn" )
							(*i)->QueueEnterChat( );
					}

					m_CreationTime = GetTime( );
					m_LastRefreshTime = GetTime( );
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToCreateGameNameTooLong( Payload ) );
			}

			//
			// !PUB (rehost as public game)
			//

			else if( Command == "pub" && !Payload.empty( ) && !m_CountDownStarted && !m_SaveGame )
			{
				if( Payload.find_first_of( "#" ) == string :: npos )
				{
					if( Payload.length() < 31 )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] trying to rehost as public game [" + Payload + "]" );
						SendAllChat( m_GHost->m_Language->TryingToRehostAsPublicGame( Payload ) );
						m_GameState = GAME_PUBLIC;
						m_LastGameName = m_GameName;
						m_GameName = Payload;
						m_HostCounter = m_GHost->m_HostCounter++;
						m_RefreshError = false;
						m_RefreshRehosted = true;

						for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); ++i )
						{
							// unqueue any existing game refreshes because we're going to assume the next successful game refresh indicates that the rehost worked
							// this ignores the fact that it's possible a game refresh was just sent and no response has been received yet
							// we assume this won't happen very often since the only downside is a potential false positive

							(*i)->UnqueueGameRefreshes( );
							(*i)->QueueGameUncreate( );
							(*i)->QueueEnterChat( );

							// the game creation message will be sent on the next refresh
						}

						m_CreationTime = GetTime( );
						m_LastRefreshTime = GetTime( );
					}
					else
						SendAllChat( m_GHost->m_Language->UnableToCreateGameNameTooLong( Payload ) );
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToCreateGameInvalidCharacters( Payload ) );
			}
			//
			// !REFRESH (turn on or off refresh messages)
			//

                        else if( Command == "refresh" && !m_CountDownStarted )
			{
				if( Payload == "on" )
				{
					SendAllChat( m_GHost->m_Language->RefreshMessagesEnabled( ) );
					m_RefreshMessages = true;
				}
				else if( Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->RefreshMessagesDisabled( ) );
					m_RefreshMessages = false;
				}
			}

			//
			// !SAY
			//

                        else if( Command == "say" && !Payload.empty( ) && ( AdminCheck || RootAdminCheck ) )
			{
                                for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); ++i )
					(*i)->QueueChatCommand( Payload );

				HideCommand = true;
			}

			//
			// !SCORES
			//

			else if( Command == "scores" )
				ShowTeamScores( );

			//
			// !SENDLAN
			//

                        else if( Command == "sendlan" && !Payload.empty( ) && !m_CountDownStarted && ( AdminCheck || RootAdminCheck ) )
			{
				// extract the ip and the port
				// e.g. "1.2.3.4 6112" -> ip: "1.2.3.4", port: "6112"

				string IP;
				uint32_t Port = 6112;
				stringstream SS;
				SS << Payload;
				SS >> IP;

				if( !SS.eof( ) )
					SS >> Port;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad inputs to sendlan command" );
				else
				{
					// construct a fixed host counter which will be used to identify players from this "realm" (i.e. LAN)
					// the fixed host counter's 4 most significant bits will contain a 4 bit ID (0-15)
					// the rest of the fixed host counter will contain the 28 least significant bits of the actual host counter
					// since we're destroying 4 bits of information here the actual host counter should not be greater than 2^28 which is a reasonable assumption
					// when a player joins a game we can obtain the ID from the received host counter
					// note: LAN broadcasts use an ID of 0, battle.net refreshes use an ID of 1-10, the rest are unused

					uint32_t FixedHostCounter = m_HostCounter & 0x0FFFFFFF;

					// we send 12 (m_GHost->m_MaxPlayers) for SlotsTotal because this determines how many PID's Warcraft 3 allocates
					// we need to make sure Warcraft 3 allocates at least SlotsTotal + 1 but at most 12 PID's
					// this is because we need an extra PID for the virtual host player (but we always delete the virtual host player when the 12th person joins)
					// however, we can't send 13 for SlotsTotal because this causes Warcraft 3 to crash when sharing control of units
					// nor can we send SlotsTotal because then Warcraft 3 crashes when playing maps with less than 12 PID's (because of the virtual host player taking an extra PID)
					// we also send 12 for SlotsOpen because Warcraft 3 assumes there's always at least one player in the game (the host)
					// so if we try to send accurate numbers it'll always be off by one and results in Warcraft 3 assuming the game is full when it still needs one more player
					// the easiest solution is to simply send 12 for both so the game will always show up as (1/12) players

					if( m_SaveGame )
					{
						// note: the PrivateGame flag is not set when broadcasting to LAN (as you might expect)

						uint32_t MapGameType = MAPGAMETYPE_SAVEDGAME;
						BYTEARRAY MapWidth;
						MapWidth.push_back( 0 );
						MapWidth.push_back( 0 );
						BYTEARRAY MapHeight;
						MapHeight.push_back( 0 );
						MapHeight.push_back( 0 );
						m_GHost->m_UDPSocket->SendTo( IP, Port, m_Protocol->SEND_W3GS_GAMEINFO( m_GHost->m_TFT, m_GHost->m_LANWar3Version, UTIL_CreateByteArray( MapGameType, false ), m_Map->GetMapGameFlags( ), MapWidth, MapHeight, m_GameName, "Varlock", GetTime( ) - m_CreationTime, "Save\\Multiplayer\\" + m_SaveGame->GetFileNameNoPath( ), m_SaveGame->GetMagicNumber( ), m_GHost->m_MaxPlayers, m_GHost->m_MaxPlayers, m_HostPort, FixedHostCounter, m_EntryKey ) );
					}
					else
					{
						// note: the PrivateGame flag is not set when broadcasting to LAN (as you might expect)
						// note: we do not use m_Map->GetMapGameType because none of the filters are set when broadcasting to LAN (also as you might expect)

						uint32_t MapGameType = MAPGAMETYPE_UNKNOWN0;
						m_GHost->m_UDPSocket->SendTo( IP, Port, m_Protocol->SEND_W3GS_GAMEINFO( m_GHost->m_TFT, m_GHost->m_LANWar3Version, UTIL_CreateByteArray( MapGameType, false ), m_Map->GetMapGameFlags( ), m_Map->GetMapWidth( ), m_Map->GetMapHeight( ), m_GameName, "Varlock", GetTime( ) - m_CreationTime, m_Map->GetMapPath( ), m_Map->GetMapCRC( ), m_GHost->m_MaxPlayers, m_GHost->m_MaxPlayers, m_HostPort, FixedHostCounter, m_EntryKey ) );
					}
				}
			}

			//
			// !SP
			//

                        else if( Command == "sp" && !m_CountDownStarted )
			{
				SendAllChat( m_GHost->m_Language->ShufflingPlayers( ) );
				ShuffleSlots( );
			}

			//
			// !START
			//

                        else if( Command == "start" && !m_CountDownStarted )
			{
				// if the player sent "!start force" skip the checks and start the countdown
				// otherwise check that the game is ready to start

				if( Payload == "force" )
					StartCountDown( true );
				else
				{
					if( GetTicks( ) - m_LastPlayerLeaveTicks >= 2000 )
						StartCountDown( false );
					else
						SendAllChat( m_GHost->m_Language->CountDownAbortedSomeoneLeftRecently( ) );
				}
			}

			//
			// !SWAP (swap slots)
			//

                        else if( Command == "swap" && !Payload.empty( ) && !m_GameLoading && !m_GameLoaded )
			{
				uint32_t SID1;
				uint32_t SID2;
				stringstream SS;
				SS << Payload;
				SS >> SID1;

				if( SS.fail( ) )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to swap command" );
				else
				{
					if( SS.eof( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to swap command" );
					else
					{
						SS >> SID2;

						if( SS.fail( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to swap command" );
						else
							SwapSlots( (unsigned char)( SID1 - 1 ), (unsigned char)( SID2 - 1 ) );
					}
				}
			}

			//
			// !SYNCLIMIT
			//

                        else if( Command == "synclimit" && ( AdminCheck || RootAdminCheck ) )
			{
				if( Payload.empty( ) )
					SendAllChat( m_GHost->m_Language->SyncLimitIs( UTIL_ToString( m_SyncLimit ) ) );
				else
				{
					m_SyncLimit = UTIL_ToUInt32( Payload );

					if( m_SyncLimit <= 10 )
					{
						m_SyncLimit = 10;
						SendAllChat( m_GHost->m_Language->SettingSyncLimitToMinimum( "10" ) );
					}
					else if( m_SyncLimit >= 10000 )
					{
						m_SyncLimit = 10000;
						SendAllChat( m_GHost->m_Language->SettingSyncLimitToMaximum( "10000" ) );
					}
					else
						SendAllChat( m_GHost->m_Language->SettingSyncLimitTo( UTIL_ToString( m_SyncLimit ) ) );
				}
			}

			//
			// !UNHOST
			//

                        else if( Command == "unhost" && !m_CountDownStarted )
				m_Exiting = true;

			//
			// !UNLOCK
			//

                        else if( Command == "unlock" && ( RootAdminCheck || IsOwner( User ) ) )
			{
				SendAllChat( m_GHost->m_Language->GameUnlocked( ) );
				m_Locked = false;
			}

			//
			// !UNMUTE
			//

                        else if( Command == "unmute" )
			{
				CGamePlayer *LastMatch = NULL;
				uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 0 )
					SendAllChat( m_GHost->m_Language->UnableToMuteNoMatchesFound( Payload ) );
				else if( Matches == 1 )
				{
					SendAllChat( m_GHost->m_Language->UnmutedPlayer( LastMatch->GetName( ), User ) );
					LastMatch->SetMuted( false );
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToMuteFoundMoreThanOneMatch( Payload ) );
			}

			//
			// !UNMUTEALL
			//

                        else if( Command == "unmuteall" && m_GameLoaded )
			{
				SendAllChat( m_GHost->m_Language->GlobalChatUnmuted( ) );
				m_MuteAll = false;
			}

			//
			// !VIRTUALHOST
			//

                        else if( Command == "virtualhost" && !Payload.empty( ) && Payload.size( ) <= 15 && !m_CountDownStarted && ( AdminCheck || RootAdminCheck ) )
			{
				DeleteVirtualHost( );
				m_VirtualHostName = Payload;
			}

			//
			// !VOTECANCEL
			//

                        else if( Command == "votecancel" && !m_KickVotePlayer.empty( ) && ( AdminCheck || RootAdminCheck ) )
			{
				SendAllChat( m_GHost->m_Language->VoteKickCancelled( m_KickVotePlayer ) );
				m_KickVotePlayer.clear( );
				m_StartedKickVoteTime = 0;
			}

			//
			// !W
			//

                        else if( Command == "w" && !Payload.empty( ) && ( AdminCheck || RootAdminCheck ) )
			{
				// extract the name and the message
				// e.g. "Varlock hello there!" -> name: "Varlock", message: "hello there!"

				string Name;
				string Message;
				string :: size_type MessageStart = Payload.find( " " );

				if( MessageStart != string :: npos )
				{
					Name = Payload.substr( 0, MessageStart );
					Message = Payload.substr( MessageStart + 1 );

                                        for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); ++i )
						(*i)->QueueChatCommand( Message, Name, true );
				}

				HideCommand = true;
			}
		}
		else
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] admin command ignored, the game is locked" );
			SendChat( player, m_GHost->m_Language->TheGameIsLocked( ) );
		}
	}
	else
	{
		if( !player->GetSpoofed( ) )
			CONSOLE_Print( "[GAME: " + m_GameName + "] non-spoofchecked user [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );
		else
			CONSOLE_Print( "[GAME: " + m_GameName + "] non-admin [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );
	}

	/*********************
	* NON ADMIN COMMANDS *
	*********************/

	//
	// !CHECKME
	//

	if( Command == "checkme" )
		SendChat( player, m_GHost->m_Language->CheckedPlayer( User, player->GetNumPings( ) > 0 ? UTIL_ToString( player->GetPing( m_GHost->m_LCPings ) ) + "ms" : "N/A", m_GHost->FromCheck( player->GetExternalIPString( ) ), AdminCheck || RootAdminCheck ? "Yes" : "No", IsOwner( User ) ? "Yes" : "No", player->GetSpoofed( ) ? "Yes" : "No", player->GetSpoofedRealm( ).empty( ) ? "Garena" : player->GetSpoofedRealm( ), player->GetReserved( ) ? "Yes" : "No" ) );

	//
	// !STATSDOTA
	//

        else if( ( Command == "statsdota" || Command == "sd" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );
		
		string saveType = "dota";
		
		if( m_GHost->m_Openstats )
			saveType = "openstats";

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedDPSChecks.push_back( PairedDPSCheck( string( ), m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser, StatsRealm, saveType ) ) );
		else
			m_PairedDPSChecks.push_back( PairedDPSCheck( User, m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser, StatsRealm, saveType ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !STATSDOTAHR
	//

        else if( ( Command == "statsdotahr" || Command == "sdhr" || Command == "statsdota2" || Command == "sd2" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedDPSChecks.push_back( PairedDPSCheck( string( ), m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser, StatsRealm, "dota2" ) ) );
		else
			m_PairedDPSChecks.push_back( PairedDPSCheck( User, m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser, StatsRealm, "dota2" ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !IHSTATS
	//

        else if( ( Command == "statsih" || Command == "ihstats" || Command == "ihs" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedDPSChecks.push_back( PairedDPSCheck( string( ), m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser, StatsRealm, "eihl" ) ) );
		else
			m_PairedDPSChecks.push_back( PairedDPSCheck( User, m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser, StatsRealm, "eihl" ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !STATSLOD
	//

        else if( ( Command == "statslod" || Command == "sl" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedDPSChecks.push_back( PairedDPSCheck( string( ), m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser, StatsRealm, "lod" ) ) );
		else
			m_PairedDPSChecks.push_back( PairedDPSCheck( User, m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser, StatsRealm, "lod" ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !TREESTATS
	//

	else if( (Command == "treestats" || Command == "ts" || Command == "statstree" || Command == "st") && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedTPSChecks.push_back( PairedTPSCheck( string( ), m_GHost->m_DB->ThreadedTreePlayerSummaryCheck( StatsUser, StatsRealm ) ) );
		else
			m_PairedTPSChecks.push_back( PairedTPSCheck( User, m_GHost->m_DB->ThreadedTreePlayerSummaryCheck( StatsUser, StatsRealm ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !SNIPESTATS
	//

	else if( (Command == "snipestats" || Command == "ss" || Command == "sniperstats" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedSPSChecks.push_back( PairedSPSCheck( string( ), m_GHost->m_DB->ThreadedSnipePlayerSummaryCheck( StatsUser, StatsRealm ) ) );
		else
			m_PairedSPSChecks.push_back( PairedSPSCheck( User, m_GHost->m_DB->ThreadedSnipePlayerSummaryCheck( StatsUser, StatsRealm ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !SHIPSTATS
	//

	else if( (Command == "shipstats" || Command == "bs" || Command == "bshipstats" || Command == "bsstats" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedBPSChecks.push_back( PairedBPSCheck( string( ), m_GHost->m_DB->ThreadedShipsPlayerSummaryCheck( StatsUser, StatsRealm ) ) );
		else
			m_PairedBPSChecks.push_back( PairedBPSCheck( User, m_GHost->m_DB->ThreadedShipsPlayerSummaryCheck( StatsUser, StatsRealm ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !CASTLESTATS
	//

	else if( (Command == "castlestats" || Command == "cfstats" || Command == "cfs" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedWPSChecks.push_back( PairedWPSCheck( string( ), m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( StatsUser, StatsRealm, "castlefight" ) ) );
		else
			m_PairedWPSChecks.push_back( PairedWPSCheck( User, m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( StatsUser, StatsRealm, "castlefight" ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !CASTLESTATSHR
	//

	else if( (Command == "castlestatshr" || Command == "cfstatshr" || Command == "cfshr" || Command == "cfs2" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedWPSChecks.push_back( PairedWPSCheck( string( ), m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( StatsUser, StatsRealm, "castlefight2" ) ) );
		else
			m_PairedWPSChecks.push_back( PairedWPSCheck( User, m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( StatsUser, StatsRealm, "castlefight2" ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !CIVSTATS
	//

	else if( (Command == "civstats" || Command == "cwstats" || Command == "cws" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedWPSChecks.push_back( PairedWPSCheck( string( ), m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( StatsUser, StatsRealm, "civwars" ) ) );
		else
			m_PairedWPSChecks.push_back( PairedWPSCheck( User, m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( StatsUser, StatsRealm, "civwars" ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !LEGIONSTATS
	//

	else if( (Command == "legionstats" || Command == "legiontdstats" || Command == "lms" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedWPSChecks.push_back( PairedWPSCheck( string( ), m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( StatsUser, StatsRealm, "legionmega" ) ) );
		else
			m_PairedWPSChecks.push_back( PairedWPSCheck( User, m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( StatsUser, StatsRealm, "legionmega" ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !LEGIONSTATSHR
	//

	else if( (Command == "legionstatshr" || Command == "lmshr" || Command == "lhr" || Command == "ls2" ) && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedWPSChecks.push_back( PairedWPSCheck( string( ), m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( StatsUser, StatsRealm, "legionmega2" ) ) );
		else
			m_PairedWPSChecks.push_back( PairedWPSCheck( User, m_GHost->m_DB->ThreadedW3MMDPlayerSummaryCheck( StatsUser, StatsRealm, "legionmega2" ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !STATS
	//

	else if( Command == "stats" && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		string StatsUser = User;

		if( !Payload.empty( ) )
			StatsUser = Payload;
		
		string StatsRealm = "";
		GetStatsUser( &StatsUser, &StatsRealm );

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedGPSChecks.push_back( PairedGPSCheck( string( ), m_GHost->m_DB->ThreadedGamePlayerSummaryCheck( StatsUser, StatsRealm ) ) );
		else
			m_PairedGPSChecks.push_back( PairedGPSCheck( User, m_GHost->m_DB->ThreadedGamePlayerSummaryCheck( StatsUser, StatsRealm ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !VERSION
	//

	else if( Command == "version" )
	{
		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			SendChat( player, m_GHost->m_Language->VersionAdmin( m_GHost->m_Version ) );
		else
			SendChat( player, m_GHost->m_Language->VersionNotAdmin( m_GHost->m_Version ) );
	}

	//
	// !EAT
	//

	else if( Command == "eat" && !player->GetMuted( ) )
	{
        uint32_t numCookies = player->GetCookies();
        
        if(numCookies > 0) {
            numCookies--;
            player->SetCookies(numCookies);
            SendAllChat( "[" + player->GetName() + "] has eaten a cookie. That was tasty! [" + player->GetName() + "] now has " + UTIL_ToString(numCookies) + " cookies remaining.");
        } else {
            SendChat( player, "Error: you have no cookies in your cookie jar. Ask an admin to give you some with !cookie.");
        }
        HideCommand = true;
	}

	//
	// !NOLAG
	//

	else if( Command == "nolag" && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		player->SetNoLag( !player->GetNoLag( ) );
		
		if( player->GetNoLag( ) )
			SendAllChat( "No lag has been enabled for player [" + player->GetName( ) + "]." );
		else
			SendAllChat( "No lag has been disabled for player [" + player->GetName( ) + "]." );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !IGNORE
	//

	else if( Command == "ignore" && !Payload.empty() )
	{
		CGamePlayer *LastMatch = NULL;
		uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

		if( Matches == 0 )
			SendChat( player, "Unable to ignore player [" + Payload + "]. No matches found." );
		else if( Matches == 1 )
		{
			player->Ignore( LastMatch->GetName( ) );
			SendChat( player, "You have ignored player [" + Payload + "]. You will not be able to send or receive messages from the player." );
		}
		else
			SendChat( player, "Unable to ignore player [" + Payload + "]. Found more than one match." );
	}

	//
	// !UNIGNORE
	//

	else if( Command == "unignore" && !Payload.empty() )
	{
		CGamePlayer *LastMatch = NULL;
		uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

		if( Matches == 0 )
			SendChat( player, "Unable to unignore player [" + Payload + "]. No matches found." );
		else if( Matches == 1 )
		{
			player->UnIgnore( LastMatch->GetName( ) );
			SendChat( player, "You have unignored player [" + Payload + "]." );
		}
		else
			SendChat( player, "Unable to unignore player [" + Payload + "]. Found more than one match." );
	}

	//
	// !SLAP
	//

	else if( (Command == "slap" )&& !Payload.empty() && !m_GHost->m_SlapPhrases.empty() )
	{
	  //pick a phrase
	  uint32_t numPhrases = m_GHost->m_SlapPhrases.size();
	  uint32_t randomPhrase = rand() % numPhrases;
	  
	  string phrase = m_GHost->m_SlapPhrases[randomPhrase];
	  uint32_t nameIndex = phrase.find_first_of("[") + 1;
	  SendAllChat("[" + User + "] " + phrase.insert(nameIndex, Payload));
	  
	  HideCommand = true;
	}

	//
	// !VOTEKICK
	//

	else if( Command == "votekick" && m_GHost->m_VoteKickAllowed && !Payload.empty( ) )
	{
		if( !m_KickVotePlayer.empty( ) )
			SendChat( player, m_GHost->m_Language->UnableToVoteKickAlreadyInProgress( ) );
		else if( m_Players.size( ) <= 3 )
			SendChat( player, m_GHost->m_Language->UnableToVoteKickNotEnoughPlayers( ) );
		else
		{
			CGamePlayer *LastMatch = NULL;
			uint32_t Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

			if( Matches == 0 )
				SendChat( player, m_GHost->m_Language->UnableToVoteKickNoMatchesFound( Payload ) );
			else if( Matches == 1 )
			{
				//see if the player is the only one left on his team
				unsigned char SID = GetSIDFromPID( LastMatch->GetPID( ) );
				bool OnlyPlayer = false;

				if( m_GameLoaded && SID < m_Slots.size( ) )
				{
					unsigned char Team = m_Slots[SID].GetTeam( );
					OnlyPlayer = true;
					char sid, team;
					
					for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++)
					{
						if( *i && LastMatch != *i && !(*i)->GetLeftMessageSent( ) )
						{
							sid = GetSIDFromPID( (*i)->GetPID( ) );
							if( sid != 255 )
							{
								team = m_Slots[sid].GetTeam( );
								if( team == Team )
								{
									OnlyPlayer = false;
									break;
								}
							}			
						}
					}
				}
				
				if( OnlyPlayer )
					SendChat( player, "Unable to votekick player [" + LastMatch->GetName( ) + "]: cannot votekick when there is only one player on victim's team." );
				else if( LastMatch == player )
					SendChat( player, "You cannot votekick yourself!" );
				else
				{
					m_KickVotePlayer = LastMatch->GetName( );
					m_StartedKickVoteTime = GetTime( );

                                        for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); ++i )
						(*i)->SetKickVote( false );

					player->SetKickVote( true );
					CONSOLE_Print( "[GAME: " + m_GameName + "] votekick against player [" + m_KickVotePlayer + "] started by player [" + User + "]" );
					SendAllChat( m_GHost->m_Language->StartedVoteKick( LastMatch->GetName( ), User, UTIL_ToString( (uint32_t)ceil( ( GetNumHumanPlayers( ) - 1 ) * (float)m_GHost->m_VoteKickPercentage / 100 ) - 1 ) ) );
					SendAllChat( m_GHost->m_Language->TypeYesToVote( string( 1, m_GHost->m_CommandTrigger ) ) );
				}
			}
			else
				SendChat( player, m_GHost->m_Language->UnableToVoteKickFoundMoreThanOneMatch( Payload ) );
		}
	}

    //
    // !VOTESTART
    //
    
    bool votestartAuth = player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) );
    bool votestartAutohost = m_GameState == GAME_PUBLIC && !m_GHost->m_AutoHostGameName.empty( ) && m_GHost->m_AutoHostMaximumGames != 0 && m_GHost->m_AutoHostAutoStartPlayers != 0 && m_AutoStartPlayers != 0;
    if( Command == "votestart" && !m_CountDownStarted && (votestartAuth || votestartAutohost || !m_GHost->m_VoteStartAutohostOnly))
        {

            if( !m_GHost->m_CurrentGame->GetLocked( ) )
                {
                    if(m_StartedVoteStartTime == 0) { //need >minplayers or admin to START a votestart
                        if (GetNumHumanPlayers() < m_GHost->m_VoteStartMinPlayers && !votestartAuth) { //need at least eight players to votestart
                            SendChat( player, "You cannot use !votestart until there are " + UTIL_ToString(m_GHost->m_VoteStartMinPlayers) + " or more players in the game!" );
                            return false;
                        }
                    
                        for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); ++i )
                            (*i)->SetStartVote( false );
                        m_StartedVoteStartTime = GetTime();

                        CONSOLE_Print( "[GAME: " + m_GameName + "] votestart started by player [" + User + "]" );
                    }

                    player->SetStartVote(true);
                    
                    uint32_t VotesNeeded = GetNumHumanPlayers( ) - 1;
                    uint32_t Votes = 0;

                    if( VotesNeeded == 1 )
                    	VotesNeeded = 2;
                    
                    for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); ++i )
                        {
                            if( (*i)->GetStartVote( ) )
                                ++Votes;
                        }

                    if(Votes < VotesNeeded) {
                        SendAllChat( UTIL_ToString(VotesNeeded - Votes) + " more votes needed to votestart.");
                    } else {
                        StartCountDown( true );
                    }
                }
            else {
                SendChat( player, "Error: cannot votestart because the game is locked. Owner is " + m_OwnerName );
            }
        }

	//
	// !YES
	//

	else if( Command == "yes" && !m_KickVotePlayer.empty( ) && player->GetName( ) != m_KickVotePlayer && !player->GetKickVote( ) )
	{
		player->SetKickVote( true );
		uint32_t VotesNeeded = (uint32_t)ceil( ( GetNumHumanPlayers( ) - 1 ) * (float)m_GHost->m_VoteKickPercentage / 100 );
		uint32_t Votes = 0;

                for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); ++i )
		{
			if( (*i)->GetKickVote( ) )
                                ++Votes;
		}

		if( Votes >= VotesNeeded )
		{
			CGamePlayer *Victim = GetPlayerFromName( m_KickVotePlayer, true );

			if( Victim )
			{
				Victim->SetDeleteMe( true );
				Victim->SetLeftReason( m_GHost->m_Language->WasKickedByVote( ) );

				if( !m_GameLoading && !m_GameLoaded )
					Victim->SetLeftCode( PLAYERLEAVE_LOBBY );
				else
					Victim->SetLeftCode( PLAYERLEAVE_LOST );

				if( !m_GameLoading && !m_GameLoaded )
					OpenSlot( GetSIDFromPID( Victim->GetPID( ) ), false );

				CONSOLE_Print( "[GAME: " + m_GameName + "] votekick against player [" + m_KickVotePlayer + "] passed with " + UTIL_ToString( Votes ) + "/" + UTIL_ToString( GetNumHumanPlayers( ) ) + " votes" );
				SendAllChat( m_GHost->m_Language->VoteKickPassed( m_KickVotePlayer ) );
				m_GHost->DenyIP( Victim->GetExternalIPString( ), 15000, "votekick passed" );
			}
			else
				SendAllChat( m_GHost->m_Language->ErrorVoteKickingPlayer( m_KickVotePlayer ) );

			m_KickVotePlayer.clear( );
			m_StartedKickVoteTime = 0;
		}
		else
			SendAllChat( m_GHost->m_Language->VoteKickAcceptedNeedMoreVotes( m_KickVotePlayer, User, UTIL_ToString( VotesNeeded - Votes ) ) );
	}
	
	//
	// !DRAW
	//
	if( m_GameLoaded && !m_MapType.empty( ) && ( Command == "draw" || Command == "undraw" ) && !m_SoftGameOver )
	{
		if( Command == "draw" )
		{
			bool ChangedVote = true;
			
			if( !player->GetDrawVote( ) )
				player->SetDrawVote( true );
			else
				ChangedVote = false; //continue in case someone left and now we have enough votes
			
			uint32_t VotesNeeded = (uint32_t)ceil( GetNumHumanPlayers( ) * (float)m_GHost->m_VoteKickPercentage / 100 );
			uint32_t Votes = 0;
			
			for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++)
			{
				if( (*i)->GetDrawVote( ) )
				{
					Votes++;
				}
			}
			
			if( Votes >= VotesNeeded )
			{
				SendAllChat( "The game has now been recorded as a draw. You may leave at any time." );
				m_Stats->LockStats( );
				m_SoftGameOver = true;
			}
			else if( ChangedVote ) //only display message if they actually changed vote
			{
				SendAllChat( "Player [" + player->GetName( ) + "] has voted to draw the game. " + UTIL_ToString( VotesNeeded - Votes ) + " more votes are needed to pass the draw vote." );
				SendChat( player, "Use !undraw to recall your vote to draw the game." );
			}
		}
		else if( Command == "undraw" && player->GetDrawVote( ) )
		{
			player->SetDrawVote( false );
			SendAllChat( "[" + player->GetName( ) + "] recalled vote to draw the game." );
		}
	}
	
	//
	// !FORFEIT
	//
	
	if( m_GameLoaded && m_ForfeitTime == 0 && ( m_MapType == "dota" || m_MapType == "dotaab" || m_MapType == "dota2" || m_MapType == "eihl" ) && ( Command == "ff" || Command == "forfeit" ) && !m_SoftGameOver )
	{
		bool ChangedVote = true;
		
		if( !player->GetForfeitVote( ) )
			player->SetForfeitVote( true );
		else
			ChangedVote = false;
		
		char playerSID = GetSIDFromPID( player->GetPID( ) );
		
		if( playerSID != 255 )
		{
			char playerTeam = m_Slots[playerSID].GetTeam( );
			
			// whether or not all players on the team of the player who typed the command forfeited
			bool AllVoted = true;
			int numVoted = 0;
			int numTotal = 0;

			for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++)
			{
				if( *i && !(*i)->GetLeftMessageSent( ) )
				{
					char sid = GetSIDFromPID( (*i)->GetPID( ) );
					
					if( sid != 255 && m_Slots[sid].GetTeam( ) == playerTeam )
					{
						numTotal++;
						
						if( !(*i)->GetForfeitVote( ) )
							AllVoted = false;
						else
							numVoted++;
					}
				}
			}
			
			m_ForfeitTeam = playerTeam;
			
			// observers cannot forfeit!
			if( m_ForfeitTeam == 0 || m_ForfeitTeam == 1 )
			{
				string ForfeitTeamString = "Sentinel";
				if( m_ForfeitTeam == 1 ) ForfeitTeamString = "Scourge";
			
				if( AllVoted )
				{
					m_Stats->SetWinner( ( playerTeam + 1 ) % 2 );
					m_ForfeitTime = GetTime( );
				
					SendAllChat( m_GHost->m_Language->TeamForfeited( ForfeitTeamString ) );
					SendAllChat( m_GHost->m_Language->ForfeitStatsWarning( ) );
				}
			
				else if( ChangedVote )
				{
					SendAllChat( m_GHost->m_Language->ForfeitVote( player->GetName( ) ) );
					SendAllChat( m_GHost->m_Language->ForfeitVotesNeeded( UTIL_ToString( numVoted ), UTIL_ToString( numTotal ), ForfeitTeamString ) );
				}
			}
		}
	}

	return HideCommand;
}

void CGame :: EventGameStarted( )
{
	CBaseGame :: EventGameStarted( );

	// record everything we need to ban each player in case we decide to do so later
	// this is because when a player leaves the game an admin might want to ban that player
	// but since the player has already left the game we don't have access to their information anymore
	// so we create a "potential ban" for each player and only store it in the database if requested to by an admin

        for( vector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); ++i )
		m_DBBans.push_back( new CDBBan( 0, (*i)->GetJoinedRealm( ), (*i)->GetName( ), (*i)->GetExternalIPString( ), string( ), string( ), string( ), string( ), string( ), string( ), 0 ) );
}

bool CGame :: IsGameDataSaved( )
{
	if( m_CallableGameAdd && m_CallableGameAdd->GetReady( ) )
		m_DatabaseID = m_CallableGameAdd->GetResult();
	
	return m_CallableGameAdd && m_CallableGameAdd->GetReady( );
}

void CGame :: SaveGameData( )
{
	CONSOLE_Print( "[GAME: " + m_GameName + "] saving game data to database" );
	m_CallableGameAdd = m_GHost->m_DB->ThreadedGameAdd( m_GHost->m_BNETs.size( ) == 1 ? m_GHost->m_BNETs[0]->GetServer( ) : string( ), m_DBGame->GetMap( ), m_GameName, m_OwnerName, m_GameTicks / 1000, m_GameState, m_CreatorName, m_CreatorServer, m_Tournament ? "uxtourney" : string( ) );
}

bool CGame :: IsAutoBanned( string name )
{
	for( vector<string> :: iterator i = m_AutoBans.begin( ); i != m_AutoBans.end( ); i++ )
	{
		if( *i == name )
			return true;
	}

	return false;
}

void CGame :: GetStatsUser( string *statsUser, string *statsRealm )
{
	// first set realm based on @ if any
	size_t index = (*statsUser).rfind( '@' );
	
	if( index != string::npos && (*statsUser).size( ) >= index )
	{
		*statsRealm = (*statsUser).substr( index + 1 );
		*statsUser = (*statsUser).substr( 0, index );
		
		// realm to lowercase
		transform( (*statsRealm).begin( ), (*statsRealm).end( ), (*statsRealm).begin( ), (int(*)(int))tolower );
		
		if( *statsRealm == "uswest" )
			*statsRealm = "uswest.battle.net";
		else if( *statsRealm == "useast" )
			*statsRealm = "useast.battle.net";
		else if( *statsRealm == "europe" )
			*statsRealm = "europe.battle.net";
		else if( *statsRealm == "asia" )
			*statsRealm = "asia.battle.net";
		else if( *statsRealm == "ec" )
			*statsRealm = "entconnect";
		else if( *statsRealm == "gclient" )
			*statsRealm = "cloud.ghostclient.com";
	}
	
	else {
		CGamePlayer *player = NULL;
		uint32_t Matches = GetPlayerFromNamePartial( *statsUser, &player );
	
		if( Matches == 1 )
		{
			*statsUser = player->GetName( );
			*statsRealm = player->GetJoinedRealm( );
		}
	}
}