//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "gamerules.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_grenade_nail.h"

#ifndef CLIENT_DLL
	#include "sdk_player.h"
	#include "items.h"
	#include "sdk_basegrenade_projectile.h"
	#include "physics_saverestore.h"
	#include "phys_controller.h"
	#include "tfc_projectile_nail.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenadeNail, DT_WeaponGrenadeNail )

BEGIN_NETWORK_TABLE(CWeaponGrenadeNail, DT_WeaponGrenadeNail)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGrenadeNail )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_grenade_nail, CWeaponGrenadeNail );
PRECACHE_WEAPON_REGISTER( weapon_grenade_nail );


#ifdef GAME_DLL
class CNailGrenadeController : public IMotionEvent
{
	DECLARE_SIMPLE_DATADESC();
public:
	virtual simresult_e	Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
	{
		// Try to get to m_vecDesiredPosition

		// Try to orient ourselves to m_angDesiredOrientation

		Vector currentPos;
		QAngle currentAng;

		pObject->GetPosition( &currentPos, &currentAng );

		Vector vecVel;
		AngularImpulse angVel;
		pObject->GetVelocity( &vecVel, &angVel );

		linear.Init();
		angular.Init();

		if ( m_bReachedPos )
		{
			// Lock at this height
			if ( vecVel.Length() > 1.0 )
			{
				AngularImpulse nil( 0,0,0 );
				pObject->SetVelocityInstantaneous( &vec3_origin, &nil );

				// For now teleport to the proper orientation
				currentAng.x = 0;
				currentAng.y = 0;
				currentAng.z = 0;
				pObject->SetPosition( currentPos, currentAng, true );
			}
		}
		else
		{
			// not at the right height yet, keep moving up
			linear.z =  50 * ( m_vecDesiredPosition.z - currentPos.z );

			if ( currentPos.z > m_vecDesiredPosition.z )
				m_bReachedPos = true; // lock into position

			// Start rotating in the right direction
			// we'll lock angles once we reach proper height to stop the oscillating
			matrix3x4_t matrix;
			// get the object's local to world transform
			pObject->GetPositionMatrix( &matrix );

			Vector m_worldGoalAxis(0,0,1);

			// Get the alignment axis in object space
			Vector currentLocalTargetAxis;
			VectorIRotate( m_worldGoalAxis, matrix, currentLocalTargetAxis );

			float invDeltaTime = (1/deltaTime);
			float m_angularLimit = 10;

			angular = ComputeRotSpeedToAlignAxes( m_worldGoalAxis, currentLocalTargetAxis, angVel, 1.0, invDeltaTime * invDeltaTime, m_angularLimit * invDeltaTime );
		}

		return SIM_GLOBAL_ACCELERATION;
	}	

public:
	void SetDesiredPosAndOrientation( Vector pos, QAngle orientation )
	{
		m_vecDesiredPosition = pos;
		m_angDesiredOrientation = orientation;

		m_bReachedPos = false;
		m_bReachedOrientation = false;
	}

private:
	Vector m_vecDesiredPosition;
	QAngle m_angDesiredOrientation;

	bool m_bReachedPos;
	bool m_bReachedOrientation;
};

class CNailProjectile : public CBaseGrenadeProjectile
{
public:
	DECLARE_CLASS( CNailProjectile, CBaseGrenadeProjectile );
	DECLARE_DATADESC();

	//Tony; by default projectiles don't have one, so make sure derived weapons do!!
	virtual SDKWeaponID GetWeaponID( void ) const {	return WEAPON_GRENADE_NAIL; }

	CNailProjectile::~CNailProjectile()
	{
		if ( m_pMotionController != NULL )
		{
			physenv->DestroyMotionController( m_pMotionController );
			m_pMotionController = NULL;
		}
	}


	// Overrides.
public:
	virtual void Spawn()
	{
		m_pMotionController = NULL;

		UseClientSideAnimation();

		BaseClass::Spawn();
	}

	virtual void Precache()
	{
		PrecacheScriptSound( "Weapon_SNailgun.Single" );

		BaseClass::Precache();
	}

	virtual void Detonate()
	{
		if ( !IsInWorld() )
		{
			Remove( );
			return;
		}

		StartEmittingNails();
	}

	virtual int OnTakeDamage( const CTakeDamageInfo &info )
	{
		if ( m_pMotionController != NULL )	// motioncontroller is animating us, dont take hits that will disorient us
			return 0;

		return BaseClass::OnTakeDamage( info );
	}

