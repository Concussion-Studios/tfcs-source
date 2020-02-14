//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "sdk_gamerules.h"
#include "effect_dispatch_data.h"
#include "weapon_sdkbase.h"
#include "weapon_sdkbase_sniper.h"
#include "movevars_shared.h"
#include "gamevars_shared.h"
#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "engine/ivdebugoverlay.h"
#include "obstacle_pushaway.h"
#include "props_shared.h"
#include "decals.h"
#include "util_shared.h"

#ifdef CLIENT_DLL
	#include "c_sdk_player.h"
	#include "c_sdk_team.h"
	#include "prediction.h"
	#include "clientmode_sdk.h"
	#include "vgui_controls/AnimationController.h"

	#define CRecipientFilter C_RecipientFilter
#else
	#include "sdk_player.h"
	#include "sdk_team.h"
#endif

ConVar sv_showimpacts("sv_showimpacts", "0", FCVAR_REPLICATED, "Shows client (red) and server (blue) bullet impact point" );

void DispatchEffect( const char *pName, const CEffectData &data );

//=============================================================================
//
// Tables.
//

// Client specific.
#ifdef CLIENT_DLL
BEGIN_RECV_TABLE_NOBASE( CSDKPlayerShared, DT_SDKPlayerSharedLocal )
	// CTF stuffs
	RecvPropInt( RECVINFO( m_bInGoalZone ) ),

	// PLayer Classes
	RecvPropInt( RECVINFO( m_iPlayerClass ) ),
	RecvPropInt( RECVINFO( m_iDesiredPlayerClass ) ),
END_RECV_TABLE()

