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

#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "gameslot.h"

//
// CSaveGame
//

class CSaveGame : public CPacked
{
private:
	string m_FileName;
	string m_FileNameNoPath;
	string m_MapPath;
	string m_GameName;
	unsigned char m_NumSlots;
	vector<CGameSlot> m_Slots;
	uint32_t m_RandomSeed;
	BYTEARRAY m_MagicNumber;

public:
	CSaveGame( );
	virtual ~CSaveGame( );

	string GetFileName( )				{ return m_FileName; }
	string GetFileNameNoPath( )			{ return m_FileNameNoPath; }
	string GetMapPath( )				{ return m_MapPath; }
	string GetGameName( )				{ return m_GameName; }
	unsigned char GetNumSlots( )		{ return m_NumSlots; }
	vector<CGameSlot> GetSlots( )		{ return m_Slots; }
	uint32_t GetRandomSeed( )			{ return m_RandomSeed; }
	BYTEARRAY GetMagicNumber( )			{ return m_MagicNumber; }

	void SetFileName( string nFileName )				{ m_FileName = nFileName; }
	void SetFileNameNoPath( string nFileNameNoPath )	{ m_FileNameNoPath = nFileNameNoPath; }

	void ParseSaveGame( );
};

#endif
