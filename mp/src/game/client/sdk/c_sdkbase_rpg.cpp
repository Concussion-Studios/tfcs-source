//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "c_baseanimating.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_SDKBaseRocket : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_SDKBaseRocket, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

			C_SDKBaseRocket();
	virtual ~C_SDKBaseRocket();

	virtual void Spawn();
	virtual int DrawModel( int flags );
	virtual void PostDataUpdate( DataUpdateType_t type );

private:
	CNetworkVector( m_vInitialVelocity );
	float m_flSpawnTime;
};


IMPLEMENT_CLIENTCLASS_DT( C_SDKBaseRocket, DT_SDKBaseRocket, CSDKBaseRocket )
	RecvPropVector( RECVINFO( m_vInitialVelocity ) )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_SDKBaseRocket::C_SDKBaseRocket()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_SDKBaseRocket::~C_SDKBaseRocket()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_SDKBaseRocket::Spawn()
{
	m_flSpawnTime = gpGlobals->curtime;
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_SDKBaseRocket::PostDataUpdate( DataUpdateType_t type )
{
	BaseClass::PostDataUpdate( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		// Now stick our initial velocity into the interpolation history 
		CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();
		
		interpolator.ClearHistory();
		float changeTime = GetLastChangeTime( LATCH_SIMULATION_VAR );

		// Add a sample 1 second back.
		Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
		interpolator.AddToHead( changeTime - 1.0, &vCurOrigin, false );

		// Add the current sample.
		vCurOrigin = GetLocalOrigin();
		interpolator.AddToHead( changeTime, &vCurOrigin, false );

		// do the same for angles
		CInterpolatedVar< QAngle > &rotInterpolator = GetRotationInterpolator();

		rotInterpolator.ClearHistory();

		// Add a rotation sample 1 second back
		QAngle vCurAngles = GetLocalAngles();
		rotInterpolator.AddToHead( changeTime - 1.0, &vCurAngles, false );

		// Add the current rotation
		rotInterpolator.AddToHead( changeTime - 1.0, &vCurAngles, false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_SDKBaseRocket::DrawModel( int flags )
{
	// During the first half-second of our life, don't draw ourselves
	if ( gpGlobals->curtime - m_flSpawnTime < 0.2 )
		return 0;

	return BaseClass::DrawModel( flags );
}


