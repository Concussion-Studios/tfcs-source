//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
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
 
//Tony; use the same name as the base context one.
#define SDK_HIDEWEAPON_THINK_CONTEXT	"BaseCombatWeapon_HideThink"

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
	virtual void OnDataChanged( DataUpdateType_t type );
	virtual void AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual	float CalcViewmodelBob( void );
#endif

	virtual void Precache( void );
	virtual void WeaponSound( WeaponSound_t sound_type, float soundtime = 0.0f );
	virtual void SetWeaponVisible( bool visible );

	// All predicted weapons need to implement and return true
	virtual bool	IsPredicted() const { return true; }
	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }
 
	// Get SDK weapon specific weapon data.
	CSDKWeaponInfo const	&GetSDKWpnData() const;
	virtual int				GetDamageType(void) { return g_aWeaponDamageTypes[GetWeaponID()]; }
 
	// Get a pointer to the player that owns this weapon
	CSDKPlayer* GetPlayerOwner() const;
 
	// override to play custom empty sounds
	virtual bool PlayEmptySound();

	// Weapon Types - this should be done in other way.
	virtual bool IsPistolWeapon() { return GetSDKWpnData().m_bIsPistol; }
	virtual bool IsShotgunWeapon() { return GetSDKWpnData().m_bIsShotgun; }
	virtual bool IsSniperWeapon() { return GetSDKWpnData().m_bIsSniper; }
	virtual bool IsHeavyWeapon() { return GetSDKWpnData().m_bIsHeavy; }
	virtual bool IsGrenadeWeapon() { return GetSDKWpnData().m_bIsGrenade; }
	virtual bool IsLaserWeapon() { return GetSDKWpnData().m_bIsLaser; }
	virtual bool IsBiozardWeapon() { return GetSDKWpnData().m_bIsBiozard; }
	virtual bool IsToolWeapon() { return GetSDKWpnData().m_bIsTool; }
 
	//Tony; these five functions return the sequences the view model uses for a particular action. -- You can override any of these in a particular weapon if you want them to do
	//something different, ie: when a pistol is out of ammo, it would show a different sequence.
	virtual Activity	GetPrimaryAttackActivity( void )	{	return	ACT_VM_PRIMARYATTACK;	}
	virtual Activity	GetIdleActivity( void ) { return ACT_VM_IDLE; }
	virtual Activity	GetDeployActivity( void ) { return ACT_VM_DRAW; }
	virtual Activity	GetReloadActivity( void ) { return ACT_VM_RELOAD; }
	virtual Activity	GetHolsterActivity( void ) { return ACT_VM_HOLSTER; }

	virtual const char	*GetDeploySound( void ) { return "Default.WeaponDeployPrimary"; }

	virtual void	WeaponIdle( void );
	virtual bool	Reload( void );
	virtual bool	Deploy();
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	SendReloadEvents();

	virtual void FallInit( void );
#ifdef GAME_DLL
	virtual void FallThink( void );	// make the weapon fall to the ground after spawning
	virtual void DoMuzzleFlash( void );
#endif

	//Tony; added so we can have base functionality without implementing it into every weapon.
	virtual void ItemPostFrame();
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();

	virtual float GetAccuracyModifier( void );

	virtual bool HasScope( void ) { return GetSDKWpnData().m_bUseScope; }	// not all of our weapons have scopes (although some do)
	virtual bool UnscopeAfterShot( void ) { return GetSDKWpnData().m_bUnscopeAfterShot; } // by default, allow scoping while firing
	virtual bool ShouldDrawScope( void ) { return m_bIsScoped; }
	virtual bool IsScoped( void ) { return m_bIsScoped; }
	virtual bool CanScope( void );
	virtual void EnterScope( void );
	virtual void ExitScope( bool unhideWeapon = true );
	virtual float GetScopeFOV( void ) { return GetSDKWpnData().m_flScopeFov; }
	virtual bool ShouldDrawCrosshair( void ) { return GetSDKWpnData().m_bDrawCosshair; }	// disables drawing crosshairs

	virtual bool ShouldDrawMuzzleFlash( void ) { return GetSDKWpnData().m_bDrawMuzzleFlash; }	// by default, all of our weapons have muzzleflashes
 
	//Tony; default weapon spread, pretty accurate - accuracy systems would need to modify this
	virtual float GetWeaponSpread() { return GetSDKWpnData().m_flSpread; }
	float m_flPrevAnimTime;

	//Tony; by default, all weapons are automatic.
	//If you add code to switch firemodes, this function would need to be overridden to return the correct current mode.
	virtual int GetFireMode() const { return FM_AUTOMATIC; }
 
	virtual float GetFireRate( void ) { return GetSDKWpnData().m_flCycleTime; };
 
	//Tony; by default, burst fire weapons use a max of 3 shots (3 - 1)
	//weapons with more, ie: a 5 round burst, can override and determine which firemode it's in.
	virtual int MaxBurstShots() const { return 2; }
 
	virtual float GetWeaponFOV() { return GetSDKWpnData().m_flWeaponFOV; }

	virtual float GetAmmoToRemove( void ) { return GetSDKWpnData().m_iAmmoToRemove; };

#ifdef GAME_DLL
	virtual void SetDieThink( bool bDie );
	virtual void Die( void );
	virtual void SetWeaponModelIndex( const char *pName ) { m_iWorldModelIndex = modelinfo->GetModelIndex( pName ); }
	virtual	void Materialize( void );
	virtual	int	ObjectCaps( void );
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

	float  m_flNextResetCheckTime;

	Vector	GetOriginalSpawnOrigin( void ) { return m_vOriginalSpawnOrigin;	}
	QAngle	GetOriginalSpawnAngles( void ) { return m_vOriginalSpawnAngles;	}

private:
 
	CNetworkVar(float, m_flDecreaseShotsFired);
	CNetworkVar(bool, m_bIsScoped);

	Vector m_vOriginalSpawnOrigin;
	QAngle m_vOriginalSpawnAngles;
 
	CWeaponSDKBase( const CWeaponSDKBase & );
};
 
 
#endif // WEAPON_SDKBASE_H