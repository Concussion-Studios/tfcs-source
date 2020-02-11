//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef SDK_WEAPON_PARSE_H
#define SDK_WEAPON_PARSE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_parse.h"
#include "networkvar.h"


//--------------------------------------------------------------------------------------------------------
class CSDKWeaponInfo : public FileWeaponInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT( CSDKWeaponInfo, FileWeaponInfo_t );
	
	CSDKWeaponInfo();
	
	virtual void Parse( ::KeyValues *pKeyValuesData, const char *szWeaponName );

	char m_szAnimExtension[16];		// string used to generate player animations with this weapon
	//int m_iDefaultAmmoClips;		//Tony; default number of clips the weapon comes with.
	float m_flWeaponFOV;		//Tony; added weapon fov, SDK uses models from a couple different games, so FOV is different.
	int m_iAmmoToRemove;
	float m_flMeleeRange;
	bool m_bDrawCosshair;
	bool m_bDrawMuzzleFlash;

	// Scope Settings
	bool m_bUnscopeAfterShot;
	bool m_bUseScope;
	float m_flScopeFov;

	// Weapon Types:
	bool m_bIsPistol;
	bool m_bIsShotgun;
	bool m_bIsSniper;
	bool m_bIsHeavy;
	bool m_bIsGrenade;
	bool m_bIsLaser;
	bool m_bIsBiozard;
	bool m_bIsTool;

	// Parameters for FX_FireBullets:
	int m_iDamage;
	int m_iBullets;
	float m_flCycleTime;
	float m_flReloadTime;
	float m_flSpread;

	// New accuracy model parameters

};


#endif // SDK_WEAPON_PARSE_H
