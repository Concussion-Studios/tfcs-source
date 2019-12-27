//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef OBJ_SENTRY_H
#define OBJ_SENTRY_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "particle_system.h"

#define TURRET_SHOTS	2
#define TURRET_RANGE	(100 * 12)
#define TURRET_SPREAD	Vector( 0, 0, 0 )
#define TURRET_TURNRATE	30		//angles per 0.1 second
#define TURRET_MAXWAIT	15		// seconds turret will stay active w/o a target
#define TURRET_MAXSPIN	5		// seconds turret barrel will spin w/o a target
#define SF_MONSTER_TURRET_AUTOACTIVATE	32
#define SF_MONSTER_TURRET_STARTINACTIVE	64
#define TURRET_GLOW_SPRITE "sprites/flare3.vmt"
#define TURRET_ORIENTATION_FLOOR 0
#define TURRET_ORIENTATION_CEILING 1

typedef enum
{
	//	TURRET_ANIM_NONE = 0,
	TURRET_ANIM_FIRE = 0,
	TURRET_ANIM_SPIN,
	TURRET_ANIM_DEPLOY,
	TURRET_ANIM_RETIRE,
	TURRET_ANIM_DIE,
} TURRET_ANIM;

class CSprite;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CBaseSentry : public CAI_BaseNPC
{
	DECLARE_CLASS( CBaseSentry, CAI_BaseNPC );
public:
	virtual void Spawn( void );
	virtual void Precache( void );
	virtual int OnTakeDamage( const CTakeDamageInfo &info );
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo );
	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual bool ShouldFadeOnDeath( void ) { return false; }

	Class_T Classify( void );

	virtual void TurretUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	
	// Think functions
	virtual void ActiveThink( void );
	virtual void SearchThink( void );
	virtual void AutoSearchThink( void );
	virtual void TurretDeath( void );
	virtual void SpinDownCall( void ) { m_iSpin = 0; }
	virtual void SpinUpCall( void ) { m_iSpin = 1; }
	virtual void Deploy( void );
	virtual void Retire( void );
	virtual void Initialize( void );
	virtual void Ping( void );
	virtual void EyeOn( void );
	virtual void EyeOff( void );
	virtual void SetTurretAnim( TURRET_ANIM anim );
	virtual int MoveTurret( void );
	virtual void Shoot( Vector &vecSrc, Vector &vecDirToEnemy ) { };

	virtual void InputActivate( inputdata_t &inputdata );
	virtual void InputDeactivate( inputdata_t &inputdata );

	float m_flMaxSpin;		// Max time to spin the barrel w/o a target
	int m_iSpin;

	CSprite *m_pEyeGlow;
	int		m_eyeBrightness;

	int	m_iDeployHeight;
	int	m_iRetractHeight;
	int m_iMinPitch;

	int m_iBaseTurnRate;	// angles per second
	float m_fTurnRate;		// actual turn rate
	int m_iOrientation;		// 0 = floor, 1 = Ceiling
	int	m_iOn;
	int m_fBeserk;			// Sometimes this bitch will just freak out
	int m_iAutoStart;		// true if the turret auto deploys when a target
							// enters its range

	Vector m_vecLastSight;
	float m_flLastSight;	// Last time we saw a target
	float m_flMaxWait;		// Max time to seach w/o a target
	int m_iSearchSpeed;		// Not Used!

	// movement
	float	m_flStartYaw;
	Vector	m_vecCurAngles;
	Vector	m_vecGoalAngles;


	float	m_flPingTime;	// Time until the next ping, used when searching
	float	m_flSpinUpTime;	// Amount of time until the barrel should spin down when searching

	float   m_flDamageTime;

	int		m_iAmmoType;

	COutputEvent	m_OnActivate;
	COutputEvent	m_OnDeactivate;

	DECLARE_DATADESC();

	typedef CAI_BaseNPC BaseClass;
};

//=========================================================
// Sentry gun
//=========================================================
class CObjSentryGun : public CBaseSentry
{
	DECLARE_CLASS( CObjSentryGun, CBaseSentry );

public:
	virtual void Spawn( );
	virtual void Precache(void);

	// other functions
	virtual void Shoot( Vector &vecSrc, Vector &vecDirToEnemy );
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void SentryTouch( CBaseEntity *pOther );

	DECLARE_DATADESC();

private:
	bool m_bStartedDeploy;	//set to true when the turret begins its deploy
};

#endif // OBJ_SENTRY_H
