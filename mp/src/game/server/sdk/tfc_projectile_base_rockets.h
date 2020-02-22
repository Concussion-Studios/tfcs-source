//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef TFC_PROJECTILE_BASE_ROCKETS_H
#define TFC_PROJECTILE_BASE_ROCKETS_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "baseanimating.h"
#include "smoke_trail.h"
#include "weapon_sdkbase.h"

class RocketTrail;
 
//================================================
// CTFCProjectileBaseRockets	
//================================================
class CTFCProjectileBaseRockets : public CBaseAnimating
{
	DECLARE_CLASS( CTFCProjectileBaseRockets, CBaseAnimating );

public:
	CTFCProjectileBaseRockets();
	~CTFCProjectileBaseRockets();
	
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void RocketTouch( CBaseEntity *pOther );
	virtual void Explode( void );
	virtual void Fire( void );
	virtual float GetGravity(void) { return 0.001f; }

	virtual float GetDamage() { return m_flDamage; }
	virtual void SetDamage( float flDamage ) { m_flDamage = flDamage; }
	virtual float GetDamageRadius()	{ return m_DmgRadius; }
	virtual void SetDamageRadius( float flDamageRadius ) { m_DmgRadius = flDamageRadius; }

	//unsigned int PhysicsSolidMaskForEntity( void ) const;

	static CTFCProjectileBaseRockets *Create( const char *szClassname, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner );	

	virtual void SetupInitialTransmittedGrenadeVelocity( const Vector &velocity );

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const {	return SDK_WEAPON_NONE; }

protected:
	virtual void DoExplosion();	

	// Creates the smoke trail
	virtual void CreateSmokeTrail( void );

	virtual void FlyThink( void );

	CHandle<SmokeTrail>	m_hRocketTrail;

	CNetworkVector( m_vInitialVelocity );

	float m_flDamage;		// Damage to inflict.
	float m_DmgRadius;		// How far do I do damage?

private:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};

#endif // TFC_PROJECTILE_BASE_ROCKETS_H