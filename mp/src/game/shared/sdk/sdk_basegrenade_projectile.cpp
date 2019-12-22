//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "sdk_basegrenade_projectile.h"

extern ConVar sv_gravity;


#ifdef CLIENT_DLL
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
	#include "soundent.h"
	#include "te_effect_dispatch.h"
BEGIN_DATADESC( CBaseGrenadeProjectile )
	// Fields
	DEFINE_FIELD( m_pMainGlow, FIELD_EHANDLE ),
	DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flNextBlipTime, FIELD_TIME ),
	DEFINE_FIELD( m_inSolid, FIELD_BOOLEAN ),
	
	// Function Pointers
	DEFINE_THINKFUNC( DangerSoundThink ),
END_DATADESC()
#endif

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;


IMPLEMENT_NETWORKCLASS_ALIASED( BaseGrenadeProjectile, DT_BaseGrenadeProjectile )

BEGIN_NETWORK_TABLE( CBaseGrenadeProjectile, DT_BaseGrenadeProjectile )
#ifdef CLIENT_DLL
	RecvPropVector( RECVINFO( m_vInitialVelocity ) )
#else
	SendPropVector( SENDINFO( m_vInitialVelocity ), 
		20,		// nbits
		0,		// flags
		-3000,	// low value
		3000	// high value
		)
#endif
END_NETWORK_TABLE()


#ifdef CLIENT_DLL

	void CBaseGrenadeProjectile::PostDataUpdate( DataUpdateType_t type )
	{
		BaseClass::PostDataUpdate( type );

		if ( type == DATA_UPDATE_CREATED )
		{
			// Now stick our initial velocity into the interpolation history 
			CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();
			
			interpolator.ClearHistory();
			float changeTime = GetLastChangeTime( LATCH_SIMULATION_VAR );

			// Add a sample 1 second back.
			Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
			interpolator.AddToHead( changeTime - 1.0, &vCurOrigin, false );

			// Add the current sample.
			vCurOrigin = GetLocalOrigin();
			interpolator.AddToHead( changeTime, &vCurOrigin, false );
		}
	}

	int CBaseGrenadeProjectile::DrawModel( int flags )
	{
		// During the first half-second of our life, don't draw ourselves if he's
		// still playing his throw animation.
		// (better yet, we could draw ourselves in his hand).
		if ( GetThrower() != C_BasePlayer::GetLocalPlayer() )
		{
			if ( gpGlobals->curtime - m_flSpawnTime < 0.5 )
			{
//Tony; FIXME!
//				C_SDKPlayer *pPlayer = dynamic_cast<C_SDKPlayer*>( GetThrower() );
//				if ( pPlayer && pPlayer->m_PlayerAnimState->IsThrowingGrenade() )
//				{
//					return 0;
//				}
			}
		}

		return BaseClass::DrawModel( flags );
	}

	void CBaseGrenadeProjectile::Spawn()
	{
		m_flSpawnTime = gpGlobals->curtime;
		BaseClass::Spawn();
	}

	void CBaseGrenadeProjectile::Precache()
	{
		BaseClass::Precache();
	}

#else

	void CBaseGrenadeProjectile::Spawn( void )
	{
		BaseClass::Spawn();

		Precache( );

		SetModel( GetGrenadeModel() );

		SetSolidFlags( FSOLID_NOT_STANDABLE );
		SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
		SetSolid( SOLID_BBOX );	// So it will collide with physics props!

		// smaller, cube bounding box so we rest on the ground
		SetSize( Vector ( -2, -2, -2 ), Vector ( 2, 2, 2 ) );
		SetCollisionGroup( COLLISION_GROUP_WEAPON );
		CreateVPhysics();

		BlipSound();
		m_flNextBlipTime = gpGlobals->curtime + GRENADE_BLIP_FREQUENCY;
	}

	void CBaseGrenadeProjectile::Precache()
	{
		BaseClass::Precache();

		PrecacheModel( GetGrenadeModel() );
		PrecacheScriptSound( GetBlipSound() );
		PrecacheModel( "sprites/light_glow02.vmt" );
		PrecacheModel( "sprites/laser.vmt" );
	}

	void CBaseGrenadeProjectile::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
	{
		IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
		if ( pPhysicsObject )
		{
			pPhysicsObject->AddVelocity( &velocity, &angVelocity );
		}
	}

// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
	class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
	{
	public:
		// It does have a base, but we'll never network anything below here..
		DECLARE_CLASS_NOBASE( CTraceFilterCollisionGroupDelta );

		CTraceFilterCollisionGroupDelta( const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup )
			: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked( collisionGroupAlreadyChecked ), m_newCollisionGroup( newCollisionGroup )
		{
		}

		virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
		{
			if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
				return false;
			CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

			if ( pEntity )
			{
				if ( g_pGameRules->ShouldCollide( m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup() ) )
					return false;
				if ( g_pGameRules->ShouldCollide( m_newCollisionGroup, pEntity->GetCollisionGroup() ) )
					return true;
			}

			return false;
		}

	protected:
		const IHandleEntity *m_pPassEnt;
		int		m_collisionGroupAlreadyChecked;
		int		m_newCollisionGroup;
	};

	void CBaseGrenadeProjectile::VPhysicsUpdate( IPhysicsObject *pPhysics )
	{
		BaseClass::VPhysicsUpdate( pPhysics );
		Vector vel;
		AngularImpulse angVel;
		pPhysics->GetVelocity( &vel, &angVel );

		Vector start = GetAbsOrigin();
		// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
		CTraceFilterCollisionGroupDelta filter( this, GetCollisionGroup(), COLLISION_GROUP_NONE );
		trace_t tr;

		// UNDONE: Hull won't work with hitboxes - hits outer hull.  But the whole point of this test is to hit hitboxes.
#if 0
		UTIL_TraceHull( start, start + vel * gpGlobals->frametime, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#else
		UTIL_TraceLine( start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#endif
		if ( tr.startsolid )
		{
			if ( !m_inSolid )
			{
				// UNDONE: Do a better contact solution that uses relative velocity?
				vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
				pPhysics->SetVelocity( &vel, NULL );
			}
			m_inSolid = true;
			return;
		}
		m_inSolid = false;
		if ( tr.DidHit() )
		{
			Vector dir = vel;
			VectorNormalize(dir);
			// send a tiny amount of damage so the character will react to getting bonked
			CTakeDamageInfo info( this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH );
			tr.m_pEnt->TakeDamage( info );

			// reflect velocity around normal
			vel = -2.0f * tr.plane.normal * DotProduct(vel,tr.plane.normal) + vel;

			// absorb 80% in impact
			vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
			angVel *= -0.5f;
			pPhysics->SetVelocity( &vel, &angVel );
		}
	}

	bool CBaseGrenadeProjectile::CreateVPhysics()
	{
		// Create the object in the physics system
		VPhysicsInitNormal( SOLID_BBOX, 0, false );
		return true;
	}

	void CBaseGrenadeProjectile::CreateEffects( void )
	{
		CSDKPlayer *pPlayer = dynamic_cast<CSDKPlayer*>( GetThrower() );
		if ( !pPlayer )
			return;

		static int r;
		static int g;
		static int b;

		switch ( pPlayer->GetTeamNumber() )
		{
			case SDK_TEAM_RED:
				r = 255;
				g = 64;
				b = 64;
				break;
			case SDK_TEAM_BLUE:
				r = 153;
				g = 204;
				b = 255;
				break;
			case SDK_TEAM_GREEN:
				r = 153;
				g = 255;
				b = 153;
				break;
			case SDK_TEAM_YELLOW:
				r = 255;
				g = 178;
				b = 0;
				break;
			default:
				break;
		}

		// Start up the eye glow
		m_pMainGlow = CSprite::SpriteCreate( "sprites/light_glow02.vmt", GetLocalOrigin(), false );

		int	nAttachment = LookupAttachment( "fuse" );

		if ( m_pMainGlow != NULL )
		{
			m_pMainGlow->FollowEntity( this );
			m_pMainGlow->SetAttachment( this, nAttachment );
			m_pMainGlow->SetTransparency( kRenderGlow, r, g, b, 200, kRenderFxNoDissipation );
			m_pMainGlow->SetScale( 0.2f );
			m_pMainGlow->SetGlowProxySize( 4.0f );
		}

		// Start up the eye trail
		m_pGlowTrail = CSpriteTrail::SpriteTrailCreate( "sprites/laser.vmt", GetLocalOrigin(), false );

		if ( m_pGlowTrail != NULL )
		{
			m_pGlowTrail->FollowEntity( this );
			m_pGlowTrail->SetAttachment( this, nAttachment );
			m_pGlowTrail->SetTransparency( kRenderTransAdd, r, g, b, 255, kRenderFxNone );
			m_pGlowTrail->SetStartWidth( 8.0f );
			m_pGlowTrail->SetEndWidth( 1.0f );
			m_pGlowTrail->SetLifeTime( 0.5f );
		}
	}

	int CBaseGrenadeProjectile::OnTakeDamage( const CTakeDamageInfo &inputInfo )
	{
		// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
		VPhysicsTakeDamage( inputInfo );

		// Grenades only suffer blast damage and burn damage.
		if( !( inputInfo.GetDamageType() & ( DMG_BLAST | DMG_BURN ) ) )
			return 0;

		return BaseClass::OnTakeDamage( inputInfo );
	}

	void CBaseGrenadeProjectile::DangerSoundThink( void )
	{
		if (!IsInWorld())
		{
			Remove( );
			return;
		}

		if( gpGlobals->curtime > m_flDetonateTime )
		{
			Detonate();
			return;
		}

		if( gpGlobals->curtime > m_flNextBlipTime )
		{
			BlipSound();
		
			m_flNextBlipTime = gpGlobals->curtime + GRENADE_BLIP_FREQUENCY;
		}

		CSoundEnt::InsertSound ( SOUND_DANGER, GetAbsOrigin() + GetAbsVelocity() * 0.5, GetAbsVelocity().Length( ), 0.2 );

		SetNextThink( gpGlobals->curtime + 0.2 );

		if (GetWaterLevel() != 0)
		{
			SetAbsVelocity( GetAbsVelocity() * 0.5 );
		}
	}

	//Sets the time at which the grenade will explode
	void CBaseGrenadeProjectile::SetDetonateTimerLength( float timer )
	{
		m_flDetonateTime = gpGlobals->curtime + timer;

		CreateEffects();
	}

	void CBaseGrenadeProjectile::ResolveFlyCollisionCustom( trace_t &trace, Vector &vecVelocity )
	{
		//Assume all surfaces have the same elasticity
		float flSurfaceElasticity = 1.0;

		//Don't bounce off of players with perfect elasticity
		if( trace.m_pEnt && trace.m_pEnt->IsPlayer() )
		{
			flSurfaceElasticity = 0.3;
		}

		// if its breakable glass and we kill it, don't bounce.
		// give some damage to the glass, and if it breaks, pass 
		// through it.
		bool breakthrough = false;

		if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable" ) )
		{
			breakthrough = true;
		}

		if( trace.m_pEnt && FClassnameIs( trace.m_pEnt, "func_breakable_surf" ) )
		{
			breakthrough = true;
		}

		if (breakthrough)
		{
			CTakeDamageInfo info( this, this, 10, DMG_CLUB );
			trace.m_pEnt->DispatchTraceAttack( info, GetAbsVelocity(), &trace );

			ApplyMultiDamage();

			if( trace.m_pEnt->m_iHealth <= 0 )
			{
				// slow our flight a little bit
				Vector vel = GetAbsVelocity();

				vel *= 0.4;

				SetAbsVelocity( vel );
				return;
			}
		}
		
		float flTotalElasticity = GetElasticity() * flSurfaceElasticity;
		flTotalElasticity = clamp( flTotalElasticity, 0.0f, 0.9f );

		// NOTE: A backoff of 2.0f is a reflection
		Vector vecAbsVelocity;
		PhysicsClipVelocity( GetAbsVelocity(), trace.plane.normal, vecAbsVelocity, 2.0f );
		vecAbsVelocity *= flTotalElasticity;

		// Get the total velocity (player + conveyors, etc.)
		VectorAdd( vecAbsVelocity, GetBaseVelocity(), vecVelocity );
		float flSpeedSqr = DotProduct( vecVelocity, vecVelocity );

		// Stop if on ground.
		if ( trace.plane.normal.z > 0.7f )			// Floor
		{
			// Verify that we have an entity.
			CBaseEntity *pEntity = trace.m_pEnt;
			Assert( pEntity );

			SetAbsVelocity( vecAbsVelocity );

			if ( flSpeedSqr < ( 30 * 30 ) )
			{
				if ( pEntity->IsStandable() )
				{
					SetGroundEntity( pEntity );
				}

				// Reset velocities.
				SetAbsVelocity( vec3_origin );
				SetLocalAngularVelocity( vec3_angle );

				//align to the ground so we're not standing on end
				QAngle angle;
				VectorAngles( trace.plane.normal, angle );

				// rotate randomly in yaw
				angle[1] = random->RandomFloat( 0, 360 );

				// TODO: rotate around trace.plane.normal
				
				SetAbsAngles( angle );			
			}
			else
			{
				Vector vecDelta = GetBaseVelocity() - vecAbsVelocity;	
				Vector vecBaseDir = GetBaseVelocity();
				VectorNormalize( vecBaseDir );
				float flScale = vecDelta.Dot( vecBaseDir );

				VectorScale( vecAbsVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, vecVelocity ); 
				VectorMA( vecVelocity, ( 1.0f - trace.fraction ) * gpGlobals->frametime, GetBaseVelocity() * flScale, vecVelocity );
				PhysicsPushEntity( vecVelocity, &trace );
			}
		}
		else
		{
			// If we get *too* slow, we'll stick without ever coming to rest because
			// we'll get pushed down by gravity faster than we can escape from the wall.
			if ( flSpeedSqr < ( 30 * 30 ) )
			{
				// Reset velocities.
				SetAbsVelocity( vec3_origin );
				SetLocalAngularVelocity( vec3_angle );
			}
			else
			{
				SetAbsVelocity( vecAbsVelocity );
			}
		}
		
		BounceSound();
	}

	void CBaseGrenadeProjectile::SetupInitialTransmittedGrenadeVelocity( const Vector &velocity )
	{
		m_vInitialVelocity = velocity;
	}

	void CBaseGrenadeProjectile::Splash()
	{
		Vector centerPoint = GetAbsOrigin();
		Vector normal( 0, 0, 1 );

		// Find our water surface by tracing up till we're out of the water
		trace_t tr;
		Vector vecTrace( 0, 0, MAX_WATER_SURFACE_DISTANCE );
		UTIL_TraceLine( centerPoint, centerPoint + vecTrace, MASK_WATER, NULL, COLLISION_GROUP_NONE, &tr );

		// If we didn't start in water, we're above it
		if ( tr.startsolid == false )
		{
			// Look downward to find the surface
			vecTrace.Init( 0, 0, -MAX_WATER_SURFACE_DISTANCE );
			UTIL_TraceLine( centerPoint, centerPoint + vecTrace, MASK_WATER, NULL, COLLISION_GROUP_NONE, &tr );

			// If we hit it, setup the explosion
			if ( tr.fraction < 1.0f )
				centerPoint = tr.endpos;
			else
				//NOTENOTE: We somehow got into a splash without being near water?
				Assert( 0 );
		}
		else if ( tr.fractionleftsolid )
		{
			// Otherwise we came out of the water at this point
			centerPoint = centerPoint + (vecTrace * tr.fractionleftsolid);
		}
		else
		{
			// Use default values, we're really deep
		}

		CEffectData	data;
		data.m_vOrigin = centerPoint;
		data.m_vNormal = normal;
		data.m_flScale = random->RandomFloat( 1.0f, 2.0f );

		if ( GetWaterType() & CONTENTS_SLIME )
			data.m_fFlags |= FX_WATER_IN_SLIME;

		DispatchEffect( "gunshotsplash", data );
	}
#endif
