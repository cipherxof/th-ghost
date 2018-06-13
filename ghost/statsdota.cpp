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
#include "ghostdb.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game_base.h"
#include "stats.h"
#include "statsdota.h"

//
// CStatsDOTA
//

CStatsDOTA :: CStatsDOTA( CBaseGame *nGame, string nConditions, string nSaveType ) : CStats( nGame ), m_SaveType( nSaveType ), m_Winner( 0 ), m_Min( 0 ), m_Sec( 0 ), m_TowerLimit( false ), m_KillLimit( 0 ), m_TimeLimit( 0 ), m_SentinelTowers( 0 ), m_ScourgeTowers( 0 ), m_SentinelKills( 0 ), m_ScourgeKills( 0 ), m_LastCreepTime( 0 )
{
	CONSOLE_Print( "[STATSDOTA] using dota stats" );

        for( unsigned int i = 0; i < 12; ++i )
		m_Players[i] = NULL;
	
	// process the win conditions
	if( !nConditions.empty( ) )
	{
		stringstream SS;
		SS << nConditions;

		while( !SS.eof( ) )
		{
			string Condition;
			SS >> Condition;

			if( SS.fail( ) )
			{
				CONSOLE_Print( "[STATSDOTA] failed to process win conditions: " + nConditions );
				break;
			}
			else if( Condition.length( ) >= 2 )
			{
				string Key = Condition.substr( 0, 2 );
				string Value = Condition.substr( 2 );
				
				if( Key == "tw" )
					m_TowerLimit = UTIL_ToUInt32( Value );
				else if( Key == "ki" )
					m_KillLimit = UTIL_ToUInt32( Value );
				else if( Key == "tm" )
					m_TimeLimit = UTIL_ToUInt32( Value );
			}
		}
	}
}

CStatsDOTA :: ~CStatsDOTA( )
{
        for( unsigned int i = 0; i < 12; ++i )
	{
		if( m_Players[i] )
			delete m_Players[i];
	}
}

