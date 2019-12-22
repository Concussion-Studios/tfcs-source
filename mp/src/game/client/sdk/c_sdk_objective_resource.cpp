//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: TFC's objective resource, transmits all objective states to players
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_sdk_objective_resource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_SDKObjectiveResource, DT_SDKObjectiveResource, CSDKObjectiveResource )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_SDKObjectiveResource::C_SDKObjectiveResource()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_SDKObjectiveResource::~C_SDKObjectiveResource()
{
}