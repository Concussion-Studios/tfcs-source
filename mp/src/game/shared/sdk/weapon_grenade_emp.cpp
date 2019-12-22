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
#include "weapon_grenade_emp.h"

#ifndef CLIENT_DLL
	#include "sdk_player.h"
	#include "items.h"
	#include "sdk_basegrenade_projectile.h"
	#include "soundent.h"
	#include "particle_parse.h"
	#include "beam_shared.h"
	
	#define	GRENADE_EMP_LEADIN	2.0f 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenadeEmp, DT_WeaponGrenadeEmp )

BEGIN_NETWORK_TABLE(CWeaponGrenadeEmp, DT_WeaponGrenadeEmp)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGrenadeEmp )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_grenade_emp, CWeaponGrenadeEmp );
PRECACHE_WEAPON_REGISTER( weapon_grenade_emp );

#ifdef GAME_DLL
class CEMPProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CEMPProjectile, CBaseGrenadeProjectile );
	DECLARE_DATADESC();

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const		{ return WEAPON_GRENADE_EMP; }

	// Overrides.
public:
	virtual void Spawn()
	{
		BaseClass::Spawn();
		
		m_bPlayedLeadIn = false;

		SetThink( &CEMPProjectile::DetonateThink );
	}

	virtual void Precache()
	{
		PrecacheScriptSound( "Emp.LeadIn" );
		PrecacheModel( "sprites/physcannon_bluelight1b.vmt" );
		PrecacheParticleSystem( "aurora_shockwave" );
		
		BaseClass::Precache();
	}

	virtual void Detonate()
	{
		if ( !IsInWorld() )
		{
			Remove();
			return;
		}

		// Explosion effect on client
		// SendDispatchEffect();

		float flRadius = 180;
		float flDamage = 1;

		// Apply some amount of EMP damage to every entity in the radius. They will calculate 
		// their own damage based on how much ammo they have or some other wacky calculation.
		CTakeDamageInfo info( this, GetThrower(), vec3_origin, GetAbsOrigin(), flDamage, /* DMG_EMP |*/ DMG_PREVENT_PHYSICS_FORCE );

		CBaseEntity *pEntityList[100];
		int nEntityCount = UTIL_EntitiesInSphere( pEntityList, 100, GetAbsOrigin(), flRadius, 0 );
		int iEntity;
		for ( iEntity = 0; iEntity < nEntityCount; ++iEntity )
		{
			CBaseEntity *pEntity = pEntityList[iEntity];

			if ( pEntity == this )
				continue;

			if ( pEntity && pEntity->IsPlayer() )
				continue;

			if ( pEntity && ( pEntity->m_takedamage == DAMAGE_YES || pEntity->m_takedamage == DAMAGE_EVENTS_ONLY ) && !InSameTeam( GetThrower() ) )
			{
				pEntity->TakeDamage( info );

				if ( pEntity->IsPlayer() )
				{
					CSDKPlayer *pPlayer = ToSDKPlayer( pEntity );
					if ( pPlayer )
					{
						CBeam *pBeam = CBeam::BeamCreate( "sprites/physcannon_bluelight1b.vmt", 3.0 );
						if ( !pBeam )
							return;

						pBeam->PointsInit( GetAbsOrigin(), pEntity->WorldSpaceCenter() );
						pBeam->SetColor( 255, 255, 255 );
						pBeam->SetBrightness( 128 );
						pBeam->SetNoise( 12.0f );
						pBeam->SetEndWidth( 3.0f );
						pBeam->SetWidth( 3.0f );
						pBeam->LiveForTime( 0.5f );	// Fail-safe
						pBeam->SetFrameRate( 25.0f );
						pBeam->SetFrame( random->RandomInt( 0, 2 ) );

						if ( pPlayer->GetActiveSDKWeapon() )
							pPlayer->RemoveAmmo( pPlayer->GetActiveSDKWeapon()->Clip1(), pPlayer->GetActiveSDKWeapon()->GetPrimaryAmmoType() );
					}
				}
				else if ( pEntity->ClassMatches( "item_ammo*" ) )
				{
					CBeam *pBeam = CBeam::BeamCreate( "sprites/physcannon_bluelight1b.vmt", 3.0 );
					if ( !pBeam )
						return;

					pBeam->PointsInit( GetAbsOrigin(), pEntity->WorldSpaceCenter() );
					pBeam->SetColor( 255, 255, 255 );
					pBeam->SetBrightness( 128 );
					pBeam->SetNoise( 12.0f );
					pBeam->SetEndWidth( 3.0f );
					pBeam->SetWidth( 3.0f );
					pBeam->LiveForTime( 0.5f );	// Fail-safe
					pBeam->SetFrameRate( 25.0f );
					pBeam->SetFrame( random->RandomInt( 0, 2 ) );

					// reduces to atom
					pEntity->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
				}
			}
		}

		DispatchParticleEffect( "aurora_shockwave", GetAbsOrigin(), vec3_angle );

		UTIL_Remove( this );
	}

	virtual void DetonateThink( void )
	{
		if ( !IsInWorld() )
		{
			Remove( );
			return;
		}

		if ( gpGlobals->curtime > GetDetonateTime() )
		{
			Detonate();
			return;
		}

		if ( !m_bPlayedLeadIn && gpGlobals->curtime > GetDetonateTime() - GRENADE_EMP_LEADIN )
		{
			//Vector soundPosition = GetAbsOrigin() + Vector( 0, 0, 5 );
			//CPASAttenuationFilter filter( soundPosition );

			//EmitSound( filter, entindex(), "Emp.LeadIn" );
			m_bPlayedLeadIn = true;
		}

		SetNextThink( gpGlobals->curtime + 0.2 );
	}

	// Grenade stuff.
