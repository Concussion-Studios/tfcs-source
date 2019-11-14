//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef ROCKET_RPG_H
#define ROCKET_RPG_H
#ifdef _WIN32
#pragma once
#endif

#define ROCKET_MODEL	"models/weapons/w_missile_launch.mdl"

#include "sdk_baserocket.h"

class CRPGRocket : public CSDKBaseRocket
{
public:
	DECLARE_CLASS( CRPGRocket, CSDKBaseRocket );

	CRPGRocket() {}

	virtual void Spawn();
	virtual void Precache();
	static CRPGRocket *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner );
	virtual SDKWeaponID GetEmitterWeaponID() { return WEAPON_RPG; }

private:
	CRPGRocket( const CRPGRocket & );
};

#endif //ROCKET_RPG_H