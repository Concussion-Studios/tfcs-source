//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "weapon_sdkbase.h"
#include "ammodef.h"
#include "datacache/imdlcache.h"
#include "sdk_fx_shared.h"
#include "sdk_gamerules.h"
#include "tfc_viewmodel.h"

#if defined( CLIENT_DLL )
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
	#include "vphysics/constraints.h"
#endif

//Crowbar/Umbrella/Wrench/Knife
acttable_t CWeaponSDKBase::m_acttableMelee[] =
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_MELEE,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_MELEE,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_MELEE,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_MELEE,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_MELEE,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_MELEE,					false },
};

// Pistol/Tranquilizer
acttable_t CWeaponSDKBase::m_acttableHandGun[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_PISTOL,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_PISTOL,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_PISTOL,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_PISTOL,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_PISTOL,					false },
};

// Shotguns
acttable_t CWeaponSDKBase::m_acttableTwoHandsGuns[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_SHOTGUN,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_SHOTGUN,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_SHOTGUN,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_SHOTGUN,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_SHOTGUN,					false },
};

// Nailgun
acttable_t CWeaponSDKBase::m_acttableNoReload[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_PHYSGUN,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_PHYSGUN,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_PHYSGUN,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_PHYSGUN,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_PHYSGUN,					false },
};

// Assault Cannon
acttable_t CWeaponSDKBase::m_acttableAC[] =
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2AC_IDLE_GATLING,						false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2AC_CROUCH_GATLING,					false },

	{ ACT_MP_RUN,						ACT_HL2AC_RUN_GATLING,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2AC_WALK_CROUCH_GATLING,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2AC_GESTURE_RANGE_ATTACK01_GATLING,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2AC_GESTURE_RANGE_ATTACK01_GATLING,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,			false },

	{ ACT_MP_JUMP,						ACT_HL2AC_JUMP_GATLING,						false },
};

// Sniper Rifle
acttable_t CWeaponSDKBase::m_acttableSniper[] =
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2AC_IDLE_SNIPER,						false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2AC_CROUCH_GATLING,					false },

	{ ACT_MP_RUN,						ACT_HL2AC_RUN_SNIPER,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2AC_WALK_CROUCH_SNIPER,					false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2AC_GESTURE_RANGE_ATTACK_SNIPER,		false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2AC_GESTURE_RANGE_ATTACK_SNIPER,		false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_SMG1,				false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_SMG1,				false },

	{ ACT_MP_JUMP,						ACT_HL2AC_JUMP_SNIPER,						false },
};

// RPG/IC
acttable_t CWeaponSDKBase::m_acttableRPG[] =
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_RPG,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_RPG,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_RPG,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_RPG,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_RPG,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_RPG,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_RPG,					false },
};

// Grenades
acttable_t CWeaponSDKBase::m_acttableGrenade[] =
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_GRENADE,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_GRENADE,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_GRENADE,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_GRENADE,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_GRENADE,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_GRENADE,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_GRENADE,					false },
};

// Tools
acttable_t CWeaponSDKBase::m_acttableTool[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_SLAM,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_SLAM,				false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_SLAM,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_SLAM,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_SLAM,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_SLAM,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_SLAM,					false },
};
 
// ----------------------------------------------------------------------------- //
// CWeaponSDKBase tables.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSDKBase, DT_WeaponSDKBase )
 
BEGIN_NETWORK_TABLE( CWeaponSDKBase, DT_WeaponSDKBase )
#ifdef CLIENT_DLL
	RecvPropFloat( RECVINFO( m_flDecreaseShotsFired ) ),
	RecvPropBool( RECVINFO( m_bIsScoped ) ),
#else
	SendPropExclude( "DT_BaseAnimating", "m_nNewSequenceParity" ),
	SendPropExclude( "DT_BaseAnimating", "m_nResetEventsParity" ),
	SendPropFloat( SENDINFO( m_flDecreaseShotsFired ) ),
	SendPropBool( SENDINFO( m_bIsScoped ) ),
#endif
END_NETWORK_TABLE()
 
#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponSDKBase )
	DEFINE_PRED_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_bIsScoped, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),	// I believe the client may want this predicted for actually displaying the scope
END_PREDICTION_DATA()
#endif
 
#ifdef GAME_DLL
BEGIN_DATADESC( CWeaponSDKBase )
	// New weapon Think and Touch Functions go here..
