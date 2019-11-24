//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Base VoteController.  Handles holding and voting on issues.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "shareddefs.h"
#include "eiface.h"
#include "team.h"
#include "gameinterface.h"
#include "fmtstr.h"
#include "tfc_voteissues.h"
#include "sdk_gamerules.h"
#include "sdk_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sv_vote_issue_restart_game_allowed( "sv_vote_issue_restart_game_allowed", "1", FCVAR_NONE, "Can players call votes to restart the game?" );
ConVar sv_vote_issue_restart_game_cooldown( "sv_vote_issue_restart_game_cooldown", "300", FCVAR_NONE, "Minimum time before another restart vote can occur (in seconds)." );
ConVar sv_vote_issue_4teams_game_allowed( "sv_vote_issue_4teams_game_allowed", "1", FCVAR_NONE, "Can players call votes to change to 4 teams mode?" );
ConVar sv_vote_issue_4teams_game_cooldown( "sv_vote_issue_4teams_game_cooldown", "300", FCVAR_NONE, "Minimum time before another 4 teams change vote can occur (in seconds)." );
ConVar sv_vote_kick_ban_duration( "sv_vote_kick_ban_duration", "20", FCVAR_NONE, "The number of minutes a vote ban should last. (0 = Disabled)" );
ConVar sv_vote_issue_changelevel_allowed( "sv_vote_issue_changelevel_allowed", "1", FCVAR_NONE, "Can players call votes to change levels?" );
ConVar sv_vote_issue_nextlevel_allowed( "sv_vote_issue_nextlevel_allowed", "1", FCVAR_NONE, "Can players call votes to set the next level?" );

//-----------------------------------------------------------------------------
// Purpose: Ban player
//-----------------------------------------------------------------------------
const char *CKickIssue::GetDetailsString( void )
{
	const char* pszDetails = m_szDetailsString;

	CBasePlayer *pPlayer = UTIL_PlayerByUserId( atoi( pszDetails ) );

	if ( !pPlayer )
		return m_szDetailsString;

	// bots need their own treatment
	if ( ( pPlayer->GetFlags() & FL_FAKECLIENT ) )
		pszDetails = UTIL_VarArgs( "%s (BOT)", pPlayer->GetPlayerName() );
	else
		pszDetails = UTIL_VarArgs( "%s", pPlayer->GetPlayerName() );

	return pszDetails;
}

bool CKickIssue::CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	// string -> int
	int m_iPlayerID = atoi( pszDetails );

	CBasePlayer *pPlayer = UTIL_PlayerByUserId( m_iPlayerID );

	if ( !pPlayer )
	{
		nFailCode = VOTE_FAILED_PLAYERNOTFOUND;
		nTime = m_flNextCallTime - gpGlobals->curtime;
		return false;
	}

	if ( engine->IsDedicatedServer() )
	{
		if ( pPlayer->IsAutoKickDisabled() == true && !( pPlayer->GetFlags() & FL_FAKECLIENT ) )
		{
			nFailCode = VOTE_FAILED_CANNOT_KICK_ADMIN;
			nTime = m_flNextCallTime - gpGlobals->curtime;
			return false;
		}
	}
	else if ( gpGlobals->maxClients > 1 )
	{
		CBasePlayer *pHostPlayer = UTIL_GetListenServerHost();

		if ( pPlayer == pHostPlayer )
		{
			nFailCode = VOTE_FAILED_CANNOT_KICK_ADMIN;
			nTime = m_flNextCallTime - gpGlobals->curtime;
			return false;
		}
	}

	return CTFCIssue::CanCallVote( nEntIndex, pszDetails, nFailCode, nTime );
}

void CKickIssue::ExecuteCommand( void )
{
	const char* pszDetails = m_szDetailsString;

	CBasePlayer *pPlayer = UTIL_PlayerByUserId( atoi( pszDetails ) );

	if ( !pPlayer )
		return;

	if ( ( pPlayer->GetFlags() & FL_FAKECLIENT ) )
	{
		engine->ServerCommand( UTIL_VarArgs( "kickid %d;", pPlayer->GetUserID() ) );
	}
	else
	{
		engine->ServerCommand( UTIL_VarArgs( "banid %i %s kick\n", sv_vote_kick_ban_duration.GetInt(), m_szNetworkIDString.String() ) );

		engine->ServerCommand( "writeip\n" );
		engine->ServerCommand( "writeid\n" );
	}
}

void CKickIssue::SetIssueDetails( const char *pszDetails )
{
	CTFCIssue::SetIssueDetails( pszDetails );

	int iID = atoi( pszDetails );

	CBasePlayer* pPlayer = UTIL_PlayerByUserId( iID );

	if ( pPlayer )
		m_szNetworkIDString = pPlayer->GetNetworkIDString();
}

