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
	
	void Spawn( void );
	void Precache( void );
	void RocketTouch( CBaseEntity *pOther );
	void Explode( void );
	void Fire( void );
	
	virtual float GetDamage() { return m_flDamage; }
	virtual void SetDamage(float flDamage) { m_flDamage = flDamage; }

	unsigned int PhysicsSolidMaskForEntity( void ) const;

	static CTFCProjectileBaseRockets *Create( const char *szClassname, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner );	

	void SetupInitialTransmittedGrenadeVelocity( const Vector &velocity );

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const {	return SDK_WEAPON_NONE; }

protected:
	virtual void DoExplosion();	

	// Creates the smoke trail
	void CreateSmokeTrail( void );

	void FlyThink( void );

	CHandle<SmokeTrail>	m_hRocketTrail;
	float m_flDamage;

	CNetworkVector( m_vInitialVelocity );

private:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};

#endif // TFC_PROJECTILE_BASE_ROCKETS_H