END_DATADESC()
#endif
 
#ifdef CLIENT_DLL
void UTIL_ClipPunchAngleOffset( QAngle &in, const QAngle &punch, const QAngle &clip )
{
	QAngle	final = in + punch;
 
	//Clip each component
	for ( int i = 0; i < 3; i++ )
	{
		if ( final[i] > clip[i] )
			final[i] = clip[i];
		else if ( final[i] < -clip[i] )
			final[i] = -clip[i];
 
		//Return the result
		in[i] = final[i] - punch[i];
	}
}
#endif

// ----------------------------------------------------------------------------- //
// CWeaponCSBase implementation. 
// ----------------------------------------------------------------------------- //
CWeaponSDKBase::CWeaponSDKBase()
{
	SetPredictionEligible( true );
 
	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.

	m_flNextResetCheckTime = 0.0f;

	m_bIsScoped = false;
}

#ifdef CLIENT_DLL
bool CWeaponSDKBase::ShouldPredict()
{
	if ( GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer())
		return true;
 
	return BaseClass::ShouldPredict();
}

void CWeaponSDKBase::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( GetPredictable() && !ShouldPredict() )
		ShutdownPredictable();
}
#endif 

const CSDKWeaponInfo &CWeaponSDKBase::GetSDKWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CSDKWeaponInfo *pSDKInfo;
 
#ifdef _DEBUG
	pSDKInfo = dynamic_cast< const CSDKWeaponInfo* >( pWeaponInfo );
	Assert( pSDKInfo );
#else
	pSDKInfo = static_cast< const CSDKWeaponInfo* >( pWeaponInfo );
#endif
 
	return *pSDKInfo;
}
 
bool CWeaponSDKBase::PlayEmptySound()
{
	CPASAttenuationFilter filter( this );
	filter.UsePredictionRules();
 
	EmitSound( filter, entindex(), "Default.ClipEmpty_Rifle" );
 
	return 0;
}
 
CSDKPlayer* CWeaponSDKBase::GetPlayerOwner() const
{
	return dynamic_cast< CSDKPlayer* >( GetOwner() );
}
 
float CWeaponSDKBase::GetAccuracyModifier()
{
	float weaponAccuracy = 1.0f; // by default, don't make any alterations

	CSDKPlayer *pPlayer = ToSDKPlayer( GetOwner() );
	if ( pPlayer )
	{
		if( !fabs( pPlayer->GetAbsVelocity().x ) && !fabs( pPlayer->GetAbsVelocity().y ) )	// player isn't moving
			weaponAccuracy *= 0.75f;
		else if( !!( pPlayer->GetFlags() & FL_DUCKING ) )	// player is ducking
			weaponAccuracy *= 0.80f;
		else if ( !( GetFlags() & FL_ONGROUND ) )	// player is not on the ground
			weaponAccuracy *= 3.0f;
		else if ( GetMoveType() == MOVETYPE_LADDER )	// player is on a ladder
			weaponAccuracy *= 3.0f;
		else if ( IsScoped() )	// player is on scope mode
			weaponAccuracy = 0.01f;
	}

	return weaponAccuracy;
}

void CWeaponSDKBase::SetWeaponVisible( bool visible )
{
	CBaseViewModel *vm = nullptr;
	CTFCViewModel *vmhands = nullptr;

	CSDKPlayer *pOwner = ToSDKPlayer( GetOwner() );
	if ( pOwner )
	{
		vm = pOwner->GetViewModel( VMINDEX_WEP );
		vmhands = static_cast< CTFCViewModel* >( pOwner->GetViewModel( VMINDEX_HANDS ) );

#ifndef CLIENT_DLL
		Assert( vm == pOwner->GetViewModel( m_nViewModelIndex ) );
#endif
	}

	if ( visible )
	{
		// Fix a weapon disappearing bug
		// Only having the client do this should fix an issue that makes players' weapons disappear to other players
		// In a majority of cases, this is not desired (I actually can't think of a single one where it would be)
#ifdef CLIENT_DLL
		RemoveEffects( EF_NODRAW );
#endif
		if ( vm )
			vm->RemoveEffects( EF_NODRAW );

		if ( vmhands )
		{
			vmhands->RemoveEffects( EF_NODRAW );
#ifdef CLIENT_DLL 
			// Let client override this if they are using a custom viewmodel that doesn't use the new hands system.
			vmhands->SetDrawVM( true );
#endif
		}
	}
	else
	{
		// Fix a weapon disappearing bug
		// Only having the client do this should fix an issue that makes players' weapons disappear to other players
		// In a majority of cases, this is not desired (I actually can't think of a single one where it would be)
#ifdef CLIENT_DLL
		AddEffects( EF_NODRAW );
#endif
		if ( vm )
			vm->AddEffects( EF_NODRAW );

		if ( vmhands ) 
			vmhands->AddEffects( EF_NODRAW );
	}
}

