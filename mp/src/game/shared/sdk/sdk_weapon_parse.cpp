//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include <KeyValues.h>
#include "sdk_weapon_parse.h"


FileWeaponInfo_t* CreateWeaponInfo()
{
	return new CSDKWeaponInfo;
}


CSDKWeaponInfo::CSDKWeaponInfo()
{
}

void CSDKWeaponInfo::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	BaseClass::Parse( pKeyValuesData, szWeaponName );

	const char *pAnimEx = pKeyValuesData->GetString( "PlayerAnimationExtension", "mp5" );
	Q_strncpy( m_szAnimExtension, pAnimEx, sizeof( m_szAnimExtension ) );

	m_iAmmoToRemove		= pKeyValuesData->GetInt( "AmmoToRemove", 1 ); // How much ammo consumes this weapon?
	//m_iDefaultAmmoClips = pKeyValuesData->GetInt( "NumClips", 2 );
	m_flWeaponFOV		= pKeyValuesData->GetFloat( "Fov", 74.0f );
	m_flMeleeRange		= pKeyValuesData->GetFloat( "Range", 32.0f );
	m_bDrawCosshair		= pKeyValuesData->GetBool( "DrawCrossHair", true );
	m_bDrawMuzzleFlash	= pKeyValuesData->GetBool( "DrawMuzzleFlash", true );

	// Scope Settings
	m_bUnscopeAfterShot = pKeyValuesData->GetBool( "UnscopeAfterShot", false );
	m_bUseScope			= pKeyValuesData->GetBool( "UseScope", false );
	m_flScopeFov		= pKeyValuesData->GetFloat( "ScopeFov", 45.0f );

	// Weapon Types:
	m_bIsPistol			= pKeyValuesData->GetBool( "IsPistolWeapon", false );
	m_bIsShotgun		= pKeyValuesData->GetBool( "IsShotgunWeapon", false );
	m_bIsSniper			= pKeyValuesData->GetBool( "IsSniperWeapon", false );
	m_bIsHeavy			= pKeyValuesData->GetBool( "IsHeavyWeapon", false );
	m_bIsGrenade		= pKeyValuesData->GetBool( "IsGrenadeWeapon", false );
	m_bIsLaser			= pKeyValuesData->GetBool( "IsLaserWeapon", false );
	m_bIsBiozard		= pKeyValuesData->GetBool( "IsBiozardWeapon", false );
	m_bIsTool			= pKeyValuesData->GetBool( "IsToolWeapon", false );

	// Parameters for FX_FireBullets:
	m_iDamage			= pKeyValuesData->GetInt( "Damage", 42 ); // Douglas Adams 1952 - 2001
	m_iBullets			= pKeyValuesData->GetInt( "Bullets", 1 );
	m_flSpread			= pKeyValuesData->GetFloat( "Spread", 0.01 );
	m_flCycleTime		= pKeyValuesData->GetFloat( "CycleTime", 0.15 );
	m_flReloadTime		= pKeyValuesData->GetFloat("ReloadTime", 0.3);

	// New accuracy model parameters
}


