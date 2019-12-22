//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "tfc_projectile_ic.h"
#include "fire.h"

LINK_ENTITY_TO_CLASS( tfc_projectile_ic, CInciendaryRocket );
PRECACHE_WEAPON_REGISTER( tfc_projectile_ic );

void CInciendaryRocket::Spawn( void )
{
	SetModel( ROCKET_MODEL );

	BaseClass::Spawn();
}

void CInciendaryRocket::Precache( void )
{
	PrecacheModel( ROCKET_MODEL );
	UTIL_PrecacheOther( "env_fire" );

	BaseClass::Precache();
}

void CInciendaryRocket::DoExplosion( void )
{
	trace_t trace;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector ( 0, 0, -128 ),  MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trace );

	// Pull out of the wall a bit
	if ( trace.fraction != 1.0 )
		SetLocalOrigin( trace.endpos + (trace.plane.normal * (m_flDamage - 24) * 0.6) );

	int contents = UTIL_PointContents ( GetAbsOrigin() );
	if ( (contents & MASK_WATER) )
	{
		UTIL_Remove( this );
		return;
	}

	// Start some fires
	int i;
	QAngle vecTraceAngles;
	Vector vecTraceDir;
	trace_t firetrace;

	CBaseEntity *pAttacker = GetOwnerEntity();
	if ( !pAttacker )
		pAttacker = GetContainingEntity( INDEXENT(0) );

	for( i = 0 ; i < 16 ; i++ )
	{
		// build a little ray
		vecTraceAngles[PITCH]	= random->RandomFloat(45, 135);
		vecTraceAngles[YAW]		= random->RandomFloat(0, 360);
		vecTraceAngles[ROLL]	= 0.0f;

		AngleVectors( vecTraceAngles, &vecTraceDir );

		Vector vecStart, vecEnd;

		vecStart = GetAbsOrigin() + ( trace.plane.normal * 128 );
		vecEnd = vecStart + vecTraceDir * 512;

		UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &firetrace );

		Vector	ofsDir = ( firetrace.endpos - GetAbsOrigin() );
		float	offset = VectorNormalize( ofsDir );

		if ( offset > 128 )
			offset = 128;

		//Get our scale based on distance
		float scale	 = 0.1f + ( 0.75f * ( 1.0f - ( offset / 128.0f ) ) );
		float growth = 0.1f + ( 0.75f * ( offset / 128.0f ) );

		if( firetrace.fraction != 1.0 )
			FireSystem_StartFire( firetrace.endpos, scale, growth, 30.0f, (SF_FIRE_START_ON), pAttacker, FIRE_NATURAL );
	}
}

CInciendaryRocket *CInciendaryRocket::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner )
{
	return static_cast<CInciendaryRocket *> ( CTFCProjectileBaseRockets::Create( "tfc_projectile_ic", vecOrigin, vecAngles, pOwner ) );
}