//Tony; added as a default primary attack if it doesn't get overridden, ie: by CSDKWeaponMelee
void CWeaponSDKBase::PrimaryAttack( void )
{
	int preShotAmmo = m_iClip1;

	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{
		Reload();
		return;
	}
 
	CSDKPlayer *pPlayer = GetPlayerOwner();
 
	if (!pPlayer)
		return;
 
	//Tony; check firemodes -- 
	switch(GetFireMode())
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
#ifdef GAME_DLL
	pPlayer->NoteWeaponFired();
#endif
 
	pPlayer->DoMuzzleFlash();
 
	SendWeaponAnim( GetPrimaryAttackActivity() );
 
	// Make sure we don't fire more than the amount in the clip
	if ( UsesClipsForAmmo1() )
		m_iClip1 --;
	else
		pPlayer->RemoveAmmo( GetAmmoToRemove(), m_iPrimaryAmmoType );
 
	pPlayer->IncreaseShotsFired();
 
	//float flSpread = GetWeaponSpread();
 
	/*FX_FireBullets( //commented out so derived classes like the nailgun dont shoot bullets, copy paste this into your weapon if you want it to shoot bullets after BaseClass::PrimaryAttack
		pPlayer->entindex(),
		pPlayer->Weapon_ShootPosition(),
		pPlayer->EyeAngles() + pPlayer->GetPunchAngle(),
		GetWeaponID(),
		0, //Tony; fire mode - this is unused at the moment, left over from CSS when SDK* was created in the first place.
		CBaseEntity::GetPredictionRandomSeed() & 255,
		flSpread
		);*/
 
 
	//Add our view kick in
	AddViewKick();
 
	if ( HasScope() && UnscopeAfterShot() && (preShotAmmo > 0) )
		ExitScope();	// done after actually shooting because it is logical and should prevent any unnecessary accuracy changes

	//Tony; update our weapon idle time
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
 
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponSDKBase::SecondaryAttack( void )
{
	// Overrides secondary attack of weapons with scopes
	// This may not be desired, but that's just how it works, so adjust all scoped weapons accordingly and keep this in mind!
	if ( HasScope() )
	{
		// Toggle scope
		if ( IsScoped() )
			ExitScope();
		else
			EnterScope();
	}
	else
	{
		Assert( 0 && "SecondaryAttack should not be called. Make sure to implement this in your subclass!\n" );
	}
}

bool CWeaponSDKBase::CanScope( void )
{
	// If this weapon doesn't have a scope, how are we expected to use it?
	// We really shouldn't have to check for this by the time this function is called, but just in case...
	if ( !HasScope() )
		return false;

	// If this weapon requires the player to unscope after firing, wait until we're allowed to fire again before the scope can be used again
	if ( UnscopeAfterShot() && (gpGlobals->curtime < m_flNextPrimaryAttack) )
		return false;

	CSDKPlayer *pOwner = ToSDKPlayer( GetOwner() );
	if ( !pOwner )
		return false;

	return true;
}

void CWeaponSDKBase::EnterScope( void )
{
	if ( !CanScope() )
		return;	// don't scope if we're not allowed to right now!

	CSDKPlayer *pOwner = ToSDKPlayer( GetOwner() );
	if ( !pOwner )
		return;
	
	// Only scope and stuff if we have an owner
	m_bIsScoped = true;
	pOwner->SetFOV( pOwner, GetScopeFOV(), 0.1f );	// zoom
	SetWeaponVisible( false );	// hide the view model

	m_flNextSecondaryAttack	= gpGlobals->curtime + 0.25f;	// make a bit of a delay between zooming/unzooming to prevent spam and possibly some bugs
}

void CWeaponSDKBase::ExitScope( bool unhideWeapon )
{
	m_bIsScoped = false;	// unscope regardless of whether or not we have an owner (should prevent some bugs)

	CSDKPlayer *pOwner = ToSDKPlayer( GetOwner() );
	if ( !pOwner )
		return;

	pOwner->SetFOV( pOwner, pOwner->GetDefaultFOV(), 0.1f );	// unzoom

	if ( unhideWeapon )	// there are some situations where we may not want to do this to prevent interfering with other systems
		SetWeaponVisible( true );	// show the view model again

	m_flNextSecondaryAttack	= gpGlobals->curtime + 0.25f;	// make a bit of a delay between zooming/unzooming to prevent spam and possibly some bugs
}

//Tony; added so we can have base functionality without implementing it into every weapon.
void CWeaponSDKBase::ItemPostFrame( void )
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;

	// We're not allowed to scope right now yet we are scoped, so unscope right away!
	if ( !CanScope() && IsScoped() )
		ExitScope();
 
	//
	//Tony; totally override the baseclass
	//
 
	if ( UsesClipsForAmmo1() )
		CheckReload();
 
	bool bFired = false;
 
	// Secondary attack has priority
	if ((pPlayer->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime) && pPlayer->CanAttack())
	{
		if (UsesSecondaryAmmo() && pPlayer->GetAmmoCount(m_iSecondaryAmmoType)<=0 )
		{
			if (m_flNextEmptySoundTime < gpGlobals->curtime)
			{
				WeaponSound(EMPTY);
				m_flNextSecondaryAttack = m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
			}
		}
		else
		{
			bFired = true;
			SecondaryAttack();
 
			// Secondary ammo doesn't have a reload animation
			if ( UsesClipsForAmmo2() )
			{
				// reload clip2 if empty
				if (m_iClip2 < 1)
				{
					pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );
					m_iClip2 = m_iClip2 + 1;
				}
			}
		}
	}
 
	if ( !bFired && (pPlayer->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime) && pPlayer->CanAttack())
	{
		// Clip empty? Or out of ammo on a no-clip weapon?
		if ( !IsMeleeWeapon() && (( UsesClipsForAmmo1() && m_iClip1 <= 0) || ( !UsesClipsForAmmo1() && pPlayer->GetAmmoCount(m_iPrimaryAmmoType)<=0 )) )
		{
			HandleFireOnEmpty();
		}
		else
		{
			PrimaryAttack();
		}
	}
 
	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if ( pPlayer->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload) 
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
 
	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pPlayer->m_nButtons & IN_ATTACK) || (pPlayer->m_nButtons & IN_ATTACK2) || (pPlayer->m_nButtons & IN_RELOAD)))
	{
		// no fire buttons down or reloading
		if ( !ReloadOrSwitchWeapons() && ( m_bInReload == false ) )
		{
			WeaponIdle();
		}
	}
 
	// Tony; decrease shots fired count - tweak the time as necessary.
	if ( !( pPlayer->m_nButtons & IN_ATTACK ) )
	{
		//Tony; check firemodes -- If we're semi or burst, we will clear shots fired now that the player has let go of the button.
		switch(GetFireMode())
		{
		case FM_SEMIAUTOMATIC:
			if (pPlayer->GetShotsFired() > 0)
				pPlayer->ClearShotsFired();
			break;
			//Tony; TODO; add an accessor to determine the max burst on a per-weapon basis!!
			//DONE!
		case FM_BURST:
			if (pPlayer->GetShotsFired() > MaxBurstShots())
				pPlayer->ClearShotsFired();
			break;
		}
 
		m_bFireOnEmpty = false;
		if ( (pPlayer->GetShotsFired() > 0) && (m_flDecreaseShotsFired < gpGlobals->curtime)	)
		{
			m_flDecreaseShotsFired = gpGlobals->curtime + 0.05495;
			pPlayer->DecreaseShotsFired();
		}
	}
}
 
