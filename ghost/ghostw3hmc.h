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

#ifndef GHOSTW3HMC_H
#define GHOSTW3HMC_H

size_t W3HMC_CURLWriteData(char *contents, size_t size, size_t nmemb, void *userp);

class CCallableDoCURL;

class CGHostW3HMC 
{
private:
	CBaseGame *m_Game;
	string m_GCFilename;
	bool m_Locked;
	bool m_DebugMode;
	uint32_t m_NumRequests;
	uint32_t m_OutstandingCallables;
	boost::mutex m_DatabaseMutex;

public:
	CGHostW3HMC( CConfig *CFG );
	virtual ~CGHostW3HMC( );

	int m_PID;
	vector<CCallableDoCURL *> m_SyncOutgoing;
	std::map<std::string, std::string> m_Arguments;

	virtual void SetMapData( CBaseGame *nGame, string nGCFilename );
	virtual void RecoverCallable( CBaseCallable *callable );
	virtual std::map<std::string, std::string> ParseArguments( string args );
	virtual void SendString ( string msg );
	virtual bool ProcessAction( CIncomingAction *Action );
	virtual void CreateThread( CBaseCallable *callable );
	virtual CCallableDoCURL *ThreadedCURL( CIncomingAction* action, string args, CBaseGame *game, string reqId, string reqType, uint32_t value, int gcLen );
};

class CCallableDoCURL : virtual public CBaseCallable
{
protected:
	CIncomingAction *m_Action;
	string m_Args;
	string m_Result;
	BYTEARRAY m_ActionData;
	string m_SpecialReq;
	CBaseGame *m_Game;

public:
	CCallableDoCURL( CIncomingAction *nAction, string nArgs, CBaseGame *nGame, BYTEARRAY nActionData, string nSpecialReq ) : CBaseCallable( ), m_Action( nAction ), m_Args( nArgs ), m_Game( nGame ), m_ActionData( nActionData ), m_SpecialReq( nSpecialReq ), m_Result( "" ) { }
	virtual ~CCallableDoCURL( );

	virtual string GetArgs( )					{ return m_Args; }
	virtual string GetResult( )					{ return m_Result; }
	virtual CIncomingAction *GetAction( )		{ return m_Action; }
	virtual BYTEARRAY GetActionData( )			{ return m_ActionData; }
	virtual string GetReq( )					{ return m_SpecialReq; }

	virtual void SetResult( string nResult )	{ m_Result = nResult; }
};

class CCURLCallableDoCURL : public CCallableDoCURL
{
public:
	CCURLCallableDoCURL( CIncomingAction *nAction, string nArgs, CBaseGame *nGame, BYTEARRAY nActionData, string nSpecialReq ) : CBaseCallable( ), CCallableDoCURL( nAction, nArgs, nGame, nActionData, nSpecialReq ) { }
	virtual ~CCURLCallableDoCURL( ) { }

	virtual void operator( )( );
};

#endif