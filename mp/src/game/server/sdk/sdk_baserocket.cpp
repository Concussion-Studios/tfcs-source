//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "sdk_baserocket.h"
#include "explode.h"
#include "sdk_gamerules.h"

ConVar mp_rocketdamage( "mp_rocketdamage", "150", FCVAR_GAMEDLL | FCVAR_CHEAT );
ConVar mp_rocketradius( "mp_rocketradius", "200", FCVAR_GAMEDLL | FCVAR_CHEAT );

BEGIN_DATADESC( CSDKBaseRocket )
	// Function Pointers
	DEFINE_FUNCTION( RocketTouch ),
	DEFINE_THINKFUNC( FlyThink ),
END_DATADESC()


IMPLEMENT_SERVERCLASS_ST( CSDKBaseRocket, DT_SDKBaseRocket )
	SendPropVector( SENDINFO( m_vInitialVelocity ),	20,	0, -3000, 3000 )
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CSDKBaseRocket::CSDKBaseRocket()
{
	m_hRocketTrail = NULL;
}

CSDKBaseRocket::~CSDKBaseRocket()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKBaseRocket::Precache( void )
{
	PrecacheScriptSound( "Weapon_RPG.Fly" );	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKBaseRocket::Spawn( void )
{
	Precache();

	SetSolid( SOLID_BBOX );

	Assert( GetModel() );	//derived classes must have set model

	UTIL_SetSize( this, -Vector(2,2,2), Vector(2,2,2) );

	SetTouch( &CSDKBaseRocket::RocketTouch );

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	
	m_takedamage = DAMAGE_NO;
	SetGravity( 0.1 );
	SetDamage( mp_rocketdamage.GetFloat() );	

	AddFlag( FL_OBJECT );

	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

	EmitSound( "Weapon_RPG.Fly" );

	// Smoke trail.
	CreateSmokeTrail();

	SetThink( &CSDKBaseRocket::FlyThink );
	SetNextThink( gpGlobals->curtime );
}

unsigned int CSDKBaseRocket::PhysicsSolidMaskForEntity( void ) const
{ 
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

//-----------------------------------------------------------------------------
// Purpose: Stops any kind of tracking and shoots dumb
//-----------------------------------------------------------------------------
void CSDKBaseRocket::Fire( void )
{
	SetThink( NULL );
	SetMoveType( MOVETYPE_FLY );

	SetModel("models/weapons/w_missile.mdl");
	UTIL_SetSize( this, vec3_origin, vec3_origin );

	EmitSound( "Weapon_RPG.Fly" );

	// Smoke trail.
	CreateSmokeTrail();
}

//-----------------------------------------------------------------------------
// The actual explosion 
//-----------------------------------------------------------------------------
void CSDKBaseRocket::DoExplosion( void )
{
	// Explode
	ExplosionCreate( 
		GetAbsOrigin(),	//DMG_ROCKET
		GetAbsAngles(),
		GetOwnerEntity(),
		GetDamage(),		//magnitude
		mp_rocketradius.GetFloat(),				//radius
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE,
		0.0f,				//explosion force
		this);				//inflictor
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKBaseRocket::Explode( void )
{
	// Don't explode against the skybox. Just pretend that 
	// the missile flies off into the distance.
	Vector vecForward = GetAbsVelocity();

	trace_t tr;
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + 60*vecForward, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	m_takedamage = DAMAGE_NO;
	if( tr.fraction == 1.0 || !(tr.surface.flags & SURF_SKY) )
	{
		DoExplosion();

		if ( !tr.m_pEnt->IsPlayer() )
			UTIL_DecalTrace( &tr, "Scorch" );
	}

	if( m_hRocketTrail )
	{
		m_hRocketTrail->SetLifetime(0.1f);
		m_hRocketTrail = NULL;
	}

	StopSound( "Weapon_RPG.Fly" );
	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CSDKBaseRocket::RocketTouch( CBaseEntity *pOther )
{
	Assert( pOther );
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;

	if ( pOther->GetCollisionGroup() == COLLISION_GROUP_WEAPON )
		return;

	// if we hit the skybox, just disappear
	const trace_t &tr = CBaseEntity::GetTouchTrace();

	const trace_t *p = &tr;
	trace_t *newTrace = const_cast<trace_t*>(p);

	if( tr.surface.flags & SURF_SKY )
	{
		UTIL_Remove( this );
		return;
	}

	if( !pOther->IsPlayer() )
	{
		CTakeDamageInfo info;
		info.SetAttacker( this );
		info.SetInflictor( this );
		info.SetDamage( 50 );
		info.SetDamageForce( vec3_origin );	// don't worry about this not having a damage force.
											// It will explode on touch and impart its own forces
		info.SetDamageType( DMG_CLUB );

		Vector dir;
		AngleVectors( GetAbsAngles(), &dir );

		pOther->DispatchTraceAttack( info, dir, newTrace );
		ApplyMultiDamage();
	}

	if( pOther->IsAlive() )
	{
		Explode();
	}

	// else we will continue our movement
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKBaseRocket::CreateSmokeTrail( void )
{
	if ( m_hRocketTrail )
		return;

	// Smoke trail.
	if ( (m_hRocketTrail = SmokeTrail::CreateSmokeTrail()) != NULL )
	{
		m_hRocketTrail->m_Opacity = 1.0f;
		m_hRocketTrail->m_SpawnRate = 65;
		m_hRocketTrail->m_ParticleLifetime = 3.0f;
		m_hRocketTrail->m_StartColor.Init( 0.65f, 0.65f , 0.65f );
		m_hRocketTrail->m_EndColor.Init( 0.65f, 0.65f, 0.65f );
		m_hRocketTrail->m_StartSize = 8;
		m_hRocketTrail->m_EndSize = 24;
		m_hRocketTrail->m_SpawnRadius = 8;
		m_hRocketTrail->m_MinSpeed = 2;
		m_hRocketTrail->m_MaxSpeed = 16;
		
		m_hRocketTrail->SetLifetime( 10 );
		m_hRocketTrail->FollowEntity( this, "0" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKBaseRocket::FlyThink( void )
{
	QAngle angles;

	VectorAngles( GetAbsVelocity(), angles );

	SetAbsAngles( angles );
	
	SetNextThink( gpGlobals->curtime + 0.1f );
}

	
//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : &vecOrigin - 
//			&vecAngles - 
//			NULL - 
//
// Output : CSDKBaseRocket
//-----------------------------------------------------------------------------
CSDKBaseRocket *CSDKBaseRocket::Create( const char *szClassname, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL )
{
	CSDKBaseRocket *pMissile = (CSDKBaseRocket *) CBaseEntity::Create( szClassname, vecOrigin, vecAngles, pOwner );
	pMissile->SetOwnerEntity( pOwner );
	pMissile->Spawn();
	
	Vector vecForward;
	AngleVectors( vecAngles, &vecForward );

	Vector vRocket = vecForward * 1300;

	pMissile->SetAbsVelocity( vRocket );	
	pMissile->SetupInitialTransmittedGrenadeVelocity( vRocket );

	pMissile->SetAbsAngles( vecAngles );

	// remember what team we should be on
	pMissile->ChangeTeam( pOwner->GetTeamNumber() );

	return pMissile;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKBaseRocket::SetupInitialTransmittedGrenadeVelocity( const Vector &velocity )
{
	m_vInitialVelocity = velocity;
}
