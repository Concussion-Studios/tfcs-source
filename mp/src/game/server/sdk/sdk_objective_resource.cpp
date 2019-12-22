//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: TFC's objective resource, transmits all objective states to players
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "sdk_objective_resource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST( CSDKObjectiveResource, DT_SDKObjectiveResource )
END_SEND_TABLE()

BEGIN_DATADESC( CSDKObjectiveResource )
END_DATADESC()

LINK_ENTITY_TO_CLASS( sdk_objective_resource, CSDKObjectiveResource );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKObjectiveResource::Spawn( void )
{
	BaseClass::Spawn();
}
