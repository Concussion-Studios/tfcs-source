//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef TFC_PROJECTILE_RPG_H
#define TFC_PROJECTILE_RPG_H
#ifdef _WIN32
#pragma once
#endif

#define ROCKET_MODEL	"models/weapons/w_bazooka_rocket.mdl"

#include "tfc_projectile_base_rockets.h"

class CRPGRocket : public CTFCProjectileBaseRockets
{
public:
	DECLARE_CLASS( CRPGRocket, CTFCProjectileBaseRockets );

	CRPGRocket() {}

	virtual void Spawn();
	virtual void Precache();
	static CRPGRocket *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner );

	virtual SDKWeaponID GetWeaponID( void ) const {	return WEAPON_RPG; }

private:
	CRPGRocket( const CRPGRocket & );
};

#endif //TFC_PROJECTILE_RPG_H