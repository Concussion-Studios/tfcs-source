//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )

	#define CWeaponPistol C_WeaponPistol
	#include "c_sdk_player.h"

#else

	#include "sdk_player.h"

#endif


class CWeaponPistol : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponPistol, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponPistol();

	virtual SDKWeaponID GetWeaponID( void ) const		{ return WEAPON_PISTOL; }
	virtual bool CanWeaponBeDropped() const				{ return false; }
	virtual int GetFireMode() const { return FM_SEMIAUTOMATIC; }

private:

	CWeaponPistol( const CWeaponPistol & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPistol, DT_WeaponPistol )

BEGIN_NETWORK_TABLE( CWeaponPistol, DT_WeaponPistol )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponPistol )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_pistol, CWeaponPistol );
PRECACHE_WEAPON_REGISTER( weapon_pistol );



CWeaponPistol::CWeaponPistol()
{
}