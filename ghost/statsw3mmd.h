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

#ifndef STATSW3MMD_H
#define STATSW3MMD_H

//
// CStatsW3MMD
//

typedef pair<uint32_t,string> VarP;

class CStatsW3MMD : public CStats
{
private:
	string m_Category;
	string m_SaveType;
	uint32_t m_NextValueID;
	uint32_t m_NextCheckID;
	map<uint32_t,string> m_PIDToName;			// pid -> player name (e.g. 0 -> "Varlock") --- note: will not be automatically converted to lower case
	map<uint32_t,string> m_Flags;				// pid -> flag (e.g. 0 -> "winner")
	map<uint32_t,bool> m_FlagsLeaver;			// pid -> leaver flag (e.g. 0 -> true) --- note: will only be present if true
	map<uint32_t,bool> m_FlagsPracticing;		// pid -> practice flag (e.g. 0 -> true) --- note: will only be present if true
	map<string,string> m_DefVarPs;				// varname -> value type (e.g. "kills" -> "int")
	map<VarP,int32_t> m_VarPInts;				// pid,varname -> value (e.g. 0,"kills" -> 5)
	map<VarP,double> m_VarPReals;				// pid,varname -> value (e.g. 0,"x" -> 0.8)
	map<VarP,string> m_VarPStrings;				// pid,varname -> value (e.g. 0,"hero" -> "heroname")
	map<string, vector<string> > m_DefEvents;	// event -> vector of arguments + format

public:
	CStatsW3MMD( CBaseGame *nGame, string nCategory, string nSaveType );
	virtual ~CStatsW3MMD( );

	virtual bool ProcessAction( CIncomingAction *Action );
	virtual void Save( CGHost *GHost, CGHostDB *DB, uint32_t GameID );
	virtual vector<string> TokenizeKey( string key );
	virtual bool IsWinner( );
	virtual void SetWinner( uint32_t nWinner );
};

#endif
