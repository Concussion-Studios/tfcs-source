//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "sdk_weapon_melee.h"

#if defined( CLIENT_DLL )
	#define CWeaponWrench C_WeaponWrench
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
	#include "obj_sentry.h"
#endif


class CWeaponWrench : public CWeaponSDKMelee
{
public:
	DECLARE_CLASS(CWeaponWrench, CWeaponSDKMelee);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponWrench();

	virtual SDKWeaponID GetWeaponID(void) const { return WEAPON_WRENCH; }
	virtual	void SecondaryAttack( void );
	virtual void Precache( void );

private:

	CWeaponWrench(const CWeaponWrench &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponWrench, DT_WeaponWrench)

BEGIN_NETWORK_TABLE(CWeaponWrench, DT_WeaponWrench)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponWrench)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_wrench, CWeaponWrench );
PRECACHE_WEAPON_REGISTER( weapon_wrench );

CWeaponWrench::CWeaponWrench()
{
}

void CWeaponWrench::Precache (void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "obj_sentry" );
#endif

	BaseClass::Precache();
}

void CWeaponWrench::SecondaryAttack()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

#ifndef CLIENT_DLL
	auto *pSentry = (CObjSentryGun*)CBaseEntity::Create( "obj_sentry", pPlayer->Weapon_ShootPosition(), vec3_angle, pPlayer );
	if ( !pSentry )
		return;
	
	pSentry->SetGroundEntity( this );
	pSentry->AddFlag( FL_ONGROUND );
	pSentry->ChangeTeam( GetTeamNumber() );
#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}
