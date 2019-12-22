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
	#define CWeaponNailGun C_WeaponNailGun
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponNailGun : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponNailGun, CWeaponSDKBase );

	CWeaponNailGun();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	virtual const char	*GetDeploySound( void ) { return "Deploy.WeaponNailgun"; }
	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_NAILGUN; }
	virtual int GetFireMode() const { return FM_AUTOMATIC; }
	
	virtual void Precache( void );
	virtual void PrimaryAttack();
	
private:
	CWeaponNailGun( const CWeaponNailGun & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponNailGun, DT_WeaponNailGun )

BEGIN_NETWORK_TABLE( CWeaponNailGun, DT_WeaponNailGun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponNailGun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_nailgun, CWeaponNailGun );
PRECACHE_WEAPON_REGISTER( weapon_nailgun );

//=========================================================
CWeaponNailGun::CWeaponNailGun()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 1200;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponNailGun::Precache( void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "tfc_projectile_nail" );
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponNailGun::PrimaryAttack(void)
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

	auto* pNail = CTFCProjectileNail::CreateNail( pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles(), pPlayer, Vector( 0,0,0 ), 9.0f );
	if ( pPlayer->GetWaterLevel() == 3 )
		pNail->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
	else
		pNail->SetAbsVelocity( vecAiming * BOLT_AIR_VELOCITY );
#endif
}
