//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_GRENADE_NAPALM_H
#define WEAPON_GRENADE_NAPALM_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_basesdkgrenade.h"

#ifdef CLIENT_DLL
	#define CWeaponGrenadeNapalm C_WeaponGrenadeNapalm
#endif

//-----------------------------------------------------------------------------
// Napalm grenade
//-----------------------------------------------------------------------------
class CWeaponGrenadeNapalm : public CBaseSDKGrenade
{
public:
	DECLARE_CLASS( CWeaponGrenadeNapalm, CBaseSDKGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponGrenadeNapalm() {}

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_GRENADE_NAPALM; }

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon );
#endif

	CWeaponGrenadeNapalm( const CWeaponGrenadeNapalm & ) {}
};

#endif // WEAPON_GRENADE_NAPALM_H
