//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_RPG_H
#define WEAPON_RPG_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_sdkbase_rpg.h"

#if defined( CLIENT_DLL )
	#define CWeaponRPG C_WeaponRPG
#endif

//-----------------------------------------------------------------------------
// RPG
//-----------------------------------------------------------------------------
class CWeaponRPG : public CWeaponSDKBaseRPG
{
public:
	DECLARE_CLASS( CWeaponRPG, CWeaponSDKBaseRPG );

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponRPG() {}

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_RPG; }
	virtual void FireRocket( void );

	CNetworkVar( bool, m_bInitialStateUpdate );

private:
	CWeaponRPG( const CWeaponRPG& );
};

#endif // WEAPON_RPG_H
