//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "sdk_gamerules.h"
#include "weapon_sdkbase.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_grenade_napalm.h"

#ifndef CLIENT_DLL
	#include "sdk_player.h"
	#include "items.h"
	#include "sdk_basegrenade_projectile.h"
	#include "soundent.h"
	#include "fire.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenadeNapalm, DT_WeaponGrenadeNapalm )

BEGIN_NETWORK_TABLE(CWeaponGrenadeNapalm, DT_WeaponGrenadeNapalm)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGrenadeNapalm )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_grenade_napalm, CWeaponGrenadeNapalm );
PRECACHE_WEAPON_REGISTER( weapon_grenade_napalm );

#ifdef GAME_DLL
class CNapalmProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CNapalmProjectile, CBaseGrenadeProjectile );

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const		{ return WEAPON_GRENADE_NAPALM; }

	// Overrides.
public:
	virtual void Spawn()
	{
		BaseClass::Spawn();
	}

	virtual void Precache()
	{
		UTIL_PrecacheOther( "env_fire" );
		
		BaseClass::Precache();
	}

	virtual void Detonate()
	{
		if ( !IsInWorld() )
		{
			Remove();
			return;
		}

		trace_t trace;
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector ( 0, 0, -128 ),  MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trace );

		// Pull out of the wall a bit
		if ( trace.fraction != 1.0 )
			SetLocalOrigin( trace.endpos + (trace.plane.normal * (m_flDamage - 24) * 0.6) );

		int contents = UTIL_PointContents ( GetAbsOrigin() );
		if ( (contents & MASK_WATER) )
		{
			UTIL_Remove( this );
			return;
		}

		// Start some fires
		int i;
		QAngle vecTraceAngles;
		Vector vecTraceDir;
		trace_t firetrace;

		CBaseEntity *pAttacker = GetThrower();
		if ( !pAttacker )
			pAttacker = GetContainingEntity( INDEXENT(0) );

		for( i = 0 ; i < 16 ; i++ )
		{
			// build a little ray
			vecTraceAngles[PITCH]	= random->RandomFloat(45, 135);
			vecTraceAngles[YAW]		= random->RandomFloat(0, 360);
			vecTraceAngles[ROLL]	= 0.0f;

			AngleVectors( vecTraceAngles, &vecTraceDir );

			Vector vecStart, vecEnd;

			vecStart = GetAbsOrigin() + ( trace.plane.normal * 128 );
			vecEnd = vecStart + vecTraceDir * 512;

			UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &firetrace );

			Vector	ofsDir = ( firetrace.endpos - GetAbsOrigin() );
			float	offset = VectorNormalize( ofsDir );

			if ( offset > 128 )
				offset = 128;

			//Get our scale based on distance
			float scale	 = 0.1f + ( 0.75f * ( 1.0f - ( offset / 128.0f ) ) );
			float growth = 0.1f + ( 0.75f * ( offset / 128.0f ) );

			if( firetrace.fraction != 1.0 )
				FireSystem_StartFire( firetrace.endpos, scale, growth, 30.0f, (SF_FIRE_START_ON), pAttacker, FIRE_NATURAL );
		}

		BaseClass::Detonate();
	}

	// Grenade stuff.
public:

	static CNapalmProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		CWeaponSDKBase *pWeapon,
		float timer )
	{
		CNapalmProjectile *pNapalm = (CNapalmProjectile*)CBaseEntity::Create( "grenade_napalm_projectile", position, angles, pOwner );

		// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
		// one second before detonation.
		pNapalm->SetVelocity( velocity, angVelocity );

		pNapalm->SetDetonateTimerLength( timer );
		pNapalm->SetAbsVelocity( velocity );
		pNapalm->SetupInitialTransmittedGrenadeVelocity( velocity );
		pNapalm->SetThrower( pOwner ); 

		pNapalm->SetGravity( BaseClass::GetGrenadeGravity() );
		pNapalm->SetFriction( BaseClass::GetGrenadeFriction() );
		pNapalm->SetElasticity( BaseClass::GetGrenadeElasticity() );

		pNapalm->m_flDamage = pWeapon->GetSDKWpnData().m_iDamage;
		pNapalm->m_DmgRadius = pNapalm->m_flDamage * 3.5f;
		pNapalm->ChangeTeam( pOwner->GetTeamNumber() );
		pNapalm->ApplyLocalAngularVelocityImpulse( angVelocity );	

		// make NPCs afaid of it while in the air
		pNapalm->SetThink( &CNapalmProjectile::DangerSoundThink );
		pNapalm->SetNextThink( gpGlobals->curtime );

		return pNapalm;
	}
};

LINK_ENTITY_TO_CLASS( grenade_napalm_projectile, CNapalmProjectile );
PRECACHE_WEAPON_REGISTER( grenade_napalm_projectile );

BEGIN_DATADESC( CWeaponGrenadeNapalm )
END_DATADESC()

void CWeaponGrenadeNapalm::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon )
{
	CNapalmProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, pWeapon, DEFAULT_GRENADE_TIMER );
}
#endif