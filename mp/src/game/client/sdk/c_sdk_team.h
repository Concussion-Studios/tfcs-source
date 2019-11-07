//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTFTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_SDK_TEAM_H
#define C_SDK_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_team.h"
#include "shareddefs.h"
#include "sdk_playerclass_info_parse.h"

class C_BaseEntity;
class C_BaseObject;
class CBaseTechnology;

//Tony; so we can call this from shared code!
#define CSDKTeam C_SDKTeam

//-----------------------------------------------------------------------------
// Purpose: TF's Team manager
//-----------------------------------------------------------------------------
class C_SDKTeam : public C_Team
{
	DECLARE_CLASS( C_SDKTeam, C_Team );
	DECLARE_CLIENTCLASS();

public:

					C_SDKTeam();
	virtual			~C_SDKTeam();

	virtual char	*Get_Name( void );

	CSDKPlayerClassInfo const &GetPlayerClassInfo( int iPlayerClass ) const;
	const unsigned char *GetEncryptionKey( void ) { return g_pGameRules->GetEncryptionKey(); }

	virtual void AddPlayerClass( const char *pszClassName );

	bool IsClassOnTeam( const char *pszClassName, int &iClassNum ) const;
	bool IsClassOnTeam( int iClassNum ) const;
	int GetNumPlayerClasses( void ) { return m_hPlayerClassInfoHandles.Count(); }

	int CountPlayersOfThisClass( int iPlayerClass );

private:
	CUtlVector < PLAYERCLASS_FILE_INFO_HANDLE >		m_hPlayerClassInfoHandles;
};

class C_SDKTeam_Unassigned : public C_SDKTeam
{
	DECLARE_CLASS( C_SDKTeam_Unassigned, C_SDKTeam );
public:
	DECLARE_CLIENTCLASS();

				     C_SDKTeam_Unassigned();
	 virtual		~C_SDKTeam_Unassigned() {}
};

class C_SDKTeam_Blue : public C_SDKTeam
{
	DECLARE_CLASS( C_SDKTeam_Blue, C_SDKTeam );
public:
	DECLARE_CLIENTCLASS();

				     C_SDKTeam_Blue();
	 virtual		~C_SDKTeam_Blue() {}
};

class C_SDKTeam_Red : public C_SDKTeam
{
	DECLARE_CLASS( C_SDKTeam_Red, C_SDKTeam );
public:
	DECLARE_CLIENTCLASS();

					 C_SDKTeam_Red();
	virtual			~C_SDKTeam_Red() {}
};

class C_SDKTeam_Green : public C_SDKTeam
{
	DECLARE_CLASS( C_SDKTeam_Green, C_SDKTeam );
public:
	DECLARE_CLIENTCLASS();

				     C_SDKTeam_Green();
	 virtual		~C_SDKTeam_Green() {}
};

class C_SDKTeam_Yellow : public C_SDKTeam
{
	DECLARE_CLASS( C_SDKTeam_Yellow, C_SDKTeam );
public:
	DECLARE_CLIENTCLASS();

					 C_SDKTeam_Yellow();
	virtual			~C_SDKTeam_Yellow() {}
};

extern C_SDKTeam *GetGlobalSDKTeam( int iIndex );

#endif // C_SDK_TEAM_H
