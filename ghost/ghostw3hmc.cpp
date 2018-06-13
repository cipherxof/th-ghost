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
#include "bnet.h"
#include "ghostdb.h"
#include "game_base.h"
#include "ghostw3hmc.h"
#include "game.h"
#include "stats.h"
#include "gameprotocol.h"
#include "gameplayer.h"
#include "map.h"

#include <signal.h>

#ifdef WIN32
 #include <winsock.h>
#endif

#include <mysql/mysql.h>
#include <curl/curl.h>
#include <boost/thread.hpp>
#include <unistd.h>

#define W3HMC_REQUEST_INIT			1
#define W3HMC_REQUEST_HTTP			2
#define W3HMC_REQUEST_PLAYERREALM	3

#define W3HMC_ACTION_SET_ARGS			1
#define W3HMC_ACTION_EXECUTE			2

//
// CGHostDBMySQL
//

CGHostW3HMC :: CGHostW3HMC( CConfig *CFG )
{
	CONSOLE_Print("[W3HMC] using Warcraft 3 Map HostBot Communication version 1");

	m_DebugMode = (CFG->GetInt( "bot_w3hmcdebug", 0 ) == 1);
	m_OutstandingCallables = 0;
}

CGHostW3HMC :: ~CGHostW3HMC( )
{
	for( vector<CCallableDoCURL *> :: iterator i = m_SyncOutgoing.begin( ); i != m_SyncOutgoing.end( ); ++i )
		m_Game->m_GHost->m_Callables.push_back( *i );

	if( m_OutstandingCallables > 0 )
		CONSOLE_Print( "[W3HMC] " + UTIL_ToString( m_OutstandingCallables ) + " outstanding callables were never recovered" );
}

CCallableDoCURL :: ~CCallableDoCURL( )
{
	//delete m_Result;
}

void CGHostW3HMC :: SetMapData( CBaseGame *nGame, string nGCFilename )
{
	m_Game = nGame;
	m_GCFilename = nGCFilename;

	CONSOLE_Print("[W3HMC] game cache filename \"" + m_GCFilename + "\"");
}

void CGHostW3HMC :: RecoverCallable( CBaseCallable *callable )
{
	boost::mutex::scoped_lock lock(m_DatabaseMutex);
	CCallableDoCURL *CURLCallable = dynamic_cast<CCallableDoCURL *>( callable );

	if( CURLCallable )
	{
		if( !CURLCallable->GetError( ).empty( ) )
			CONSOLE_Print( "[W3HMC] CURL error --- " + CURLCallable->GetError( ) );

		if( !CURLCallable->GetError( ).empty( ) )
        	--m_NumRequests;

		if( m_OutstandingCallables == 0 )
			CONSOLE_Print( "[W3HMC] recovered a CURL callable with zero outstanding" );
		else
        	--m_OutstandingCallables;
	}
	else
	{
		CONSOLE_Print( "[W3HMC] tried to recover a non-CURL callable" );
	}
}

void CGHostW3HMC :: CreateThread( CBaseCallable *callable )
{
	try
	{
		boost :: thread Thread( boost :: ref( *callable ) );
	}
	catch( boost :: thread_resource_error tre )
	{
		CONSOLE_Print( "[W3HMC] error spawning thread on attempt #1 [" + string( tre.what( ) ) + "], pausing execution and trying again in 50ms" );
		MILLISLEEP( 50 );

		try
		{
			boost :: thread Thread( boost :: ref( *callable ) );
		}
		catch( boost :: thread_resource_error tre2 )
		{
			CONSOLE_Print( "[W3HMC] error spawning thread on attempt #2 [" + string( tre2.what( ) ) + "], giving up" );
			callable->SetReady( true );
		}
	}
}

void CGHostW3HMC :: SendString ( string msg )
{
	unsigned char triggerID1 = m_Game->m_GHost->m_Map->GetMapW3HMCTriggerID1();
	unsigned char triggerID2 = m_Game->m_GHost->m_Map->GetMapW3HMCTriggerID2();
	unsigned char packetdata[]	= { 0x60, triggerID1, triggerID2, 0x0, 0x0, triggerID1, triggerID2, 0x0, 0x0 };

	BYTEARRAY data;
	BYTEARRAY CRC;
	UTIL_AppendByteArray(data, packetdata, 9);
	UTIL_AppendByteArrayFast(data, msg);

	m_Game->m_Actions.push( new CIncomingAction( m_PID, CRC, data ) );
}

