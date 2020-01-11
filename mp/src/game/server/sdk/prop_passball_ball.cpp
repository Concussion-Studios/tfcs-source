#include "cbase.h"
#include "prop_passball_ball.h"

LINK_ENTITY_TO_CLASS( prop_passball_ball, CPropPassBall );

BEGIN_DATADESC(  CPropPassBall )

	//KeyFiels
	DEFINE_KEYFIELD( m_fRadius, FIELD_FLOAT, "radius"),

	// Output
	DEFINE_INPUTFUNC( FIELD_VOID, "SetTeam", InputSetTeam ),
	DEFINE_OUTPUT( m_OnTeamChange, "OnTeamChange" ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPropPassBall::CPropPassBall()
{
	ChangeTeam( TEAM_UNASSIGNED );

	m_bWasTouched = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPropPassBall::~CPropPassBall()
{

}

//-----------------------------------------------------------------------------
// Purpose: Precache assets used by this entity
//-----------------------------------------------------------------------------
void CPropPassBall::Precache( void )
{
	PrecacheModel( STRING( GetModelName() ) );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CPropPassBall::Spawn( void )
{
	char *szModel = (char *)STRING( GetModelName() );
	if ( !szModel || !*szModel )
	{
		Warning( "%s at %.0f %.0f %0.f missing modelname\n", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
		UTIL_Remove( this );
		return;
	}

	Precache();

	SetModel( STRING( GetModelName() ) );

	// In order to pick it up, needs to be physics.
	CreateVPhysics();

	SetThink( &CPropPassBall::Think );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
bool CPropPassBall::CreateVPhysics()
{
	SetSolid( SOLID_BBOX );
	SetCollisionBounds( -Vector( m_fRadius ), Vector( m_fRadius ) );
	objectparams_t params = g_PhysDefaultObjectParams;
	params.pGameData = static_cast<void *>( this );

	IPhysicsObject *pPhysicsObject = physenv->CreateSphereObject( m_fRadius, GetModelPtr()->GetRenderHdr()->textureindex, GetAbsOrigin(), GetAbsAngles(), &params, false );
	if ( pPhysicsObject )
	{
		VPhysicsSetObject( pPhysicsObject );
		SetMoveType( MOVETYPE_VPHYSICS );
		pPhysicsObject->Wake();
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPropPassBall::ObjectCaps()
{ 
	return ( BaseClass::ObjectCaps() | FCAP_IMPULSE_USE );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CPropPassBall::Think()
{
	BaseClass::Think();

	if ( m_bWasTouched && gpGlobals->curtime - m_flTouchTime > 10 )
	{
		DevMsg( "[CPropPassBall] Reseting team to %s!\n", GetTeamNumber() );
		ChangeTeam( TEAM_UNASSIGNED );

		m_bWasTouched = false;

		m_OnTeamChange.FireOutput( this, this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: On StartTouch
//-----------------------------------------------------------------------------
void CPropPassBall::StartTouch( CBaseEntity* pOther )
{
	BaseClass::StartTouch( pOther );

	m_flTouchTime = gpGlobals->curtime;
	m_bWasTouched = true;

	if ( m_bWasTouched )
	{
		CBasePlayer *pPlayer = ToBasePlayer( pOther );
		if ( pPlayer && pPlayer->IsAlive() )
		{
			DevMsg( "[CPropPassBall] Changing team to %s!\n", pPlayer->GetTeam() );
			ChangeTeam( pPlayer->GetTeamNumber() );

			m_OnTeamChange.FireOutput( this, this );
		}	
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropPassBall::OnReachGoal()
{ 
	GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
	//ForceRespawn();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropPassBall::InputSetTeam( inputdata_t &inputData )
{
	BaseClass::InputSetTeam( inputData );

	//Fire output
	m_OnTeamChange.FireOutput( this, this );
}