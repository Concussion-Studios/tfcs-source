//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "weapon_rpg.h"

#ifdef CLIENT_DLL
	#include "c_sdk_player.h"
	#include "model_types.h"
	#include "beamdraw.h"
	#include "fx_line.h"
	#include "view.h"
#else
	#include "basecombatcharacter.h"
	#include "movie_explosion.h"
	#include "soundent.h"
	#include "player.h"
	#include "rope.h"
	#include "vstdlib/random.h"
	#include "engine/IEngineSound.h"
	#include "explode.h"
	#include "util.h"
	#include "in_buttons.h"
	#include "shake.h"
	#include "te_effect_dispatch.h"
	#include "triggers.h"
	#include "smoke_trail.h"
	#include "collisionutils.h"
	#include "sdk_shareddefs.h"
	#include "weapon_sdkbase.h"
	#include "basecombatweapon_shared.h"
	#include "tfc_projectile_base.h"
#endif

#include "debugoverlay_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	RPG_AIR_SPEED	1500
#define	RPG_WATER_SPEED	800

#ifndef CLIENT_DLL

#endif



#ifndef CLIENT_DLL

class CWeaponRPG;

#endif

#define	RPG_BEAM_SPRITE		"effects/laser1.vmt"
#define	RPG_BEAM_SPRITE_NOZ	"effects/laser1_noz.vmt"
#define	RPG_LASER_SPRITE	"sprites/redglow1"

//=============================================================================
// RPG
//=============================================================================

LINK_ENTITY_TO_CLASS( weapon_rpg, CWeaponRPG );
PRECACHE_WEAPON_REGISTER(weapon_rpg);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRPG, DT_WeaponRPG )

BEGIN_NETWORK_TABLE( CWeaponRPG, DT_WeaponRPG )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bInitialStateUpdate ) ),
#else
	SendPropBool( SENDINFO( m_bInitialStateUpdate ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponRPG )
	DEFINE_PRED_FIELD( m_bInitialStateUpdate, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA()

#endif

#ifndef CLIENT_DLL
acttable_t	CWeaponRPG::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_RPG,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_RPG,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_RPG,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_RPG,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_RPG,		false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_RPG,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_RPG,				false },
};

IMPLEMENT_ACTTABLE(CWeaponRPG);

#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponRPG::CWeaponRPG()
{
	m_bReloadsSingly = true;
	m_bInitialStateUpdate= false;

	m_fMinRange1 = m_fMinRange2 = 40*12;
	m_fMaxRange1 = m_fMaxRange2 = 500*12;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponRPG::~CWeaponRPG()
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "Missile.Ignite" );
	PrecacheScriptSound( "Missile.Accelerate" );

	// Laser dot...
	PrecacheModel( "sprites/redglow1.vmt" );
	PrecacheModel( RPG_LASER_SPRITE );
	PrecacheModel( RPG_BEAM_SPRITE );
	PrecacheModel( RPG_BEAM_SPRITE_NOZ );

#ifndef CLIENT_DLL
	UTIL_PrecacheOther("tf_proj_rocket");
#endif

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG::Activate( void )
{
	BaseClass::Activate();

}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG::PrimaryAttack( void )
{
	if (UsesClipsForAmmo1() && !m_iClip1)
	{
		Reload();
		return;
	}
	// Only the player fires this way so we can cast
	CSDKPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	// Can't have an active missile out
	/*if ( m_hMissile != NULL )
		return;*/

	// Can't be reloading
	if (GetActivity() == ACT_VM_RELOAD_DEPLOYED)
		return;

	Vector vecOrigin;
	Vector vecForward;

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	CSDKPlayer *pOwner = GetPlayerOwner();
	
	if ( pOwner == NULL )
		return;

	Vector	vForward, vRight, vUp;

	pOwner->EyeVectors( &vForward, &vRight, &vUp );

	Vector	muzzlePoint = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;

#ifndef CLIENT_DLL
	Vector vecAiming = pPlayer->GetAutoaimVector(0);
	QAngle vecAngles;
	VectorAngles( vForward, vecAngles );

	CTFCProjectileBase* pBolt = CTFCProjectileBase::Create("tf_proj_rocket", pPlayer->Weapon_ShootPosition(), pPlayer->EyeAngles(), pPlayer, Vector(0, 0, 0), 8.0f);
	if (pPlayer->GetWaterLevel() == 3)
		pBolt->SetAbsVelocity(vecAiming * RPG_WATER_SPEED);
	else
		pBolt->SetAbsVelocity(vecAiming * RPG_AIR_SPEED);

	//CMissile *pMissile = CMissile::Create( muzzlePoint, vecAngles, GetOwner()->edict() );
	//pMissile->m_hOwner = this;

	// If the shot is clear to the player, give the missile a grace period
	trace_t	tr;
	Vector vecEye = pOwner->EyePosition();
	UTIL_TraceLine( vecEye, vecEye + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	/*if ( tr.fraction == 1.0 )
	{
		pMissile->SetGracePeriod( 0.3 );
	}

	pMissile->SetDamage(GetSDKWpnData().m_iDamage);
	*/
	//m_hMissile = pMissile;
#endif

	DecrementAmmo( GetOwner() );
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	WeaponSound( SINGLE );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponRPG::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	
	// Take away our primary ammo type
	if (UsesClipsForAmmo1())
		m_iClip1--;
	else
		pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponRPG::Deploy( void )
{
	m_bInitialStateUpdate = true;

	return BaseClass::Deploy();
}

bool CWeaponRPG::CanHolster( void )
{
	//Can't have an active missile out
	/*if ( m_hMissile != NULL )
		return false;*/

	return BaseClass::CanHolster();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponRPG::Holster( CBaseCombatWeapon *pSwitchingTo )
{

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponRPG::Drop( const Vector &vecVelocity )
{

	BaseClass::Drop( vecVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponRPG::Reload( void )
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	FillClip();
	// Play reload on different channel as otherwise steals channel away from fire sound
	WeaponSound(RELOAD);
	SendWeaponAnim(ACT_VM_RELOAD_DEPLOYED);

	return true;

}

void CWeaponRPG::FillClip(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	// Add them to the clip
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
	{
		if (Clip1() < GetMaxClip1())
		{
			m_iClip1 +=1;
			pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
		}
	}
}

#ifdef CLIENT_DLL


#endif	//CLIENT_DLL
