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

#ifndef BNCSUTIL_INTERFACE_H
#define BNCSUTIL_INTERFACE_H

//
// CBNCSUtilInterface
//

class CBNCSUtilInterface
{
private:
	void *m_NLS;
	BYTEARRAY m_EXEVersion;			// set in HELP_SID_AUTH_CHECK
	BYTEARRAY m_EXEVersionHash;		// set in HELP_SID_AUTH_CHECK
	string m_EXEInfo;				// set in HELP_SID_AUTH_CHECK
	BYTEARRAY m_KeyInfoROC;			// set in HELP_SID_AUTH_CHECK
	BYTEARRAY m_KeyInfoTFT;			// set in HELP_SID_AUTH_CHECK
	BYTEARRAY m_ClientKey;			// set in HELP_SID_AUTH_ACCOUNTLOGON
	BYTEARRAY m_M1;					// set in HELP_SID_AUTH_ACCOUNTLOGONPROOF
	BYTEARRAY m_PvPGNPasswordHash;	// set in HELP_PvPGNPasswordHash

public:
	CBNCSUtilInterface( string userName, string userPassword );
	~CBNCSUtilInterface( );

	BYTEARRAY GetEXEVersion( )								{ return m_EXEVersion; }
	BYTEARRAY GetEXEVersionHash( )							{ return m_EXEVersionHash; }
	string GetEXEInfo( )									{ return m_EXEInfo; }
	BYTEARRAY GetKeyInfoROC( )								{ return m_KeyInfoROC; }
	BYTEARRAY GetKeyInfoTFT( )								{ return m_KeyInfoTFT; }
	BYTEARRAY GetClientKey( )								{ return m_ClientKey; }
	BYTEARRAY GetM1( )										{ return m_M1; }
	BYTEARRAY GetPvPGNPasswordHash( )						{ return m_PvPGNPasswordHash; }

	void SetEXEVersion( BYTEARRAY &nEXEVersion )			{ m_EXEVersion = nEXEVersion; }
	void SetEXEVersionHash( BYTEARRAY &nEXEVersionHash )	{ m_EXEVersionHash = nEXEVersionHash; }

	void Reset( string userName, string userPassword );

	bool HELP_SID_AUTH_CHECK( bool TFT, string war3Path, string keyROC, string keyTFT, string valueStringFormula, string mpqFileName, BYTEARRAY clientToken, BYTEARRAY serverToken );
	bool HELP_SID_AUTH_ACCOUNTLOGON( );
	bool HELP_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY salt, BYTEARRAY serverKey );
	bool HELP_PvPGNPasswordHash( string userPassword );

private:
	BYTEARRAY CreateKeyInfo( string key, uint32_t clientToken, uint32_t serverToken );
};

#endif
