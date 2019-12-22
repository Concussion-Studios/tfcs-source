//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "sdk_weapon_melee.h"

#if defined( CLIENT_DLL )

#define CWeaponWrench C_WeaponWrench
#include "c_sdk_player.h"

#else

#include "sdk_player.h"

#endif


class CWeaponWrench : public CWeaponSDKMelee
{
public:
	DECLARE_CLASS(CWeaponWrench, CWeaponSDKMelee);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponWrench();

	virtual SDKWeaponID GetWeaponID(void) const		{ return WEAPON_WRENCH; }
	virtual float	GetRange(void)					{ return	64.0f; }	//Tony; let the crowbar swing further.
	virtual bool CanWeaponBeDropped() const				{ return false; }

private:

	CWeaponWrench(const CWeaponWrench &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponWrench, DT_WeaponWrench)

BEGIN_NETWORK_TABLE(CWeaponWrench, DT_WeaponWrench)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponWrench)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_spanner, CWeaponWrench);
PRECACHE_WEAPON_REGISTER(weapon_spanner);

CWeaponWrench::CWeaponWrench()
{
}
