//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "weapon_sdkbase_machinegun.h"
#include "in_buttons.h"

#if defined( CLIENT_DLL )
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( SDKMachineGun, DT_SDKMachineGun )

BEGIN_NETWORK_TABLE( CSDKMachineGun, DT_SDKMachineGun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CSDKMachineGun )
END_PREDICTION_DATA()

BEGIN_DATADESC( CSDKMachineGun )
	DEFINE_FIELD( m_nShotsFired,	FIELD_INTEGER ),
	DEFINE_FIELD( m_flNextSoundTime, FIELD_TIME ),
END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSDKMachineGun::CSDKMachineGun( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const Vector &CSDKMachineGun::GetBulletSpread( void )
{
	static Vector cone = VECTOR_CONE_3DEGREES;
	return cone;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
//-----------------------------------------------------------------------------
void CSDKMachineGun::FireBullets( const FireBulletsInfo_t &info )
{
	if(CBasePlayer *pPlayer = ToBasePlayer ( GetOwner() ) )
	{
		pPlayer->FireBullets(info);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKMachineGun::DoMachineGunKick( CBasePlayer *pPlayer, float dampEasy, float maxVerticleKickAngle, float fireDurationTime, float slideLimitTime )
{
	#define	KICK_MIN_X 0.2f	//Degrees
	#define	KICK_MIN_Y 0.2f	//Degrees
	#define	KICK_MIN_Z 0.1f	//Degrees

	QAngle vecScratch;
	int iSeed = CBaseEntity::GetPredictionRandomSeed() & 255;
	
	//Find how far into our accuracy degradation we are
	float duration	= ( fireDurationTime > slideLimitTime ) ? slideLimitTime : fireDurationTime;
	float kickPerc = duration / slideLimitTime;

	// do this to get a hard discontinuity, clear out anything under 10 degrees punch
	pPlayer->ViewPunchReset( 10 );

	//Apply this to the view angles as well
	vecScratch.x = -( KICK_MIN_X + ( maxVerticleKickAngle * kickPerc ) );
	vecScratch.y = -( KICK_MIN_Y + ( maxVerticleKickAngle * kickPerc ) ) / 3;
	vecScratch.z = KICK_MIN_Z + ( maxVerticleKickAngle * kickPerc ) / 8;

	RandomSeed( iSeed );

	//Wibble left and right
	if ( RandomInt( -1, 1 ) >= 0 )
		vecScratch.y *= -1;

	iSeed++;

	//Wobble up and down
	if ( RandomInt( -1, 1 ) >= 0 )
		vecScratch.z *= -1;

	//Clip this to our desired min/max
	//UTIL_ClipPunchAngleOffset( vecScratch, pPlayer->m_Local.m_vecPunchAngle, QAngle( 24.0f, 3.0f, 1.0f ) );

	//Add it to the view punch
	// NOTE: 0.5 is just tuned to match the old effect before the punch became simulated
	pPlayer->ViewPunch( vecScratch * 0.5 );
}

//-----------------------------------------------------------------------------
// Purpose: Reset our shots fired
//-----------------------------------------------------------------------------
bool CSDKMachineGun::Deploy( void )
{
	m_nShotsFired = 0;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: Make enough sound events to fill the estimated think interval
// returns: number of shots needed
//-----------------------------------------------------------------------------
int CSDKMachineGun::WeaponSoundRealtime( WeaponSound_t shoot_type )
{
	int numBullets = 0;

	// ran out of time, clamp to current
	if ( m_flNextSoundTime < gpGlobals->curtime )
	{
		m_flNextSoundTime = gpGlobals->curtime;
	}

	// make enough sound events to fill up the next estimated think interval
	float dt = clamp( m_flAnimTime - m_flPrevAnimTime, 0, 0.2 );
	if ( m_flNextSoundTime < gpGlobals->curtime + dt )
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}

	if ( m_flNextSoundTime < gpGlobals->curtime + dt )
	{
		WeaponSound( SINGLE_NPC, m_flNextSoundTime );
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}

	return numBullets;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKMachineGun::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	// Debounce the recoiling counter
	if ( ( pOwner->m_nButtons & IN_ATTACK ) == false )
	{
		m_nShotsFired = 0;
	}

	BaseClass::ItemPostFrame();
}