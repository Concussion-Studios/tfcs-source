//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "sdk_fx_shared.h"


#if defined( CLIENT_DLL )

#define CWeapon12Gauge C_Weapon12Gauge
#include "c_sdk_player.h"

#else

#include "sdk_player.h"
#include "te_firebullets.h"

#endif


class CWeapon12Gauge : public CWeaponSDKBase
{
public:
	DECLARE_CLASS(CWeapon12Gauge, CWeaponSDKBase);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeapon12Gauge();

	virtual void PrimaryAttack();
	virtual bool Reload();
	virtual void WeaponIdle();

	virtual SDKWeaponID GetWeaponID(void) const		{ return WEAPON_12GAUGE; }
	virtual float GetWeaponSpread() { return 0.04362f; }


private:

	CWeapon12Gauge(const CWeapon12Gauge  &);

	float m_flPumpTime;
	CNetworkVar(int, m_iInSpecialReload);

};

IMPLEMENT_NETWORKCLASS_ALIASED(Weapon12Gauge, DT_Weapon12Gauge)

BEGIN_NETWORK_TABLE(CWeapon12Gauge, DT_Weapon12Gauge)

#ifdef CLIENT_DLL
RecvPropInt(RECVINFO(m_iInSpecialReload))
#else
SendPropInt(SENDINFO(m_iInSpecialReload), 2, SPROP_UNSIGNED)
#endif

END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeapon12Gauge)
#ifdef CLIENT_DLL
DEFINE_PRED_FIELD(m_iInSpecialReload, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
#endif
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_12gauge, CWeapon12Gauge);
PRECACHE_WEAPON_REGISTER(weapon_12gauge);



CWeapon12Gauge::CWeapon12Gauge()
{
	m_flPumpTime = 0;
}

void CWeapon12Gauge::PrimaryAttack()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if (!pPlayer)
		return;

	// don't fire underwater
	if (pPlayer->GetWaterLevel() == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.15;
		return;
	}
	bool doPunch = true;
	if (m_iClip1 <= 0)
		doPunch = false;

	BaseClass::PrimaryAttack();

	if (doPunch)
	{
		m_iInSpecialReload = 0;

		// Update punch angles.
		QAngle angle = pPlayer->GetPunchAngle();

		if (pPlayer->GetFlags() & FL_ONGROUND)
		{
			angle.x -= SharedRandomInt("ShotgunPunchAngleGround", 4, 6);
		}
		else
		{
			angle.x -= SharedRandomInt("ShotgunPunchAngleAir", 8, 11);
		}

		pPlayer->SetPunchAngle(angle);
	}
}


bool CWeapon12Gauge::Reload()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0 || m_iClip1 == GetMaxClip1())
		return true;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return true;

	// check to see if we're ready to reload
	if (m_iInSpecialReload == 0)
	{
		pPlayer->SetAnimation(PLAYER_RELOAD);

		SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);
		m_iInSpecialReload = 1;
		pPlayer->m_flNextAttack = gpGlobals->curtime + 0.5;
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;
		SetWeaponIdleTime(gpGlobals->curtime + 0.5);
		return true;
	}
	else if (m_iInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > gpGlobals->curtime)
			return true;
		// was waiting for gun to move to side
		m_iInSpecialReload = 2;

		SendWeaponAnim(ACT_VM_RELOAD);
		SetWeaponIdleTime(gpGlobals->curtime + 0.45);
	}
	else if (m_iInSpecialReload == 2) // Sanity, make sure it's actually in the right state.
	{
		// Add them to the clip
		m_iClip1 += 2;

#ifdef GAME_DLL
		SendReloadEvents();
#endif
		CSDKPlayer *pPlayer = GetPlayerOwner();

		if (pPlayer)
			pPlayer->RemoveAmmo(2, m_iPrimaryAmmoType);

		m_iInSpecialReload = 1;
	}

	return true;
}


void CWeapon12Gauge::WeaponIdle()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (m_flPumpTime && m_flPumpTime < gpGlobals->curtime)
	{
		// play pumping sound
		m_flPumpTime = 0;
	}

	if (m_flTimeWeaponIdle < gpGlobals->curtime)
	{
		if (m_iClip1 == 0 && m_iInSpecialReload == 0 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType))
		{
			Reload();
		}
		else if (m_iInSpecialReload != 0)
		{
			if (m_iClip1 != 8 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType))
			{
				Reload();
			}
			else
			{
				// reload debounce has timed out
				SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);

				// play cocking sound
				m_iInSpecialReload = 0;
				SetWeaponIdleTime(gpGlobals->curtime + 1.5);
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.15; // Add a small delay between finishing reload and firing again
			}
		}
		else
		{
			SendWeaponAnim(ACT_VM_IDLE);
		}
	}
}


//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeapon12Gauge::m_acttable[] =
{
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_SHOTGUN, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_SHOTGUN, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_SHOTGUN, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_SHOTGUN, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, false },
};

IMPLEMENT_ACTTABLE(CWeapon12Gauge);