bool CStatsDOTA :: ProcessAction( CIncomingAction *Action )
{
	if( m_Locked )
		return m_Winner != 0;

	unsigned int i = 0;
	BYTEARRAY *ActionData = Action->GetAction( );
	BYTEARRAY Data;
	BYTEARRAY Key;
	BYTEARRAY Value;

	// dota actions with real time replay data start with 0x6b then the null terminated string "dr.x"
	// unfortunately more than one action can be sent in a single packet and the length of each action isn't explicitly represented in the packet
	// so we have to either parse all the actions and calculate the length based on the type or we can search for an identifying sequence
	// parsing the actions would be more correct but would be a lot more difficult to write for relatively little gain
	// so we take the easy route (which isn't always guaranteed to work) and search the data for the sequence "6b 64 72 2e 78 00" and hope it identifies an action

	while( ActionData->size( ) >= i + 6 )
	{
		if( (*ActionData)[i] == 0x6b && (*ActionData)[i + 1] == 0x64 && (*ActionData)[i + 2] == 0x72 && (*ActionData)[i + 3] == 0x2e && (*ActionData)[i + 4] == 0x78 && (*ActionData)[i + 5] == 0x00 )
		{
			// we think we've found an action with real time replay data (but we can't be 100% sure)
			// next we parse out two null terminated strings and a 4 byte integer

			if( ActionData->size( ) >= i + 7 )
			{
				// the first null terminated string should either be the strings "Data" or "Global" or a player id in ASCII representation, e.g. "1" or "2"

				Data = UTIL_ExtractCString( *ActionData, i + 6 );

				if( ActionData->size( ) >= i + 8 + Data.size( ) )
				{
					// the second null terminated string should be the key

					Key = UTIL_ExtractCString( *ActionData, i + 7 + Data.size( ) );

					if( ActionData->size( ) >= i + 12 + Data.size( ) + Key.size( ) )
					{
						// the 4 byte integer should be the value

						Value = BYTEARRAY( ActionData->begin( ) + i + 8 + Data.size( ) + Key.size( ), ActionData->begin( ) + i + 12 + Data.size( ) + Key.size( ) );
						string DataString = string( Data.begin( ), Data.end( ) );
						string KeyString = string( Key.begin( ), Key.end( ) );
						uint32_t ValueInt = UTIL_ByteArrayToUInt32( Value, false );

						// CONSOLE_Print( "[STATS] " + DataString + ", " + KeyString + ", " + UTIL_ToString( ValueInt ) );

						if( DataString == "Data" )
						{
							// these are received during the game
							// you could use these to calculate killing sprees and double or triple kills (you'd have to make up your own time restrictions though)
							// you could also build a table of "who killed who" data

							if( KeyString.size( ) >= 5 && KeyString.substr( 0, 4 ) == "Hero" )
							{
								// a hero died

								string VictimColourString = KeyString.substr( 4 );
								uint32_t VictimColour = UTIL_ToUInt32( VictimColourString );
								CGamePlayer *Killer = m_Game->GetPlayerFromColour( ValueInt );
								CGamePlayer *Victim = m_Game->GetPlayerFromColour( VictimColour );
								
								if( Killer && Victim )
								{
									if( ( ValueInt >= 1 && ValueInt <= 5 ) || ( ValueInt >= 7 && ValueInt <= 11 ) )
									{
										if ((ValueInt <= 5 && VictimColour <= 5) || (ValueInt >= 7 && VictimColour >= 7))
										{
											// He denied a team-mate, don't count that.
										}
										else
										{
											// A legit kill, lets count that.
						
											if (!m_Players[ValueInt])
												m_Players[ValueInt] = new CDBDotAPlayer( );

											if (ValueInt != VictimColour)
												m_Players[ValueInt]->SetKills( m_Players[ValueInt]->GetKills() + 1 );
											
											if( ValueInt >= 1 && ValueInt <= 5 )
												m_SentinelKills++;
											else
												m_ScourgeKills++;
										}
									}
								
									if( ( VictimColour >= 1 && VictimColour <= 5 ) || ( VictimColour >= 7 && VictimColour <= 11 ) )
									{
										if (!m_Players[VictimColour])
											m_Players[VictimColour] = new CDBDotAPlayer( );
										
										m_Players[VictimColour]->SetDeaths( m_Players[VictimColour]->GetDeaths() + 1 );
									}
									
									CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] player [" + Killer->GetName( ) + "] killed player [" + Victim->GetName( ) + "]" );
								}
								
								else if( Victim )
								{
									
									if( ( VictimColour >= 1 && VictimColour <= 5 ) || ( VictimColour >= 7 && VictimColour <= 11 ) )
									{
										if (!m_Players[VictimColour])
											m_Players[VictimColour] = new CDBDotAPlayer( );
			
										m_Players[VictimColour]->SetDeaths( m_Players[VictimColour]->GetDeaths() + 1 );
									}
									
									if( ValueInt == 0 )
									{
										m_SentinelKills++;
										CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] the Sentinel killed player [" + Victim->GetName( ) + "]" );
									}
									else if( ValueInt == 6 )
									{
										m_ScourgeKills++;
										CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] the Scourge killed player [" + Victim->GetName( ) + "]" );
									}
								}
							}
							else if( KeyString.size( ) >= 8 && KeyString.substr( 0, 7 ) == "Courier" )
							{
								// a courier died

								if( ( ValueInt >= 1 && ValueInt <= 5 ) || ( ValueInt >= 7 && ValueInt <= 11 ) )
								{
									if( !m_Players[ValueInt] )
										m_Players[ValueInt] = new CDBDotAPlayer( );

									m_Players[ValueInt]->SetCourierKills( m_Players[ValueInt]->GetCourierKills( ) + 1 );
								}

								string VictimColourString = KeyString.substr( 7 );
								uint32_t VictimColour = UTIL_ToUInt32( VictimColourString );
								CGamePlayer *Killer = m_Game->GetPlayerFromColour( ValueInt );
								CGamePlayer *Victim = m_Game->GetPlayerFromColour( VictimColour );

								if( Killer && Victim )
									CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] player [" + Killer->GetName( ) + "] killed a courier owned by player [" + Victim->GetName( ) + "]" );
								else if( Victim )
								{
									if( ValueInt == 0 )
										CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] the Sentinel killed a courier owned by player [" + Victim->GetName( ) + "]" );
									else if( ValueInt == 6 )
										CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] the Scourge killed a courier owned by player [" + Victim->GetName( ) + "]" );
								}
							}
							else if( KeyString.size( ) >= 8 && KeyString.substr( 0, 5 ) == "Tower" )
							{
								// a tower died

								if( ( ValueInt >= 1 && ValueInt <= 5 ) || ( ValueInt >= 7 && ValueInt <= 11 ) )
								{
									if( !m_Players[ValueInt] )
										m_Players[ValueInt] = new CDBDotAPlayer( );

									m_Players[ValueInt]->SetTowerKills( m_Players[ValueInt]->GetTowerKills( ) + 1 );
								}

								string Alliance = KeyString.substr( 5, 1 );
								string Level = KeyString.substr( 6, 1 );
								string Side = KeyString.substr( 7, 1 );
								CGamePlayer *Killer = m_Game->GetPlayerFromColour( ValueInt );
								string AllianceString;
								string SideString;

								if( Alliance == "0" )
								{
									m_ScourgeTowers++;
									AllianceString = "Sentinel";
								}
								else if( Alliance == "1" )
								{
									m_SentinelTowers++;
									AllianceString = "Scourge";
								}
								else
									AllianceString = "unknown";

								if( Side == "0" )
									SideString = "top";
								else if( Side == "1" )
									SideString = "mid";
								else if( Side == "2" )
									SideString = "bottom";
								else
									SideString = "unknown";

								if( Killer )
									CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] player [" + Killer->GetName( ) + "] destroyed a level [" + Level + "] " + AllianceString + " tower (" + SideString + ")" );
								else
								{
									if( ValueInt == 0 )
										CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] the Sentinel destroyed a level [" + Level + "] " + AllianceString + " tower (" + SideString + ")" );
									else if( ValueInt == 6 )
										CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] the Scourge destroyed a level [" + Level + "] " + AllianceString + " tower (" + SideString + ")" );
								}
							}
							else if( KeyString.size( ) >= 6 && KeyString.substr( 0, 3 ) == "Rax" )
							{
								// a rax died

								if( ( ValueInt >= 1 && ValueInt <= 5 ) || ( ValueInt >= 7 && ValueInt <= 11 ) )
								{
									if( !m_Players[ValueInt] )
										m_Players[ValueInt] = new CDBDotAPlayer( );

									m_Players[ValueInt]->SetRaxKills( m_Players[ValueInt]->GetRaxKills( ) + 1 );
								}

								string Alliance = KeyString.substr( 3, 1 );
								string Side = KeyString.substr( 4, 1 );
								string Type = KeyString.substr( 5, 1 );
								CGamePlayer *Killer = m_Game->GetPlayerFromColour( ValueInt );
								string AllianceString;
								string SideString;
								string TypeString;

								if( Alliance == "0" )
									AllianceString = "Sentinel";
								else if( Alliance == "1" )
									AllianceString = "Scourge";
								else
									AllianceString = "unknown";

								if( Side == "0" )
									SideString = "top";
								else if( Side == "1" )
									SideString = "mid";
								else if( Side == "2" )
									SideString = "bottom";
								else
									SideString = "unknown";

								if( Type == "0" )
									TypeString = "melee";
								else if( Type == "1" )
									TypeString = "ranged";
								else
									TypeString = "unknown";

								if( Killer )
									CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] player [" + Killer->GetName( ) + "] destroyed a " + TypeString + " " + AllianceString + " rax (" + SideString + ")" );
								else
								{
									if( ValueInt == 0 )
										CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] the Sentinel destroyed a " + TypeString + " " + AllianceString + " rax (" + SideString + ")" );
									else if( ValueInt == 6 )
										CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] the Scourge destroyed a " + TypeString + " " + AllianceString + " rax (" + SideString + ")" );
								}
							}
							else if( KeyString.size( ) >= 6 && KeyString.substr( 0, 6 ) == "Throne" )
							{
								// the frozen throne got hurt

								CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] the Frozen Throne is now at " + UTIL_ToString( ValueInt ) + "% HP" );
							}
							else if( KeyString.size( ) >= 4 && KeyString.substr( 0, 4 ) == "Tree" )
							{
								// the world tree got hurt

								CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] the World Tree is now at " + UTIL_ToString( ValueInt ) + "% HP" );
							}
							else if( KeyString.size( ) >= 2 && KeyString.substr( 0, 2 ) == "CK" )
							{
								// a player disconnected
							}
							else if( KeyString.size( ) >= 3 && KeyString.substr( 0, 3 ) == "CSK" )
							{       
								// creep kill value recieved (aprox every 3 - 4)
								string PlayerID = KeyString.substr( 3 );
								uint32_t ID = UTIL_ToUInt32( PlayerID );
								
								if( ( ID >= 1 && ID <= 5 ) || ( ID >= 7 && ID <= 11 ) )
								{
									if (!m_Players[ID])
										m_Players[ID] = new CDBDotAPlayer( );
													
									m_Players[ID]->SetCreepKills(ValueInt);
								}
								
								m_LastCreepTime = GetTime( );
							}
							else if( KeyString.size( ) >= 3 && KeyString.substr( 0, 3 ) == "CSD" )
							{
								// creep denie value recieved (aprox every 3 - 4)
								string PlayerID = KeyString.substr( 3 );
								uint32_t ID = UTIL_ToUInt32( PlayerID );
								
								if( ( ID >= 1 && ID <= 5 ) || ( ID >= 7 && ID <= 11 ) )
								{
									
									if (!m_Players[ID])
										m_Players[ID] = new CDBDotAPlayer( );
								
									m_Players[ID]->SetCreepDenies(ValueInt);
								}
								
								m_LastCreepTime = GetTime( );
							}
							else if( KeyString.size( ) >= 2 && KeyString.substr( 0, 2 ) == "NK" )
							{
								// creep denie value recieved (aprox every 3 - 4)
								string PlayerID = KeyString.substr( 2 );
								uint32_t ID = UTIL_ToUInt32( PlayerID );
								
								if( ( ID >= 1 && ID <= 5 ) || ( ID >= 7 && ID <= 11 ) )
								{
									if (!m_Players[ID])
										m_Players[ID] = new CDBDotAPlayer( );
											
									m_Players[ID]->SetNeutralKills(ValueInt);
								}
							}
							else if( KeyString.size( ) >= 7 && KeyString.substr( 0, 6 ) == "Assist" )
							{
								string AssistString = KeyString.substr( 6 );
								uint32_t Assist = UTIL_ToUInt32(AssistString);
								
								CGamePlayer *Player = m_Game->GetPlayerFromColour( Assist );
								CGamePlayer *Victim = m_Game->GetPlayerFromColour( ValueInt );

								if (Player && Victim)
								{
									if( ( Assist >= 1 && Assist <= 5 ) || ( Assist >= 7 && Assist <= 11 ) )
									{
										if (!m_Players[Assist])
											m_Players[Assist] = new CDBDotAPlayer( );

										m_Players[Assist]->SetAssists( m_Players[Assist]->GetAssists() + 1 );
									}
									//CONSOLE_Print( "[OBSERVER: " + m_Game->GetGameName( ) + "] Assist detected on team " + UTIL_ToString(Player->GetTeam()) + " by: " + Player->GetName() );
								}
							}
						}
						else if( DataString == "Global" )
						{
							// these are only received at the end of the game

							if( KeyString == "Winner" && m_Winner != 1 && m_Winner != 2 )
							{
								// Value 1 -> sentinel
								// Value 2 -> scourge

								m_Winner = ValueInt;

								if( m_Winner == 1 )
									CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] detected winner: Sentinel" );
								else if( m_Winner == 2 )
									CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] detected winner: Scourge" );
								else
									CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] detected winner: " + UTIL_ToString( ValueInt ) );
							}
							else if( KeyString == "m" )
								m_Min = ValueInt;
							else if( KeyString == "s" )
								m_Sec = ValueInt;
						}
						else if( DataString.size( ) <= 2 && DataString.find_first_not_of( "1234567890" ) == string :: npos )
						{
							// these are only received at the end of the game

							uint32_t ID = UTIL_ToUInt32( DataString );

							if( ( ID >= 1 && ID <= 5 ) || ( ID >= 7 && ID <= 11 ) )
							{
								if( !m_Players[ID] )
								{
									m_Players[ID] = new CDBDotAPlayer( );
									m_Players[ID]->SetColour( ID );
								}

								// Key "1"		-> Kills
								// Key "2"		-> Deaths
								// Key "3"		-> Creep Kills
								// Key "4"		-> Creep Denies
								// Key "5"		-> Assists
								// Key "6"		-> Current Gold
								// Key "7"		-> Neutral Kills
								// Key "8_0"	-> Item 1
								// Key "8_1"	-> Item 2
								// Key "8_2"	-> Item 3
								// Key "8_3"	-> Item 4
								// Key "8_4"	-> Item 5
								// Key "8_5"	-> Item 6
								// Key "id"		-> ID (1-5 for sentinel, 6-10 for scourge, accurate after using -sp and/or -switch)

								if( KeyString == "1" )
									m_Players[ID]->SetKills( ValueInt );
								else if( KeyString == "2" )
									m_Players[ID]->SetDeaths( ValueInt );
								else if( KeyString == "3" )
									m_Players[ID]->SetCreepKills( ValueInt );
								else if( KeyString == "4" )
									m_Players[ID]->SetCreepDenies( ValueInt );
								else if( KeyString == "5" )
									m_Players[ID]->SetAssists( ValueInt );
								else if( KeyString == "6" )
									m_Players[ID]->SetGold( ValueInt );
								else if( KeyString == "7" )
									m_Players[ID]->SetNeutralKills( ValueInt );
								else if( KeyString == "8_0" )
									m_Players[ID]->SetItem( 0, string( Value.rbegin( ), Value.rend( ) ) );
								else if( KeyString == "8_1" )
									m_Players[ID]->SetItem( 1, string( Value.rbegin( ), Value.rend( ) ) );
								else if( KeyString == "8_2" )
									m_Players[ID]->SetItem( 2, string( Value.rbegin( ), Value.rend( ) ) );
								else if( KeyString == "8_3" )
									m_Players[ID]->SetItem( 3, string( Value.rbegin( ), Value.rend( ) ) );
								else if( KeyString == "8_4" )
									m_Players[ID]->SetItem( 4, string( Value.rbegin( ), Value.rend( ) ) );
								else if( KeyString == "8_5" )
									m_Players[ID]->SetItem( 5, string( Value.rbegin( ), Value.rend( ) ) );
								else if( KeyString == "9" )
									m_Players[ID]->SetHero( string( Value.rbegin( ), Value.rend( ) ) );
								else if( KeyString == "id" )
								{
									// DotA sends id values from 1-10 with 1-5 being sentinel players and 6-10 being scourge players
									// unfortunately the actual player colours are from 1-5 and from 7-11 so we need to deal with this case here

									if( ValueInt >= 6 )
										m_Players[ID]->SetNewColour( ValueInt + 1 );
									else
										m_Players[ID]->SetNewColour( ValueInt );
								}
							}
						}

						i += 12 + Data.size( ) + Key.size( );
					}
					else
                                                ++i;
				}
				else
                                        ++i;
			}
			else
                                ++i;
		}
		else
                        ++i;
	}
	
	// set winner if any win conditions have been met
	if( m_Winner == 0 )
	{
		if( m_KillLimit != 0 )
		{
			if( m_SentinelKills >= m_KillLimit )
			{
				m_Winner = 1;
				return true;
			}
			else if( m_ScourgeKills >= m_KillLimit )
			{
				m_Winner = 2;
				return true;
			}
		}
		
		if( m_TowerLimit != 0)
		{
			if( m_SentinelTowers >= m_TowerLimit )
			{
				m_Winner = 1;
				return true;
			}
			else if( m_ScourgeTowers >= m_TowerLimit )
			{
				m_Winner = 2;
				return true;
			}
		}
		
		if( m_TimeLimit != 0 && m_Game->GetGameTicks( ) > m_TimeLimit * 1000 )
		{
			// we must determine a winner at this point
			// or at least we must try...!
			
			if( m_SentinelKills > m_ScourgeKills )
			{
				m_Winner = 1;
				return true;
			}
			else if( m_SentinelKills < m_ScourgeKills )
			{
				m_Winner = 2;
				return true;
			}
			
			// ok, base on creep kills then?
			uint32_t SentinelTotal = 0;
			uint32_t ScourgeTotal = 0;
			
			for( unsigned int i = 0; i < 12; ++i )
			{
				if( m_Players[i] )
				{
					uint32_t Colour = i;
					
					if( m_Players[i]->GetNewColour( ) != 0 )
						Colour = m_Players[i]->GetNewColour( );

					if( Colour >= 1 && Colour <= 5 )
						SentinelTotal += m_Players[i]->GetCreepKills( ) + m_Players[i]->GetCreepDenies( );
					if( Colour >= 7 && Colour <= 11 )
						ScourgeTotal += m_Players[i]->GetCreepKills( ) + m_Players[i]->GetCreepDenies( );
				}
			}
			
			if( SentinelTotal > ScourgeTotal )
			{
				m_Winner = 1;
				return true;
			}
			else if( SentinelTotal < ScourgeTotal )
			{
				m_Winner = 2;
				return true;
			}
		}
	}

	return m_Winner != 0;
}

