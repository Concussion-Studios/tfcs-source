//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

#define CWeaponAC C_WeaponAC
#include "c_sdk_player.h"

#else

#include "sdk_player.h"

#endif


class CWeaponAC : public CWeaponSDKBase
{
public:
	DECLARE_CLASS(CWeaponAC, CWeaponSDKBase);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponAC();

	virtual SDKWeaponID GetWeaponID(void) const		{ return WEAPON_AC; }
	virtual float GetWeaponSpread() { return 0.08f; }
	virtual bool CanWeaponBeDropped() const				{ return false; }

private:

	CWeaponAC(const CWeaponAC &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponAC, DT_WeaponAC)

BEGIN_NETWORK_TABLE(CWeaponAC, DT_WeaponAC)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponAC)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_ac, CWeaponAC);
PRECACHE_WEAPON_REGISTER(weapon_ac);



CWeaponAC::CWeaponAC()
{
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponAC::m_acttable[] =
{
	{ ACT_MP_STAND_IDLE, ref_aim_AC, false },
	{ ACT_MP_CROUCH_IDLE, crouch_aim_AC, false },
	{ ACT_MP_PRONE_IDLE, ACT_DOD_PRONE_AIM_TOMMY, false },

	{ ACT_MP_RUN, run2, false },
	{ ACT_MP_WALK, walk2handed, false },
	{ ACT_MP_CROUCHWALK, ACT_DOD_CROUCHWALK_AIM_TOMMY, false },
	{ ACT_MP_PRONE_CRAWL, ACT_DOD_PRONEWALK_IDLE_TOMMY, false },
	{ ACT_SPRINT, ACT_DOD_SPRINT_IDLE_TOMMY, false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE, ref_shoot_AC, false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE, ACT_DOD_PRIMARYATTACK_TOMMY, false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE, ACT_DOD_PRIMARYATTACK_PRONE_TOMMY, false },
	{ ACT_MP_ATTACK_STAND_SECONDARYFIRE, ACT_DOD_SECONDARYATTACK_TOMMY, false },
	{ ACT_MP_ATTACK_CROUCH_SECONDARYFIRE, ACT_DOD_SECONDARYATTACK_CROUCH_TOMMY, false },
	{ ACT_MP_ATTACK_PRONE_SECONDARYFIRE, ACT_DOD_SECONDARYATTACK_PRONE_TOMMY, false },

	{ ACT_MP_RELOAD_STAND, ACT_DOD_RELOAD_TOMMY, false },
	{ ACT_MP_RELOAD_CROUCH, ACT_DOD_RELOAD_CROUCH_TOMMY, false },
	{ ACT_MP_RELOAD_PRONE, ACT_DOD_RELOAD_PRONE_TOMMY, false },

};

IMPLEMENT_ACTTABLE(CWeaponAC);

