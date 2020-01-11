#ifndef PROP_PASSBALL_BALL_H
#define PROP_PASSBALL_BALL_H
#ifdef _WIN32
#pragma once
#endif

#include "props.h"
#include "entityoutput.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CPropPassBall: public CPhysicsProp
{
public:
	DECLARE_CLASS( CPropPassBall, CPhysicsProp );
	DECLARE_DATADESC();
 
	CPropPassBall();
	~CPropPassBall();

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual bool CreateVPhysics();
	virtual void StartTouch( CBaseEntity* pOther );
	virtual void Think();
	virtual int ObjectCaps();
	void OnReachGoal();

	//
	// Input handlers.
	//
	void InputSetTeam( inputdata_t &inputdata );

private:

	float m_fRadius;
	float m_flTouchTime; // Time since i touch this ent
	bool m_bWasTouched; // I touch this ent?

	COutputEvent m_OnTeamChange;
};

#endif // PROP_PASSBALL_BALL_H