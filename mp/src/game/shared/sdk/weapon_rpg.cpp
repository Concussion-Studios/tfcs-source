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

acttable_t CWeaponRPG::m_acttable[] = 
{
	{ ACT_DOD_STAND_IDLE,					ACT_DOD_STAND_IDLE_BAZOOKA,				false },
	{ ACT_DOD_CROUCH_IDLE,					ACT_DOD_CROUCH_IDLE_BAZOOKA,			false },
	{ ACT_DOD_CROUCHWALK_IDLE,				ACT_DOD_CROUCHWALK_IDLE_BAZOOKA,		false },
	{ ACT_DOD_WALK_IDLE,					ACT_DOD_WALK_IDLE_BAZOOKA,				false },
	{ ACT_DOD_RUN_IDLE,						ACT_DOD_RUN_IDLE_BAZOOKA,				false },
	{ ACT_SPRINT,							ACT_DOD_SPRINT_IDLE_BAZOOKA,			false },
	{ ACT_DOD_IDLE_ZOOMED,					ACT_DOD_STAND_ZOOM_BAZOOKA,				false },
	{ ACT_DOD_CROUCH_ZOOMED,				ACT_DOD_CROUCH_ZOOM_BAZOOKA,			false },
	{ ACT_DOD_CROUCHWALK_ZOOMED,			ACT_DOD_CROUCHWALK_ZOOM_BAZOOKA,		false },
	{ ACT_DOD_WALK_ZOOMED,					ACT_DOD_WALK_ZOOM_BAZOOKA,				false },
	{ ACT_RANGE_ATTACK1,					ACT_DOD_PRIMARYATTACK_BAZOOKA,			false },
	{ ACT_DOD_PRIMARYATTACK_CROUCH,			ACT_DOD_PRIMARYATTACK_BAZOOKA,			false },
	{ ACT_RELOAD,							ACT_DOD_RELOAD_BAZOOKA,					false },
	{ ACT_DOD_RELOAD_CROUCH,				ACT_DOD_RELOAD_CROUCH_BAZOOKA,			false },
	{ ACT_DOD_RELOAD_DEPLOYED,				ACT_DOD_ZOOMLOAD_BAZOOKA,				false },
};

IMPLEMENT_ACTTABLE( CWeaponRPG );

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
