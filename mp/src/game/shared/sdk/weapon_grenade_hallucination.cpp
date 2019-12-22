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
#include "weapon_grenade_hallucination.h"

#ifndef CLIENT_DLL
	#include "sdk_player.h"
	#include "items.h"
	#include "sdk_basegrenade_projectile.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenadeHallucination, DT_WeaponGrenadeHallucination )

BEGIN_NETWORK_TABLE(CWeaponGrenadeHallucination, DT_WeaponGrenadeHallucination)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGrenadeHallucination )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_grenade_hallucination, CWeaponGrenadeHallucination );
PRECACHE_WEAPON_REGISTER( weapon_grenade_hallucination );


#ifdef GAME_DLL
class CHallucinationProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CHallucinationProjectile, CBaseGrenadeProjectile );

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_GRENADE_HALLUCINATION; }

	// Overrides.
public:
	virtual void Spawn()
	{
		BaseClass::Spawn();
	}

	// Grenade stuff.
public:

	static CHallucinationProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		CWeaponSDKBase *pWeapon,
		float timer )
	{
		CHallucinationProjectile *pHall = (CHallucinationProjectile*)CBaseEntity::Create( "grenade_hallucination_projectile", position, angles, pOwner );

		// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
		// one second before detonation.
		pHall->SetVelocity( velocity, angVelocity );

		pHall->SetDetonateTimerLength( timer );
		pHall->SetAbsVelocity( velocity );
		pHall->SetupInitialTransmittedGrenadeVelocity( velocity );
		pHall->SetThrower( pOwner ); 

		pHall->SetGravity( BaseClass::GetGrenadeGravity() );
		pHall->SetFriction( BaseClass::GetGrenadeFriction() );
		pHall->SetElasticity( BaseClass::GetGrenadeElasticity() );

		pHall->m_flDamage = pWeapon->GetSDKWpnData().m_iDamage;
		pHall->m_DmgRadius = pHall->m_flDamage * 3.5f;
		pHall->ChangeTeam( pOwner->GetTeamNumber() );
		pHall->ApplyLocalAngularVelocityImpulse( angVelocity );	

		// make NPCs afaid of it while in the air
		pHall->SetThink( &CHallucinationProjectile::DangerSoundThink );
		pHall->SetNextThink( gpGlobals->curtime );

		return pHall;
	}
};

LINK_ENTITY_TO_CLASS( grenade_hallucination_projectile, CHallucinationProjectile );
PRECACHE_WEAPON_REGISTER( grenade_hallucination_projectile );

BEGIN_DATADESC( CWeaponGrenadeHallucination )
END_DATADESC()

void CWeaponGrenadeHallucination::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon )
{
	CHallucinationProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, pWeapon, DEFAULT_GRENADE_TIMER );
}
	
#endif