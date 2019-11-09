//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "weapon_sdkbase_combatweapon.h"
#include "tfc_projectile_base.h"
#include "sdk_fx_shared.h"

#ifdef CLIENT_DLL
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif

#ifdef CLIENT_DLL
	#define CWeaponSuperNailGun C_WeaponSuperNailGun
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BOLT_AIR_VELOCITY	3500
#define BOLT_WATER_VELOCITY	1500

class CWeaponSuperNailGun : public CSDKMachineGun
{
public:
	DECLARE_CLASS( CWeaponSuperNailGun, CSDKMachineGun );

	CWeaponSuperNailGun();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_SUPERNAILGUN; }
	virtual bool CanWeaponBeDropped() const { return false; }

	void Precache( void );
	void AddViewKick( void );
	virtual void PrimaryAttack();

	int GetMinBurst() { return 2; }
	int GetMaxBurst() { return 5; }

	virtual void Equip( CBaseCombatCharacter *pOwner );
	bool Reload( void );

	float GetFireRate( void ) { return 0.075f; }	// 13.3hz
	Activity GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_5DEGREES;
		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;

private:
	CWeaponSuperNailGun( const CWeaponSuperNailGun & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSuperNailGun, DT_WeaponSuperNailGun )

BEGIN_NETWORK_TABLE( CWeaponSuperNailGun, DT_WeaponSuperNailGun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSuperNailGun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_supernailgun, CWeaponSuperNailGun );
PRECACHE_WEAPON_REGISTER( weapon_supernailgun );

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
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{
		Reload();
		return;
	}
 
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;
 
	//Tony; check firemodes -- 
	switch(GetFireMode())
	{
	case FM_SEMIAUTOMATIC:
		if ( pPlayer->GetShotsFired() > 0 )
			return;
		break;
		//Tony; added an accessor to determine the max burst on a per-weapon basis.
	case FM_BURST:
		if ( pPlayer->GetShotsFired() > MaxBurstShots() )
			return;
		break;
	}

#ifndef CLIENT_DLL
	Vector vecAiming = pPlayer->GetAutoaimVector( 0 );
	Vector vecSrc = pPlayer->Weapon_ShootPosition();

	QAngle angAiming;
	VectorAngles( vecAiming, angAiming );

	CTFCProjectileBase* pBolt = CTFCProjectileBase::Create( "tf_proj_nail", pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles(), pPlayer, Vector( 0,0,0 ), 8.0f );
	if ( pPlayer->GetWaterLevel() == 3 )
		pBolt->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
	else
		pBolt->SetAbsVelocity( vecAiming * BOLT_AIR_VELOCITY );

#endif

#ifdef GAME_DLL
	pPlayer->NoteWeaponFired();
#endif
 
	pPlayer->DoMuzzleFlash();
 
	SendWeaponAnim( GetPrimaryAttackActivity() );
 
	// Make sure we don't fire more than the amount in the clip
	if ( UsesClipsForAmmo1() )
		m_iClip1 --;
	else
		pPlayer->RemoveAmmo( 1, m_iPrimaryAmmoType );
 
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
 
	//Tony; update our weapon idle time
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
 
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

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
