//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_sdk_player.h"
#else
#include "sdk_player.h"
#include "basegrenade_shared.h"
#endif

#include "weapon_sdkbase.h"
#include "weapon_hl2mpbase_machinegun.h"
#include "tfc_projectile_base.h"

#ifdef CLIENT_DLL
#define CWeaponSuperNailGun C_WeaponSuperNailGun
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SMG1_GRENADE_DAMAGE 100.0f
#define SMG1_GRENADE_RADIUS 250.0f
#define BOLT_AIR_VELOCITY	3500
#define BOLT_WATER_VELOCITY	1500

class CWeaponSuperNailGun : public CHL2MPMachineGun
{
public:
	DECLARE_CLASS(CWeaponSuperNailGun, CHL2MPMachineGun);

	CWeaponSuperNailGun();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual SDKWeaponID GetWeaponID(void) const		{ return WEAPON_SUPERNAILGUN; }

	void	Precache(void);
	void	AddViewKick(void);
	virtual void	PrimaryAttack();

	int		GetMinBurst() { return 2; }
	int		GetMaxBurst() { return 5; }

	virtual void Equip(CBaseCombatCharacter *pOwner);
	bool	Reload(void);

	float	GetFireRate(void) { return 0.075f; }	// 13.3hz
	Activity	GetPrimaryAttackActivity(void);

	virtual const Vector& GetBulletSpread(void)
	{
		static const Vector cone = VECTOR_CONE_5DEGREES;
		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;

private:
	CWeaponSuperNailGun(const CWeaponSuperNailGun &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSuperNailGun, DT_WeaponSuperNailGun)

BEGIN_NETWORK_TABLE(CWeaponSuperNailGun, DT_WeaponSuperNailGun)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponSuperNailGun)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_supernailgun, CWeaponSuperNailGun);
PRECACHE_WEAPON_REGISTER(weapon_supernailgun);

#ifndef CLIENT_DLL
acttable_t CWeaponSuperNailGun::m_acttable[] =
{
	{ ACT_MP_STAND_IDLE, ACT_DOD_STAND_IDLE_TOMMY, false },
	{ ACT_MP_CROUCH_IDLE, ACT_DOD_CROUCH_IDLE_TOMMY, false },
	{ ACT_MP_PRONE_IDLE, ACT_DOD_PRONE_AIM_TOMMY, false },

	{ ACT_MP_RUN, ACT_DOD_RUN_AIM_TOMMY, false },
	{ ACT_MP_WALK, ACT_DOD_WALK_AIM_TOMMY, false },
	{ ACT_MP_CROUCHWALK, ACT_DOD_CROUCHWALK_AIM_TOMMY, false },
	{ ACT_MP_PRONE_CRAWL, ACT_DOD_PRONEWALK_IDLE_TOMMY, false },
	{ ACT_SPRINT, ACT_DOD_SPRINT_IDLE_TOMMY, false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE, ACT_DOD_PRIMARYATTACK_TOMMY, false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE, ACT_DOD_PRIMARYATTACK_TOMMY, false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE, ACT_DOD_PRIMARYATTACK_PRONE_TOMMY, false },
	{ ACT_MP_ATTACK_STAND_SECONDARYFIRE, ACT_DOD_SECONDARYATTACK_TOMMY, false },
	{ ACT_MP_ATTACK_CROUCH_SECONDARYFIRE, ACT_DOD_SECONDARYATTACK_CROUCH_TOMMY, false },
	{ ACT_MP_ATTACK_PRONE_SECONDARYFIRE, ACT_DOD_SECONDARYATTACK_PRONE_TOMMY, false },

	{ ACT_MP_RELOAD_STAND, ACT_DOD_RELOAD_TOMMY, false },
	{ ACT_MP_RELOAD_CROUCH, ACT_DOD_RELOAD_CROUCH_TOMMY, false },
	{ ACT_MP_RELOAD_PRONE, ACT_DOD_RELOAD_PRONE_TOMMY, false },

};

IMPLEMENT_ACTTABLE(CWeaponSuperNailGun);
#endif

//=========================================================
CWeaponSuperNailGun::CWeaponSuperNailGun()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 1400;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSuperNailGun::Precache(void)
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther("projectile_nail");
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponSuperNailGun::Equip(CBaseCombatCharacter *pOwner)
{
	m_fMaxRange1 = 1400;

	BaseClass::Equip(pOwner);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponSuperNailGun::GetPrimaryAttackActivity(void)
{
	if (m_nShotsFired < 2)
		return ACT_VM_PRIMARYATTACK;

	if (m_nShotsFired < 3)
		return ACT_VM_RECOIL1;

	if (m_nShotsFired < 4)
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponSuperNailGun::Reload(void)
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound(RELOAD);
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSuperNailGun::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	1.0f	//Degrees
#define	SLIDE_LIMIT			2.0f	//Seconds

	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSuperNailGun::PrimaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	// Abort here to handle burst and auto fire modes
	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	m_nShotsFired++;

	pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;
	float fireRate = GetFireRate();

	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		// MUST call sound before removing a round from the clip of a CHLMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if (UsesClipsForAmmo1())
	{
		if (iBulletsToFire > m_iClip1)
			iBulletsToFire = m_iClip1;
		m_iClip1 -= iBulletsToFire;
	}

	//Fire the Nails
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

#ifndef CLIENT_DLL
	Vector vecAiming = pOwner->GetAutoaimVector(0);
	Vector vecSrc = pOwner->Weapon_ShootPosition();

	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);

	CTFCProjectileBase* pBolt = CTFCProjectileBase::Create("tf_proj_nail", pOwner->Weapon_ShootPosition(), pOwner->EyeAngles(), pOwner, Vector(0, 0, 0), 8.0f);

	if (pOwner->GetWaterLevel() == 3)
	{
		pBolt->SetAbsVelocity(vecAiming * BOLT_WATER_VELOCITY);
	}
	else
	{
		pBolt->SetAbsVelocity(vecAiming * BOLT_AIR_VELOCITY);
	}

#endif

	//Factor in the view kick
	AddViewKick();

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	SendWeaponAnim(GetPrimaryAttackActivity());
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Decrease ammo
	if (UsesClipsForAmmo1())
	{
		pPlayer->RemoveAmmo(1, m_iClip1);
	}
	else
	{
		pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
	}


	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;

}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponSuperNailGun::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 10.0 / 3.0, 0.75 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