CCallableDoCURL *CGHostW3HMC :: ThreadedCURL( CIncomingAction* action, string args, CBaseGame *game, string reqId, string reqType, uint32_t value, int gcLen )
{
	BYTEARRAY *ActionData = action->GetAction( );
	BYTEARRAY copy = BYTEARRAY( ActionData->begin( ), ActionData->begin( ) + (0 + ( gcLen + 4 ) + reqType.size( ) + reqType.size( )) );

	CCallableDoCURL *Callable = new CCURLCallableDoCURL( action, args, game, copy, reqId );
	CreateThread( Callable );
    ++m_OutstandingCallables;
	return Callable;
}

size_t W3HMC_CURLWriteData(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::map<std::string, std::string> CGHostW3HMC :: ParseArguments( string args )
{
	std::map<std::string, std::string> Arg;
	istringstream SS(args);
	string CurrentKey, LastKey, Args = "";

	while ( getline( SS, CurrentKey, ' ' ) ) 
	{
		if (LastKey.length() > 0) 
			Arg[LastKey] = CurrentKey;

		LastKey = CurrentKey;
	}	

	return Arg;
}

string W3HMC_CURLRequest( string args )
{
	string Result = "";

	curl_global_init(CURL_GLOBAL_ALL);
	CURL* CURLHandle = curl_easy_init();
	struct curl_slist *HeaderList = NULL;
	string CURLStringBuffer;

	// Parse the arguments string
	string URL, ReqParams, CurrentKey, LastKey = "";
	bool IsPost, NoReply = false;
	istringstream SS(args);

	while ( getline( SS, CurrentKey, ' ' ) ) 
	{
		if (LastKey.compare("url") == 0)
			URL = CurrentKey;

		if (LastKey.compare("post") == 0 && CurrentKey == "true")
			IsPost = true;

		if (LastKey.compare("noreply") == 0 && CurrentKey == "true")
			NoReply = true;

		if (LastKey.compare("header") == 0) 
			HeaderList = curl_slist_append(HeaderList, CurrentKey.c_str());

		if (LastKey.compare("params") == 0)
			ReqParams += CurrentKey;

		if (LastKey.compare("realm") == 0) 
			ReqParams += "&hmc_realm=" + CurrentKey;

		if (LastKey.compare("hmcsec") == 0) 
			ReqParams += "&hmc_secure=" + CurrentKey;

		if (LastKey.compare("player_name") == 0) 
			ReqParams += "&hmc_playername=" + CurrentKey;

		LastKey = CurrentKey;
	}

	// Perform the request
	curl_easy_setopt(CURLHandle, CURLOPT_URL, URL.c_str());
	curl_easy_setopt(CURLHandle, CURLOPT_WRITEFUNCTION, W3HMC_CURLWriteData);
	curl_easy_setopt(CURLHandle, CURLOPT_WRITEDATA, &CURLStringBuffer);

	if (HeaderList != NULL)
		curl_easy_setopt(CURLHandle, CURLOPT_HTTPHEADER, HeaderList);

	if (IsPost)
	{
		curl_easy_setopt(CURLHandle, CURLOPT_POST, 1);
		curl_easy_setopt(CURLHandle, CURLOPT_POSTFIELDS, ReqParams.c_str());
	}

	CURLcode CURLResource = curl_easy_perform(CURLHandle);

	curl_slist_free_all(HeaderList);

    if(CURLResource != CURLE_OK) {
    	Result = curl_easy_strerror(CURLResource);
    }
    else{
    	Result = CURLStringBuffer;
    }

    curl_easy_cleanup(CURLHandle);

    if (NoReply)
    	Result = "noreply";
    
	return Result;
}

void CCURLCallableDoCURL :: operator( )( )
{
	CBaseCallable :: Init( );

	m_Result = W3HMC_CURLRequest( m_Args );

	// Clear arguments for the instance
	((CGame*)m_Game)->m_GHost->m_W3HMC->m_Arguments.erase(GetReq());

	CBaseCallable :: Close( );
}

bool CGHostW3HMC :: ProcessAction( CIncomingAction *Action )
{
	if( m_Locked )
		return false;

	unsigned int i = 0;
	BYTEARRAY *ActionData = Action->GetAction( );
	BYTEARRAY MissionKey, Key, Value, GCFile;
	string GCFileStr, Instance;
	int Len = m_GCFilename.length();
	uint32_t ValueInt;

	// Scan each action in the packet
	while( ActionData->size( ) >= i + (Len + 2) )
	{
		// When bot_w3hmcdebug is enabled this will print the values needed for map configs which allows us to spoof chat events
		if (m_DebugMode && (*ActionData)[i] == 0x60)
		{
			m_Game->SendAllChat( "Detected Trigger Chat Event: map_w3hmctid1 = " + UTIL_ToString((*ActionData)[i + 1]) + "; " + "map_w3hmctid2 = " + UTIL_ToString((*ActionData)[i + 2]) + "; " );
		}

		// Attempt to read the gamecache's name
		GCFile = UTIL_ExtractCString( *ActionData, i + 1 );
		GCFileStr = string(GCFile.begin(), GCFile.end());

		// Check for SyncStoredInteger (0x6B)
		if( (*ActionData)[i] == 0x6B && GCFileStr.compare(m_GCFilename) == 0)
		{
			// Attempt to read the mission key
			if( ActionData->size( ) >= i + (Len + 3) )
			{
				MissionKey = UTIL_ExtractCString( *ActionData, i + (Len + 2) );

				// Attempt to read the key
				if( ActionData->size( ) >= i + (Len + 4) + MissionKey.size( ) )
				{
					Key = UTIL_ExtractCString( *ActionData, i + (Len + 3) + MissionKey.size( ) );

					if( ActionData->size( ) >= i + (Len + 8) + MissionKey.size( ) + Key.size( ) )
					{
						// Find the syncing value
						Value = BYTEARRAY( ActionData->begin( ) + (i + (Len + 4) + MissionKey.size( ) + Key.size( )), ActionData->begin( ) + (i + (Len + 8) + MissionKey.size( ) + Key.size( )));
						ValueInt = UTIL_ByteArrayToUInt32(Value, false, 0);

						// The instance # is stored in the mission key of the game cache
						Instance = string(MissionKey.begin(), MissionKey.end());

						// If the instance is already running we ignore duplicate requests.
						// This allows maps to use the player with the lowest latency to process requests.
						if (m_Arguments[Instance].compare("running") == 0)
						{
						}
						// Set the arguments for a request instance. This should be done before the request is executed.
						else if (ValueInt == W3HMC_ACTION_SET_ARGS)
						{
							m_Arguments[Instance] = string(Key.begin(), Key.end());
						}
						// Execute a W3HMC request.
						else if (ValueInt == W3HMC_ACTION_EXECUTE)
						{
							CGamePlayer *player = m_Game->GetPlayerFromPID(Action->GetPID());

							if (player != NULL)
							{
								string ReqIDStr = string(Key.begin(), Key.end());
								int ReqID = atoi(ReqIDStr.c_str());

								std::map<std::string, std::string> Arg = ParseArguments(m_Arguments[Instance]);
								string ArgStr = "";

								switch(ReqID)
								{
									case W3HMC_REQUEST_INIT:
										SendString(Instance + " Ok.");

										break;

									case W3HMC_REQUEST_HTTP:

										if (Arg["realm"].compare("true") != 0)
											ArgStr = m_Arguments[Instance];
										else
											ArgStr = m_Arguments[Instance] + " realm " + player->GetJoinedRealm();

										if (Arg["player_name"].compare("true") == 0)
											ArgStr += " player_name " + player->GetName(); // appends a second player_name (the latest gets used)

										if (Arg["secure"].compare("true") == 0)
											ArgStr += " hmcsec " + m_Game->m_GHost->m_Map->GetMapW3HMCSecret();

										m_SyncOutgoing.push_back(m_Game->m_GHost->m_W3HMC->ThreadedCURL(Action, ArgStr, m_Game, Instance, ReqIDStr, ValueInt, Len));
										m_Arguments[Instance] = "running";

										break;

									case W3HMC_REQUEST_PLAYERREALM:
										player = m_Game->GetPlayerFromPID(Action->GetPID());

										if (player == NULL)
											SendString(Instance + " Invalid PID \"" + Arg["pid"] + "\"");
										else
											SendString(Instance + " " + player->GetJoinedRealm());

										break;
								}
							}
							else
							{
								++i;
							}
						}
					}
				}
			}
		}
		++i;
	}

	return false;
}