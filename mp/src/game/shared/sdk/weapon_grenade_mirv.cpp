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
#include "weapon_grenade_mirv.h"

#ifndef CLIENT_DLL
	#include "sdk_player.h"
	#include "items.h"
	#include "sdk_basegrenade_projectile.h"

	#define	GRENADE_MIRV_LEADIN	2.0f 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenadeMirv, DT_WeaponGrenadeMirv )

BEGIN_NETWORK_TABLE(CWeaponGrenadeMirv, DT_WeaponGrenadeMirv)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGrenadeMirv )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_grenade_mirv, CWeaponGrenadeMirv );
PRECACHE_WEAPON_REGISTER( weapon_grenade_mirv );


#ifdef GAME_DLL
class CMirvProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CMirvProjectile, CBaseGrenadeProjectile );
	DECLARE_DATADESC();

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const		{	return WEAPON_GRENADE_MIRV; }

	// Overrides.
public:
	virtual void Spawn()
	{
		BaseClass::Spawn();

		m_bPlayedLeadIn = false;

		SetThink( &CMirvProjectile::DetonateThink );
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

		if ( !m_bPlayedLeadIn && gpGlobals->curtime > GetDetonateTime() - GRENADE_MIRV_LEADIN )
		{
			//Vector soundPosition = GetAbsOrigin() + Vector( 0, 0, 5 );
			//CPASAttenuationFilter filter( soundPosition );

			//EmitSound( filter, entindex(), "Weapon_Grenade_Mirv.LeadIn" );
			m_bPlayedLeadIn = true;
		}

		SetNextThink( gpGlobals->curtime + 0.2 );
	}

	virtual void Explode( trace_t *pTrace, int bitsDamageType )
	{
		// Pass through.
		BaseClass::Explode( pTrace, bitsDamageType );

		m_bPlayedLeadIn = false;

		// Create the bomblets.
		for ( int iBomb = 0; iBomb < 2; ++iBomb )
		{
			Vector vecSrc = pTrace->endpos + Vector( 0, 0, 1.0f ); 
			Vector vecVelocity( random->RandomFloat( -75.0f, 75.0f ) * 3.0f,
								random->RandomFloat( -75.0f, 75.0f ) * 3.0f,
								random->RandomFloat( 30.0f, 70.0f ) * 5.0f );
			Vector vecZero( 0,0,0 );
			CSDKPlayer *pPlayer = ToSDKPlayer( GetThrower() );
			float flTime = 2.0f + random->RandomFloat( 0.0f, 1.0f );

			CMirvProjectile::Create( vecSrc, GetAbsAngles(), vecVelocity, vecZero, pPlayer, flTime );
		}
	}

	// Grenade stuff.
public:

	static CMirvProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		float timer )
	{
		CMirvProjectile *pMirv = (CMirvProjectile*)CBaseEntity::Create( "grenade_mirv_projectile", position, angles, pOwner );

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

		pMirv->m_flDamage = 180.0f;
		pMirv->m_DmgRadius = 198.0f;
		pMirv->ChangeTeam( pOwner->GetTeamNumber() );
		pMirv->ApplyLocalAngularVelocityImpulse( angVelocity );	

		IPhysicsObject *pPhysicsObject = pMirv->VPhysicsGetObject();
		if ( pPhysicsObject )
			pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	
		// make NPCs afaid of it while in the air
		pMirv->SetThink( &CMirvProjectile::DangerSoundThink );
		pMirv->SetNextThink( gpGlobals->curtime );
		pMirv->Remove();

		return pMirv;
	}

	static CMirvProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		CWeaponSDKBase *pWeapon,
		float timer )
	{
		CMirvProjectile *pMirv = (CMirvProjectile*)CBaseEntity::Create( "grenade_mirv_projectile", position, angles, pOwner );

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
		pMirv->SetThink( &CMirvProjectile::DangerSoundThink );
		pMirv->SetNextThink( gpGlobals->curtime );

		return pMirv;
	}

private:

	bool m_bPlayedLeadIn;
};

LINK_ENTITY_TO_CLASS( grenade_mirv_projectile, CMirvProjectile );
PRECACHE_WEAPON_REGISTER( grenade_mirv_projectile );

BEGIN_DATADESC( CMirvProjectile )
	DEFINE_THINKFUNC( DetonateThink ),
END_DATADESC()

BEGIN_DATADESC( CWeaponGrenadeMirv )
END_DATADESC()

void CWeaponGrenadeMirv::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon )
{
	CMirvProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, pWeapon, DEFAULT_GRENADE_TIMER );
}
	
#endif