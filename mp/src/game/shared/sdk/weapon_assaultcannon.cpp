//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "sdk_fx_shared.h"

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

	enum ACState_t
	{
		ACT_IDLE,
		ACT_STARTFIRING,
		ACT_FIRING
	};

	CNetworkVar(ACState_t, m_iWeaponState);

	virtual SDKWeaponID GetWeaponID(void) const		{ return WEAPON_AC; }
	virtual float GetWeaponSpread() { return 0.25f; }
	virtual void PrimaryAttack();
	virtual void WeaponIdle();
	void WeaponReset();
	void SharedAttack();
	void WindUp();
	void WindDown();
	virtual bool CanWeaponBeDropped() const				{ return false; }

private:

	CWeaponAC(const CWeaponAC &);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponAC, DT_WeaponAC)

BEGIN_NETWORK_TABLE(CWeaponAC, DT_WeaponAC)
#ifdef CLIENT_DLL
	RecvPropInt(RECVINFO(m_iWeaponState))
#else
	SendPropInt(SENDINFO(m_iWeaponState), 4, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN)
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
	BEGIN_PREDICTION_DATA(CWeaponAC)
	DEFINE_FIELD(m_iWeaponState, FIELD_INTEGER)
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_ac, CWeaponAC);
PRECACHE_WEAPON_REGISTER(weapon_ac);



CWeaponAC::CWeaponAC()
{
	WeaponReset();
}

void CWeaponAC::WindUp()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;
#ifndef CLIENT_DLL
	CPASAttenuationFilter filter(pPlayer, "Weapon_AC.RevUp"); // Filters
	EmitSound(filter, pPlayer->entindex(), "Weapon_AC.RevUp"); // Play Weapon_AC.RevUp sound
#endif
	SendWeaponAnim(ACT_MP_ATTACK_STAND_PREFIRE);
	

	m_iWeaponState = ACT_STARTFIRING;
}

void CWeaponAC::WindDown()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;
#ifndef CLIENT_DLL
	CPASAttenuationFilter filter(pPlayer, "Weapon_AC.RevDown"); // Filters
	EmitSound(filter, pPlayer->entindex(), "Weapon_AC.RevDown"); // Play Weapon_AC.RevDown sound
#endif
	SendWeaponAnim(ACT_MP_ATTACK_STAND_POSTFIRE);
	// Set us back to Idle.
	m_iWeaponState = ACT_IDLE;

}

//Tony; added as a default primary attack if it doesn't get overridden, ie: by CSDKWeaponMelee
void CWeaponAC::PrimaryAttack(void)
{
	SharedAttack();
}

void CWeaponAC::WeaponIdle(void)
{
	// Has the throw animation finished playing
	if (gpGlobals->curtime < m_flTimeWeaponIdle)
		return;
	if (m_iWeaponState != ACT_IDLE)
	{
		CSDKPlayer *pPlayer = GetPlayerOwner();
		if (pPlayer)
		{
			pPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_POST);
		}
		WindDown();
		return;

	}
	BaseClass::WeaponIdle();

	m_flTimeWeaponIdle = gpGlobals->curtime + 12.5f;
}

void CWeaponAC::WeaponReset()
{
	m_iWeaponState = ACT_IDLE;
}

void CWeaponAC::SharedAttack()
{
	// If my clip is empty (and I use clips) start reload
	if (UsesClipsForAmmo1() && !m_iClip1)
	{
		Reload();
		return;
	}

	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	//Tony; check firemodes -- 
	switch (GetFireMode())
	{
	case FM_SEMIAUTOMATIC:
		if (pPlayer->GetShotsFired() > 0)
			return;
		break;
		//Tony; added an accessor to determine the max burst on a per-weapon basis.
	case FM_BURST:
		if (pPlayer->GetShotsFired() > MaxBurstShots())
			return;
		break;
	}

	switch (m_iWeaponState)
	{
	default:
	case ACT_IDLE:
		{
			WindUp();
			float flSpinnupTime = 1.0f;
			flSpinnupTime = Max(flSpinnupTime, FLT_EPSILON);
			if (pPlayer->GetViewModel(m_iViewModelIndex))
				pPlayer->GetViewModel()->SetPlaybackRate(0.75 / flSpinnupTime);

			m_flNextPrimaryAttack = gpGlobals->curtime + flSpinnupTime;

			m_flTimeWeaponIdle = gpGlobals->curtime + flSpinnupTime;
			pPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRE);
			break;
		}
	case ACT_STARTFIRING:
		{
			if (m_flNextPrimaryAttack <= gpGlobals->curtime)
			{
#ifndef CLIENT_DLL
				CPASAttenuationFilter filter(pPlayer, "Weapon_AC.Rev"); // Filters
				EmitSound(filter, pPlayer->entindex(), "Weapon_AC.Rev"); // Play Weapon_AC.RevDown sound
#endif
				m_iWeaponState = ACT_FIRING;

				m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flTimeWeaponIdle = gpGlobals->curtime + 0.1;
			}
			break;
		}
	case ACT_FIRING:
		{
#ifdef GAME_DLL
			pPlayer->NoteWeaponFired();
#endif

			pPlayer->DoMuzzleFlash();

			SendWeaponAnim(GetPrimaryAttackActivity());

			// Make sure we don't fire more than the amount in the clip
			if (UsesClipsForAmmo1())
				m_iClip1--;
			else
				pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);

			pPlayer->IncreaseShotsFired();

			float flSpread = GetWeaponSpread();

			FX_FireBullets(
				pPlayer->entindex(),
				pPlayer->Weapon_ShootPosition(),
				pPlayer->EyeAngles() + pPlayer->GetPunchAngle(),
				GetWeaponID(),
				0, //Tony; fire mode - this is unused at the moment, left over from CSS when SDK* was created in the first place.
				CBaseEntity::GetPredictionRandomSeed() & 255,
				flSpread
				);

			//Add our view kick in
			AddViewKick();

			pPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);
			m_flTimeWeaponIdle = gpGlobals->curtime + 0.2;

		}
	}


	//Tony; update our weapon idle time
	//SetWeaponIdleTime(gpGlobals->curtime + SequenceDuration());

	/*m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	*/
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

