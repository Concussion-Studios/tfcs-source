//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

	#define CWeaponMP5 C_WeaponMP5
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif


class CWeaponMP5 : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponMP5, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponMP5();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return WEAPON_MP5; }
	virtual bool CanWeaponBeDropped() const				{ return false; }

private:

	CWeaponMP5( const CWeaponMP5 & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMP5, DT_WeaponMP5 )

BEGIN_NETWORK_TABLE( CWeaponMP5, DT_WeaponMP5 )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMP5 )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_mp5, CWeaponMP5 );
PRECACHE_WEAPON_REGISTER( weapon_mp5 );

CWeaponMP5::CWeaponMP5()
{
}