	void StartEmittingNails( void )
	{
		// 0.4 seconds later, emit nails
		IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

		if ( pPhysicsObject )
		{
			m_pMotionController = physenv->CreateMotionController( &m_GrenadeController );
			m_pMotionController->AttachObject( pPhysicsObject, true );

			pPhysicsObject->EnableGravity( false );

			pPhysicsObject->Wake();
		}

		QAngle ang(0,0,0);
		Vector pos = GetAbsOrigin();
		pos.z += 32;
		m_GrenadeController.SetDesiredPosAndOrientation( pos, ang );

		m_flNailAngle = 0;
		m_iNumNailBurstsLeft = 40;

		int animDesired = SelectWeightedSequence( ACT_RANGE_ATTACK1 );
		ResetSequence( animDesired );
		SetPlaybackRate( 1.0 );

		Vector soundPosition = GetAbsOrigin() + Vector( 0, 0, 5 );
		CPASAttenuationFilter filter( soundPosition );
		EmitSound( filter, entindex(), "Weapon_SNailgun.Single" );

		SetThink( &CNailProjectile::EmitNails );
		SetNextThink( gpGlobals->curtime + 0.4 );
	}

	void EmitNails( void )
	{
		m_iNumNailBurstsLeft--;

		if ( m_iNumNailBurstsLeft < 0 )
		{
			BaseClass::Detonate();
			return;
		}

		Vector forward, up;
		float flAngleToAdd = random->RandomFloat( 30, 40 );

		// else release some nails
		for ( int i=0; i < 4 ;i++ )
		{
			m_flNailAngle = UTIL_AngleMod( m_flNailAngle + flAngleToAdd );

			QAngle angNail( random->RandomFloat( -3, 3 ), m_flNailAngle, 0 );

			// Emit a nail
			auto* pNail = CTFCProjectileNail::CreateNailShower( GetAbsOrigin(), angNail, this );
			if ( pNail )
				pNail->SetDamage( 18 );
		}

		SetNextThink( gpGlobals->curtime + 0.1 );
	}
	// Grenade stuff.
public:

	static CNailProjectile* Create( 
		const Vector &position, 
		const QAngle &angles, 
		const Vector &velocity, 
		const AngularImpulse &angVelocity, 
		CBaseCombatCharacter *pOwner,
		CWeaponSDKBase *pWeapon,
		float timer )
	{
		CNailProjectile *pNail = (CNailProjectile*)CBaseEntity::Create( "grenade_nail_projectile", position, angles, pOwner );

		// Set the timer for 1 second less than requested. We're going to issue a SOUND_DANGER
		// one second before detonation.
		pNail->SetVelocity( velocity, angVelocity );

		pNail->SetDetonateTimerLength( timer );
		pNail->SetAbsVelocity( velocity );
		pNail->SetupInitialTransmittedGrenadeVelocity( velocity );
		pNail->SetThrower( pOwner ); 

		pNail->SetGravity( BaseClass::GetGrenadeGravity() );
		pNail->SetFriction( BaseClass::GetGrenadeFriction() );
		pNail->SetElasticity( BaseClass::GetGrenadeElasticity() );

		pNail->m_flDamage = pWeapon->GetSDKWpnData().m_iDamage;
		pNail->m_DmgRadius = pNail->m_flDamage * 3.5f;
		pNail->ChangeTeam( pOwner->GetTeamNumber() );
		pNail->ApplyLocalAngularVelocityImpulse( angVelocity );	

		// make NPCs afaid of it while in the air
		pNail->SetThink( &CNailProjectile::DangerSoundThink );
		pNail->SetNextThink( gpGlobals->curtime );

		return pNail;
	}

	CNailGrenadeController m_GrenadeController;
	IPhysicsMotionController* m_pMotionController;

public:
	bool m_bActivated;
	float m_flNailAngle;
	int m_iNumNailBurstsLeft;
};

LINK_ENTITY_TO_CLASS( grenade_nail_projectile, CNailProjectile );
PRECACHE_WEAPON_REGISTER( grenade_nail_projectile );

BEGIN_DATADESC( CNailProjectile )
	DEFINE_THINKFUNC( EmitNails ),
	DEFINE_EMBEDDED( m_GrenadeController ),
	DEFINE_PHYSPTR( m_pMotionController ),
END_DATADESC()

BEGIN_DATADESC( CWeaponGrenadeNail )
END_DATADESC()

BEGIN_SIMPLE_DATADESC( CNailGrenadeController )
END_DATADESC()

void CWeaponGrenadeNail::EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer, CWeaponSDKBase *pWeapon )
{
	CNailProjectile::Create( vecSrc, vecAngles, vecVel, angImpulse, pPlayer, pWeapon, DEFAULT_GRENADE_TIMER );
}
	
#endif