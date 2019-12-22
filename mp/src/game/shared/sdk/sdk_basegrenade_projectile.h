//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef SDK_BASEGRENADE_PROJECTILE_H
#define SDK_BASEGRENADE_PROJECTILE_H
#ifdef _WIN32
#pragma once
#endif


#include "basegrenade_shared.h"
#ifdef GAME_DLL
#include "Sprite.h"
#include "SpriteTrail.h"
#endif // GAME_DLL


#define	MAX_WATER_SURFACE_DISTANCE	512
#define DEFAULT_GRENADE_MODEL "models/Weapons/w_eq_fraggrenade_thrown.mdl"
#define DEFAULT_GRENADE_BOUNCE "Grenade.Bounce"
#define GRENADE_BLIP_FREQUENCY 4.0f

#ifdef CLIENT_DLL
	#define CBaseGrenadeProjectile C_BaseGrenadeProjectile
#endif


class CBaseGrenadeProjectile : public CBaseGrenade
{
public:
	DECLARE_CLASS( CBaseGrenadeProjectile, CBaseGrenade );
	DECLARE_NETWORKCLASS(); 

	virtual void Spawn();
	virtual void Precache( void );

public:
	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const {	return SDK_WEAPON_NONE; }

	// This gets sent to the client and placed in the client's interpolation history
	// so the projectile starts out moving right off the bat.
	CNetworkVector( m_vInitialVelocity );


#ifdef CLIENT_DLL
	CBaseGrenadeProjectile() {}
	CBaseGrenadeProjectile( const CBaseGrenadeProjectile& ) {}
	virtual int DrawModel( int flags );
	virtual void PostDataUpdate( DataUpdateType_t type );
	
	float m_flSpawnTime;
#else
	DECLARE_DATADESC();

	virtual bool CreateVPhysics( void );
	virtual void SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	virtual void VPhysicsUpdate( IPhysicsObject *pPhysics );

	virtual void CreateEffects( void );

	//Constants for all SDK Grenades
	static inline float GetGrenadeGravity() { return 0.4f; }
	static inline const float GetGrenadeFriction() { return 0.2f; }
	static inline const float GetGrenadeElasticity() { return 0.45f; }
	virtual float GetDetonateTime( void ){ return m_flDetonateTime; }

	//Think function to emit danger sounds for the AI
	virtual void DangerSoundThink( void );
	
	virtual float GetShakeAmplitude( void ) { return 0.0f; }
	virtual void Splash();

	virtual int	OnTakeDamage( const CTakeDamageInfo &inputInfo );

	// Specify what velocity we want the grenade to have on the client immediately.
	// Without this, the entity wouldn't have an interpolation history initially, so it would
	// sit still until it had gotten a few updates from the server.
	virtual void SetupInitialTransmittedGrenadeVelocity( const Vector &velocity );

	virtual void BlipSound() { EmitSound( GetBlipSound() ); }
	virtual const char *GetBlipSound() { return DEFAULT_GRENADE_BOUNCE; }
	virtual const char *GetGrenadeModel() { return DEFAULT_GRENADE_MODEL; }

protected:

	//Set the time to detonate ( now + timer )
	virtual void SetDetonateTimerLength( float timer );

private:	
	
	//Custom collision to allow for constant elasticity on hit surfaces
	virtual void ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity );

	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;
	
	float m_flDetonateTime;
	Vector		vecLastOrigin;

	bool	m_inSolid;
	float	m_flNextBlipTime;
#endif
};


#endif // SDK_BASEGRENADE_PROJECTILE_H
