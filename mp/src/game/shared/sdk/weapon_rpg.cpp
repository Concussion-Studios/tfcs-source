//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "weapon_rpg.h"

#if defined( CLIENT_DLL )
	#include "c_sdk_player.h"
#else
	#include "tfc_projectile_rpg.h"
	#include "sdk_player.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRPG, DT_WeaponRPG )

BEGIN_NETWORK_TABLE( CWeaponRPG, DT_WeaponRPG )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponRPG )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_rpg, CWeaponRPG );
PRECACHE_WEAPON_REGISTER( weapon_rpg );

void CWeaponRPG::FireRocket( void )
{
#ifndef CLIENT_DLL

	CBasePlayer *pPlayer = GetPlayerOwner();

#ifdef DEBUG
	CRPGRocket *pRocket = 
#endif //DEBUG		
		CRPGRocket::Create( pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles(), pPlayer );

	Assert( pRocket );

#endif
}
