//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//
#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )
	#define CWeaponTranq C_WeaponTranq
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
	#include "tfc_projectile_tranq.h"
#endif

class CWeaponTranq : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponTranq, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponTranq();

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_TRANQ; }
	virtual int GetFireMode() const { return FM_SEMIAUTOMATIC; }

	virtual void Precache( void );
	virtual void PrimaryAttack();

private:

	CWeaponTranq( const CWeaponTranq & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponTranq, DT_WeaponTranq )

BEGIN_NETWORK_TABLE( CWeaponTranq, DT_WeaponTranq )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponTranq )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_tranq, CWeaponTranq );
PRECACHE_WEAPON_REGISTER( weapon_tranq );

CWeaponTranq::CWeaponTranq()
{
}

void CWeaponTranq::Precache( void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "tfc_projectile_tranq" );
#endif

	BaseClass::Precache();
}

void CWeaponTranq::PrimaryAttack(void)
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

	auto* pTranq = CTFCProjectileTranq::CreateTranq( pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles(), pPlayer, Vector( 0,0,0 ), 9.0f );
	if ( pPlayer->GetWaterLevel() == 3 )
		pTranq->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
	else
		pTranq->SetAbsVelocity( vecAiming * BOLT_AIR_VELOCITY );
#endif
}