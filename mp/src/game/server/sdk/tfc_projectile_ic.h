//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef TFC_PROJECTILE_IC_H
#define TFC_PROJECTILE_IC_H
#ifdef _WIN32
#pragma once
#endif

#define ROCKET_MODEL	"models/weapons/w_bazooka_rocket.mdl"

#include "tfc_projectile_base_rockets.h"

class CInciendaryRocket : public CTFCProjectileBaseRockets
{
public:
	DECLARE_CLASS( CInciendaryRocket, CTFCProjectileBaseRockets );

	CInciendaryRocket() {}

	virtual void Spawn();
	virtual void Precache();
	virtual void DoExplosion();	

	static CInciendaryRocket *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner );

	virtual SDKWeaponID GetWeaponID( void ) const {	return WEAPON_RPG; }

private:
	CInciendaryRocket( const CInciendaryRocket & );
};

#endif //TFC_PROJECTILE_IC_H