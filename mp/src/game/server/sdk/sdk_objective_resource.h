//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: TFC's objective resource, transmits all objective states to players
//
// $NoKeywords: $
//=============================================================================//
#ifndef SDK_OBJECTIVE_RESOURCE_H
#define SDK_OBJECTIVE_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "team_objectiveresource.h"

class CSDKObjectiveResource : public CBaseTeamObjectiveResource
{
	DECLARE_CLASS( CSDKObjectiveResource, CBaseTeamObjectiveResource );
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Spawn( void );
};

#endif	// SDK_OBJECTIVE_RESOURCE_H

