//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_grenade_caltrop.h"

#ifndef CLIENT_DLL
	#include "sdk_player.h"
	#include "items.h"
	#include "sdk_basegrenade_projectile.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenadeCaltrop, DT_WeaponGrenadeCaltrop )

BEGIN_NETWORK_TABLE(CWeaponGrenadeCaltrop, DT_WeaponGrenadeCaltrop)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGrenadeCaltrop )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_grenade_caltrop, CWeaponGrenadeCaltrop );
PRECACHE_WEAPON_REGISTER( weapon_grenade_caltrop );


#ifdef GAME_DLL
class CCaltropProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CCaltropProjectile, CBaseGrenadeProjectile );

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_GRENADE_CALTROP; }

	// Overrides.
public:
	virtual void Spawn()
	{
		BaseClass::Spawn();

		SetTouch( &CCaltropProjectile::Touch );

		// We want to get touch functions called so we can damage enemy players
		AddSolidFlags( FSOLID_TRIGGER );
	}

	void Detonate()
	{
		if ( !IsInWorld() )
		{
			Remove( );
			return;
		}

		// have the caltrop disappear
		UTIL_Remove( this );
	}

	void Touch( CBaseEntity *pOther )
	{
		if ( !pOther->IsPlayer() || !( pOther->GetFlags() & FL_ONGROUND ) || !pOther->IsAlive() )
			return;

		// Don't hurt friendlies
		if ( GetTeamNumber() == pOther->GetTeamNumber() )
			return;

		// Caltrops need to be on the ground. Check to see if we're still moving.
		Vector vecVelocity;
		VPhysicsGetObject()->GetVelocity( &vecVelocity, NULL );
		if ( vecVelocity.LengthSqr() > (1*1) )
			return;

		// Do the leg damage to the player
		CTakeDamageInfo info( this, GetThrower(), 10, DMG_PARALYZE | DMG_PREVENT_PHYSICS_FORCE | DMG_NEVERGIB );
		pOther->TakeDamage( info );

		// have the caltrop disappear
		UTIL_Remove( this );
	}

	// Grenade stuff.
public:

	static CCaltropProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		CWeaponSDKBase *pWeapon,
		float timer )
	{
		CCaltropProjectile *pMirv = (CCaltropProjectile*)CBaseEntity::Create( "grenade_caltrop_projectile", position, angles, pOwner );

		// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
		// one second before detonation.
		pMirv->SetVelocity( velocity, angVelocity );

		pMirv->SetDetonateTimerLength( timer );
		pMirv->SetAbsVelocity( velocity );
		pMirv->SetupInitialTransmittedGrenadeVelocity( velocity );
		pMirv->SetThrower( pOwner ); 

		pMirv->SetGravity( BaseClass::GetGrenadeGravity() );
		pMirv->SetFriction( BaseClass::GetGrenadeFriction() );
		pMirv->SetElasticity( BaseClass::GetGrenadeElasticity() );

		pMirv->m_flDamage = pWeapon->GetSDKWpnData().m_iDamage;
		pMirv->m_DmgRadius = pMirv->m_flDamage * 3.5f;
		pMirv->ChangeTeam( pOwner->GetTeamNumber() );
		pMirv->ApplyLocalAngularVelocityImpulse( angVelocity );	

		// make NPCs afaid of it while in the air
		pMirv->SetThink( &CCaltropProjectile::DangerSoundThink );
		pMirv->SetNextThink( gpGlobals->curtime );

		return pMirv;
	}
};

LINK_ENTITY_TO_CLASS( grenade_caltrop_projectile, CCaltropProjectile );
PRECACHE_WEAPON_REGISTER( grenade_caltrop_projectile );

BEGIN_DATADESC( CWeaponGrenadeCaltrop )
END_DATADESC()

void CWeaponGrenadeCaltrop::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon )
{
	CCaltropProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, pWeapon, DEFAULT_GRENADE_TIMER );
}
	
#endif