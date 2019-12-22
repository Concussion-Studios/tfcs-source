//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef WEAPON_IC_H
#define WEAPON_IC_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_sdkbase_rpg.h"

#if defined( CLIENT_DLL )
	#define CWeaponInciendaryCanon C_WeaponInciendaryCanon
#endif

//-----------------------------------------------------------------------------
// IC
//-----------------------------------------------------------------------------
class CWeaponInciendaryCanon : public CWeaponSDKBaseRPG
{
public:
	DECLARE_CLASS( CWeaponInciendaryCanon, CWeaponSDKBaseRPG );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponInciendaryCanon() {}

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_IC; }
	virtual void FireRocket( void );

	CNetworkVar( bool, m_bInitialStateUpdate );

private:
	CWeaponInciendaryCanon( const CWeaponInciendaryCanon& );
};

#endif // WEAPON_IC_H