public:

	static CEMPProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		CWeaponSDKBase *pWeapon,
		float timer )
	{
		CEMPProjectile *pEmp = (CEMPProjectile*)CBaseEntity::Create( "grenade_emp_projectile", position, angles, pOwner );

		// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
		// one second before detonation.
		pEmp->SetVelocity( velocity, angVelocity );

		pEmp->SetDetonateTimerLength( timer );
		pEmp->SetAbsVelocity( velocity );
		pEmp->SetupInitialTransmittedGrenadeVelocity( velocity );
		pEmp->SetThrower( pOwner ); 

		pEmp->SetGravity( BaseClass::GetGrenadeGravity() );
		pEmp->SetFriction( BaseClass::GetGrenadeFriction() );
		pEmp->SetElasticity( BaseClass::GetGrenadeElasticity() );

		pEmp->m_flDamage = pWeapon->GetSDKWpnData().m_iDamage;
		pEmp->m_DmgRadius = pEmp->m_flDamage * 3.5f;
		pEmp->ChangeTeam( pOwner->GetTeamNumber() );
		pEmp->ApplyLocalAngularVelocityImpulse( angVelocity );	

		// make NPCs afaid of it while in the air
		pEmp->SetThink( &CEMPProjectile::DangerSoundThink );
		pEmp->SetNextThink( gpGlobals->curtime );

		return pEmp;
	}
	
private:

	bool m_bPlayedLeadIn;
};

LINK_ENTITY_TO_CLASS( grenade_emp_projectile, CEMPProjectile );
PRECACHE_WEAPON_REGISTER( grenade_emp_projectile );

BEGIN_DATADESC( CEMPProjectile )
	DEFINE_THINKFUNC( DetonateThink ),
END_DATADESC()

BEGIN_DATADESC( CWeaponGrenadeEmp )
END_DATADESC()

void CWeaponGrenadeEmp::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon )
{
	CEMPProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, pWeapon, DEFAULT_GRENADE_TIMER );
}
#endif