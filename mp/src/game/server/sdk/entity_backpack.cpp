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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------
// Applies ammo quantity scale.
//---------------------------------------------------------
//extern int FillAmmo( CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false );

// ========================================================================
//	>> CEntityBackPack
// ========================================================================
class CEntityBackPack : public CItem
{
	DECLARE_CLASS( CEntityBackPack, CItem );

	int m_iammo_cells;
	int m_iammo_nails;
	int m_iammo_rockets;
	int m_iammo_shells;
	int m_iammo_grenades1;
	int m_iammo_grenades2;
	int m_iammo_detpack;
	int m_iarmor;
	int m_ihealth;

	//int m_ishouldwerespawn;
	float m_flRespawnTime;

	DECLARE_DATADESC();

public:


	void Spawn( void )
	{
		char *szModel = (char *)STRING( GetModelName() );
		if ( !szModel || !*szModel )
		{
			Warning( "entity_backpack at %.0f %.0f %0.f missing modelname\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
			UTIL_Remove( this );
			return;
		}

		Precache();

		SetModel( STRING( GetModelName() ) );

		BaseClass::Spawn();
	}

	void Precache( void )
	{
		PrecacheModel( STRING( GetModelName() ) );
	}

	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer )
		{
			pPlayer->GiveAmmo(m_iammo_shells, AMMO_SHELLS);
			pPlayer->GiveAmmo(m_iammo_cells, AMMO_CELLS);
			pPlayer->GiveAmmo(m_iammo_rockets, AMMO_ROCKETS);
			pPlayer->GiveAmmo(m_iammo_nails, AMMO_NAILS);
			pPlayer->GiveAmmo(m_iammo_grenades1, AMMO_GRENADES1);
			pPlayer->GiveAmmo(m_iammo_grenades2, AMMO_GRENADES2);
			pPlayer->GiveAmmo(m_iammo_detpack, AMMO_DETPACK);

			if (m_ihealth > 0)
			{
				pPlayer->TakeHealth(m_iHealth, DMG_GENERIC);
			}

			if (m_iarmor > 0)
			{
				pPlayer->IncrementArmorValue(m_iarmor);
			}
			/*FillAmmo( pPlayer, m_iammo_shells, "shells" );
			FillAmmo( pPlayer, m_iammo_cells, "cell" );
			FillAmmo( pPlayer, m_iammo_nails, "nail" );
			FillAmmo( pPlayer, m_iammo_rockets, "explosive" );*/

			if ( g_pGameRules->ItemShouldRespawn( this ) == m_flRespawnTime )
				UTIL_Remove( this );

			return true;
		}
		return false;
	}
};
BEGIN_DATADESC( CEntityBackPack )
	DEFINE_KEYFIELD( m_iammo_cells, FIELD_INTEGER, "Cells" ),
	DEFINE_KEYFIELD( m_iammo_nails, FIELD_INTEGER, "Nails" ),
	DEFINE_KEYFIELD( m_iammo_rockets, FIELD_INTEGER, "Rockets" ),
	DEFINE_KEYFIELD( m_iammo_shells, FIELD_INTEGER, "Shells" ),
	DEFINE_KEYFIELD( m_flRespawnTime, FIELD_FLOAT, "RespawnTime" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( entity_backpack, CEntityBackPack );
