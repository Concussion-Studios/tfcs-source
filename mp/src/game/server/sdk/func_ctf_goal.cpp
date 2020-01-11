#include "cbase.h"
#include "triggers.h"
#include "sdk_player.h"
#include "prop_passball_ball.h"
#include "sdk_team.h"

//-----------------------------------------------------------------------------
// Purpose: CTF/Pass The Ball Goal
//-----------------------------------------------------------------------------
class CFuncCTFGoal : public CBaseTrigger
{
public:
	DECLARE_CLASS( CFuncCTFGoal, CBaseTrigger );
	DECLARE_DATADESC();

	CFuncCTFGoal();

	virtual void Spawn();
	virtual void GoalTouch( CBaseEntity* pOther );
};


LINK_ENTITY_TO_CLASS( func_ctf_goal, CFuncCTFGoal );

BEGIN_DATADESC( CFuncCTFGoal )
	DEFINE_FUNCTION( GoalTouch ),
END_DATADESC()

CFuncCTFGoal::CFuncCTFGoal()
{
}

void CFuncCTFGoal::Spawn()
{
	InitTrigger();
	SetTouch( &CFuncCTFGoal::GoalTouch );
}
	
void CFuncCTFGoal::GoalTouch( CBaseEntity* pOther )
{
	auto *pSDK = ToSDKPlayer( pOther );
	if ( pSDK )
	{
		// compare player team with goal zone team number
		if ( pSDK->GetTeamNumber() == GetTeamNumber() )
		{
			pSDK->m_Shared.SetGoalState( true );
			ClientPrint( pSDK, HUD_PRINTCENTER, "BRING THE ENEMY FLAG TO THIS ZONE." );
		}
	}

	auto* pBall = dynamic_cast<CPropPassBall*>( pOther );
	if ( pBall )
	{
		if ( pBall->GetOwnerEntity() == pSDK )
			GetGlobalSDKTeam( pSDK->GetTeamNumber() )->AddScore( 1 ); // Reward the player's team for bringing up the ball

		//pBall->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
	}
}

