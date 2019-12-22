//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//
#include "cbase.h"
#include "weapon_sdkbase_sniper.h"

#if defined( CLIENT_DLL )
	#define CWeaponAutoRifle C_WeaponAutoRifle
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif

class CWeaponAutoRifle : public CWeaponSDKBaseSniper
{
public:
	DECLARE_CLASS( CWeaponAutoRifle, CWeaponSDKBaseSniper );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponAutoRifle();

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_AUTORIFLE; }

private:

	CWeaponAutoRifle( const CWeaponAutoRifle & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAutoRifle, DT_WeaponAutoRifle )

BEGIN_NETWORK_TABLE( CWeaponAutoRifle, DT_WeaponAutoRifle )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponAutoRifle )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_autorifle, CWeaponAutoRifle );
PRECACHE_WEAPON_REGISTER( weapon_autorifle );

CWeaponAutoRifle::CWeaponAutoRifle()
{
}