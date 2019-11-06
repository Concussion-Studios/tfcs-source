//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: SDK CPlayerResource
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "sdk_player.h"
#include "player_resource.h"
#include "sdk_player_resource.h"
#include <coordsize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Datatable
IMPLEMENT_SERVERCLASS_ST(CSDKPlayerResource, DT_SDKPlayerResource)
	SendPropArray3( SENDINFO_ARRAY3(m_iPlayerClass), SendPropInt( SENDINFO_ARRAY(m_iPlayerClass), 4 ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CSDKPlayerResource )
END_DATADESC()

LINK_ENTITY_TO_CLASS( sdk_player_manager, CSDKPlayerResource );

CSDKPlayerResource::CSDKPlayerResource( void )
{
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKPlayerResource::UpdatePlayerData( void )
{
	int i;

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CSDKPlayer *pPlayer = (CSDKPlayer*)UTIL_PlayerByIndex( i );
		if ( pPlayer && pPlayer->IsConnected() )
			m_iPlayerClass.Set( i, pPlayer->m_Shared.PlayerClass() );
	}

	BaseClass::UpdatePlayerData();
}

void CSDKPlayerResource::Spawn( void )
{
	int i;

	for ( i=0; i < MAX_PLAYERS+1; i++ )
		m_iPlayerClass.Set( i, PLAYERCLASS_UNDEFINED );

	BaseClass::Spawn();
}