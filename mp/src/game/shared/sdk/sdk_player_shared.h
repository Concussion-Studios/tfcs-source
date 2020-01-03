//============ Copyright © 1996-2008, Valve Corporation, All rights reserved. ===============//
//
// Purpose: Shared Player Variables / Functions and variables that may or may not be networked
//
//===========================================================================================//
#ifndef SDK_PLAYER_SHARED_H
#define SDK_PLAYER_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"
#include "weapon_sdkbase.h"

#ifdef CLIENT_DLL
class C_SDKPlayer;
#else
class CSDKPlayer;
#endif

//=============================================================================
//
// Tables.
//
#ifdef CLIENT_DLL
	EXTERN_RECV_TABLE( DT_SDKPlayerShared );
#else
	EXTERN_SEND_TABLE( DT_SDKPlayerShared );
#endif

//=============================================================================
//
// Shared player class.
//
class CSDKPlayerShared
{
public:

#ifdef CLIENT_DLL
	friend class C_SDKPlayer;
	typedef C_SDKPlayer OuterClass;
	DECLARE_PREDICTABLE();
#else
	friend class CSDKPlayer;
	typedef CSDKPlayer OuterClass;
#endif

	DECLARE_EMBEDDED_NETWORKVAR()
	DECLARE_CLASS_NOBASE( CSDKPlayerShared );

	CSDKPlayerShared();
	~CSDKPlayerShared();

	void	Init( OuterClass *pOuter );

	CWeaponSDKBase* GetActiveSDKWeapon() const;

	Vector	GetAttackSpread( CWeaponSDKBase *pWeapon, CBaseEntity *pTarget = NULL );
	void	SetJumping( bool bJumping );
	void	ForceUnzoom( void );
	void	ComputeWorldSpaceSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );

	bool	IsSniperZoomed( void ) const;
	bool	IsDucking( void ) const; 
	bool	IsOnGround( void ) const;
	bool	IsOnGodMode() const;
	int		GetButtons();
	bool	IsButtonPressing( int btn );
	bool	IsButtonPressed( int btn );
	bool	IsButtonReleased( int btn );
	bool	IsJumping( void ) { return m_bJumping; }

	// CTF Functions
	bool	IsInGoalZone() { return m_bInGoalZone; }
	void	SetGoalState( bool state );

	void	SetDesiredPlayerClass( int playerclass );
	int		DesiredPlayerClass( void );
	int		GetClassIndex( void ) { return m_iPlayerClass; }
	void	SetPlayerClass( int playerclass );
	int		PlayerClass( void );
	bool	IsClass( int playerclass ) const { return ( m_iPlayerClass == playerclass ); }

private:

	CNetworkVar( bool, m_bInGoalZone );
	CNetworkVar( int, m_iPlayerClass );
	CNetworkVar( int, m_iDesiredPlayerClass );

public:

	bool m_bJumping;
	float m_flLastViewAnimationTime;
	float m_flMaxSpeed;

private:

	OuterClass *m_pOuter;
};			   

#endif // SDK_PLAYER_SHARED_H
