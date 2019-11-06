//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "weapon_sdkbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	tfc_classlimit_scout( "tfc_classlimit_scout", "-1", FCVAR_REPLICATED, "Class limit for Scout" );
ConVar	tfc_classlimit_soldier( "tfc_classlimit_soldier", "-1", FCVAR_REPLICATED, "Class limit for Soldier" );
ConVar	tfc_classlimit_pyro( "tfc_classlimit_pyro", "-1", FCVAR_REPLICATED, "Class limit for Pyro" );
ConVar	tfc_classlimit_hwguy( "tfc_classlimit_hwguy", "-1", FCVAR_REPLICATED, "Class limit for Heavy Weapons Guy" );
ConVar	tfc_classlimit_demoman( "tfc_classlimit_demoman", "-1", FCVAR_REPLICATED, "Class limit for Demoman" );
ConVar	tfc_classlimit_engineer( "tfc_classlimit_engineer", "-1", FCVAR_REPLICATED, "Class limit for Engineer" );
ConVar	tfc_classlimit_medic( "tfc_classlimit_medic", "-1", FCVAR_REPLICATED, "Class limit for Medic" );
ConVar	tfc_classlimit_sniper( "tfc_classlimit_sniper", "-1", FCVAR_REPLICATED, "Class limit for Sniper" );
ConVar	tfc_classlimit_spy( "tfc_classlimit_spy", "-1", FCVAR_REPLICATED, "Class limit for Spy" );
ConVar	tfc_classlimit_civilian( "tfc_classlimit_civilian", "-1", FCVAR_REPLICATED, "Class limit for Civilian" );

const char *pszTeamBlueClasses[] = 
{
	"scout",
	"soldier",
	"pyro",
	"hwguy",
	"demoman",
	"engineer",
	"medic",
	"sniper",
	"spy",
	"civilian",
	NULL
};

const char *pszTeamRedClasses[] = 
{
	"scout",
	"soldier",
	"pyro",
	"hwguy",
	"demoman",
	"engineer",
	"medic",
	"sniper",
	"spy",
	"civilian",
	NULL
};

const char *pszTeamNames[] =
{
	"#SDK_Team_Unassigned",
	"#SDK_Team_Spectator",
	"#SDK_Team_Blue",
	"#SDK_Team_Red",
};

// Precache all possible player models that we're going to use
const char *pszPossiblePlayerModels[] =
{
	"models/player/scout.mdl",
	"models/player/soldier.mdl",
	"models/player/pyro.mdl",
	"models/player/hwguy.mdl",
	"models/player/demo.mdl",
	"models/player/engineer.mdl",
	"models/player/medic.mdl",
	"models/player/sniper.mdl",
	"models/player/spy.mdl",
	"models/player/civilian.mdl",
	NULL
};

// ----------------------------------------------------------------------------- //
// Global Weapon Definitions
// ----------------------------------------------------------------------------- //

//--------------------------------------------------------------------------------------------------------
static const char * s_WeaponAliasInfo[] = 
{
	"none",				// WEAPON_NONE

	"mp5",				// WEAPON_MP5
	"shotgun",			// WEAPON_SHOTGUN
	"12gauge",			// WEAPON_SHOTGUN12GAUGE
	"nailgun",			// WEAPON_NAILGUN
	"grenade",			// WEAPON_GRENADE
	"pistol",			// WEAPON_PISTOL
	"crowbar",			// WEAPON_CROWBAR
	"umbrella",			// WEAPON_UMBRELLA
	"supernailgun",		// WEAPON_SUPERNAILGUN
	"knife",			// WEAPON_KNIFE
	"wrench",			// WEAPON_WRENCH
	"ac",				// WEAPON_AC
	"medkit", 			// WEAPON_MEDKIT

	NULL,				// WEAPON_NONE
};

//--------------------------------------------------------------------------------------------------------
//
// Given an alias, return the associated weapon ID
//
int AliasToWeaponID( const char *alias )
{
	if (alias)
	{
		for( int i=0; s_WeaponAliasInfo[i] != NULL; ++i )
			if (!Q_stricmp( s_WeaponAliasInfo[i], alias ))
				return i;
	}

	return WEAPON_NONE;
}

//--------------------------------------------------------------------------------------------------------
//
// Given a weapon ID, return its alias
//
const char *WeaponIDToAlias( int id )
{
	if ( (id >= WEAPON_MAX) || (id < 0) )
		return NULL;

	return s_WeaponAliasInfo[id];
}


