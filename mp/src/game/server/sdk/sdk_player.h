//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for SDK Game
//
// $NoKeywords: $
//=============================================================================//

#ifndef SDK_PLAYER_H
#define SDK_PLAYER_H
#ifdef _WIN32
#pragma once
#endif

#include "basemultiplayerplayer.h"
#include "server_class.h"
#include "sdk_playeranimstate.h"
#include "sdk_player_shared.h"
#include "sdk_basegrenade_projectile.h"
#include "vphysics/player_controller.h"

#define MAX_DAMAGER_HISTORY 2

#define SDK_PUSHAWAY_THINK_CONTEXT	"SDKPushawayThink"
#define SDK_PHYSDAMAGE_SCALE 4.0f

// Function table for each player state.
class CSDKPlayerStateInfo
{
public:
	SDKPlayerState m_iPlayerState;
	const char *m_pStateName;
	
	void (CSDKPlayer::*pfnEnterState)();	// Init and deinit the state.
	void (CSDKPlayer::*pfnLeaveState)();
	void (CSDKPlayer::*pfnPreThink)();	// Do a PreThink() in this state.
};

struct DamagerHistory_t
{
	DamagerHistory_t()
	{
		Reset();
	}
	void Reset()
	{
		hDamager = NULL;
		flTimeDamage = 0;
	}
	EHANDLE hDamager;
	float	flTimeDamage;
};

//=============================================================================
// >> SDK Game player
//=============================================================================
class CSDKPlayer : public CBaseMultiplayerPlayer
{
public:
	DECLARE_CLASS( CSDKPlayer, CBaseMultiplayerPlayer );
	DECLARE_SERVERCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();

	CSDKPlayer();
	~CSDKPlayer();

	static CSDKPlayer *CreatePlayer( const char *className, edict_t *ed );
	static CSDKPlayer* Instance( int iEnt );

	// This passes the event to the client's and server's CPlayerAnimState.
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );
	virtual void SetupBones( matrix3x4_t *pBoneToWorld, int boneMask );

	virtual void FlashlightTurnOn( void );
	virtual void FlashlightTurnOff( void );
	virtual int FlashlightIsOn( void );

	virtual void PreThink();
	virtual void PostThink();
	virtual void Spawn();
	virtual void InitialSpawn();

	// Animstate handles this.
	void SetAnimation( PLAYER_ANIM playerAnim ) { return; }

	virtual void Precache();
	virtual void Splash( void );

	virtual void OnDamagedByExplosion( const CTakeDamageInfo &info );
	virtual int	 OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual int	 OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void BecomeAGibs( const CTakeDamageInfo &info );
	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );

	virtual void LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles );

	virtual void FireBullets( const FireBulletsInfo_t& info );
	
	void AddDamagerToHistory(EHANDLE hDamager);
	void ClearDamagerHistory();
	DamagerHistory_t &GetDamagerHistory(int i) { return m_DamagerHistory[i]; }

	Class_T Classify( void );
	
	CWeaponSDKBase* GetActiveSDKWeapon() const;
	CWeaponSDKBase* GetWeaponOwnerID( int iID );

	virtual void	CreateViewModel( int viewmodelindex = 0 );
	virtual void	SetHandsModel( const char* model );

	virtual bool BumpWeapon( CBaseCombatWeapon *pWeapon );

	// custom player functions
	virtual void CheatImpulseCommands( int iImpulse );
	//CSDKPlayerClassInfo* GetClassInfo(void);

	// Armor
	virtual void IncrementArmorValue( int nCount, int nMaxValue = -1 );
	virtual void SetArmorValue( int value );
	virtual void AdjustArmor( int Armorvalue );
	virtual void SetMaxArmorValue( int MaxArmorValue );
	virtual int GetArmorValue()	{ return m_ArmorValue; }
	virtual int GetMaxArmorValue() { return m_MaxArmorValue; }
	virtual void SetArmorClass(float value) { m_flArmorClass = value; };
	virtual float GetArmorClass() { return m_flArmorClass; }

	// Ammo and items
	virtual int GiveAmmo(int iCount, int iAmmoIndex, bool bSuppressSound = false);
	void		DiscardAmmo(bool bDead = false);							//Chrits; discard unused ammo

	// Health
	virtual void SetHealth( int value );
	virtual void SetMaxHealth( int MaxValue );
	virtual void AdjustHealth( int Healthvalue );
	virtual int GetHealth() { return m_iHealth; }
	virtual int GetMaxHealth()	{ return m_iMaxHealth;	}
	virtual void IncrementHealthValue( int nCount );

	CNetworkQAngle( m_angEyeAngles );	// Copied from EyeAngles() so we can send it to the client.
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 

	int GetPlayerClassAsString( char *pDest, int iDestSize );
	void SetClassMenuOpen( bool bIsOpen );
	bool IsClassMenuOpen( void );

	void PhysObjectSleep();
	void PhysObjectWake();

	virtual void ChangeTeam( int iTeamNum );

	// Player avoidance
	virtual	bool		ShouldCollide( int collisionGroup, int contentsMask ) const;
	void SDKPushawayThink(void);
	void InitVCollision( const Vector& vecAbsOrigin, const Vector& vecAbsVelocity );

	Vector m_vecTotalBulletForce;	//Accumulator for bullet force in a single frame

