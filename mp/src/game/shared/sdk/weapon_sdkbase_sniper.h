//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_SDKBASE_SNIPER_H
#define WEAPON_SDKBASE_SNIPER_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
	#define CWeaponSDKBaseSniper C_WeaponSDKBaseSniper
#endif

#include "cbase.h"
#include "weapon_sdkbase.h"

class CWeaponSDKBaseSniper : public CWeaponSDKBase
{
	DECLARE_CLASS( CWeaponSDKBaseSniper, CWeaponSDKBase );

public:
	CWeaponSDKBaseSniper( void );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:
	virtual void PrimaryAttack( void );

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone;

		if ( IsScoped() )
			cone = Vector( 0, 0, 0 );	// do not take bullet spread into account when scoped
		else
			cone = VECTOR_CONE_10DEGREES;	// unscoped snipers are not at all accurate

		return cone;
	}

private:
	CWeaponSDKBaseSniper( const CWeaponSDKBaseSniper & );
};

#endif	// WEAPON_SDKBASE_SNIPER_H