void CWeaponSDKBase::WeaponIdle( void )
{
	//Idle again if we've finished
	if ( HasWeaponIdleTimeElapsed() )
	{
		SendWeaponAnim( GetIdleActivity() );
		SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
	}
}
bool CWeaponSDKBase::Reload( void )
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;
 
	fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), GetReloadActivity() );
	if ( fRet )
	{
		// Unscope while reloading
		if ( HasScope() )
			ExitScope();

		SendReloadEvents();
 
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;
 
		WeaponSound( RELOAD );
		if (GetPlayerOwner()) 
			GetPlayerOwner()->ClearShotsFired();
	}
 
	return fRet;
}

void CWeaponSDKBase::SendReloadEvents()
{
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if ( !pPlayer )
		return;
 
#ifdef GAME_DLL
	// Send a message to any clients that have this entity to play the reload.
	CPASFilter filter( pPlayer->GetAbsOrigin() );
	filter.RemoveRecipient( pPlayer );
 
	UserMessageBegin( filter, "ReloadEffect" );
	WRITE_SHORT( pPlayer->entindex() );
	MessageEnd();
#endif
	// Make the player play his reload animation.
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_RELOAD );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponSDKBase::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( GetDeploySound() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponSDKBase::Deploy( )
{
	MDLCACHE_CRITICAL_SECTION();

	if ( HasScope() )
		ExitScope();

	EmitSound( GetDeploySound() );
 
	//Tony; on deploy clear shots fired.
	if (GetPlayerOwner())
		GetPlayerOwner()->ClearShotsFired();
 
	return DefaultDeploy( (char*)GetViewModel(), (char*)GetWorldModel(), GetDeployActivity(), (char*)GetAnimPrefix() );
}
 
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponSDKBase::Holster( CBaseCombatWeapon *pSwitchingTo )
{ 
	MDLCACHE_CRITICAL_SECTION();
 
	// cancel any reload in progress.
	m_bInReload = false; 
 
	// kill any think functions
	SetThink( NULL );
 
	// Some weapon's don't have holster anims yet, so detect that
	float flSequenceDuration = 0;
	SendWeaponAnim( GetHolsterActivity() );
		flSequenceDuration = SequenceDuration();

	if ( HasScope() )
		ExitScope( false );
 
	CBaseCombatCharacter *pOwner = GetOwner();
	if (pOwner)
		pOwner->SetNextAttack( gpGlobals->curtime + flSequenceDuration );
 
	// If we don't have a holster anim, hide immediately to avoid timing issues
	if ( !flSequenceDuration )
		SetWeaponVisible( false );
	else
		// Hide the weapon when the holster animation's finished
		SetContextThink( &CBaseCombatWeapon::HideThink, gpGlobals->curtime + flSequenceDuration, SDK_HIDEWEAPON_THINK_CONTEXT );
 
	return true;
}

acttable_t *CWeaponSDKBase::ActivityList( int &iActivityCount )
{
	acttable_t *pTable = NULL;

	switch( GetWeaponID() )
	{
	case WEAPON_TRANQ:
		pTable = m_acttableHandGun;
		iActivityCount = ARRAYSIZE( m_acttableHandGun );
		break;
	case WEAPON_SHOTGUN:
	case WEAPON_12GAUGE:
	case WEAPON_GRENADELAUNCHER:
	case WEAPON_PIPEBOMBLAUNCHER:
	//case WEAPON_MP5:
		pTable = m_acttableTwoHandsGuns;
		iActivityCount = ARRAYSIZE( m_acttableTwoHandsGuns );
		break;
	case WEAPON_CROWBAR:
	case WEAPON_UMBRELLA:
	case WEAPON_KNIFE:
	case WEAPON_WRENCH:
	default:
		pTable = m_acttableMelee;
		iActivityCount = ARRAYSIZE( m_acttableMelee );
		break;
	case WEAPON_AC:
		pTable = m_acttableAC;
		iActivityCount = ARRAYSIZE( m_acttableAC );
		break;
	case WEAPON_RAILGUN:
	case WEAPON_SNIPERRIFLE:
	case WEAPON_AUTORIFLE:
		pTable = m_acttableSniper;
		iActivityCount = ARRAYSIZE( m_acttableSniper );
		break;
	case WEAPON_NAILGUN:
	case WEAPON_SUPERNAILGUN:
		pTable = m_acttableNoReload;
		iActivityCount = ARRAYSIZE( m_acttableNoReload );
		break;
	case WEAPON_GRENADE:
	case WEAPON_GRENADE_CONCUSSION:
	case WEAPON_GRENADE_NAPALM:
	case WEAPON_GRENADE_EMP:
	case WEAPON_GRENADE_MIRV:
	case WEAPON_GRENADE_CALTROP:
	case WEAPON_GRENADE_HALLUCINATION:
		pTable = m_acttableGrenade;
		iActivityCount = ARRAYSIZE( m_acttableGrenade );
		break;
	case WEAPON_RPG:
	case WEAPON_IC:
		pTable = m_acttableRPG;
		iActivityCount = ARRAYSIZE( m_acttableRPG );
	case WEAPON_MEDKIT:
		pTable = m_acttableTool;
		iActivityCount = ARRAYSIZE( m_acttableTool );
		break;
	}

	return pTable;
}
 
void CWeaponSDKBase::WeaponSound( WeaponSound_t sound_type, float soundtime /* = 0.0f */ )
{
#ifdef CLIENT_DLL
	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetWpnData().aShootSounds[ sound_type ]; 
	if ( !shootsound || !shootsound[0] )
		return;

	CBroadcastRecipientFilter filter; // this is client side only
	if ( !te->CanPredict() )
		return;
				
	CBaseEntity::EmitSound( filter, GetPlayerOwner()->entindex(), shootsound, &GetPlayerOwner()->GetAbsOrigin() ); 
#else
	BaseClass::WeaponSound( sound_type, soundtime );
#endif
}

void CWeaponSDKBase::FallInit( void )
{
#ifndef CLIENT_DLL
	SetModel( GetWorldModel() );
	VPhysicsDestroyObject();

	if ( HasSpawnFlags(SF_NORESPAWN) == false )
	{
		SetMoveType( MOVETYPE_NONE );
		SetSolid( SOLID_BBOX );
		AddSolidFlags( FSOLID_TRIGGER );

		UTIL_DropToFloor( this, MASK_SOLID );
	}
	else
	{
		if ( !VPhysicsInitNormal(SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false) )
		{
			SetMoveType( MOVETYPE_NONE );
			SetSolid( SOLID_BBOX );
			AddSolidFlags( FSOLID_TRIGGER );
		}
		else
		{
			// Constrained start?
			if ( HasSpawnFlags(SF_WEAPON_START_CONSTRAINED) )
			{
				//Constrain the weapon in place
				IPhysicsObject *pReferenceObject, *pAttachedObject;
				
				pReferenceObject = g_PhysWorldObject;
				pAttachedObject = VPhysicsGetObject();

				if ( pReferenceObject && pAttachedObject )
				{
					constraint_fixedparams_t fixed;
					fixed.Defaults();
					fixed.InitWithCurrentObjectState( pReferenceObject, pAttachedObject );
					
					fixed.constraint.forceLimit	= lbs2kg( 10000 );
					fixed.constraint.torqueLimit = lbs2kg( 10000 );

					IPhysicsConstraint *pConstraint = GetConstraint();
					pConstraint = physenv->CreateFixedConstraint( pReferenceObject, pAttachedObject, NULL, fixed );
					pConstraint->SetGameData( (void *) this );
				}
			}
		}
	}

	SetPickupTouch();
	
	SetThink( &CWeaponSDKBase::FallThink );

	SetNextThink( gpGlobals->curtime + 0.1f );
#endif	// !CLIENT_DLL
}

#ifdef CLIENT_DLL
ConVar cl_bobcycle( "cl_bobcycle", "0.45", 0 , "How fast the bob cycles", true, 0.01f, false, 0.0f );
ConVar cl_bobup( "cl_bobup", "0.5", 0 , "Don't change...", true, 0.01f, true, 0.99f );
ConVar cl_bobvertscale( "cl_bobvertscale", "0.6", 0, "Vertical scale" ); // Def. is 0.1
ConVar cl_boblatscale( "cl_boblatscale", "0.8", 0, "Lateral scale" );
ConVar cl_bobenable( "cl_bobenable", "1" );

float g_lateralBob;
float g_verticalBob;

float CWeaponSDKBase::CalcViewmodelBob()
{
	static float	bobtime;
	static float	lastbobtime;
	float	cycle;

	float	bobup = cl_bobup.GetFloat();
	float	bobcycle = cl_bobcycle.GetFloat();

	C_BasePlayer* player = GetPlayerOwner();

	//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it
	if (!player || !gpGlobals->frametime || bobcycle <= 0.0f || bobup <= 0.0f || bobup >= 1.0f )
		return 0.0f;

	float speed = player->GetLocalVelocity().Length2D();

	speed = clamp( speed, -320, 320 );

	float bob_offset = RemapVal( speed, 0, 320, 0.0f, 1.0f );
	
	bobtime += ( gpGlobals->curtime - lastbobtime ) * bob_offset;
	lastbobtime = gpGlobals->curtime;

	//Calculate the vertical bob
	cycle = bobtime - (int)(bobtime/bobcycle)*bobcycle;
	cycle /= bobcycle;

	if ( cycle < bobup )
		cycle = M_PI * cycle / bobup;
	else
		cycle = M_PI + M_PI*(cycle-bobup)/(1.0 - bobup);
   
	g_verticalBob = speed*0.005f;
	g_verticalBob = g_verticalBob*0.3 + g_verticalBob*0.7*sin(cycle);

	g_verticalBob = clamp( g_verticalBob, -7.0f, 4.0f );

	//Calculate the lateral bob
	cycle = bobtime - (int)(bobtime/bobcycle*2)*bobcycle*2;
	cycle /= bobcycle*2;

	if ( cycle < bobup )
		cycle = M_PI * cycle / bobup;
	else
		cycle = M_PI + M_PI*(cycle-bobup)/(1.0 - bobup);

	g_lateralBob = speed*0.005f;
	g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.7*sin(cycle);
	g_lateralBob = clamp( g_lateralBob, -7.0f, 4.0f );
	
	//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
	return 0.0f;
}

void CWeaponSDKBase::AddViewmodelBob( CBaseViewModel *viewmodel, Vector& origin, QAngle& angles )
{
	if ( !cl_bobenable.GetBool() )
		return;

	Vector	forward, right;
	AngleVectors( angles, &forward, &right, NULL );

	CalcViewmodelBob();

	// Apply bob, but scaled down to 40%
	VectorMA( origin, g_verticalBob * cl_bobvertscale.GetFloat(), forward, origin );
	
	// Z bob a bit more
	origin[2] += g_verticalBob * 0.1f;
	
	// bob the angles
	angles[ ROLL ]	+= g_verticalBob * 0.5f;
	angles[ PITCH ]	-= g_verticalBob * 0.4f;

	angles[ YAW ]	-= g_lateralBob  * 0.3f;

	VectorMA( origin, g_lateralBob * cl_boblatscale.GetFloat(), right, origin );
}
#else
void CWeaponSDKBase::FallThink(void)
{
	// Prevent the common HL2DM weapon respawn bug from happening
	// When a weapon is spawned, the following chain of events occurs:
	// - Spawn() is called (duh), which then calls FallInit()
	// - FallInit() is called, and prepares the weapon's 'Think' function (CBaseCombatWeapon::FallThink())
	// - FallThink() is called, and performs several checks before deciding whether the weapon should Materialize()
	// - Materialize() is called (the HL2DM version above), which sets the weapon's respawn location.
	// The problem occurs when a weapon isn't placed properly by a level designer.
	// If the weapon is unable to move from its location (e.g. if its bounding box is halfway inside a wall), Materialize() never gets called.
	// Since Materialize() never gets called, the weapon's respawn location is never set, so if a person picks it up, it respawns forever at
	// 0 0 0 on the map (infinite loop of fall, wait, respawn, not nice at all for performance and bandwidth!)
	
	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
	{
		if ( GetOriginalSpawnOrigin() == vec3_origin )
		{
			m_vOriginalSpawnOrigin = GetAbsOrigin();
			m_vOriginalSpawnAngles = GetAbsAngles();
		}
	}

	return BaseClass::FallThink();
}

void CWeaponSDKBase::Materialize( void )
{
	if ( IsEffectActive( EF_NODRAW ) )
	{
		// changing from invisible state to visible.
		EmitSound( "BaseCombatWeapon.WeaponMaterialize" );
		
		RemoveEffects( EF_NODRAW );
		DoMuzzleFlash();
	}

	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
	{
		VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false );
		SetMoveType( MOVETYPE_VPHYSICS );

		SDKGameRules()->AddLevelDesignerPlacedObject( this );
	}

	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
	{
		if ( GetOriginalSpawnOrigin() == vec3_origin )
		{
			m_vOriginalSpawnOrigin = GetAbsOrigin();
			m_vOriginalSpawnAngles = GetAbsAngles();
		}
	}

	SetPickupTouch();

	SetThink (NULL);
}

int CWeaponSDKBase::ObjectCaps()
{
	return BaseClass::ObjectCaps() & ~FCAP_IMPULSE_USE;
}

void CWeaponSDKBase::DoMuzzleFlash( void )
{
	if ( !ShouldDrawMuzzleFlash() )
		return;	// this weapon shouldn't have a muzzleflash drawn, so don't

	BaseClass::DoMuzzleFlash();
}
#endif