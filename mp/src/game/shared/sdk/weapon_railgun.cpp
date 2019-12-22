//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"
#include "beam_shared.h"
#include "ammodef.h"

#if defined( CLIENT_DLL )
	#define CWeaponRailGun C_WeaponRailGun
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
	#include "baseanimating.h"
#endif

class CWeaponRailGun : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponRailGun, CWeaponSDKBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponRailGun();

	virtual SDKWeaponID GetWeaponID( void ) const { return WEAPON_RAILGUN; }
	virtual int GetFireMode() const { return FM_SEMIAUTOMATIC; }
	virtual void DrawBeam( const Vector &startPos, const Vector &endPos, float width );
	virtual void DoImpactEffect( trace_t &tr, int nDamageType );
	virtual void PrimaryAttack();
#ifndef CLIENT_DLL
	virtual void FireBeam( trace_t &tr, CBaseCombatCharacter *pOperator );
#endif

private:

	CWeaponRailGun( const CWeaponRailGun & );
	
	int m_nBulletType;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRailGun, DT_WeaponRailGun )

BEGIN_NETWORK_TABLE( CWeaponRailGun, DT_WeaponRailGun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponRailGun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_railgun, CWeaponRailGun );
PRECACHE_WEAPON_REGISTER( weapon_railgun );

CWeaponRailGun::CWeaponRailGun()
{
	m_nBulletType = -1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponRailGun::PrimaryAttack( void )
{
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 ) 
	{
		Reload();
		return;
	}
 
	CSDKPlayer *pPlayer = GetPlayerOwner();
	if (!pPlayer)
		return;
 
	//Tony; check firemodes -- 
	switch( GetFireMode() )
	{
	case FM_SEMIAUTOMATIC:
		if ( pPlayer->GetShotsFired() > 0 )
			return;
		break;
		//Tony; added an accessor to determine the max burst on a per-weapon basis.
	case FM_BURST:
		if ( pPlayer->GetShotsFired() > MaxBurstShots() )
			return;
		break;
	}

#ifdef GAME_DLL
	pPlayer->NoteWeaponFired();
#endif
 
	pPlayer->DoMuzzleFlash();
 
	SendWeaponAnim( GetPrimaryAttackActivity() );
 
	// Make sure we don't fire more than the amount in the clip
	if ( UsesClipsForAmmo1() )
		m_iClip1 --;
	else
		pPlayer->RemoveAmmo( GetAmmoToRemove(), m_iPrimaryAmmoType );
 
	pPlayer->IncreaseShotsFired();
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponRailGun::FireBeam( trace_t &tr, CBaseCombatCharacter *pOperator )
{
	CWeaponRailGun::PrimaryAttack();

	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOperator->Weapon_ShootPosition();
	DrawBeam( vecShootOrigin, tr.endpos, 15.5 );
 
	//Add our view kick in
	AddViewKick();
 
	//Tony; update our weapon idle time
	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
 
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
}
#endif // CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose:
// Input : &startPos - where the beam should begin
// &endPos - where the beam should end
// width - what the diameter of the beam should be (units?)
//-----------------------------------------------------------------------------
void CWeaponRailGun::DrawBeam( const Vector &startPos, const Vector &endPos, float width )
{
	//Tracer down the middle
	UTIL_Tracer( startPos, endPos, 0, TRACER_DONT_USE_ATTACHMENT, 6500, false, "GaussTracer" );
	
	CBeam *pBeam = CBeam::BeamCreate( "sprites/laser.vmt", 15.5 ); //Draw the main beam shaft
	pBeam->SetStartPos( startPos ); // It starts at startPos
	// This sets up some things that the beam uses to figure out where
	// it should start and end
	pBeam->PointEntInit( endPos, this );
	pBeam->SetEndAttachment( LookupAttachment("Muzzle") );	// This makes it so that the laser appears to come from the muzzle of the pistol
	pBeam->SetWidth( width );
	// pBeam->SetEndWidth( 0.05f );
	pBeam->SetBrightness( 255 );	// Higher brightness means less transparent
	pBeam->SetColor( 255, 185+random->RandomInt( -16, 16 ), 40 );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( 0.1f );	// The beam should only exist for a very short time
}

//-----------------------------------------------------------------------------
// Purpose:
// Input : &tr - used to figure out where to do the effect
// nDamageType - ???
//-----------------------------------------------------------------------------
void CWeaponRailGun::DoImpactEffect( trace_t &tr, int nDamageType )
{
	//Draw our beam
	DrawBeam( tr.startpos, tr.endpos, 15.5 );
	if ( ( tr.surface.flags & SURF_SKY ) == false )
	{
		CPVSFilter filter( tr.endpos );
		te->GaussExplosion( filter, 0.0f, tr.endpos, tr.plane.normal, 0 );
		m_nBulletType = GetAmmoDef()->Index( "GaussEnergy" );
		UTIL_ImpactTrace( &tr, m_nBulletType );
	}
}