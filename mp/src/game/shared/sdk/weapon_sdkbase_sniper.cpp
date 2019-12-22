//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "weapon_sdkbase_sniper.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSDKBaseSniper, DT_WeaponSDKBaseSniper )

BEGIN_NETWORK_TABLE( CWeaponSDKBaseSniper, DT_WeaponSDKBaseSniper )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSDKBaseSniper )
END_PREDICTION_DATA()

CWeaponSDKBaseSniper::CWeaponSDKBaseSniper()
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = false;
}

void CWeaponSDKBaseSniper::PrimaryAttack( void )
{
	BaseClass::PrimaryAttack();

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}