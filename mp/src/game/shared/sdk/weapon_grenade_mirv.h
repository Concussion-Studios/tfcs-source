//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_GRENADE_MIRV_H
#define WEAPON_GRENADE_MIRV_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_basesdkgrenade.h"


#ifdef CLIENT_DLL
	
	#define CWeaponGrenadeMirv C_WeaponGrenadeMirv

#endif

//-----------------------------------------------------------------------------
// Nail grenades
//-----------------------------------------------------------------------------
class CWeaponGrenadeMirv : public CBaseSDKGrenade
{
public:
	DECLARE_CLASS( CWeaponGrenadeMirv, CBaseSDKGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponGrenadeMirv() {}

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_GRENADE_MIRV; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon );
	
#endif

	CWeaponGrenadeMirv( const CWeaponGrenadeMirv & ) {}
};


#endif // WEAPON_GRENADE_MIRV_H
