//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//
#include "cbase.h"
#include "weapon_sdkbase.h"

#if defined( CLIENT_DLL )
	#define CWeaponMedkit C_WeaponMedkit
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif

extern ConVar mp_ignorefriendlyjustheal;

class CWeaponMedkit : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponMedkit, CWeaponSDKBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponMedkit();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack() {}

	virtual SDKWeaponID GetWeaponID(void) const	{ return WEAPON_MEDKIT; }
	virtual bool CanWeaponBeDropped() const				{ return false; }
	virtual float GetRange(void) { return 64.0f; }
	virtual bool CanWeaponBeDropped() const	{ return false; }

private:
	CWeaponMedkit(const CWeaponMedkit &);

	bool CanHealPlayer( void );
	bool HealPlayer( void );

	int m_bFired;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponMedkit, DT_WeaponMedkit )

BEGIN_NETWORK_TABLE( CWeaponMedkit, DT_WeaponMedkit )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponMedkit )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_medkit, CWeaponMedkit );
PRECACHE_WEAPON_REGISTER( weapon_medkit );

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
CWeaponMedkit::CWeaponMedkit()
{
	m_bFired = 0;
}

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CWeaponMedkit::PrimaryAttack()
{
	// Checks if it can heal
	if ( CanHealPlayer() )
		HealPlayer();
}

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
bool CWeaponMedkit::CanHealPlayer( void )
{
	// If it hasn't been fired
	if ( !m_bFired )
	{ 
		CSDKPlayer *pOwner = ToSDKPlayer( GetOwner() ); // Get owner of this weapon
		if ( !pOwner )
			return false;

		Vector vecSrc, vecAiming;
		vecSrc = pOwner->EyePosition();      // Take the eye position and direction		
		QAngle angles = pOwner->GetLocalAngles();	// Get local angles
		AngleVectors( angles, &vecAiming );
		trace_t tr;		// Create a new trace to use
		Vector   vecEnd = vecSrc + (vecAiming * 42);
		UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
	  
		if ( tr.fraction < 1.0 )
		{
			// Don't attach to a living creature
			if ( tr.m_pEnt  )
			{
				CBaseEntity *pEntity = tr.m_pEnt;
				CBaseCombatCharacter *pBCC = ToBaseCombatCharacter( pEntity );
				if ( pBCC )
				{
					// If player does not have 100 health
					if (pBCC->GetHealth() < pBCC->GetMaxHealth())
						return true;
				}
			}
			return false;
		}
		else
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
}

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
bool CWeaponMedkit::HealPlayer( void )
{
	// Sets it to fired
	m_bFired = 1;

	CSDKPlayer *pOwner = ToSDKPlayer( GetOwner() );
	if ( !pOwner )
		return false;

	Vector vecSrc, vecAiming;

	// Take the eye position and direction
	vecSrc = pOwner->EyePosition();   
	QAngle angles = pOwner->GetLocalAngles();
	AngleVectors( angles, &vecAiming );
	trace_t tr;
	Vector   vecEnd = vecSrc + (vecAiming * 42);
	UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr );
   
	if ( tr.fraction < 1.0 )
	{
		// Don't attach to a living creature
		if ( tr.m_pEnt )
		{
			CBaseEntity *pEntity = tr.m_pEnt;
			if ( pEntity->IsPlayer() )
			{
				if ( pEntity->GetHealth() < pEntity->GetMaxHealth() )
				{
#ifndef CLIENT_DLL
					CBasePlayer *pPlayer = ToBasePlayer( pEntity );
					CPASAttenuationFilter filter( pPlayer, "HealthVial.Touch" ); // Filters
					EmitSound( filter, pPlayer->entindex(), "HealthVial.Touch" ); // Play HealthVial.Touch sound
					pEntity->TakeHealth( 20, DMG_GENERIC ); // Damage 20 health under generic reason
#endif
					return true;
				}
			}
		}

		return false;
	}
	else
	{
		return false;
	}
}

//Tony; todo; add ACT_MP_PRONE* activities, so we have them.
acttable_t CWeaponMedkit::m_acttable[] =
{
	{ ACT_MP_STAND_IDLE, ACT_DOD_STAND_AIM_SPADE, false },
	{ ACT_MP_CROUCH_IDLE, ACT_DOD_CROUCH_AIM_SPADE, false },
	{ ACT_MP_PRONE_IDLE, ACT_DOD_PRONE_AIM_SPADE, false },

	{ ACT_MP_RUN, ACT_DOD_RUN_AIM_SPADE, false },
	{ ACT_MP_WALK, ACT_DOD_WALK_AIM_SPADE, false },
	{ ACT_MP_CROUCHWALK, ACT_DOD_CROUCHWALK_AIM_SPADE, false },
	{ ACT_MP_PRONE_CRAWL, ACT_DOD_PRONEWALK_AIM_SPADE, false },
	{ ACT_SPRINT, ACT_DOD_SPRINT_AIM_SPADE, false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE, ACT_DOD_PRIMARYATTACK_SPADE, false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE, ACT_DOD_PRIMARYATTACK_SPADE, false },
	{ ACT_MP_ATTACK_PRONE_PRIMARYFIRE, ACT_DOD_PRIMARYATTACK_PRONE_SPADE, false },
};

IMPLEMENT_ACTTABLE(CWeaponMedkit);

