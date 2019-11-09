//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "weapon_sdkbase_combatweapon.h"
#include "sdk_player_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( SDKCombatWeapon, CSDKCombatWeapon );

IMPLEMENT_NETWORKCLASS_ALIASED( SDKCombatWeapon , DT_SDKCombatWeapon )

BEGIN_NETWORK_TABLE( CSDKCombatWeapon , DT_SDKCombatWeapon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CSDKCombatWeapon )
END_PREDICTION_DATA()

CSDKCombatWeapon::CSDKCombatWeapon( void )
{

}

#if defined( CLIENT_DLL )
//-----------------------------------------------------------------------------
Vector CSDKCombatWeapon::GetBulletSpread( WeaponProficiency_t proficiency )
{
	return CBaseCombatWeapon::GetBulletSpread( proficiency );
}

//-----------------------------------------------------------------------------
float CSDKCombatWeapon::GetSpreadBias( WeaponProficiency_t proficiency )
{
	return CBaseCombatWeapon::GetSpreadBias( proficiency );
}
//-----------------------------------------------------------------------------

const WeaponProficiencyInfo_t *CSDKCombatWeapon::GetProficiencyValues()
{
	return NULL;
}

#else

//-----------------------------------------------------------------------------
Vector CSDKCombatWeapon::GetBulletSpread( WeaponProficiency_t proficiency )
{
	Vector baseSpread = CBaseCombatWeapon::GetBulletSpread( proficiency );

	const WeaponProficiencyInfo_t *pProficiencyValues = GetProficiencyValues();
	float flModifier = (pProficiencyValues)[ proficiency ].spreadscale;
	return ( baseSpread * flModifier );
}

//-----------------------------------------------------------------------------
float CSDKCombatWeapon::GetSpreadBias( WeaponProficiency_t proficiency )
{
	const WeaponProficiencyInfo_t *pProficiencyValues = GetProficiencyValues();
	return (pProficiencyValues)[ proficiency ].bias;
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CSDKCombatWeapon::GetProficiencyValues()
{
	return GetDefaultProficiencyValues();
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CSDKCombatWeapon::GetDefaultProficiencyValues()
{
	// Weapon proficiency table. Keep this in sync with WeaponProficiency_t enum in the header!!
	static WeaponProficiencyInfo_t g_BaseWeaponProficiencyTable[] =
	{
		{ 2.50, 1.0	},
		{ 2.00, 1.0	},
		{ 1.50, 1.0	},
		{ 1.25, 1.0 },
		{ 1.00, 1.0	},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(g_BaseWeaponProficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return g_BaseWeaponProficiencyTable;
}

#endif