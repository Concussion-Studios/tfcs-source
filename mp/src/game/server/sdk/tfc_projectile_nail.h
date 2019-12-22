//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef TFC_PROJECTILE_NAIL_H
#define TFC_PROJECTILE_NAIL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "tfc_projectile_base.h"

#define PROJWCTILW_NAIL_MODEL "models/weapons/w_models/w_nail.mdl"

//=============================================================================
// projectile shooted from the nailgun
//=============================================================================
class CTFCProjectileNail : public CTFCProjectileBase
{
public:
	DECLARE_CLASS( CTFCProjectileNail, CTFCProjectileBase );
	DECLARE_DATADESC();

			CTFCProjectileNail();
	virtual	~CTFCProjectileNail( void );

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_NAILGUN; }

	virtual void Precache( void ) OVERRIDE;
	virtual void Spawn( void ) OVERRIDE;

	virtual void ProjectileStickTo( CBaseEntity *pOther, trace_t &tr ) OVERRIDE;
	virtual void ProjectileTouch( CBaseEntity* pOther ) OVERRIDE;

	static CTFCProjectileNail* CreateNail( const Vector& vecOrigin,
										   const QAngle& vecAngles, 
										   CBaseEntity* pOwner, 
										   const Vector& vecVelocity, 
										   float flDamage );

	static CTFCProjectileNail* CreateNailShower( const Vector& vecOrigin,
										   const QAngle& vecAngles, 
										   CBaseEntity* pOwner );
};
#endif //TFC_PROJECTILE_NAIL_H