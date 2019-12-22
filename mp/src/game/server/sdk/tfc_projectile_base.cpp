//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "tfc_projectile_base.h"
#include "sdk_player.h"

BEGIN_DATADESC( CTFCProjectileBase )
	DEFINE_ENTITYFUNC( ProjectileTouch ),
	DEFINE_THINKFUNC( ProjectileThink ),
END_DATADESC()

CTFCProjectileBase::CTFCProjectileBase()
{
	UseClientSideAnimation();
}

CTFCProjectileBase::~CTFCProjectileBase()
{
}

bool CTFCProjectileBase::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );

	return true;
}

unsigned int CTFCProjectileBase::PhysicsSolidMaskForEntity() const
{
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}