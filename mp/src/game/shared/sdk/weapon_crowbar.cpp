//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "sdk_weapon_melee.h"

#if defined( CLIENT_DLL )

	#define CWeaponCrowbar C_WeaponCrowbar
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif


class CWeaponCrowbar : public CWeaponSDKMelee
{
public:
	DECLARE_CLASS( CWeaponCrowbar, CWeaponSDKMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponCrowbar();

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_CROWBAR; }

private:

	CWeaponCrowbar( const CWeaponCrowbar & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCrowbar, DT_WeaponCrowbar )

BEGIN_NETWORK_TABLE( CWeaponCrowbar, DT_WeaponCrowbar )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponCrowbar )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_crowbar, CWeaponCrowbar );
PRECACHE_WEAPON_REGISTER( weapon_crowbar );

CWeaponCrowbar::CWeaponCrowbar()
{
}

