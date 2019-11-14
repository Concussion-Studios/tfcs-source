//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_RPG_H
#define WEAPON_RPG_H
#endif
#ifdef _WIN32
#pragma once
#endif

#include "weapon_sdkbase_rpg.h"

class CWeaponRPG : public CWeaponSDKBaseRPG
{
public:
	DECLARE_CLASS( CWeaponRPG, CWeaponSDKBaseRPG );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponRPG() {}

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_RPG; }
	virtual void FireRocket( void );

private:
	CWeaponRPG( const CWeaponRPG& );
};

// WEAPON_RPG_H