BEGIN_RECV_TABLE_NOBASE( CSDKPlayerShared, DT_SDKPlayerShared )
	// Local Data.
	RecvPropDataTable( "sdksharedlocaldata", 0, 0, &REFERENCE_RECV_TABLE( DT_SDKPlayerSharedLocal ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA_NO_BASE( CSDKPlayerShared )
END_PREDICTION_DATA()

#else
BEGIN_SEND_TABLE_NOBASE( CSDKPlayerShared, DT_SDKPlayerSharedLocal )
	// CTF stuffs
	SendPropInt( SENDINFO( m_bInGoalZone ), 1, SPROP_UNSIGNED ),

	// PLayer Classes
	SendPropInt( SENDINFO( m_iPlayerClass), 4 ),
	SendPropInt( SENDINFO( m_iDesiredPlayerClass ), 4 ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CSDKPlayerShared, DT_SDKPlayerShared )
	// Local Data.
	SendPropDataTable( "sdksharedlocaldata", 0, &REFERENCE_SEND_TABLE( DT_SDKPlayerSharedLocal ), SendProxy_SendLocalDataTable ),	
END_SEND_TABLE()
#endif

// --------------------------------------------------------------------------------------------------- //
// CSDKPlayerShared implementation.
// --------------------------------------------------------------------------------------------------- //
CSDKPlayerShared::CSDKPlayerShared()
{
	SetDesiredPlayerClass( PLAYERCLASS_UNDEFINED );

	m_bInGoalZone = false;
}

CSDKPlayerShared::~CSDKPlayerShared()
{
}

void CSDKPlayerShared::Init( CSDKPlayer *pPlayer )
{
	m_pOuter = pPlayer;
}

bool CSDKPlayerShared::IsDucking( void ) const
{
	return ( m_pOuter->GetFlags() & FL_DUCKING ) ? true : false;
}

bool CSDKPlayerShared::IsOnGround() const
{
	return ( m_pOuter->GetFlags() & FL_ONGROUND ) ? true : false;
}

bool CSDKPlayerShared::IsOnGodMode() const
{
	return ( m_pOuter->GetFlags() & FL_GODMODE ) ? true : false;
}

int CSDKPlayerShared::GetButtons()
{
	return m_pOuter->m_nButtons;
}

bool CSDKPlayerShared::IsButtonPressing( int btn )
{
	return ( ( m_pOuter->m_nButtons & btn ) ) ? true : false;
}

bool CSDKPlayerShared::IsButtonPressed( int btn )
{
	return ( ( m_pOuter->m_afButtonPressed & btn ) ) ? true : false;
}

bool CSDKPlayerShared::IsButtonReleased( int btn )
{
	return ( ( m_pOuter->m_afButtonReleased & btn ) ) ? true : false;
}

void CSDKPlayerShared::SetJumping( bool bJumping )
{
	m_bJumping = bJumping;
	
	if ( IsSniperZoomed() )
		ForceUnzoom();
}

void CSDKPlayerShared::SetGoalState( bool state )
{
	m_bInGoalZone = state;
}

//-----------------------------------------------------------------------------
// Consider the weapon's built-in accuracy, this character's proficiency with
// the weapon, and the status of the target. Use this information to determine
// how accurately to shoot at the target.
//-----------------------------------------------------------------------------
Vector CSDKPlayerShared::GetAttackSpread( CWeaponSDKBase *pWeapon, CBaseEntity *pTarget )
{
	if (pWeapon)
		return pWeapon->GetBulletSpread() * pWeapon->GetAccuracyModifier();

	return VECTOR_CONE_15DEGREES;
}

void CSDKPlayerShared::ForceUnzoom( void )
{
	CWeaponSDKBase *pWeapon = GetActiveSDKWeapon();
	if( pWeapon && ( pWeapon->GetSDKWpnData().m_bIsSniper ) )
	{
		CWeaponSDKBaseSniper *pSniper = dynamic_cast<CWeaponSDKBaseSniper *>( pWeapon );
		if ( pSniper )
			pSniper->ExitScope();
	}
}

bool CSDKPlayerShared::IsSniperZoomed( void ) const
{
	CWeaponSDKBase *pWeapon = GetActiveSDKWeapon();
	if( pWeapon && ( pWeapon->GetSDKWpnData().m_bIsSniper ) )
	{
		CWeaponSDKBaseSniper *pSniper = (CWeaponSDKBaseSniper *)pWeapon;
		Assert( pSniper );
		return pSniper->IsScoped();
	}

	return false;
}


void CSDKPlayerShared::SetDesiredPlayerClass( int playerclass )
{
	m_iDesiredPlayerClass = playerclass;
}

int CSDKPlayerShared::DesiredPlayerClass( void )
{
	return m_iDesiredPlayerClass;
}

void CSDKPlayerShared::SetPlayerClass( int playerclass )
{
	m_iPlayerClass = playerclass;
}

int CSDKPlayerShared::PlayerClass( void )
{
	return m_iPlayerClass;
}

CWeaponSDKBase* CSDKPlayerShared::GetActiveSDKWeapon() const
{
	CBaseCombatWeapon *pWeapon = m_pOuter->GetActiveWeapon();
	if ( pWeapon )
	{
		Assert( dynamic_cast< CWeaponSDKBase* >( pWeapon ) == static_cast< CWeaponSDKBase* >( pWeapon ) );
		return static_cast< CWeaponSDKBase* >( pWeapon );
	}
	else
	{
		return NULL;
	}
}

void CSDKPlayerShared::ComputeWorldSpaceSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	Vector org = m_pOuter->GetAbsOrigin();

	static Vector vecMin(-32, -32, 0 );
	static Vector vecMax(32, 32, 72 );

	VectorAdd( vecMin, org, *pVecWorldMins );
	VectorAdd( vecMax, org, *pVecWorldMaxs );
}

// --------------------------------------------------------------------------------------------------- //
// CSDKPlayer shared implementations.
// --------------------------------------------------------------------------------------------------- //
void CSDKPlayer::FireBullet( 
						   Vector vecSrc,	// shooting postion
						   const QAngle &shootAngles,  //shooting angle
						   float vecSpread, // spread vector
						   int iDamage, // base damage
						   int iBulletType, // ammo type
						   CBaseEntity *pevAttacker, // shooter
						   bool bDoEffects,	// create impact effect ?
						   float x,	// spread x factor
						   float y	// spread y factor
						   )
{
	float fCurrentDamage = iDamage;   // damage of the bullet at it's current trajectory
	float flCurrentDistance = 0.0;  //distance that the bullet has traveled so far

	Vector vecDirShooting, vecRight, vecUp;
	AngleVectors( shootAngles, &vecDirShooting, &vecRight, &vecUp );

	if ( !pevAttacker )
		pevAttacker = this;  // the default attacker is ourselves

	// add the spray 
	Vector vecDir = vecDirShooting +
		x * vecSpread * vecRight +
		y * vecSpread * vecUp;

	VectorNormalize( vecDir );

	float flMaxRange = 8000;

	Vector vecEnd = vecSrc + vecDir * flMaxRange; // max bullet range is 10000 units

	trace_t tr; // main enter bullet trace

	UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID|CONTENTS_DEBRIS|CONTENTS_HITBOX, this, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction == 1.0f )
		return; // we didn't hit anything, stop tracing shoot

	if ( sv_showimpacts.GetBool() )
	{
#ifdef CLIENT_DLL
		// draw red client impact markers
		debugoverlay->AddBoxOverlay( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,0,127, 4 );

		if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			C_BasePlayer *player = ToBasePlayer( tr.m_pEnt );
			player->DrawClientHitboxes( 4, true );
		}
#else
		// draw blue server impact markers
		NDebugOverlay::Box( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), 0,0,255,127, 4 );

		if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
		{
			CBasePlayer *player = ToBasePlayer( tr.m_pEnt );
			player->DrawServerHitboxes( 4, true );
		}
