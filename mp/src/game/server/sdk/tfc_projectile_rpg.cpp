//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "tfc_projectile_rpg.h"

LINK_ENTITY_TO_CLASS( tfc_projectile_rpg, CRPGRocket );
PRECACHE_WEAPON_REGISTER( tfc_projectile_rpg );

void CRPGRocket::Spawn( void )
{
	SetModel( ROCKET_MODEL );

	BaseClass::Spawn();
}

void CRPGRocket::Precache( void )
{
	PrecacheModel( ROCKET_MODEL );

	BaseClass::Precache();
}

CRPGRocket *CRPGRocket::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner )
{
	return static_cast<CRPGRocket *> ( CTFCProjectileBaseRockets::Create( "tfc_projectile_rpg", vecOrigin, vecAngles, pOwner ) );
}