//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_GRENADE_CONCUSSION_H
#define WEAPON_GRENADE_CONCUSSION_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_basesdkgrenade.h"

#ifdef CLIENT_DLL
	#define CWeaponGrenadeConcussion C_WeaponGrenadeConcussion
#endif

//-----------------------------------------------------------------------------
// Concussion grenade
//-----------------------------------------------------------------------------
class CWeaponGrenadeConcussion : public CBaseSDKGrenade
{
public:
	DECLARE_CLASS( CWeaponGrenadeConcussion, CBaseSDKGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponGrenadeConcussion() {}

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_GRENADE_CONCUSSION; }

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon );
#endif

	CWeaponGrenadeConcussion( const CWeaponGrenadeConcussion & ) {}
};

#endif // WEAPON_GRENADE_CONCUSSION_H