// In shared code.
public:
	void FireBullet( 
		Vector vecSrc, 
		const QAngle &shootAngles, 
		float vecSpread, 
		int iDamage, 
		int iBulletType,
		CBaseEntity *pevAttacker,
		bool bDoEffects,
		float x,
		float y );

	CNetworkVarEmbedded( CSDKPlayerShared, m_Shared );
	virtual void PlayerDeathThink( void ) {	/* overridden, do nothing - our states handle this now */ }
	virtual bool ClientCommand( const CCommand &args );

	Vector GetAttackSpread( CWeaponSDKBase *pWeapon, CBaseEntity *pTarget = NULL );

	virtual void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );

	void IncreaseShotsFired() { m_iShotsFired++; if (m_iShotsFired > 16) m_iShotsFired = 16; }
	void DecreaseShotsFired() { m_iShotsFired--; if (m_iShotsFired < 0) m_iShotsFired = 0; }
	void ClearShotsFired() { m_iShotsFired = 0; }
	int GetShotsFired() { return m_iShotsFired; }

	// Returns true if the player is allowed to attack.
	bool CanAttack( void );

	virtual int GetPlayerStance();

	// Called whenever this player fires a shot.
	void NoteWeaponFired();
	virtual bool WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const;

// ------------------------------------------------------------------------------------------------ //
// Player state management.
// ------------------------------------------------------------------------------------------------ //
public:

	void State_Transition( SDKPlayerState newState );
	SDKPlayerState State_Get() const;				// Get the current state.

	virtual bool	ModeWantsSpectatorGUI( int iMode ) { return ( iMode != OBS_MODE_DEATHCAM && iMode != OBS_MODE_FREEZECAM ); }

private:
	bool SelectSpawnSpot( const char *pEntClassName, CBaseEntity* &pSpot );

	void State_Enter( SDKPlayerState newState );	// Initialize the new state.
	void State_Leave();								// Cleanup the previous state.
	void State_PreThink();							// Update the current state.

	// Specific state handler functions.
	void State_Enter_WELCOME();
	void State_PreThink_WELCOME();

	void State_Enter_PICKINGTEAM();
	void State_Enter_PICKINGCLASS();

public: //Tony; I had this private but I need it public for initial spawns.
	void MoveToNextIntroCamera();
private:

	void State_Enter_ACTIVE();
	void State_PreThink_ACTIVE();

	void State_Enter_OBSERVER_MODE();
	void State_PreThink_OBSERVER_MODE();

	void State_Enter_DEATH_ANIM();
	void State_PreThink_DEATH_ANIM();

	// Find the state info for the specified state.
	static CSDKPlayerStateInfo* State_LookupInfo( SDKPlayerState state );

	// This tells us which state the player is currently in (joining, observer, dying, etc).
	// Each state has a well-defined set of parameters that go with it (ie: observer is movetype_noclip, non-solid,
	// invisible, etc).
	CNetworkVar( SDKPlayerState, m_iPlayerState );

	CSDKPlayerStateInfo *m_pCurStateInfo;			// This can be NULL if no state info is defined for m_iPlayerState.
	bool HandleCommand_JoinTeam( int iTeam );
	bool	m_bTeamChanged;		//have we changed teams this spawn? Used to enforce one team switch per death rule

	bool HandleCommand_JoinClass( int iClass );
	void ShowClassSelectMenu();
	bool m_bIsClassMenuOpen;


	void InitSpeeds( void ); //Tony; called EVERY spawn on server and client after class has been chosen (if any!)

	// from CBasePlayer
	void			SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize );

	CBaseEntity*	EntSelectSpawnPoint();
	bool			CanMove( void ) const;

	virtual void	SharedSpawn();

	virtual const Vector	GetPlayerMins( void ) const; // uses local player
	virtual const Vector	GetPlayerMaxs( void ) const; // uses local player

	virtual void		CommitSuicide( bool bExplode = false, bool bForce = false );

private:
	// Last usercmd we shot a bullet on.
	int m_iLastWeaponFireUsercmd;

	float m_flHealthRegenDelay;

	// When the player joins, it cycles their view between trigger_camera entities.
	// This is the current camera, and the time that we'll switch to the next one.
	EHANDLE m_pIntroCamera;
	DamagerHistory_t m_DamagerHistory[DAMAGERS_HISTORY_MAX];
	float m_fIntroCamTime;

	void CreateRagdollEntity();
	void DestroyRagdoll( void );

	
	CSDKPlayerAnimState *m_PlayerAnimState;

	CNetworkVar( bool, m_bSpawnInterpCounter );

	float m_flArmorClass;
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_ArmorValue );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_iHealth );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_iMaxHealth );
	CNetworkVarForDerived( int, m_MaxArmorValue );
};


inline CSDKPlayer *ToSDKPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast<CSDKPlayer*>( pEntity ) != 0 );
#endif
	return static_cast< CSDKPlayer* >( pEntity );
}

inline SDKPlayerState CSDKPlayer::State_Get() const
{
	return m_iPlayerState;
}


#endif	// SDK_PLAYER_H
