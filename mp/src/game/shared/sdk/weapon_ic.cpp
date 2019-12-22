//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "weapon_ic.h"

#if defined( CLIENT_DLL )
	#include "c_sdk_player.h"
#else
	#include "tfc_projectile_ic.h"
	#include "sdk_player.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponInciendaryCanon, DT_WeaponInciendaryCanon )

BEGIN_NETWORK_TABLE( CWeaponInciendaryCanon, DT_WeaponInciendaryCanon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponInciendaryCanon )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_ic, CWeaponInciendaryCanon );
PRECACHE_WEAPON_REGISTER( weapon_ic );

void CWeaponInciendaryCanon::FireRocket( void )
{
#ifndef CLIENT_DLL

	CBasePlayer *pPlayer = GetPlayerOwner();

#ifdef DEBUG
	CInciendaryRocket *pIC = 
#endif //DEBUG		
		CInciendaryRocket::Create( pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles(), pPlayer );

	Assert( pIC );

#endif
}
