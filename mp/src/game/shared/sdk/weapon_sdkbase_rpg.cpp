//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "sdk_gamerules.h"
#include "weapon_sdkbase_rpg.h"

#ifdef CLIENT_DLL
	#include "c_sdk_player.h"
	#include "prediction.h"
#else
	#include "sdk_player.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSDKBaseRPG, DT_WeaponSDKBaseRPG )

BEGIN_NETWORK_TABLE( CWeaponSDKBaseRPG, DT_WeaponSDKBaseRPG )

END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSDKBaseRPG )
END_PREDICTION_DATA()

CWeaponSDKBaseRPG::CWeaponSDKBaseRPG()
{
}

void CWeaponSDKBaseRPG::Precache()
{
	BaseClass::Precache();
}

bool CWeaponSDKBaseRPG::Reload( void )
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return false;

	Activity actReload;

	actReload = ACT_VM_RELOAD;

	int iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), actReload );
	if ( !iResult )
		return false;

	pPlayer->SetAnimation( PLAYER_RELOAD );

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	return true;
}

void CWeaponSDKBaseRPG::PrimaryAttack()
{
	CSDKPlayer *pPlayer = ToSDKPlayer( GetPlayerOwner() );
	
	// Out of ammo?
	if ( m_iClip1 <= 0 )
	{
		if ( m_bFireOnEmpty )
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
		}

		return;
	}
#ifdef CLIENT_DLL
	// Play Firing Sound
	WeaponSound(SINGLE);
#endif
	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
		
	FireRocket();

	DoFireEffects();

	m_iClip1--; 

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flTimeWeaponIdle = gpGlobals->curtime + GetFireRate();	//length of the fire anim!
}

void CWeaponSDKBaseRPG::FireRocket( void )
{
	Assert( !"Derived classes must implement this." );
}

void CWeaponSDKBaseRPG::DoFireEffects()
{
	CBasePlayer *pPlayer = GetPlayerOwner();
	if ( pPlayer )
		 pPlayer->DoMuzzleFlash();

	//smoke etc
}

void CWeaponSDKBaseRPG::WeaponIdle()
{
	if (m_flTimeWeaponIdle > gpGlobals->curtime)
		return;

	SendWeaponAnim( GetIdleActivity() );

	m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
}