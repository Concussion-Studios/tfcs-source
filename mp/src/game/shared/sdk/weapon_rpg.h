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
class RocketTrail;
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

	bool	CanHolster( void );

	virtual void Drop( const Vector &vecVelocity );

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 1; }
	float	GetMinRestTime() { return 4.0; }
	float	GetMaxRestTime() { return 4.0; }

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif
	
protected:

	CNetworkVar( bool, m_bInitialStateUpdate );


private:
	
	CWeaponRPG( const CWeaponRPG & );
};
// WEAPON_RPG_H
