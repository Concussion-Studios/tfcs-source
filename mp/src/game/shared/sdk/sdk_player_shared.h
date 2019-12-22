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

	Vector	GetAttackSpread(CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget = NULL);
	bool	IsSniperZoomed( void ) const;
	bool	IsDucking( void ) const; 

	void	SetDesiredPlayerClass( int playerclass );
	int		DesiredPlayerClass( void );
	int		GetClassIndex( void ) { return m_iPlayerClass; }
	void	SetPlayerClass( int playerclass );
	int		PlayerClass( void );
	bool	IsClass( int playerclass ) const { return ( m_iPlayerClass == playerclass ); }

	CWeaponSDKBase* GetActiveSDKWeapon() const;

	bool	IsJumping( void ) { return m_bJumping; }
	void	SetJumping( bool bJumping );

	void	ForceUnzoom( void );

	void ComputeWorldSpaceSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs );

private:

	CNetworkVar( int, m_iPlayerClass );
	CNetworkVar( int, m_iDesiredPlayerClass );

public:

	bool m_bJumping;

	float m_flLastViewAnimationTime;

	float m_flMaxSpeed;

private:

	OuterClass *m_pOuter;
};			   

#endif //SDK_PLAYER_SHARED_H
