//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_RPG_H
#define WEAPON_RPG_H
#endif

#ifdef _WIN32
#pragma once
#endif

#include "weapon_sdkbase_combatweapon.h"

#ifdef CLIENT_DLL

	#include "iviewrender_beams.h"

#endif

#ifndef CLIENT_DLL
#include "Sprite.h"
#include "npcevent.h"
#include "beam_shared.h"

class CWeaponRPG;
class CLaserDot;
class RocketTrail;
 
//###########################################################################
//	>> CMissile		(missile launcher class is below this one!)
//###########################################################################
class CMissile : public CBaseCombatCharacter
{
	DECLARE_CLASS( CMissile, CBaseCombatCharacter );

public:
	CMissile();
	~CMissile();

#ifdef HL1_DLL
	Class_T Classify( void ) { return CLASS_NONE; }
#else
	Class_T Classify( void ) { return CLASS_MISSILE; }
#endif
	
	void	Spawn( void );
	void	Precache( void );
	void	MissileTouch( CBaseEntity *pOther );
	void	Explode( void );
	void	ShotDown( void );
	void	AccelerateThink( void );
	void	AugerThink( void );
	void	IgniteThink( void );
	void	SeekThink( void );
	void	DumbFire( void );
	void	SetGracePeriod( float flGracePeriod );

	int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void	Event_Killed( const CTakeDamageInfo &info );
	
	virtual float	GetDamage() { return m_flDamage; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }

	unsigned int PhysicsSolidMaskForEntity( void ) const;

	CHandle<CWeaponRPG>		m_hOwner;

	static CMissile *Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner );

protected:
	virtual void DoExplosion();	
	virtual void ComputeActualDotPosition( CLaserDot *pLaserDot, Vector *pActualDotPosition, float *pHomingSpeed );
	virtual int AugerHealth() { return m_iMaxHealth - 20; }

	// Creates the smoke trail
	void CreateSmokeTrail( void );

	// Gets the shooting position 
	void GetShootPosition( CLaserDot *pLaserDot, Vector *pShootPosition );

	CHandle<RocketTrail>	m_hRocketTrail;
	float					m_flAugerTime;		// Amount of time to auger before blowing up anyway
	float					m_flMarkDeadTime;
	float					m_flDamage;

private:
	float					m_flGracePeriodEndsAt;

	DECLARE_DATADESC();
};
#endif
//-----------------------------------------------------------------------------
// RPG
//-----------------------------------------------------------------------------

#ifdef CLIENT_DLL
#define CWeaponRPG C_WeaponRPG
#endif

class CWeaponRPG : public CSDKCombatWeapon
{
	DECLARE_CLASS(CWeaponRPG, CSDKCombatWeapon);
public:

	CWeaponRPG();
	~CWeaponRPG();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	virtual SDKWeaponID GetWeaponID(void) const		{ return WEAPON_RPG; }
	void	Precache( void );

	void	PrimaryAttack( void );
	virtual float GetFireRate( void ) { return 1; };
	void	ItemPostFrame( void );

	void	Activate( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	bool	Reload( void );
	void	FillClip();
	bool	WeaponShouldBeLowered( void );
	bool	Lower( void );

	bool	CanHolster( void );

	virtual void Drop( const Vector &vecVelocity );

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 1; }
	float	GetMinRestTime() { return 4.0; }
	float	GetMaxRestTime() { return 4.0; }

	void	StartGuiding( void );
	void	StopGuiding( void );
	void	ToggleGuiding( void );
	bool	IsGuiding( void );

	void	NotifyRocketDied( void );

	bool	HasAnyAmmo( void );

	void	SuppressGuiding( bool state = true );

	void	CreateLaserPointer( void );
	void	UpdateLaserPosition( Vector vecMuzzlePos = vec3_origin, Vector vecEndPos = vec3_origin );
	Vector	GetLaserPosition( void );

	// NPC RPG users cheat and directly set the laser pointer's origin
	void	UpdateNPCLaserPosition( const Vector &vecTarget );
	void	SetNPCLaserPosition( const Vector &vecTarget );
	const Vector &GetNPCLaserPosition( void );
	
#ifdef CLIENT_DLL

	// We need to render opaque and translucent pieces
	virtual RenderGroup_t	GetRenderGroup( void ) {	return RENDER_GROUP_TWOPASS;	}

	virtual void	NotifyShouldTransmit( ShouldTransmitState_t state );
	virtual int		DrawModel( int flags );
	virtual void	ViewModelDrawn( C_BaseViewModel *pBaseViewModel );
	virtual bool	IsTranslucent( void );

	void			InitBeam( void );
	void			GetWeaponAttachment( int attachmentId, Vector &outVector, Vector *dir = NULL );
	void			DrawEffects( void );
//	void			DrawLaserDot( void );

	CMaterialReference	m_hSpriteMaterial;	// Used for the laser glint
	CMaterialReference	m_hBeamMaterial;	// Used for the laser beam
	Beam_t				*m_pBeam;			// Laser beam temp entity

#endif	//CLIENT_DLL

	CBaseEntity *GetMissile( void ) { return m_hMissile; }

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif
	
protected:

	CNetworkVar( bool, m_bInitialStateUpdate );
	CNetworkVar( bool, m_bGuiding );
	CNetworkVar( bool, m_bHideGuiding );

	CNetworkHandle( CBaseEntity,	m_hMissile );
	CNetworkVar(	Vector,			m_vecLaserDot );

#ifndef CLIENT_DLL
	CHandle<CLaserDot>	m_hLaserDot;
#endif

private:
	
	CWeaponRPG( const CWeaponRPG & );
};
// WEAPON_RPG_H
