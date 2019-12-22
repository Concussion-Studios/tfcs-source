//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
 
#ifndef WEAPON_SDKBASE_H
#define WEAPON_SDKBASE_H
#ifdef _WIN32
#pragma once
#endif
 
#include "sdk_playeranimstate.h"
#include "sdk_weapon_parse.h"
 
#if defined( CLIENT_DLL )
	#define CWeaponSDKBase C_WeaponSDKBase
#endif
 
class CSDKPlayer;
 
// These are the names of the ammo types that the weapon script files reference.
class CWeaponSDKBase : public CBaseCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponSDKBase, CBaseCombatWeapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CWeaponSDKBase();

 #ifdef CLIENT_DLL
	virtual bool ShouldPredict();
#endif

	// All predicted weapons need to implement and return true
	virtual bool	IsPredicted() const { return true; }
	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }
 
	// Get SDK weapon specific weapon data.
	CSDKWeaponInfo const	&GetSDKWpnData() const;
 
	// Get a pointer to the player that owns this weapon
	CSDKPlayer* GetPlayerOwner() const;
 
	// override to play custom empty sounds
	virtual bool PlayEmptySound();
 
	//Tony; these five functions return the sequences the view model uses for a particular action. -- You can override any of these in a particular weapon if you want them to do
	//something different, ie: when a pistol is out of ammo, it would show a different sequence.
	virtual Activity	GetPrimaryAttackActivity( void )	{	return	ACT_VM_PRIMARYATTACK;	}
	virtual Activity	GetIdleActivity( void ) { return ACT_VM_IDLE; }
	virtual Activity	GetDeployActivity( void ) { return ACT_VM_DRAW; }
	virtual Activity	GetReloadActivity( void ) { return ACT_VM_RELOAD; }
	virtual Activity	GetHolsterActivity( void ) { return ACT_VM_HOLSTER; }

	virtual void	WeaponIdle( void );
	virtual bool	Reload( void );
	virtual bool	Deploy();
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	SendReloadEvents();

	virtual void	AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual	float	CalcViewmodelBob( void );

	//Tony; added so we can have base functionality without implementing it into every weapon.
	virtual void ItemPostFrame();
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
 
	//Tony; default weapon spread, pretty accurate - accuracy systems would need to modify this
	virtual float GetWeaponSpread() { return 0.01f; }
	float		m_flPrevAnimTime;

	//Tony; by default, all weapons are automatic.
	//If you add code to switch firemodes, this function would need to be overridden to return the correct current mode.
	virtual int GetFireMode() const { return FM_AUTOMATIC; }
 
	virtual float GetFireRate( void ) { return GetSDKWpnData().m_flCycleTime; };
 
	//Tony; by default, burst fire weapons use a max of 3 shots (3 - 1)
	//weapons with more, ie: a 5 round burst, can override and determine which firemode it's in.
	virtual int MaxBurstShots() const { return 2; }
 
	float GetWeaponFOV() { return GetSDKWpnData().m_flWeaponFOV; }

#ifdef GAME_DLL
	void SetDieThink( bool bDie );
	void Die( void );
	void SetWeaponModelIndex( const char *pName ) { m_iWorldModelIndex = modelinfo->GetModelIndex( pName ); }
#endif
 
	virtual bool CanWeaponBeDropped() const { return true; }

	virtual acttable_t *ActivityList( int &iActivityCount );

	static acttable_t m_acttableTwoHandsGuns[];
	static acttable_t m_acttableHandGun[];
	static acttable_t m_acttableNoReload[];
	static acttable_t m_acttableAC[];
	static acttable_t m_acttableSniper[];
	static acttable_t m_acttableMelee[];
	static acttable_t m_acttableRPG[];
	static acttable_t m_acttableGrenade[];
	static acttable_t m_acttableTool[];
private:
 
	CNetworkVar(float, m_flDecreaseShotsFired);
 
	CWeaponSDKBase( const CWeaponSDKBase & );
};
 
 
#endif // WEAPON_SDKBASE_H