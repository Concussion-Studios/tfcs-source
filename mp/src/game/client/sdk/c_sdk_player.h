//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_SDK_PLAYER_H
#define C_SDK_PLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "sdk_playeranimstate.h"
#include "c_baseplayer.h"
#include "baseparticleentity.h"
#include "sdk_player_shared.h"
#include "beamdraw.h"
#include "flashlighteffect.h"

#define SDK_AVOID_MAX_RADIUS_SQR		5184.0f			// Based on player extents and max buildable extents.
#define SDK_OO_AVOID_MAX_RADIUS_SQR		0.00029f
#define FLASHLIGHT_DISTANCE 1000

//Tony; m_pFlashlightEffect is private, so just subclass. We may want to do some more stuff with it later anyway.
class CSDKFlashlightEffect : public CFlashlightEffect
{
public:
	CSDKFlashlightEffect(int nIndex = 0) : CFlashlightEffect( nIndex  ) {}
	~CSDKFlashlightEffect() {};

	virtual void UpdateLight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance );
};

class C_SDKPlayer : public C_BasePlayer
{
public:
	DECLARE_CLASS( C_SDKPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_SDKPlayer();
	~C_SDKPlayer();

	static C_SDKPlayer* GetLocalSDKPlayer();

	virtual const QAngle& GetRenderAngles();
	virtual void UpdateClientSideAnimation();
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual void OnDataChanged( DataUpdateType_t updateType );

	virtual void ProcessMuzzleFlashEvent();
	virtual void AddEntity( void );
	virtual void ItemPreFrame( void );
	virtual void ItemPostFrame( void );
	virtual void NotifyShouldTransmit( ShouldTransmitState_t state );
	virtual void CreateLightEffects( void ) {}

	virtual void CalcVehicleView( IClientVehicle* pVehicle, Vector& eyeOrigin, QAngle& eyeAngles, float& zNear, float& zFar, float& fov);
	virtual void CalcPlayerView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	virtual void CalcViewRoll( QAngle& eyeAngles );
	virtual void CalcViewBob( Vector& eyeOrigin );
	virtual void CalcViewIdle( QAngle& eyeAngles );

	virtual void ThirdPersonSwitch( bool bThirdperson );

	virtual void TeamChange( int iNewTeam ) OVERRIDE;
	static void TeamChangeStatic( int iNewTeam );
	virtual void OnSpawn( void );

	float ViewBob;
	double BobTime;
	float BobLastTime;
	float IdleScale;

	virtual void DoImpactEffect( trace_t &tr, int nDamageType );
	
	// Player avoidance
	bool ShouldCollide( int collisionGroup, int contentsMask ) const;
	void AvoidPlayers( CUserCmd *pCmd );
	float m_fNextThinkPushAway;
	virtual bool CreateMove( float flInputSampleTime, CUserCmd *pCmd );
	virtual void ClientThink();
	virtual const QAngle& EyeAngles( void );

	virtual Vector GetAutoaimVector( float flDelta );

	// Have this player play the sounds from his view model's reload animation.
	void PlayReloadEffect();

	virtual bool ShouldResetSequenceOnNewModel( void ) { return false; }

	virtual ShadowType_t ShadowCastType();
	virtual bool ShouldReceiveProjectedTextures( int flags );

// Called by shared code.
public:
	SDKPlayerState State_Get() const;
	
	void DoAnimationEvent( PlayerAnimEvent_t event, int nData = 0 );
	virtual void CalculateIKLocks( float currentTime );
	virtual bool ShouldDraw();

	CWeaponSDKBase *GetActiveSDKWeapon() const;
	CWeaponSDKBase* GetWeaponOwnerID( int iID );

	virtual C_BaseAnimating *BecomeRagdollOnClient();
	virtual IRagdoll* GetRepresentativeRagdoll() const;

	Vector GetAttackSpread( CWeaponSDKBase *pWeapon, CBaseEntity *pTarget = NULL );

	virtual void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );

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

	void IncreaseShotsFired() { m_iShotsFired++; if (m_iShotsFired > 16) m_iShotsFired = 16; }
	void DecreaseShotsFired() { m_iShotsFired--; if (m_iShotsFired < 0) m_iShotsFired = 0; }
	void ClearShotsFired() { m_iShotsFired = 0; }
	int GetShotsFired() { return m_iShotsFired; }

	virtual void SharedSpawn();
	
	void InitSpeeds( void ); //Tony; called EVERY spawn on server and client after class has been chosen (if any!)

	// Returns true if the player is allowed to move.
	bool CanMove() const;

	// Returns true if the player is allowed to attack.
	bool CanAttack( void );

	CSDKPlayerShared m_Shared;

	virtual const Vector	GetPlayerMins( void ) const; // uses local player
	virtual const Vector	GetPlayerMaxs( void ) const; // uses local player

// Not Shared, but public.
public:

	bool CanShowTeamMenu();
	bool CanShowClassMenu();

	void LocalPlayerRespawn( void );

	//Tony; update lookat, if our model has moving eyes setup, they need to be updated.
	void UpdateLookAt( void );
	int GetIDTarget() const;
	void UpdateIDTarget( void );

	//Tony; when model is changed, need to init some stuff.
	virtual CStudioHdr *OnNewModel( void );
	void InitializePoseParams( void );

public: // Public Variables
	CSDKPlayerAnimState *m_PlayerAnimState;

	QAngle	m_angEyeAngles;
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	EHANDLE	m_hRagdoll;

	int	m_headYawPoseParam;
	int	m_headPitchPoseParam;
	float m_headYawMin;
	float m_headYawMax;
	float m_headPitchMin;
	float m_headPitchMax;

	Vector m_vLookAtTarget;

	float m_flLastBodyYaw;
	float m_flCurrentHeadYaw;
	float m_flCurrentHeadPitch;

	int	  m_iIDEntIndex;

	// Armor
	int GetArmorValue() { return m_ArmorValue; }
	int GetMaxArmorValue() { return m_MaxArmorValue; }

private:
	void UpdateSoundEvents();

	CNetworkVar( bool, m_bSpawnInterpCounter );
	bool m_bSpawnInterpCounterCache;

	CNetworkVar( SDKPlayerState, m_iPlayerState );

	C_SDKPlayer( const C_SDKPlayer & );

	virtual void UpdateFlashlight( void ); //Tony; override.
	void ReleaseFlashlight( void );
	Beam_t	*m_pFlashlightBeam;

	CSDKFlashlightEffect *m_pSDKFlashLightEffect;

	int m_ArmorValue, m_MaxArmorValue;

	class CSDKSoundEvent
	{
	public:
		string_t m_SoundName;
		float m_flEventTime;	// Play the event when gpGlobals->curtime goes past this.
	};
	CUtlLinkedList<CSDKSoundEvent,int> m_SoundEvents;

};


inline C_SDKPlayer* ToSDKPlayer( CBaseEntity *pPlayer )
{
	if ( !pPlayer || !pPlayer->IsPlayer() )
		return NULL;

	return static_cast< C_SDKPlayer* >( pPlayer );
}

inline SDKPlayerState C_SDKPlayer::State_Get() const
{
	return m_iPlayerState;
}

#endif // C_SDK_PLAYER_H