void CStatsDOTA :: Save( CGHost *GHost, CGHostDB *DB, uint32_t GameID )
{
	if( DB->Begin( ) )
	{
		// since we only record the end game information it's possible we haven't recorded anything yet if the game didn't end with a tree/throne death
		// this will happen if all the players leave before properly finishing the game
		// the dotagame stats are always saved (with winner = 0 if the game didn't properly finish)
		// the dotaplayer stats are only saved if the game is properly finished

		unsigned int Players = 0;

		// save the dotagame
		// no need to ask for lock on callables mutex: we already have it from CGame
		
		GHost->m_Callables.push_back( DB->ThreadedDotAGameAdd( GameID, m_Winner, m_Min, m_Sec, m_SaveType ) );

		// check for invalid colours and duplicates
		// this can only happen if DotA sends us garbage in the "id" value but we should check anyway

                for( unsigned int i = 0; i < 12; ++i )
		{
			if( m_Players[i] )
			{
				uint32_t Colour = m_Players[i]->GetNewColour( );

				if( !( ( Colour >= 1 && Colour <= 5 ) || ( Colour >= 7 && Colour <= 11 ) ) )
				{
					CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] discarding player data, invalid colour found" );
					DB->Commit( );
					return;
				}

                                for( unsigned int j = i + 1; j < 12; ++j )
				{
					if( m_Players[j] && Colour == m_Players[j]->GetNewColour( ) )
					{
						CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] discarding player data, duplicate colour found" );
						DB->Commit( );
						return;
					}
				}
			}
		}

		// save the dotaplayers
		// no need to ask for lock on callables, we already have it from CGame
		
                for( unsigned int i = 0; i < 12; ++i )
		{
			if( m_Players[i] )
			{
				GHost->m_Callables.push_back( DB->ThreadedDotAPlayerAdd( GameID, m_Players[i]->GetColour( ), m_Players[i]->GetKills( ), m_Players[i]->GetDeaths( ), m_Players[i]->GetCreepKills( ), m_Players[i]->GetCreepDenies( ), m_Players[i]->GetAssists( ), m_Players[i]->GetGold( ), m_Players[i]->GetNeutralKills( ), m_Players[i]->GetItem( 0 ), m_Players[i]->GetItem( 1 ), m_Players[i]->GetItem( 2 ), m_Players[i]->GetItem( 3 ), m_Players[i]->GetItem( 4 ), m_Players[i]->GetItem( 5 ), m_Players[i]->GetHero( ), m_Players[i]->GetNewColour( ), m_Players[i]->GetTowerKills( ), m_Players[i]->GetRaxKills( ), m_Players[i]->GetCourierKills( ), m_SaveType ) );
                                ++Players;
			}
		}

		if( DB->Commit( ) )
			CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] saving " + UTIL_ToString( Players ) + " players" );
		else
			CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] unable to commit database transaction, data not saved" );
	}
	else
		CONSOLE_Print( "[STATSDOTA: " + m_Game->GetGameName( ) + "] unable to begin database transaction, data not saved" );
}
