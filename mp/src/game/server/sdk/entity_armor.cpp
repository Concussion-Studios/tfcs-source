//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TFC Armor.
//
//=============================================================================//
#include "cbase.h"
#include "sdk_gamerules.h"
#include "sdk_player.h"
#include "sdk_team.h"
#include "engine/IEngineSound.h"
#include "entity_armor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void CEntityArmor::Spawn( void )
{ 
	Precache();
	SetModel( ARMOR_MODEL );
	BaseClass::Spawn( );
}

void CEntityArmor::Precache( void )
{
	PrecacheModel ( ARMOR_MODEL );
	PrecacheScriptSound( ARMOR_PICKUP_SOUND );
}

bool CEntityArmor::MyTouch( CBasePlayer *pPlayer )
{
	bool bSuccess = false;

	CSDKPlayer *pSDKPlayer = ToSDKPlayer( pPlayer );
	if ( pSDKPlayer )
	{
		const float MaxArmor = pSDKPlayer->GetMaxArmorValue();
		float Armor = pSDKPlayer->GetArmorValue();
		float DefaultArmot = ARMOR_CAPACITY;

		if ( MaxArmor == Armor )
			return false;

		if ( Armor <= MaxArmor )
			pSDKPlayer->IncrementArmorValue( DefaultArmot );
	
		CSingleUserRecipientFilter user( pPlayer );
		user.MakeReliable();
		UserMessageBegin( user, "ItemPickup" );
			WRITE_STRING( GetClassname() );
		MessageEnd();

		CPASAttenuationFilter filter( this, ARMOR_PICKUP_SOUND );
		EmitSound( filter, entindex(), ARMOR_PICKUP_SOUND );

		bSuccess = true;
	}

	return bSuccess;
}

LINK_ENTITY_TO_CLASS( entity_armor, CEntityArmor );
PRECACHE_REGISTER( entity_armor );