//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_GRENADE_NAIL_H
#define WEAPON_GRENADE_NAIL_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_basesdkgrenade.h"


#ifdef CLIENT_DLL

	#define CWeaponGrenadeNail C_WeaponGrenadeNail
#endif

//-----------------------------------------------------------------------------
// Nail grenades
//-----------------------------------------------------------------------------
class CWeaponGrenadeNail : public CBaseSDKGrenade
{
public:
	DECLARE_CLASS( CWeaponGrenadeNail, CBaseSDKGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponGrenadeNail() {}

	virtual SDKWeaponID GetWeaponID( void ) const		{ return WEAPON_GRENADE_NAIL; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon );
	
#endif

	CWeaponGrenadeNail( const CWeaponGrenadeNail & ) {}
};


#endif // WEAPON_GRENADE_NAIL_H
