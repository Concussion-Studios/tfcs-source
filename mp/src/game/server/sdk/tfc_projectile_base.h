#ifndef TFC_PROJECTILE_BASE_H
#define TFC_PROJECTILE_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "props.h"

#define BOLT_AIR_VELOCITY	3500
#define BOLT_WATER_VELOCITY	1500

//=============================================================================
//
// projectile base ( some generic projectile to derive off )
//
//=============================================================================
class CTFCProjectileBase : public CPhysicsProp
{
public:
	DECLARE_CLASS( CTFCProjectileBase, CPhysicsProp );
	DECLARE_DATADESC();

			CTFCProjectileBase();
	virtual	~CTFCProjectileBase( void );

	virtual bool CreateVPhysics();
	unsigned int PhysicsSolidMaskForEntity() const;

	virtual float SetDamageAmount( float damage ) { return flDamage = damage; }
	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }

	virtual void ProjectileStickTo( CBaseEntity *pOther, trace_t &tr ) { Assert( 0 && "ProjectileBubbleThink should not be called. Make sure to implement this in your subclass!\n" ); }
	virtual void ProjectileTouch( CBaseEntity* pOther ) { Assert( 0 && "ProjectileTouch should not be called. Make sure to implement this in your subclass!\n" ); }
	virtual void ProjectileThink( void ) { Assert( 0 && "ProjectileThink should not be called. Make sure to implement this in your subclass!\n" ); }

protected:
	float flDamage;
};

#endif //TFC_PROJECTILE_BASE_H