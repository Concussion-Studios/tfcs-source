//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//
#include "cbase.h"
#include "weapon_sdkbase_sniper.h"

#if defined( CLIENT_DLL )
	#define CWeaponSniperRifle C_WeaponSniperRifle
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif

class CWeaponSniperRifle : public CWeaponSDKBaseSniper
{
public:
	DECLARE_CLASS( CWeaponSniperRifle, CWeaponSDKBaseSniper );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponSniperRifle();

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_SNIPERRIFLE; }

private:

	CWeaponSniperRifle( const CWeaponSniperRifle & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSniperRifle, DT_WeaponSniperRifle )

BEGIN_NETWORK_TABLE( CWeaponSniperRifle, DT_WeaponSniperRifle )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSniperRifle )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_sniperrifle, CWeaponSniperRifle );
PRECACHE_WEAPON_REGISTER( weapon_sniperrifle );

CWeaponSniperRifle::CWeaponSniperRifle()
{
}