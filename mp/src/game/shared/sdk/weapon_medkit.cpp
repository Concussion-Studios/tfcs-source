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
	#include "gamevars_shared.h"
#endif

extern ConVar mp_ignorefriendlyjustheal;
extern ConVar friendlyfire;

class CWeaponMedkit : public CWeaponSDKBase
{
public:
	DECLARE_CLASS( CWeaponMedkit, CWeaponSDKBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponMedkit();

	virtual void PrimaryAttack();
	virtual void SecondaryAttack() {}

	virtual SDKWeaponID GetWeaponID(void) const	{ return WEAPON_MEDKIT; }

private:
	CWeaponMedkit(const CWeaponMedkit &);

	bool CanHealPlayer( void );
	bool HealPlayer( void );
	bool InfectPlayer(void);

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
	if (CanHealPlayer())
	{
		HealPlayer();
	}
	else
	{
		InfectPlayer();
	}
		
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
					if (pBCC->InSameTeam(pOwner) || mp_ignorefriendlyjustheal.GetBool())
					{
						// If player does not have 100 health
						if (pBCC->GetHealth() < pBCC->GetMaxHealth())
							return true;
					}
					else
					{
						return false;
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

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
bool CWeaponMedkit::InfectPlayer(void)
{
	// Sets it to fired
	m_bFired = 1;

	CSDKPlayer *pOwner = ToSDKPlayer(GetOwner());
	if (!pOwner)
		return false;

	Vector vecSrc, vecAiming;

	// Take the eye position and direction
	vecSrc = pOwner->EyePosition();
	QAngle angles = pOwner->GetLocalAngles();
	AngleVectors(angles, &vecAiming);
	trace_t tr;
	Vector   vecEnd = vecSrc + (vecAiming * 42);
	UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0)
	{
		// Don't attach to a living creature
		if (tr.m_pEnt)
		{
			CBaseEntity *pEntity = tr.m_pEnt;
			if (pEntity->IsPlayer() && !pEntity->InSameTeam(pOwner) || pEntity->IsPlayer() && friendlyfire.GetBool())
			{
#ifndef CLIENT_DLL
				CBasePlayer *pPlayer = ToBasePlayer(pEntity);
				CPASAttenuationFilter filter(pPlayer, "HealthVial.Touch"); // Filters
				EmitSound(filter, pPlayer->entindex(), "HealthVial.Touch"); // Play HealthVial.Touch sound
				CTakeDamageInfo info(pPlayer, pPlayer, this, 5, DMG_POISON, 8);
				pEntity->TakeDamage(info); // Damage 20 health under Poison
#endif
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