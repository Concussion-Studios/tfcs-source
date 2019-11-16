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
#include "soundenvelope.h"

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

	CSoundPatch		*m_pSoundCur;				// the sound being currently played.
	int				m_iACSoundCur;			// the enum value of sound being played.
	float	m_flBarrelCurrentVelocity;
	float	m_flBarrelTargetVelocity;

	enum ACState_t
	{
		ACT_IDLE,
		ACT_STARTFIRING,
		ACT_SPINNING,
		ACT_FIRING
	};

	CNetworkVar(ACState_t, m_iWeaponState);

	virtual SDKWeaponID GetWeaponID(void) const		{ return WEAPON_AC; }
	virtual float GetWeaponSpread() { return 0.25f; }
	virtual void PrimaryAttack();
	virtual void WeaponIdle();
	virtual void	UpdateOnRemove(void);
	virtual void	ItemPreFrame(void);
	void WeaponReset();
	void SharedAttack();
	void WindUp();
	void WindDown();
	void WeaponSoundUpdate();
	void UpdateBarrelMovement();
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
	SendWeaponAnim(ACT_MP_ATTACK_STAND_PREFIRE);
	

	m_iWeaponState = ACT_STARTFIRING;
#ifdef CLIENT_DLL 
	WeaponSoundUpdate();
#endif
}

void CWeaponAC::WindDown()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;
	SendWeaponAnim(ACT_MP_ATTACK_STAND_POSTFIRE);
	// Set us back to Idle.
	m_iWeaponState = ACT_IDLE;
#ifdef CLIENT_DLL 
	WeaponSoundUpdate();
#endif

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

	m_flBarrelCurrentVelocity = 0;
	m_flBarrelTargetVelocity = 0;

#ifdef CLIENT_DLL
	if (m_pSoundCur)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pSoundCur);
		m_pSoundCur = NULL;
	}

	m_iACSoundCur = -1;

#endif
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
				m_iWeaponState = ACT_SPINNING;

				m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flTimeWeaponIdle = gpGlobals->curtime + 0.1;
			}
			break;
		}
	case ACT_SPINNING:
		{
			if(pPlayer->GetAmmoCount(m_iPrimaryAmmoType) > 0)
			{
				m_iWeaponState = ACT_FIRING;
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
			m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

		}
	}


	//Tony; update our weapon idle time
	//SetWeaponIdleTime(gpGlobals->curtime + SequenceDuration());

	/*m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	*/
}
void CWeaponAC::UpdateOnRemove(void)
{
#ifdef CLIENT_DLL
	if (m_pSoundCur)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pSoundCur);
		m_pSoundCur = NULL;
	}
#endif

	BaseClass::UpdateOnRemove();
}

void CWeaponAC::ItemPreFrame(void)
{
#ifdef CLIENT_DLL
	UpdateBarrelMovement();
#endif
	BaseClass::ItemPreFrame();
}
#ifdef CLIENT_DLL

void CWeaponAC::UpdateBarrelMovement()
{
	if (m_flBarrelCurrentVelocity != m_flBarrelTargetVelocity)
	{
		float flSpinupTime = 1.0f;
		flSpinupTime = Max(flSpinupTime, FLT_EPSILON); // Don't divide by 0

		// update barrel velocity to bring it up to speed or to rest
		m_flBarrelCurrentVelocity = Approach(m_flBarrelTargetVelocity, m_flBarrelCurrentVelocity, 0.1 / flSpinupTime);

		if (0 == m_flBarrelCurrentVelocity)
		{
			// if we've stopped rotating, turn off the wind-down sound
			WeaponSoundUpdate();
		}
	}
}


void CWeaponAC::WeaponSoundUpdate()
{
	// determine the desired sound for our current state
	int iSound = -1;
	switch (m_iWeaponState)
	{
	case ACT_IDLE:
		if (m_flBarrelCurrentVelocity > 0)
		{
			iSound = SPECIAL3;	// wind down sound
			if (m_flBarrelTargetVelocity > 0)
			{
				m_flBarrelTargetVelocity = 0;
			}
		}
		else
			iSound = -1;
		break;
	case ACT_STARTFIRING:
		iSound = SPECIAL2;	// wind up sound
		break;
	case ACT_FIRING:
	{
		iSound = SINGLE; // firing sound
	}
	break;
	case ACT_SPINNING:
		iSound = SPECIAL1;	// spinning sound
		break;
	default:
		Assert(false);
		break;
	}

	// if we're already playing the desired sound, nothing to do
	if (m_iACSoundCur == iSound)
		return;

	// if we're playing some other sound, stop it
	if (m_pSoundCur)
	{
		// Stop the previous sound immediately
		CSoundEnvelopeController::GetController().SoundDestroy(m_pSoundCur);
		m_pSoundCur = NULL;
	}
	m_iACSoundCur = iSound;
	// if there's no sound to play for current state, we're done
	if (-1 == iSound)
		return;

	// play the appropriate sound
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	const char *shootsound = GetShootSound(iSound);
	CLocalPlayerFilter filter;
	m_pSoundCur = controller.SoundCreate(filter, entindex(), shootsound);
	controller.Play(m_pSoundCur, 1.0, 100);
	controller.SoundChangeVolume(m_pSoundCur, 1.0, 0.1);
}
#endif

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

