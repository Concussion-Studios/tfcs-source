//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Handling for the suit batteries.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "sdk_player.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "items.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CItemArmor : public CItem
{
public:
	DECLARE_CLASS( CItemArmor, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/battery.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/battery.mdl");

		PrecacheScriptSound( "Armor.Touch" );

	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (pPlayer)
		{
			CSDKPlayer *pSDKPlayer = dynamic_cast<CSDKPlayer *>(pPlayer);
			pSDKPlayer->IncrementArmorValue(50, -1);

			return true;
		}
		
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_armor, CItemArmor);
PRECACHE_REGISTER(item_armor);

