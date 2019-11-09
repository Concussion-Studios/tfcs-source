//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_SDKBASE_MACHINEGUN_H
#define WEAPON_SDKBASE_MACHINEGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )
	#define CSDKMachineGun C_SDKMachineGun
#endif

//=========================================================
// Machine gun base class
//=========================================================
class CSDKMachineGun : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CSDKMachineGun, CWeaponSDKBase );
	DECLARE_DATADESC();

	CSDKMachineGun();
	
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	// Default calls through to m_hOwner, but plasma weapons can override and shoot projectiles here.
	virtual void ItemPostFrame( void );
	virtual void FireBullets( const FireBulletsInfo_t &info );
	virtual bool Deploy( void );

	virtual const Vector &GetBulletSpread( void );

	int WeaponSoundRealtime( WeaponSound_t shoot_type );

	// utility function
	static void DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime );

private:

	CSDKMachineGun( const CSDKMachineGun & );

protected:

	int	m_nShotsFired;	// Number of consecutive shots fired

	float	m_flNextSoundTime;	// real-time clock of when to make next sound
};

#endif // WEAPON_SDKBASE_MACHINEGUN_H
