//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef SDK_SHAREDDEFS_H
#define SDK_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#define SDK_GAME_DESCRIPTION	"Team Fortress Classic: Source"
#define SDK_MAX_PLAYERS	128

enum sdkteams_e
{
	SDK_TEAM_BLUE = FIRST_GAME_TEAM,
	SDK_TEAM_RED,
	SDK_TEAM_GREEN,
	SDK_TEAM_YELLOW,
};

extern const char *pszTeamNames[];

#define CONTENTS_REDTEAM	CONTENTS_TEAM1
#define CONTENTS_BLUETEAM	CONTENTS_TEAM2
#define CONTENTS_GREENTEAM	CONTENTS_UNUSED
#define CONTENTS_YELLOWTEAM	CONTENTS_UNUSED6

#define SDK_NUM_PLAYERCLASSES 10
#define SDK_PLAYERCLASS_IMAGE_LENGTH 64

#define PLAYERCLASS_RANDOM		-2
#define PLAYERCLASS_UNDEFINED	-1

#define PLAYERCLASS_SCOUT	0
#define PLAYERCLASS_SOLDIER	1
#define PLAYERCLASS_PYRO	2
#define PLAYERCLASS_HEAVY	3
#define PLAYERCLASS_DEMOMAN	4
#define PLAYERCLASS_ENGINEER	5
#define PLAYERCLASS_MEDIC	6
#define PLAYERCLASS_SNIPER	7
#define PLAYERCLASS_SPY		8
#define PLAYERCLASS_CIVILIAN	9

#define PANEL_CLASS_BLUE		"class_blue"
#define PANEL_CLASS_RED			"class_red"
#define PANEL_CLASS_GREEN		"class_green"
#define PANEL_CLASS_YELLOW		"class_yellow"

extern const char *pszPlayerClasses[];

//Tony; We need to precache all possible player models that we're going to use
extern const char *pszPossiblePlayerModels[];

//Anthony; We need all the Gib models we will use for gibing a player, let's precache that!
extern const char *pszPossibleGibModels[];

//Tony; these defines handle the default speeds for all of these - all are listed regardless of which option is enabled.
#define SDK_DEFAULT_PLAYER_RUNSPEED			220

#define SDK_PLAYER_INDEX_NONE			( MAX_PLAYERS + 1 )

// m_hViewModel
#define VMHANDS_FALLBACKMODEL   "models/weapons/c_arms_hev.mdl"
#define VMINDEX_WEP         0
#define VMINDEX_HANDS       1

// For the game rules to determine which type of game we're playing
enum TFCGameModes
{
	GAMEMODE_UNDEFINED = 0,
	GAMEMODE_CTF, 		// Capture the Flag
	GAMEMODE_CP, 		// Control Point
	GAMEMODE_TC, 		// Territory Control
	GAMEMODE_AD, 		// Atack & Defend
	GAMEMODE_ESC, 		// Escort/VIP
	GAMEMODE_TDM, 		// Team Deathmatch
	GAMEMODE_DOM, 		// Domination
	GAMEMODE_LAST
};

extern const char *pszGameModeNames[];	// localized gamemode names

// Ammo types
typedef enum
{
	AMMO_NONE = 0,
	AMMO_SHELLS,
	AMMO_CELLS,
	AMMO_ROCKETS,
	AMMO_NAILS,
	AMMO_GRENADES1,
	AMMO_GRENADES2,
	AMMO_DETPACK,

	AMMO_LAST
};

extern const char *s_AmmoNames[];

// Weapon IDs for all SDK Game weapons
typedef enum
{
	WEAPON_NONE = 0,

	SDK_WEAPON_NONE = WEAPON_NONE,

	WEAPON_TRANQ,
	WEAPON_SHOTGUN,
	WEAPON_12GAUGE,
	WEAPON_NAILGUN,
	WEAPON_GRENADE,
	WEAPON_GRENADE_CONCUSSION,
	WEAPON_GRENADE_EMP,
	WEAPON_GRENADE_NAPALM,
	WEAPON_GRENADE_MIRV,
	WEAPON_GRENADE_CALTROP,
	WEAPON_GRENADE_NAIL,
	WEAPON_GRENADE_HALLUCINATION,
	WEAPON_CROWBAR,
	WEAPON_UMBRELLA,
	WEAPON_SUPERNAILGUN,
	WEAPON_KNIFE,
	WEAPON_WRENCH,
	WEAPON_AC,
	WEAPON_MEDKIT,
	WEAPON_RAILGUN,
	WEAPON_RPG,
	WEAPON_IC,
	WEAPON_SNIPERRIFLE,
	WEAPON_AUTORIFLE,
	WEAPON_GRENADELAUNCHER,
	WEAPON_PIPEBOMBLAUNCHER,
	
	WEAPON_MAX,		// number of weapons weapon index
} SDKWeaponID;

extern int g_aWeaponDamageTypes[];

typedef enum
{
	FM_AUTOMATIC = 0,
	FM_SEMIAUTOMATIC,
	FM_BURST,

} SDK_Weapon_Firemodes;

const char *WeaponIDToAlias( int id );
int AliasToWeaponID( const char *alias );

// The various states the player can be in during the join game process.
enum SDKPlayerState
{
	STATE_ACTIVE=0,
	STATE_WELCOME,				// Show the level intro screen.
	STATE_PICKINGTEAM,			// Choosing team.
	STATE_PICKINGCLASS,			// Choosing class.
	STATE_DEATH_ANIM,			// Playing death anim, waiting for that to finish.
	STATE_OBSERVER_MODE,		// Noclipping around, watching players, etc.
	NUM_PLAYER_STATES
};

#define SDK_PLAYER_DEATH_TIME	5.0f	//Minimum Time before respawning

// Special Damage types
enum
{
	SDK_DMG_CUSTOM_NONE = 0,
	SDK_DMG_CUSTOM_SUICIDE,
};

// Assit Time
#define ASSIT_KILL_TIME 3.0f

//Max amount of Damagers
#define DAMAGERS_HISTORY_MAX 2

// Player avoidance
#define PUSHAWAY_THINK_INTERVAL		(1.0f / 20.0f)

#endif // SDK_SHAREDDEFS_H
