//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_GRENADE_EMP_H
#define WEAPON_GRENADE_EMP_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_basesdkgrenade.h"

#ifdef CLIENT_DLL
	#define CWeaponGrenadeEmp C_WeaponGrenadeEmp
#endif

//-----------------------------------------------------------------------------
// EMP grenade
//-----------------------------------------------------------------------------
class CWeaponGrenadeEmp : public CBaseSDKGrenade
{
public:
	DECLARE_CLASS( CWeaponGrenadeEmp, CBaseSDKGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponGrenadeEmp() {}

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_GRENADE_EMP; }

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon );
#endif

	CWeaponGrenadeEmp( const CWeaponGrenadeEmp & ) {}
};

#endif // WEAPON_GRENADE_EMP_H
