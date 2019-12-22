//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_GRENADE_HALLUCINATION_H
#define WEAPON_GRENADE_HALLUCINATION_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_basesdkgrenade.h"

#ifdef CLIENT_DLL
	#define CWeaponGrenadeHallucination C_WeaponGrenadeHallucination
#endif

//-----------------------------------------------------------------------------
// Hallucination grenades
//-----------------------------------------------------------------------------
class CWeaponGrenadeHallucination : public CBaseSDKGrenade
{
public:
	DECLARE_CLASS( CWeaponGrenadeHallucination, CBaseSDKGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponGrenadeHallucination() {}

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_GRENADE_HALLUCINATION; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon );
	
#endif

	CWeaponGrenadeHallucination( const CWeaponGrenadeHallucination & ) {}
};


#endif // WEAPON_GRENADE_HALLUCINATION_H
