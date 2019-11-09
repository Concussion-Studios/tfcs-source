//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_SDKBASE_COMBATWEAPON_H
#define WEAPON_SDKBASE_COMBATWEAPON_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_sdkbase_machinegun.h"

#ifdef CLIENT_DLL
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif

#if defined( CLIENT_DLL )
	#define CSDKCombatWeapon C_SDKCombatWeapon
#endif

class CSDKCombatWeapon : public CSDKMachineGun
{
	DECLARE_CLASS( CSDKCombatWeapon, CSDKMachineGun );

public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CSDKCombatWeapon();

	virtual Vector	GetBulletSpread( WeaponProficiency_t proficiency );
	virtual float	GetSpreadBias( WeaponProficiency_t proficiency );

	virtual const	WeaponProficiencyInfo_t *GetProficiencyValues();
	static const	WeaponProficiencyInfo_t *GetDefaultProficiencyValues();
private:
	
	CSDKCombatWeapon( const CSDKCombatWeapon & );
};

#endif // WEAPON_SDKBASE_COMBATWEAPON_H