#endif
	}

		//calculate the damage based on the distance the bullet travelled.
		flCurrentDistance += tr.fraction * flMaxRange;

		// damage get weaker of distance
		fCurrentDamage *= pow ( 0.85f, (flCurrentDistance / 500));

		int iDamageType = DMG_BULLET | DMG_NEVERGIB;

		if( bDoEffects )
		{
			// See if the bullet ended up underwater + started out of the water
			if ( enginetrace->GetPointContents( tr.endpos ) & (CONTENTS_WATER|CONTENTS_SLIME) )
			{	
				trace_t waterTrace;
				UTIL_TraceLine( vecSrc, tr.endpos, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), this, COLLISION_GROUP_NONE, &waterTrace );

				if( waterTrace.allsolid != 1 )
				{
					CEffectData	data;
					data.m_vOrigin = waterTrace.endpos;
					data.m_vNormal = waterTrace.plane.normal;
					data.m_flScale = random->RandomFloat( 8, 12 );

					if ( waterTrace.contents & CONTENTS_SLIME )
					{
						data.m_fFlags |= FX_WATER_IN_SLIME;
					}

					DispatchEffect( "gunshotsplash", data );
				}
			}
			else
			{
				//Do Regular hit effects

				// Don't decal nodraw surfaces
				if ( !( tr.surface.flags & (SURF_SKY|SURF_NODRAW|SURF_HINT|SURF_SKIP) ) )
				{
					CBaseEntity *pEntity = tr.m_pEnt;
					//Tony; only while using teams do we check for friendly fire.
					if ( pEntity && pEntity->IsPlayer() && (pEntity->GetBaseAnimating() && !pEntity->GetBaseAnimating()->IsRagdoll()) )
					{
						if ( pEntity->GetTeamNumber() == GetTeamNumber() )
						{
							if ( !friendlyfire.GetBool() )
								UTIL_ImpactTrace( &tr, iDamageType );
						}

						//UTIL_ImpactTrace( &tr, iDamageType );
					}
					//Tony; non player, just go nuts,
					else
					{
						UTIL_ImpactTrace( &tr, iDamageType );
					}
				}
			}
		} // bDoEffects

		// add damage to entity that we hit

