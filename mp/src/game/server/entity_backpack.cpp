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
extern int FillAmmo( CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false );

// ========================================================================
//	>> CEntityBackPack
// ========================================================================
class CEntityBackPack : public CItem
{
	DECLARE_CLASS( CEntityBackPack, CItem );

	int m_iammo_cells;
	int m_iammo_nails;
	int m_iammo_explosives;
	int m_iammo_shells;
	int m_ishouldwerespawn;

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

	void Precache(void)
	{
		PrecacheModel( STRING( GetModelName() ) );
	}

	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (pPlayer)
		{
			FillAmmo( pPlayer, m_iammo_shells, "shell" );
			FillAmmo( pPlayer, m_iammo_cells, "cell" );
			FillAmmo( pPlayer, m_iammo_nails, "nail" );
			FillAmmo( pPlayer, m_iammo_explosives, "explosive" );

			if ( g_pGameRules->ItemShouldRespawn( this ) == m_ishouldwerespawn )
				UTIL_Remove( this );

			return true;
		}
		return false;
	}
};
BEGIN_DATADESC( CEntityBackPack )
	DEFINE_FIELD( m_iammo_cells, FIELD_INTEGER, "Cells" ),
	DEFINE_FIELD( m_iammo_nails, FIELD_INTEGER, "Nails" ),
	DEFINE_FIELD( m_iammo_explosives, FIELD_INTEGER, "Explosives" ),
	DEFINE_FIELD( m_iammo_shells, FIELD_INTEGER, "Shells" ),
	DEFINE_FIELD( m_ishouldwerespawn, FIELD_INTEGER, "Shouldwespawn" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( entity_backpack, CEntityBackPack );
