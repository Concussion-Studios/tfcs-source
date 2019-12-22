//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_GRENADE_CALTROP_H
#define WEAPON_GRENADE_CALTROP_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_basesdkgrenade.h"


#ifdef CLIENT_DLL
	#define CWeaponGrenadeCaltrop C_WeaponGrenadeCaltrop
#endif

//-----------------------------------------------------------------------------
// Nail grenades
//-----------------------------------------------------------------------------
class CWeaponGrenadeCaltrop : public CBaseSDKGrenade
{
public:
	DECLARE_CLASS( CWeaponGrenadeCaltrop, CBaseSDKGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponGrenadeCaltrop() {}

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_GRENADE_CALTROP; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon );
	
#endif

	CWeaponGrenadeCaltrop( const CWeaponGrenadeCaltrop & ) {}
};


#endif // WEAPON_GRENADE_CALTROP_H
