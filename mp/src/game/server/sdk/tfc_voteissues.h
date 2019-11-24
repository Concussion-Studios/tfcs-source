//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Player-driven Voting System for Multiplayer Source games.
//
//=============================================================================//
#ifndef TFC_VOTEISSUES_H
#define TFC_VOTEISSUES_H
#ifdef _WIN32
#pragma once
#endif

#include "vote_controller.h"

class CTFCIssue : public CBaseIssue
{
public:
	CTFCIssue( const char* pszName ) : CBaseIssue( pszName ) { }

	virtual void		ExecuteCommand( void ) {}
	virtual void		ListIssueDetails( CBasePlayer *pForWhom ) {}
};

class CKickIssue : public CTFCIssue
{
public:
	virtual bool IsEnabled( void );
	virtual bool CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual bool IsTeamRestrictedVote() { return true; }
	virtual const char	*GetDetailsString( void );
	virtual const char *GetDisplayString( void ) { return "#TF_vote_kick_player"; }
	virtual void ExecuteCommand( void );
	virtual const char *GetVotePassedString( void ) { return "#TF_vote_passed_kick_player"; }
	virtual void SetIssueDetails( const char *pszDetails );
	CKickIssue() : CTFCIssue( "Kick" ) { }
	CUtlString m_szNetworkIDString;
};

class CRestartGameIssue : public CTFCIssue
{
public:
	void Init( void );
	virtual bool IsEnabled( void );
	virtual const char *GetDisplayString( void ) { return "#TF_vote_restart_game"; }
	virtual void ExecuteCommand( void );
	virtual const char *GetVotePassedString( void ) { return "#TF_vote_passed_restart_game"; }

	CRestartGameIssue() : CTFCIssue( "RestartGame" ) { }
};

class CChangeLevelIssue : public CTFCIssue
{
public:
	virtual bool IsEnabled( void );
	virtual const char *GetDisplayString( void ) { return "#TF_vote_changelevel"; }
	virtual void ExecuteCommand( void );
	virtual const char *GetVotePassedString( void )	{ return "#TF_vote_passed_changelevel"; }
	virtual bool CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );

	CChangeLevelIssue() : CTFCIssue( "ChangeLevel" ) { }
};

class CNextLevelIssue : public CTFCIssue
{
public:
	virtual bool IsEnabled( void );
	virtual bool CanCallVote( int nEntIndex, const char *pszDetails, vote_create_failed_t &nFailCode, int &nTime );
	virtual const char *GetDisplayString( void );
	virtual void ExecuteCommand( void );
	virtual const char *GetVotePassedString( void )	{ return "#TF_vote_passed_nextlevel"; }
	virtual bool IsYesNoVote( void );
	virtual bool GetVoteOptions( CUtlVector <const char*> &vecNames );

	CNextLevelIssue() : CTFCIssue( "NextLevel" ) { }
};

class C4TeamsMode : public CTFCIssue
{
public:
	void Init( void );
	virtual bool IsEnabled( void );
	virtual const char *GetDisplayString( void ) { return "#TF_vote_4teams_game"; }
	virtual void ExecuteCommand( void );
	virtual const char *GetVotePassedString( void ) { return "#TF_vote_passed_4teams_game"; }

	C4TeamsMode() : CTFCIssue( "4Teams" ) { }
};

#endif // TFC_VOTEISSUES_H
