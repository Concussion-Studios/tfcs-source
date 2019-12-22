//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: projectile shooted from the nailgun
//
//=============================================================================//
#include "cbase.h"
#include "tfc_projectile_nail.h"
#include "particle_parse.h"
#include "IEffects.h"
#include "sdk_player.h"
#include "props.h"
#include "func_break.h"
#include "props.h"
#include "te_effect_dispatch.h"

BEGIN_DATADESC( CTFCProjectileNail )
	DEFINE_ENTITYFUNC( ProjectileTouch ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( tfc_projectile_nail, CTFCProjectileNail );
PRECACHE_REGISTER( tfc_projectile_nail );

CTFCProjectileNail::CTFCProjectileNail()
{
}

CTFCProjectileNail::~CTFCProjectileNail()
{
}

void CTFCProjectileNail::Precache( void )
{
	PrecacheModel( PROJWCTILW_NAIL_MODEL );
}

void CTFCProjectileNail::Spawn()
{
	Precache( );

	SetModel( PROJWCTILW_NAIL_MODEL );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	UTIL_SetSize( this, -Vector( 1, 1, 1 ), Vector( 1, 1, 1 ) );
	SetSolid( SOLID_BBOX );
	SetGravity( GetGravity() );
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	
	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch( &CTFCProjectileBase::ProjectileTouch );
}

void CTFCProjectileNail::ProjectileStickTo( CBaseEntity *pOther, trace_t &tr )
{
	SetMoveType( MOVETYPE_NONE );
	
	if ( !pOther->IsWorld() )
	{
		SetParent( pOther );
		SetSolid( SOLID_NONE );
		SetSolidFlags( FSOLID_NOT_SOLID );
	}
	
	Vector vecVelocity = GetAbsVelocity();

	SetTouch( NULL );

	static int s_nImpactCount = 0;
	s_nImpactCount++;
	if ( s_nImpactCount & 0x01 )
	{
		UTIL_ImpactTrace( &tr, DMG_BULLET );
		
		// Shoot some sparks
		if ( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER)
			g_pEffects->Sparks( GetAbsOrigin() );
	}
}

void CTFCProjectileNail::ProjectileTouch( CBaseEntity* pOther )
{
	if ( pOther->IsSolidFlagSet( FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER ) )
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ( ( pOther->m_takedamage == DAMAGE_NO ) || ( pOther->m_takedamage == DAMAGE_EVENTS_ONLY ) )
			return;
	}

	if ( FClassnameIs( pOther, "tfc_projectile_nail" ) )
		return;

	trace_t	tr;
	tr = BaseClass::GetTouchTrace();

	if ( pOther->m_takedamage != DAMAGE_NO )
	{
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize( vecNormalizedVel );

		CBreakable *pBreak = dynamic_cast <CBreakable *>(pOther);
		if ( pBreak && ( pBreak->GetMaterialType() == matGlass || pBreak->GetMaterialType() == matWeb ) )
			flDamage = MAX( pOther->GetHealth(), flDamage );

		CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), flDamage, DMG_SLASH );
		CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
		dmgInfo.SetDamagePosition( tr.endpos );
		pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );

		ApplyMultiDamage();

		// Keep going through breakable glass.
		if ( pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS )
			 return;
			 
		SetAbsVelocity( Vector( 0, 0, 0 ) );

		Vector vForward;
		AngleVectors( GetAbsAngles(), &vForward );
		VectorNormalize ( vForward );

		trace_t	tr2;
		UTIL_TraceLine( GetAbsOrigin(),	GetAbsOrigin() + vForward * 128, MASK_BLOCKLOS, pOther, COLLISION_GROUP_NONE, &tr2 );

		if ( ( ( pOther->GetMoveType() == MOVETYPE_VPHYSICS ) || ( pOther->GetMoveType() == MOVETYPE_PUSH ) ) && ( ( pOther->GetHealth() > 0 ) || ( pOther->m_takedamage == DAMAGE_EVENTS_ONLY ) ) )
		{
			CPhysicsProp *pProp = dynamic_cast<CPhysicsProp *>( pOther );
			if ( pProp )
				pProp->SetInteraction( PROPINTER_PHYSGUN_NOTIFY_CHILDREN );
		
			// We hit a physics object that survived the impact. Stick to it.
			ProjectileStickTo( pOther, tr );
		}
		else
		{
			SetTouch( NULL );
			SetThink( NULL );

			UTIL_Remove( this );
		}
	}
	else
	{
		// See if we struck the world
		if ( pOther->GetMoveType() == MOVETYPE_NONE && !( tr.surface.flags & SURF_SKY ) )
		{
			// We hit a physics object that survived the impact. Stick to it.
			ProjectileStickTo( pOther, tr );
		}
		else if( pOther->GetMoveType() == MOVETYPE_PUSH && FClassnameIs(pOther, "func_breakable") )
		{
			// We hit a func_breakable, stick to it.
			// The MOVETYPE_PUSH is a micro-optimization to cut down on the classname checks.
			ProjectileStickTo( pOther, tr );
		}
		else if ( pOther->IsPlayer() )
		{
			// We hit a player. Stick to it.
			ProjectileStickTo( pOther, tr );
		}
		else
		{
			// Put a mark unless we've hit the sky
			if ( ( tr.surface.flags & SURF_SKY ) == false )
				UTIL_ImpactTrace( &tr, DMG_BULLET );

			UTIL_Remove( this );
		}
	}
}

CTFCProjectileNail *CTFCProjectileNail::CreateNail(
	const Vector& vecOrigin, 
	const QAngle& vecAngles, 
	CBaseEntity* pOwner, 
	const Vector& vecVelocity, 
	float flDamage )
{
	auto* pNail = static_cast< CTFCProjectileNail* >( CreateEntityByName( "tfc_projectile_nail" ) );
	if ( !pNail )
	{
		Assert( pNail );
		return NULL;
	}

	pNail->SetOwnerEntity( pOwner );
	pNail->Spawn();
	pNail->SetAbsVelocity( vecVelocity );
	pNail->SetAbsAngles( vecAngles );
	pNail->SetAbsOrigin( vecOrigin );
	pNail->ChangeTeam( pOwner->GetTeamNumber() );
	pNail->SetDamageAmount( flDamage );

	return pNail;
}

CTFCProjectileNail *CTFCProjectileNail::CreateNailShower(
	const Vector& vecOrigin, 
	const QAngle& vecAngles, 
	CBaseEntity* pOwner  )
{
	auto* pNail = static_cast< CTFCProjectileNail* >( CreateEntityByName( "tfc_projectile_nail" ) );
	if ( !pNail )
	{
		Assert( pNail );
		return NULL;
	}

	pNail->SetOwnerEntity( pOwner );
	pNail->Spawn();
	pNail->SetAbsAngles( vecAngles );
	pNail->SetAbsOrigin( vecOrigin );
	pNail->ChangeTeam( pOwner->GetTeamNumber() );

	return pNail;
}