//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The various ammo types for TFC	
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Ammo counts given by ammo items
#define SIZE_AMMO_SHELL_LARGE		225
#define SIZE_AMMO_CELL_LARGE		225
#define SIZE_AMMO_NAIL_LARGE		225
#define SIZE_AMMO_EXPLOSIVE_LARGE	225

//---------------------------------------------------------
// Applies ammo quantity scale.
//---------------------------------------------------------
int FillAmmo( CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false )
{
	int iAmmoType = GetAmmoDef()->Index( pszAmmoName );
	if ( iAmmoType == -1 )
	{
		Msg( "ERROR: Attempting to give unknown ammo type (%s)\n",pszAmmoName );
		return 0;
	}

	flCount *= g_pGameRules->GetAmmoQuantityScale(iAmmoType);

	// Don't give out less than 1 of anything.
	flCount = MAX( 1.0f, flCount );

	return pPlayer->GiveAmmo( flCount, iAmmoType, bSuppressSound );
}

// ========================================================================
//	>> CEntityAmmo
// ========================================================================
class CEntityAmmo : public CItem
{
public:
	DECLARE_CLASS( CEntityAmmo, CItem );

	void Spawn( void )
	{
		Precache();

		SetModel( WorldModel() );

		BaseClass::Spawn();
	}

	const char* WorldModel() 
	{
		return nullptr; 
	}

	const char* AmmoName() 
	{
		return nullptr; 
	}

	float AmmoSize()
	{
		return 0;
	}

	void Precache( void )
	{
		PrecacheModel( WorldModel() );
	}

	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( FillAmmo( pPlayer, AmmoSize(), AmmoName() ) )
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
				UTIL_Remove( this );

			return true;
		}

		return false;
	}
};

// ========================================================================
//	>> CBoxShellRounds
// ========================================================================
class CBoxShellRounds : public CEntityAmmo
{
public:
	DECLARE_CLASS( CBoxShellRounds, CEntityAmmo );

	const char* WorldModel() { return "models/items/boxmrounds.mdl"; }
	const char* AmmoName() { return "shell"; }
	float AmmoSize() { return SIZE_AMMO_SHELL_LARGE; }
};

LINK_ENTITY_TO_CLASS( item_ammo_shells, CBoxShellRounds );

// ========================================================================
//	>> CBoxShellRounds
// ========================================================================
class CBoxCellRounds : public CEntityAmmo
{
public:
	DECLARE_CLASS( CBoxCellRounds, CEntityAmmo );

	const char* WorldModel() { return "models/items/boxmrounds.mdl"; }
	const char* AmmoName() { return "cell"; }
	float AmmoSize() { return SIZE_AMMO_CELL_LARGE; }
};

LINK_ENTITY_TO_CLASS( item_ammo_cells, CBoxShellRounds );

// ========================================================================
//	>> CBoxShellRounds
// ========================================================================
class CBoxNailRounds : public CEntityAmmo
{
public:
	DECLARE_CLASS( CBoxNailRounds, CEntityAmmo );

	const char* WorldModel() { return "models/items/boxmrounds.mdl"; }
	const char* AmmoName() { return "nail"; }
	float AmmoSize() { return SIZE_AMMO_NAIL_LARGE; }
};

LINK_ENTITY_TO_CLASS( item_ammo_nails, CBoxShellRounds );

// ========================================================================
//	>> CBoxShellRounds
// ========================================================================
class CBoxExplosiveRounds : public CEntityAmmo
{
public:
	DECLARE_CLASS( CBoxExplosiveRounds, CEntityAmmo );

	const char* WorldModel() { return "models/items/boxmrounds.mdl"; }
	const char* AmmoName() { return "explosive"; }
	float AmmoSize() { return SIZE_AMMO_EXPLOSIVE_LARGE; }
};

LINK_ENTITY_TO_CLASS( item_ammo_explosives, CBoxShellRounds );

// ========================================================================
//	>> CEntityBackPack
// ========================================================================
class CEntityBackPack : public CItem
{
public:
	DECLARE_CLASS( CEntityBackPack, CItem );

	void Spawn( void )
	{
		Precache();

		SetModel( WorldModel() );

		BaseClass::Spawn();
	}

	void Precache( void )
	{
		PrecacheModel( WorldModel() );
	}

	const char* WorldModel() 
	{ 
		return "models/items/boxmrounds.mdl"; 
	}

	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer )
		{
			FillAmmo( pPlayer, SIZE_AMMO_EXPLOSIVE_LARGE, "shell" );
			FillAmmo( pPlayer, SIZE_AMMO_EXPLOSIVE_LARGE, "cell" );
			FillAmmo( pPlayer, SIZE_AMMO_EXPLOSIVE_LARGE, "nail" );
			FillAmmo( pPlayer, SIZE_AMMO_EXPLOSIVE_LARGE, "explosive" );

			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_AMMO_RESPAWN_YES )
				UTIL_Remove( this );

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS( entity_backpack, CEntityBackPack );