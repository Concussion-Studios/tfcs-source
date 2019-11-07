//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "sdk_team.h"
#include "entitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// Datatable
IMPLEMENT_SERVERCLASS_ST(CSDKTeam, DT_SDKTeam)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( sdk_team_manager, CSDKTeam );

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to the specified TF team manager
//-----------------------------------------------------------------------------
CSDKTeam *GetGlobalSDKTeam( int iIndex )
{
	return (CSDKTeam*)GetGlobalTeam( iIndex );
}


//-----------------------------------------------------------------------------
// Purpose: Needed because this is an entity, but should never be used
//-----------------------------------------------------------------------------
void CSDKTeam::Init( const char *pName, int iNumber )
{
	BaseClass::Init( pName, iNumber );

	// Only detect changes every half-second.
	NetworkProp()->SetUpdateInterval( 0.75f );
}

void CSDKTeam::AddPlayerClass( const char *szClassName )
{
	PLAYERCLASS_FILE_INFO_HANDLE hPlayerClassInfo;

	if ( ReadPlayerClassDataFromFileForSlot( filesystem, szClassName, &hPlayerClassInfo, GetEncryptionKey() ) )
	{
		m_hPlayerClassInfoHandles.AddToTail( hPlayerClassInfo );
	}
	else
	{
		Assert( !"missing playerclass script file" );
		Msg( "Missing playerclass script file for class: %s\n", szClassName );
	}	
}

const CSDKPlayerClassInfo &CSDKTeam::GetPlayerClassInfo( int iPlayerClass ) const
{
	Assert( iPlayerClass >= 0 && iPlayerClass < m_hPlayerClassInfoHandles.Count() );

	const FilePlayerClassInfo_t *pPlayerClassInfo = GetFilePlayerClassInfoFromHandle( m_hPlayerClassInfoHandles[iPlayerClass] );
	const CSDKPlayerClassInfo *pSDKInfo;

	#ifdef _DEBUG
		pSDKInfo = dynamic_cast< const CSDKPlayerClassInfo* >( pPlayerClassInfo );
		Assert( pSDKInfo );
	#else
		pSDKInfo = static_cast< const CSDKPlayerClassInfo* >( pPlayerClassInfo );
	#endif

	return *pSDKInfo;
}

bool CSDKTeam::IsClassOnTeam( const char *pszClassName, int &iClassNum ) const
{
	iClassNum = PLAYERCLASS_UNDEFINED;

	// Random is always on every team
	if( FStrEq( pszClassName, "cls_random" ) )
	{
		iClassNum = PLAYERCLASS_RANDOM;
		return true;
	}
	
	for( int i=0;i<m_hPlayerClassInfoHandles.Count(); i++ )
	{
		FilePlayerClassInfo_t *pPlayerClassInfo = GetFilePlayerClassInfoFromHandle( m_hPlayerClassInfoHandles[i] );

		if( stricmp( pszClassName, pPlayerClassInfo->m_szSelectCmd ) == 0 )
		{
			iClassNum = i;
			return true;
		}
	}

	return false;
}

void CSDKTeam::ResetScores( void )
{
	SetRoundsWon(0);
	SetScore(0);
}

// BLUE TEAM
//==================
class CSDKTeam_Blue : public CSDKTeam
{
	DECLARE_CLASS( CSDKTeam_Blue, CSDKTeam );
	DECLARE_SERVERCLASS();

	virtual void Init( const char *pName, int iNumber )
	{
		BaseClass::Init( pName, iNumber );

		int i = 0;
		while( pszPlayerClasses[i] != NULL )
		{
			AddPlayerClass( pszPlayerClasses[i] );
			i++;
		}
	}

	virtual const char *GetTeamName( void ) { return "#Teamname_Blue"; }
};

IMPLEMENT_SERVERCLASS_ST(CSDKTeam_Blue, DT_SDKTeam_Blue)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( sdk_team_blue, CSDKTeam_Blue );


// RED TEAM
//==================
class CSDKTeam_Red : public CSDKTeam
{
	DECLARE_CLASS( CSDKTeam_Red, CSDKTeam );
	DECLARE_SERVERCLASS();

	virtual void Init( const char *pName, int iNumber )
	{
		BaseClass::Init( pName, iNumber );

		int i = 0;
		while( pszPlayerClasses[i] != NULL )
		{
			AddPlayerClass( pszPlayerClasses[i] );
			i++;
		}
	}

	virtual const char *GetTeamName( void ) { return "#Teamname_Red"; }
};

IMPLEMENT_SERVERCLASS_ST(CSDKTeam_Red, DT_SDKTeam_Red)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( sdk_team_red, CSDKTeam_Red );

// GREEN TEAM
//==================
class CSDKTeam_Green : public CSDKTeam
{
	DECLARE_CLASS( CSDKTeam_Green, CSDKTeam );
	DECLARE_SERVERCLASS();

	virtual void Init( const char *pName, int iNumber )
	{
		BaseClass::Init( pName, iNumber );

		int i = 0;
		while( pszPlayerClasses[i] != NULL )
		{
			AddPlayerClass( pszPlayerClasses[i] );
			i++;
		}
	}

	virtual const char *GetTeamName( void ) { return "#Teamname_Green"; }
};

IMPLEMENT_SERVERCLASS_ST(CSDKTeam_Green, DT_SDKTeam_Green)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( sdk_team_green, CSDKTeam_Green );


// YELLOW TEAM
//==================
class CSDKTeam_Yellow : public CSDKTeam
{
	DECLARE_CLASS( CSDKTeam_Yellow, CSDKTeam );
	DECLARE_SERVERCLASS();

	virtual void Init( const char *pName, int iNumber )
	{
		BaseClass::Init( pName, iNumber );

		int i = 0;
		while( pszPlayerClasses[i] != NULL )
		{
			AddPlayerClass( pszPlayerClasses[i] );
			i++;
		}
	}

	virtual const char *GetTeamName( void ) { return "#Teamname_Yellow"; }
};

IMPLEMENT_SERVERCLASS_ST(CSDKTeam_Yellow, DT_SDKTeam_Yellow)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( sdk_team_yellow, CSDKTeam_Yellow );

// UNASSIGNED
//==================
class CSDKTeam_Unassigned : public CSDKTeam
{
	DECLARE_CLASS( CSDKTeam_Unassigned, CSDKTeam );
	DECLARE_SERVERCLASS();

	virtual void Init( const char *pName, int iNumber )
	{
		BaseClass::Init( pName, iNumber );

		/*int i = 0;
		while( pszPlayerClasses[i] != NULL )
		{
			AddPlayerClass( pszPlayerClasses[i] );
			i++;
		}*/
	}

	virtual const char *GetTeamName( void ) { return "#Teamname_Unassigned"; }
};

IMPLEMENT_SERVERCLASS_ST(CSDKTeam_Unassigned, DT_SDKTeam_Unassigned)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( sdk_team_unassigned, CSDKTeam_Unassigned );

