//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: TFC's objective resource, transmits all objective states to players
//
// $NoKeywords: $
//=============================================================================//
#ifndef C_SDK_OBJECTIVE_RESOURCE_H
#define C_SDK_OBJECTIVE_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "const.h"
#include "c_baseentity.h"
#include <igameresources.h>
#include "c_team_objectiveresource.h"

class C_SDKObjectiveResource : public C_BaseTeamObjectiveResource
{
	DECLARE_CLASS( C_SDKObjectiveResource, C_BaseTeamObjectiveResource );
public:
	DECLARE_CLIENTCLASS();

			C_SDKObjectiveResource();
	virtual ~C_SDKObjectiveResource();
};

inline C_SDKObjectiveResource *SDKObjectiveResource()
{
	return static_cast< C_SDKObjectiveResource* >( g_pObjectiveResource );
}

#endif // C_SDK_OBJECTIVE_RESOURCE_H