#ifdef GAME_DLL
		ClearMultiDamage();

		CTakeDamageInfo info( pevAttacker, pevAttacker, fCurrentDamage, iDamageType );
		CalculateBulletDamageForce( &info, iBulletType, vecDir, tr.endpos );
		tr.m_pEnt->DispatchTraceAttack( info, vecDir, &tr );

		TraceAttackToTriggers( info, tr.startpos, tr.endpos, vecDir );

		ApplyMultiDamage();
#endif
}


bool CSDKPlayer::CanMove( void ) const
{
	bool bValidMoveState = (State_Get() == STATE_ACTIVE || State_Get() == STATE_OBSERVER_MODE);		
	if ( !bValidMoveState )
		return false;

	return true;
}

// BUG! This is not called on the client at respawn, only first spawn!
void CSDKPlayer::SharedSpawn()
{	
	BaseClass::SharedSpawn();

	// Reset the animation state or we will animate to standing
	// when we spawn
	m_Shared.SetJumping( false );
}

bool CSDKPlayer::CanAttack( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the players mins - overridden for prone
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector CSDKPlayer::GetPlayerMins( void ) const
{
	if ( IsObserver() )
		return VEC_OBS_HULL_MIN;	
	else
	{
		if ( GetFlags() & FL_DUCKING )
			return VEC_DUCK_HULL_MIN;
		else
			return VEC_HULL_MIN;
	}
}

//-----------------------------------------------------------------------------
// Purpose:  Returns the players Maxs - overridden for prone
// Input  : 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector CSDKPlayer::GetPlayerMaxs( void ) const
{	
	if ( IsObserver() )
		return VEC_OBS_HULL_MAX;	
	else
	{
		if ( GetFlags() & FL_DUCKING )
			return VEC_DUCK_HULL_MAX;
		else
			return VEC_HULL_MAX;
	}
}

//CSDKPlayerClassInfo *CSDKPlayer::GetClassInfo(void)
//{
//	
//}

CWeaponSDKBase *CSDKPlayer::GetWeaponOwnerID( int iID )
{
	for (int i = 0;i < WeaponCount(); i++) 
	{
		CWeaponSDKBase *pWpn = ( CWeaponSDKBase *)GetWeapon( i );

		if ( pWpn == NULL )
			continue;

		if ( pWpn->GetWeaponID() == iID )
			return pWpn;
	}

	return NULL;
}

void CSDKPlayer::InitSpeeds()
{
	int playerclass = m_Shared.PlayerClass();

	//Tony; error checkings.
	if ( playerclass == PLAYERCLASS_UNDEFINED )
	{
		m_Shared.m_flMaxSpeed = SDK_DEFAULT_PLAYER_RUNSPEED;
	}
	else
	{
		CSDKTeam *pTeam = GetGlobalSDKTeam( GetTeamNumber() );
		const CSDKPlayerClassInfo &pClassInfo = pTeam->GetPlayerClassInfo( playerclass );

		m_Shared.m_flMaxSpeed = pClassInfo.m_flMaxSpeed;
	}

	// Set the absolute max speed
	SetMaxSpeed( m_Shared.m_flMaxSpeed );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : collisionGroup - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CSDKPlayer::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	// Only check these when using teams, otherwise it's normal!
	if ( collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT || collisionGroup == COLLISION_GROUP_PROJECTILE )
	{
		switch( GetTeamNumber() )
		{
		case SDK_TEAM_RED:
			if ( !( contentsMask & CONTENTS_REDTEAM ) )
				return false;
			break;

		case SDK_TEAM_BLUE:
			if ( !( contentsMask & CONTENTS_BLUETEAM ) )
				return false;
			break;

		case SDK_TEAM_GREEN:
			if ( !(contentsMask & CONTENTS_GREENTEAM ) )
				return false;
			break;

		case SDK_TEAM_YELLOW:
			if ( !(contentsMask & CONTENTS_YELLOWTEAM ) )
				return false;
			break;
		}
	}
	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}