bool CKickIssue::IsEnabled()
{
	if ( sv_vote_kick_ban_duration.GetInt() <= 0 )
		return false;
	else
		return true;
}

//-----------------------------------------------------------------------------
// Purpose: Restart the game
//-----------------------------------------------------------------------------
void CRestartGameIssue::Init()
{
	SetIssueCooldownDuration( sv_vote_issue_restart_game_cooldown.GetFloat() );
}

void CRestartGameIssue::ExecuteCommand()
{
	CTFCIssue::ExecuteCommand();

	engine->ServerCommand( "mp_restartgame 3\n" );
}

bool CRestartGameIssue::IsEnabled()
{
	if ( sv_vote_issue_restart_game_allowed.GetBool() )
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------
// Purpose: Change Level
//-----------------------------------------------------------------------------
void CChangeLevelIssue::ExecuteCommand()
{
	CTFCIssue::ExecuteCommand();

	engine->ServerCommand( UTIL_VarArgs( "changelevel %s\n", m_szDetailsString ) );
}

bool CChangeLevelIssue::IsEnabled()
{
	if ( sv_vote_issue_changelevel_allowed.GetBool() )
		return true;
	else
		return false;
}

bool CChangeLevelIssue::CanCallVote( int iEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if ( pszDetails[ 0 ] == '\0' )
	{
		nFailCode = VOTE_FAILED_MAP_NAME_REQUIRED;
		nTime = m_flNextCallTime - gpGlobals->curtime;
		return false;
	}


	if ( MultiplayRules()->IsMapInMapCycle( pszDetails ) == false )
	{
		nFailCode = VOTE_FAILED_MAP_NOT_VALID;
		nTime = m_flNextCallTime - gpGlobals->curtime;
		return false;
	}

	return CTFCIssue::CanCallVote( iEntIndex, pszDetails, nFailCode, nTime );
}

//-----------------------------------------------------------------------------
// Purpose: NextLevel
//-----------------------------------------------------------------------------
const char *CNextLevelIssue::GetDisplayString()
{
	// dedicated server vote
	if ( m_szDetailsString[ 0 ] == '\0' )
		return "#TF_vote_nextlevel_choices";
	else
		return "#TF_vote_nextlevel";
}

bool CNextLevelIssue::IsYesNoVote( void )
{
	if ( m_szDetailsString[0] != '\0' )
		return true;
	else
		return false;
}

bool CNextLevelIssue::CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime )
{
	if ( pszDetails[ 0 ] == '\0' )
	{
		nFailCode = VOTE_FAILED_MAP_NAME_REQUIRED;
		nTime = m_flNextCallTime - gpGlobals->curtime;
		return false;
	}

	if ( MultiplayRules()->IsMapInMapCycle( pszDetails ) == false )
	{
		nFailCode = VOTE_FAILED_MAP_NOT_VALID;
		nTime = m_flNextCallTime - gpGlobals->curtime;
		return false;
	}

	return CTFCIssue::CanCallVote( nEntIndex, pszDetails, nFailCode, nTime );
}

bool CNextLevelIssue::GetVoteOptions( CUtlVector <const char*> &vecNames )
{	
	if ( m_szDetailsString[0] != '\0' )
	{
		// The default vote issue is a Yes/No vote
		vecNames.AddToHead( "Yes" );
		vecNames.AddToTail( "No" );

		return true;
	}
	else
	{	
		CUtlVector< char* > m_MapList;

		m_MapList.AddVectorToTail( SDKGameRules()->GetMapList() );

		while ( vecNames.Count() < 6 )
		{
			if ( !m_MapList.Count() )
				break;

			int i;

			i = RandomInt( 0, m_MapList.Count() - 1 );

			vecNames.AddToTail( m_MapList[ i ] );
			m_MapList.Remove( i );
		}

		return true;
	}
}

void CNextLevelIssue::ExecuteCommand()
{
	CTFCIssue::ExecuteCommand();

	// extern ConVar nextlevel;
	ConVarRef nextlevel( "nextlevel" );

	nextlevel.SetValue( m_szDetailsString );
}

bool CNextLevelIssue::IsEnabled()
{
	if ( sv_vote_issue_nextlevel_allowed.GetBool() )
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------
// Purpose: 4Teams Mode
//-----------------------------------------------------------------------------
void C4TeamsMode::Init()
{
	SetIssueCooldownDuration( sv_vote_issue_4teams_game_cooldown.GetFloat() );
}

void C4TeamsMode::ExecuteCommand()
{
	CTFCIssue::ExecuteCommand();

	ConVarRef mp_4team( "mp_4team" );
	mp_4team.SetValue( !mp_4team.GetBool() );
}

bool C4TeamsMode::IsEnabled()
{
	if ( sv_vote_issue_4teams_game_allowed.GetBool() )
		return true;
	else
		return false;
}