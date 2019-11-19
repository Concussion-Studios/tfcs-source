//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "sdk_weapon_melee.h"

#if defined( CLIENT_DLL )

#define CWeaponUmbrella C_WeaponUmbrella
#include "c_sdk_player.h"

#else

#include "sdk_player.h"

#endif


class CWeaponUmbrella : public CWeaponSDKMelee
{
public:
	DECLARE_CLASS(CWeaponUmbrella, CWeaponSDKMelee);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponUmbrella();

	virtual SDKWeaponID GetWeaponID(void) const		{ return WEAPON_UMBRELLA; }
	virtual float	GetRange(void)					{ return	64.0f; }	//Tony; let the crowbar swing further.
	virtual bool CanWeaponBeDropped() const				{ return false; }

private:

	CWeaponUmbrella(const CWeaponUmbrella &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponUmbrella, DT_WeaponUmbrella)

BEGIN_NETWORK_TABLE(CWeaponUmbrella, DT_WeaponUmbrella)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponUmbrella)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_umbrella, CWeaponUmbrella);
PRECACHE_WEAPON_REGISTER(weapon_umbrella);

CWeaponUmbrella::CWeaponUmbrella()
{
}