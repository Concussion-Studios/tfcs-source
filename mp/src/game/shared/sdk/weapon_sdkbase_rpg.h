//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_SDKBASE_RPG_H
#define WEAPON_SDKBASE_RPG_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_sdkbase.h"
 
#if defined( CLIENT_DLL )
	#define CWeaponSDKBaseRPG C_WeaponSDKBaseRPG
#endif

//-----------------------------------------------------------------------------
// RPG
//-----------------------------------------------------------------------------
class CWeaponSDKBaseRPG : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponSDKBaseRPG, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponSDKBaseRPG();

	virtual bool CanWeaponBeDropped() const { return false; }
	virtual bool CanDrop( void ) { return false; }
	virtual void PrimaryAttack();
	virtual bool Reload();
	virtual void WeaponIdle();
	void DoFireEffects();
	void Precache( void );
	virtual	void FireRocket( void );

private:
	CWeaponSDKBaseRPG( const CWeaponSDKBaseRPG & );
};

#endif // WEAPON_SDKBASE_RPG_H