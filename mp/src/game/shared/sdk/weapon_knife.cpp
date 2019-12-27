//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "sdk_weapon_melee.h"

#if defined( CLIENT_DLL )

#define CWeaponKnife C_WeaponKnife
#include "c_sdk_player.h"

#else

#include "sdk_player.h"

#endif


class CWeaponKnife : public CWeaponSDKMelee
{
public:
	DECLARE_CLASS(CWeaponKnife, CWeaponSDKMelee);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponKnife();

	virtual SDKWeaponID GetWeaponID(void) const		{ return WEAPON_KNIFE; }

private:

	CWeaponKnife(const CWeaponKnife &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponKnife, DT_WeaponKnife)

BEGIN_NETWORK_TABLE(CWeaponKnife, DT_WeaponKnife)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponKnife)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_knife, CWeaponKnife);
PRECACHE_WEAPON_REGISTER(weapon_knife);

CWeaponKnife::CWeaponKnife()
{
}