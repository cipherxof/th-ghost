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

#ifndef STATSDOTA_H
#define STATSDOTA_H

//
// CStatsDOTA
//

class CDBDotAPlayer;

class CStatsDOTA : public CStats
{
private:
	CDBDotAPlayer *m_Players[12];
	uint32_t m_Winner;
	uint32_t m_Min;
	uint32_t m_Sec;
	string m_SaveType;
	
	// win conditions
	uint32_t m_TowerLimit; // win condition on number of towers destroyed
	uint32_t m_KillLimit; // win condition on number of kills
	uint32_t m_TimeLimit; // time limit win condition; winner is more kills, or if even then higher (creep kills + creep denies) value
	
	uint32_t m_SentinelTowers;
	uint32_t m_ScourgeTowers;
	uint32_t m_SentinelKills;
	uint32_t m_ScourgeKills;
	uint32_t m_LastCreepTime; // last time we received creep stats, for the time limit win condition

public:
	CStatsDOTA( CBaseGame *nGame, string nConditions, string nSaveType );
	virtual ~CStatsDOTA( );

	virtual bool ProcessAction( CIncomingAction *Action );
	virtual void Save( CGHost *GHost, CGHostDB *DB, uint32_t GameID );
	virtual bool IsWinner( ) { return m_Winner != 0; }
	
	// set the winner of the game
	// we add one here because nWinner is the winning team while
	//  m_Winner is 0 for draw, 1 for Sentinel, 2 for Scourge
	virtual void SetWinner( uint32_t nWinner ) { m_Winner = nWinner + 1; }
};

#endif
