//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules object
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#ifndef SDK_GAMERULES_H
#define SDK_GAMERULES_H
#ifdef _WIN32
#pragma once
#endif

#include "teamplay_gamerules.h"
#include "convar.h"
#include "gamevars_shared.h"
#include "weapon_sdkbase.h"

#ifdef CLIENT_DLL
	#include "c_baseplayer.h"
#else
	#include "player.h"
	#include "sdk_player.h"
	#include "utlqueue.h"
	#include "playerclass_info_parse.h"
#endif


#ifdef CLIENT_DLL
	#define CSDKGameRules C_SDKGameRules
	#define CSDKGameRulesProxy C_SDKGameRulesProxy
#endif

class CSDKGameRulesProxy : public CGameRulesProxy
{
public:
	DECLARE_CLASS( CSDKGameRulesProxy, CGameRulesProxy );
	DECLARE_NETWORKCLASS();
};

class CSDKViewVectors : public CViewVectors
{
public:
	CSDKViewVectors( 
		Vector vView,
		Vector vHullMin,
		Vector vHullMax,
		Vector vDuckHullMin,
		Vector vDuckHullMax,
		Vector vDuckView,
		Vector vObsHullMin,
		Vector vObsHullMax,
		Vector vDeadViewHeight
#if defined ( SDK_USE_PRONE )
		,Vector vProneHullMin,
		Vector vProneHullMax,
		Vector vProneView
#endif
		) :
			CViewVectors( 
				vView,
				vHullMin,
				vHullMax,
				vDuckHullMin,
				vDuckHullMax,
				vDuckView,
				vObsHullMin,
				vObsHullMax,
				vDeadViewHeight )
	{
#if defined( SDK_USE_PRONE )
		m_vProneHullMin = vProneHullMin;
		m_vProneHullMax = vProneHullMax;
		m_vProneView = vProneView;
#endif 
	}
#if defined ( SDK_USE_PRONE )
	Vector m_vProneHullMin;
	Vector m_vProneHullMax;	
	Vector m_vProneView;
#endif
};

class CSDKGameRules : public CTeamplayRules
{
public:
	DECLARE_CLASS( CSDKGameRules, CTeamplayRules );

			CSDKGameRules();
	virtual ~CSDKGameRules();

	virtual bool	ShouldCollide( int collisionGroup0, int collisionGroup1 );

	virtual int	PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual bool	IsTeamplay( void ) { return true; }

	// Get the view vectors for this mod.
	virtual const CViewVectors* GetViewVectors() const;
	virtual const CSDKViewVectors *GetSDKViewVectors() const;

	//Tony; define a default encryption key.
	virtual const unsigned char *GetEncryptionKey( void ) { return (unsigned char *)"a1b2c3d4"; }

	//Tony; in shared space
	const char *GetPlayerClassName( int cls, int team );

	virtual bool IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer ) { return true; }

#ifdef CLIENT_DLL
	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.
#else
	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.

	virtual const char *GetGameDescription( void ) { return SDK_GAME_DESCRIPTION; } 
	virtual bool ClientCommand( CBaseEntity *pEdict, const CCommand &args );
	virtual void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore );
	virtual void Think();

	void InitTeams( void );

	void CreateStandardEntities( void );

	virtual void InitDefaultAIRelationships( void );
	virtual const char*	AIClassText(int classType);

	virtual const char *GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer );
	virtual const char *GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer );

	CBaseEntity *GetPlayerSpawnSpot( CBasePlayer *pPlayer );
	bool IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer );
	virtual void PlayerSpawn( CBasePlayer *pPlayer );

	bool IsPlayerClassOnTeam( int cls, int team );
	bool CanPlayerJoinClass( CSDKPlayer *pPlayer, int cls );
	void ChooseRandomClass( CSDKPlayer *pPlayer );
	bool ReachedClassLimit( int team, int cls );
	int CountPlayerClass( int team, int cls );
	int GetClassLimit( int team, int cls );
 
	bool TeamFull( int team_id );
	bool TeamStacked( int iNewTeam, int iCurTeam );
	int SelectDefaultTeam( void );

	virtual void ServerActivate();
	virtual void Activate();

protected:
	void CheckPlayerPositions( void );

private:
	void CheckLevelInitialized( void );
	bool m_bLevelInitialized;

	Vector2D	m_vecPlayerPositions[MAX_PLAYERS];

	int	m_iSpawnPointCount_Blue;		//number of blue spawns on the map
	int	m_iSpawnPointCount_Red;			//number of red spawns on the map
	int	m_iSpawnPointCount_Yellow;		//number of yellow spawns on the map
	int	m_iSpawnPointCount_Green;		//number of green spawns on the map
	int	m_iSpawnPointCount_deathmatch;	//number of deathmatch spawns on the map

	void RadiusDamage( const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, bool bIgnoreWorld );

public:
	virtual void DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	const char *GetKillingWeaponName( const CTakeDamageInfo &info, CSDKPlayer *pVictim, int *iWeaponID );
	CBasePlayer* GetAssister( CBasePlayer *pVictim, CBasePlayer *pScorer, CBaseEntity *pInflictor );
	CSDKPlayer* GetRecentDamager( CSDKPlayer *pVictim, int iDamager, float flMaxElapsed );

#endif
	// changes, restor our gamemode based on the map/logic ent.
	bool InGameMode( int nGamemode );
	void AddGameMode( int nGamemode );
	void RemoveGameMode( int nGamemode );
	void CheckGameMode( void );

public:
	float GetMapRemainingTime();	// time till end of map, -1 if timelimit is disabled
	float GetMapElapsedTime();		// How much time has elapsed since the map started.

private:
	CNetworkVar( float, m_flGameStartTime );
	CNetworkVar( int, m_nGamemode ); // Type of game mode this map is ( CTF, CP )
};

//-----------------------------------------------------------------------------
// Gets us at the team fortress classic game rules
//-----------------------------------------------------------------------------
inline CSDKGameRules* SDKGameRules()
{
	return static_cast<CSDKGameRules*>(g_pGameRules);
}

#endif // SDK_GAMERULES_H
