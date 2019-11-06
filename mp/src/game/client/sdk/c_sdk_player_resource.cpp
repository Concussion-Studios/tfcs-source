//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: SDK C_PlayerResource
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_sdk_player_resource.h"
#include "c_sdk_player.h"
#include "sdk_gamerules.h"
#include <shareddefs.h>
#include "hud.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT(C_SDK_PlayerResource, DT_SDKPlayerResource, CSDKPlayerResource)
	RecvPropArray3( RECVINFO_ARRAY(m_iPlayerClass), RecvPropInt( RECVINFO(m_iPlayerClass[0]))),
END_RECV_TABLE()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_SDK_PlayerResource::C_SDK_PlayerResource()
{
	m_Colors[SDK_TEAM_BLUE] = COLOR_BLUE;
	m_Colors[SDK_TEAM_RED] = COLOR_RED;

	m_Colors[TEAM_UNASSIGNED] = COLOR_YELLOW;
	m_Colors[TEAM_SPECTATOR] = COLOR_GREY;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_SDK_PlayerResource::~C_SDK_PlayerResource()
{
}

int C_SDK_PlayerResource::GetPlayerClass( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return PLAYERCLASS_UNDEFINED;

	return m_iPlayerClass[iIndex];
}

C_SDK_PlayerResource * SDKGameResources( void )
{
	return dynamic_cast<C_SDK_PlayerResource *>(GameResources());
}