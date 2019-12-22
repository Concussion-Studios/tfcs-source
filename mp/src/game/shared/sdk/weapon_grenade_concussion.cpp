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
#include "weapon_grenade_concussion.h"

#ifndef CLIENT_DLL
	#include "sdk_player.h"
	#include "items.h"
	#include "sdk_basegrenade_projectile.h"
	#include "particle_parse.h"
	#include "soundent.h"
	#include "beam_flags.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenadeConcussion, DT_WeaponGrenadeConcussion )

BEGIN_NETWORK_TABLE(CWeaponGrenadeConcussion, DT_WeaponGrenadeConcussion)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGrenadeConcussion )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_grenade_concussion, CWeaponGrenadeConcussion );
PRECACHE_WEAPON_REGISTER( weapon_grenade_concussion );

#ifdef GAME_DLL
// For our ring explosion
int s_nConcussionTexture = -1;

class CConcussionProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CConcussionProjectile, CBaseGrenadeProjectile );

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const		{ return WEAPON_GRENADE_CONCUSSION; }

	// Overrides.
public:
	virtual void Spawn()
	{
		BaseClass::Spawn();
	}

	virtual void Precache()
	{
		PrecacheScriptSound( "Concussion.Explode" );
		
		//PrecacheParticleSystem( "aurora_shockwave" );
		s_nConcussionTexture = PrecacheModel( "sprites/lgtning.vmt" );
		
		BaseClass::Precache();
	}

	virtual void Detonate()
	{
		if ( !IsInWorld() )
		{
			Remove();
			return;
		}

		// The trace start/end.
		Vector vecStart = GetAbsOrigin() + Vector( 0.0f, 0.0f, 8.0f );
		Vector vecEnd = vecStart + Vector( 0.0f, 0.0f, -32.0f );

		trace_t	trace;
		UTIL_TraceLine ( vecStart, vecEnd, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &trace );

		// Explode (concuss).
		Explode( &trace, DMG_SONIC );

		// Screen shake.
		if ( GetShakeAmplitude() )
			UTIL_ScreenShake( GetAbsOrigin(), GetShakeAmplitude(), 150.0, 1.0, GetShakeRadius(), SHAKE_START );
	}

	virtual void Explode( trace_t *pTrace, int bitsDamageType )
	{
		// Invisible.
		SetModelName( NULL_STRING );	
		AddSolidFlags( FSOLID_NOT_SOLID );
		m_takedamage = DAMAGE_NO;

		// Pull out of the wall a bit
		if ( pTrace->fraction != 1.0 )
			SetAbsOrigin( pTrace->endpos + ( pTrace->plane.normal * 1.0f ) );

		// Use the thrower's position as the reported position
		Vector vecReported = GetThrower() ? GetThrower()->GetAbsOrigin() : vec3_origin;
		float flRadius = GetDamageRadius();

		CSDKPlayer *pPlayer = ToSDKPlayer( GetThrower() );;

		static int r;
		static int g;
		static int b;

		switch ( pPlayer->GetTeamNumber() )
		{
			case SDK_TEAM_RED:
				r = 255;
				g = 64;
				b = 64;
				break;
			case SDK_TEAM_BLUE:
				r = 153;
				g = 204;
				b = 255;
				break;
			case SDK_TEAM_GREEN:
				r = 153;
				g = 255;
				b = 153;

				break;
			case SDK_TEAM_YELLOW:
				r = 255;
				g = 178;
				b = 0;
				break;
			default:
				break;
		}

		//Ring Effect
		//DispatchParticleEffect( "aurora_shockwave", PATTACH_ABSORIGIN_FOLLOW, this );
		CBroadcastRecipientFilter filter;
		te->BeamRingPoint( filter, 0, GetAbsOrigin(),	//origin
			0,	//start radius
			flRadius * 1.2,		//end radius
			s_nConcussionTexture, //texture
			0,			//halo index
			0,			//start frame
			1,			//framerate
			0.3f,		//life
			30,			//width
			0,			//spread
			0,			//amplitude
			r,			//r
			g,			//g
			b,			//b
			64,			//a
			0,			//speed
			FBEAM_FADEOUT
			);

		// Explosion sound.
		CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0 );

		// Explosion damage.
		CTakeDamageInfo info( this, pPlayer, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported );
		RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

		// Concussion.
		CBaseEntity *pEntityList[64];
		int nEntityCount = UTIL_EntitiesInSphere( pEntityList, 64, GetAbsOrigin(), flRadius, FL_CLIENT );
		for ( int iEntity = 0; iEntity < nEntityCount; ++iEntity )
		{
			CBaseEntity *pEntity = pEntityList[iEntity];
			CSDKPlayer *pTestPlayer = ToSDKPlayer( pEntity );

			// You can concuss yourself.
			if ( !pEntity->InSameTeam( pPlayer ) )
			{
				if ( pTestPlayer || ( pPlayer == pTestPlayer ) )
					pTestPlayer->ViewPunch( QAngle( -5, 0, 0 ) * 0.0004f * flRadius );
			}
		}

		EmitSound( "Concussion.Explode" );

		// Reset.
		SetThink( &CBaseGrenade::SUB_Remove );
		SetTouch( NULL );
		AddEffects( EF_NODRAW );
		SetAbsVelocity( vec3_origin );
		SetNextThink( gpGlobals->curtime );
		StopParticleEffects( this );
	}

	// Grenade stuff.
public:

	static CConcussionProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		CWeaponSDKBase *pWeapon,
		float timer )
	{
		CConcussionProjectile *pConc = (CConcussionProjectile*)CBaseEntity::Create( "grenade_concussion_projectile", position, angles, pOwner );

		// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
		// one second before detonation.
		pConc->SetVelocity( velocity, angVelocity );

		pConc->SetDetonateTimerLength( timer );
		pConc->SetAbsVelocity( velocity );
		pConc->SetupInitialTransmittedGrenadeVelocity( velocity );
		pConc->SetThrower( pOwner ); 

		pConc->SetGravity( BaseClass::GetGrenadeGravity() );
		pConc->SetFriction( BaseClass::GetGrenadeFriction() );
		pConc->SetElasticity( BaseClass::GetGrenadeElasticity() );

		pConc->m_flDamage = pWeapon->GetSDKWpnData().m_iDamage;
		pConc->m_DmgRadius = pConc->m_flDamage * 3.5f;
		pConc->ChangeTeam( pOwner->GetTeamNumber() );
		pConc->ApplyLocalAngularVelocityImpulse( angVelocity );	

		// make NPCs afaid of it while in the air
		pConc->SetThink( &CConcussionProjectile::DangerSoundThink );
		pConc->SetNextThink( gpGlobals->curtime );

		return pConc;
	}
};

LINK_ENTITY_TO_CLASS( grenade_concussion_projectile, CConcussionProjectile );
PRECACHE_WEAPON_REGISTER( grenade_concussion_projectile );

BEGIN_DATADESC( CWeaponGrenadeConcussion )
END_DATADESC()

void CWeaponGrenadeConcussion::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon )
{
	CConcussionProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, pWeapon, DEFAULT_GRENADE_TIMER );
}
#endif