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

//---------------------------------------------------------
// Applies ammo quantity scale.
//---------------------------------------------------------
int FillBAmmo(CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false)
{
	int iAmmoType = GetAmmoDef()->Index(pszAmmoName);
	if (iAmmoType == -1)
	{
		Msg("ERROR: Attempting to give unknown ammo type (%s)\n", pszAmmoName);
		return 0;
	}

	flCount *= g_pGameRules->GetAmmoQuantityScale(iAmmoType);

	// Don't give out less than 1 of anything.
	flCount = MAX(1.0f, flCount);

	return pPlayer->GiveAmmo(flCount, iAmmoType, bSuppressSound);
}

// ========================================================================
//	>> CEntityAmmo
// ========================================================================
class CEntityAmmo : public CItem
{
public:
	DECLARE_CLASS(CEntityAmmo, CItem);

	void Spawn(void)
	{
		Precache();

		SetModel(WorldModel());

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

	void Precache(void)
	{
		PrecacheModel(WorldModel());
	}

	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (FillBAmmo(pPlayer, AmmoSize(), AmmoName()))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
				UTIL_Remove(this);

			return true;
		}

		return false;
	}
};

// ========================================================================
//	>> CEntityBackPack
// ========================================================================
class CEntityBackPack : public CItem
{
	DECLARE_CLASS(CEntityBackPack, CItem);

	int m_iammo_cells = 50;
	int m_iammo_nails = 50;
	int m_iammo_explosives = 20;
	int m_iammo_shells = 50;
	int m_ishouldwerespawn = GR_AMMO_RESPAWN_NO;
	const char* m_sworldmodel = "models/items/boxmrounds.mdl";

	DECLARE_DATADESC();



public:


	void Spawn(void)
	{
		Precache();

		SetModel(WorldModel());

		BaseClass::Spawn();
	}

	void Precache(void)
	{
		PrecacheModel(WorldModel());
	}

	const char* WorldModel()
	{
		return m_sworldmodel;
	}

	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (pPlayer)
		{
			FillBAmmo(pPlayer, m_iammo_shells, "shell");
			FillBAmmo(pPlayer, m_iammo_cells, "cell");
			FillBAmmo(pPlayer, m_iammo_nails, "nail");
			FillBAmmo(pPlayer, m_iammo_explosives, "explosive");

			if (g_pGameRules->ItemShouldRespawn(this) == m_ishouldwerespawn)
				UTIL_Remove(this);

			return true;
		}
		return false;
	}
};
BEGIN_DATADESC(CEntityBackPack)

DEFINE_FIELD(m_iammo_cells, FIELD_INTEGER, "cells"),
DEFINE_FIELD(m_iammo_nails, FIELD_INTEGER, "nails"),
DEFINE_FIELD(m_iammo_explosives, FIELD_INTEGER, "explosives"),
DEFINE_FIELD(m_iammo_shells, FIELD_INTEGER, "shells"),
DEFINE_FIELD(m_ishouldwerespawn, FIELD_INTEGER, "shouldwespawn"),
DEFINE_FIELD(m_sworldmodel, FIELD_MODELNAME, "model"),
END_DATADESC()
LINK_ENTITY_TO_CLASS(entity_backpack, CEntityBackPack);
