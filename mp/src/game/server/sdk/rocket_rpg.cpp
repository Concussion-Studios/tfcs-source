//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "rocket_rpg.h"

LINK_ENTITY_TO_CLASS( rocket_rpg, CRPGRocket );
PRECACHE_WEAPON_REGISTER( rocket_rpg );

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
	return static_cast<CRPGRocket *> ( CSDKBaseRocket::Create( "rocket_rpg", vecOrigin, vecAngles, pOwner ) );
}