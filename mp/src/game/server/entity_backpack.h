#ifndef SDK_ENTITY_BACKPACK_H
#define SDK_ENTITY_BACKPACK_H
#ifdef _WIN32
#pragma once
#endif

#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"

#define TFC_BACKPACK_MODEL "models/items/backpack.mdl"

class CEntityBackPack : public CItem
{
	DECLARE_CLASS(CEntityBackPack, CItem);

	

	DECLARE_DATADESC();
public:
	void Spawn(void);
	void Precache(void);

	void GiveAmmo(int iCount, int iAmmoIndex);
	void SetRespawnTime(float flTime) { m_flRespawnTime = flTime; }
	void SetNextOwnerTouch(float flTime) { m_flNextOwnerPickup = flTime; }

	string_t GetModelName() { return AllocPooledString("models/items/backpack.mdl"); }

	bool MyTouch(CBasePlayer *pPlayer);
private:
	int m_iammo_cells;
	int m_iammo_nails;
	int m_iammo_rockets;
	int m_iammo_shells;
	int m_iammo_grenades1;
	int m_iammo_grenades2;
	int m_iammo_detpack;
	int m_iarmor;
	int m_ihealth;

	float m_flNextOwnerPickup;

	//int m_ishouldwerespawn;
	float m_flRespawnTime;
};
#endif //SDK_ENTITY_BACKPACK_H