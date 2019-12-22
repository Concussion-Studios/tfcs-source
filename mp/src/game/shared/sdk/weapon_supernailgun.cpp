//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#ifdef CLIENT_DLL
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
	#include "tfc_projectile_nail.h"
#endif

#ifdef CLIENT_DLL
	#define CWeaponSuperNailGun C_WeaponSuperNailGun
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponSuperNailGun : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponSuperNailGun, CWeaponSDKBase );

	CWeaponSuperNailGun();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual const char	*GetDeploySound( void ) { return "Deploy.WeaponNailgun"; }
	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_SUPERNAILGUN; }

	virtual void Precache( void );
	virtual void PrimaryAttack();

private:
	CWeaponSuperNailGun( const CWeaponSuperNailGun & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSuperNailGun, DT_WeaponSuperNailGun )

BEGIN_NETWORK_TABLE( CWeaponSuperNailGun, DT_WeaponSuperNailGun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSuperNailGun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_supernailgun, CWeaponSuperNailGun );
PRECACHE_WEAPON_REGISTER( weapon_supernailgun );

//=========================================================
CWeaponSuperNailGun::CWeaponSuperNailGun()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 1400;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSuperNailGun::Precache (void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "tfc_projectile_nail" );
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSuperNailGun::PrimaryAttack( void )
{
	BaseClass::PrimaryAttack();
 
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

#ifndef CLIENT_DLL
	Vector vecAiming = pPlayer->GetAutoaimVector( 0 );
	Vector vecSrc = pPlayer->Weapon_ShootPosition();

	QAngle angAiming;
	VectorAngles( vecAiming, angAiming );

	auto* pNail = CTFCProjectileNail::CreateNail( pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles(), pPlayer, Vector( 0,0,0 ), 13.0f );
	if ( pPlayer->GetWaterLevel() == 3 )
		pNail->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
	else
		pNail->SetAbsVelocity( vecAiming * BOLT_AIR_VELOCITY );

#endif
 
	// Make sure we don't fire more than the amount in the clip
	if ( UsesClipsForAmmo1() )
		m_iClip1 -= GetAmmoToRemove();
	else
		pPlayer->RemoveAmmo( GetAmmoToRemove(), m_iPrimaryAmmoType );
}