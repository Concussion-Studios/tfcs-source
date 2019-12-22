//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL1.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "sdk_player.h"
#include "sdk_team.h"
#include "sdk_gamerules.h"
#include "weapon_sdkbase.h"
#include "predicted_viewmodel.h"
#include "iservervehicle.h"
#include "viewport_panel_names.h"
#include "info_camera_link.h"
#include "GameStats.h"
#include "obstacle_pushaway.h"
#include "in_buttons.h"
#include "game.h"
#include "gib.h"
#include "steam/steam_api.h"
#include "cdll_int.h"
#include "datacache/imdlcache.h"
#include "tier0/vprof.h"
#include "bone_setup.h"
#include "ilagcompensationmanager.h"
#include "effect_dispatch_data.h"
#include "te_effect_dispatch.h"
#include "weapon_basesdkgrenade.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int gEvilImpulse101;
bool IsInCommentaryMode( void );

ConVar SDK_ShowStateTransitions( "sdk_ShowStateTransitions", "-2", FCVAR_CHEAT, "sdk_ShowStateTransitions <ent index or -1 for all>. Show player state transitions." );

EHANDLE g_pLastDMSpawn;
EHANDLE g_pLastBlueSpawn;
EHANDLE g_pLastRedSpawn;
EHANDLE g_pLastGreenSpawn;
EHANDLE g_pLastYellowSpawn;

class CPhysicsPlayerCallback : public IPhysicsPlayerControllerEvent
{
public:
	int ShouldMoveTo( IPhysicsObject *pObject, const Vector &position )
	{
		auto *pPlayer = ( CSDKPlayer* )pObject->GetGameData();
		if ( pPlayer )
		{
			if ( pPlayer->TouchedPhysics() )
				return 0;
		}

		return 1;
	}
};


static CPhysicsPlayerCallback playerCallback;

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //
class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEPlayerAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

	CTEPlayerAnimEvent( const char *name ) : CBaseTempEntity( name ) {}

	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nData ), 32 )
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent( "PlayerAnimEvent" );

void TE_PlayerAnimEvent( CBasePlayer *pPlayer, PlayerAnimEvent_t event, int nData )
{
	CPVSFilter filter( (const Vector&)pPlayer->EyePosition() );

	//Tony; pull the player who is doing it out of the recipientlist, this is predicted!!
	filter.RemoveRecipient( pPlayer );

	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.m_nData = nData;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}

void* SendProxy_SendNonLocalDataTable( const SendProp* pProp, const void* pStruct, const void* pVarData, CSendProxyRecipients* pRecipients, int objectID )
{
	pRecipients->SetAllRecipients();
	pRecipients->ClearRecipient( objectID - 1 );
	return (void*)pVarData;
}

// -------------------------------------------------------------------------------- //
// Tables.
// -------------------------------------------------------------------------------- //
BEGIN_DATADESC( CSDKPlayer )
	DEFINE_THINKFUNC( SDKPushawayThink ),

	DEFINE_FIELD( m_ArmorValue, FIELD_INTEGER ),
	DEFINE_FIELD( m_MaxArmorValue, FIELD_INTEGER ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( player, CSDKPlayer );
PRECACHE_REGISTER(player);

// CSDKPlayerShared Data Tables
//=============================

// specific to the local player
BEGIN_SEND_TABLE_NOBASE( CSDKPlayerShared, DT_SDKSharedLocalPlayerExclusive )
	SendPropInt( SENDINFO( m_iPlayerClass), 4 ),
	SendPropInt( SENDINFO( m_iDesiredPlayerClass ), 4 ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CSDKPlayerShared, DT_SDKPlayerShared )
	SendPropDataTable( "sdksharedlocaldata", 0, &REFERENCE_SEND_TABLE(DT_SDKSharedLocalPlayerExclusive), SendProxy_SendLocalDataTable ),
END_SEND_TABLE()

extern void SendProxy_Origin( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID );

BEGIN_SEND_TABLE_NOBASE( CSDKPlayer, DT_SDKLocalPlayerExclusive )
	SendPropInt( SENDINFO( m_iShotsFired ), 8, SPROP_UNSIGNED ),
	// send a hi-res origin to the local player for use in prediction
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),

	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
//	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),

	SendPropInt( SENDINFO( m_ArmorValue ), 8, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_MaxArmorValue ), 8, SPROP_UNSIGNED ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CSDKPlayer, DT_SDKNonLocalPlayerExclusive )
	// send a lo-res origin to other players
	SendPropVector	(SENDINFO(m_vecOrigin), -1,  SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin ),

	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),
END_SEND_TABLE()


// main table
IMPLEMENT_SERVERCLASS_ST( CSDKPlayer, DT_SDKPlayer )
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseAnimating", "m_nNewSequenceParity" ),
	SendPropExclude( "DT_BaseAnimating", "m_nResetEventsParity" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),
	
	// playeranimstate and clientside animation takes care of these on the client
	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),

	// Data that only gets sent to the local player.
	SendPropDataTable( SENDINFO_DT( m_Shared ), &REFERENCE_SEND_TABLE( DT_SDKPlayerShared ) ),

	// Data that only gets sent to the local player.
	SendPropDataTable( "sdklocaldata", 0, &REFERENCE_SEND_TABLE(DT_SDKLocalPlayerExclusive), SendProxy_SendLocalDataTable ),
	// Data that gets sent to all other players
	SendPropDataTable( "sdknonlocaldata", 0, &REFERENCE_SEND_TABLE(DT_SDKNonLocalPlayerExclusive), SendProxy_SendNonLocalDataTable ),

	SendPropEHandle( SENDINFO( m_hRagdoll ) ),

	SendPropInt( SENDINFO( m_iPlayerState ), Q_log2( NUM_PLAYER_STATES )+1, SPROP_UNSIGNED ),

	SendPropBool( SENDINFO( m_bSpawnInterpCounter ) ),

END_SEND_TABLE()

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //
class CSDKRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CSDKRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState() { return SetTransmitState( FL_EDICT_ALWAYS );	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( sdk_ragdoll, CSDKRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CSDKRagdoll, DT_SDKRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()


// -------------------------------------------------------------------------------- //

void cc_CreatePredictionError_f()
{
	CBaseEntity *pEnt = CBaseEntity::Instance( 1 );
	pEnt->SetAbsOrigin( pEnt->GetAbsOrigin() + Vector( 63, 0, 0 ) );
}

ConCommand cc_CreatePredictionError( "CreatePredictionError", cc_CreatePredictionError_f, "Create a prediction error", FCVAR_CHEAT );

void CSDKPlayer::SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize )
{
	BaseClass::SetupVisibility( pViewEntity, pvs, pvssize );

	int area = pViewEntity ? pViewEntity->NetworkProp()->AreaNum() : NetworkProp()->AreaNum();
	PointCameraSetupVisibility( this, area, pvs, pvssize );
}

CSDKPlayer::CSDKPlayer()
{
	//Tony; create our player animation state.
	m_PlayerAnimState = CreateSDKPlayerAnimState( this );
	m_iLastWeaponFireUsercmd = 0;
	
	m_Shared.Init( this );

	UseClientSideAnimation();

	m_angEyeAngles.Init();

	m_pCurStateInfo = NULL;	// no state yet
}

CSDKPlayer::~CSDKPlayer()
{
	DestroyRagdoll();
	m_PlayerAnimState->Release();
}


CSDKPlayer *CSDKPlayer::CreatePlayer( const char *className, edict_t *ed )
{
	CSDKPlayer::s_PlayerEdict = ed;
	return (CSDKPlayer*)CreateEntityByName( className );
}

void CSDKPlayer::LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles )
{
	BaseClass::LeaveVehicle( vecExitPoint, vecExitAngles );

	//teleport physics shadow too
	// Vector newPos = GetAbsOrigin();
	// QAngle newAng = GetAbsAngles();

	// Teleport( &newPos, &newAng, &vec3_origin );
}

void CSDKPlayer::PreThink(void)
{
	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;

	// Riding a vehicle?
	if ( IsInAVehicle() )	
	{
		// make sure we update the client, check for timed damage and update suit even if we are in a vehicle
		UpdateClientData();		
		CheckTimeBasedDamage();

		// Allow the suit to recharge when in the vehicle.
		CheckSuitUpdate();
		
		WaterMove();	
		return;
	}

	BaseClass::PreThink();
}


void CSDKPlayer::PostThink()
{
	BaseClass::PostThink();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );
	
	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

	m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );
}


void CSDKPlayer::Precache()
{
	//Tony; go through our list of player models that we may be using and cache them
	int i = 0;
	while( pszPossiblePlayerModels[i] != NULL )
	{
		PrecacheModel( pszPossiblePlayerModels[i] );
		i++;
	}

	int g = 0;
	while (pszPossibleGibModels[g] != NULL)
	{
		PrecacheModel(pszPossibleGibModels[g]);
		g++;
	}
	
	BaseClass::Precache();
}

void CSDKPlayer::SDKPushawayThink(void)
{
	// Push physics props out of our way.
	PerformObstaclePushaway( this );
	SetNextThink( gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL, SDK_PUSHAWAY_THINK_CONTEXT );
}

void CSDKPlayer::Spawn()
{
	switch ( GetTeamNumber() )
	{
		case SDK_TEAM_RED:
			m_nSkin = 0;
			break;

		case SDK_TEAM_BLUE:
			m_nSkin = 1;
			break;

		case SDK_TEAM_GREEN:
			m_nSkin = 2;
			break;

		case SDK_TEAM_YELLOW:
			m_nSkin = 3;
			break;
	}

	SetBloodColor( BLOOD_COLOR_RED );
	
	SetMoveType( MOVETYPE_WALK );
	RemoveSolidFlags( FSOLID_NOT_SOLID );

	//Tony; if we're spawning in active state, equip the suit so the hud works. -- Gotta love base code !
	if ( State_Get() == STATE_ACTIVE )
	{
		EquipSuit( false );	

		//Player spawn sound.
		CPASFilter filter( GetAbsOrigin() );
		filter.UsePredictionRules();
		EmitSound( filter, this->entindex(),"Player.Spawn" );

		if ( IsInCommentaryMode() && !IsFakeClient() )
		{
			// Player is spawning in commentary mode. Tell the commentary system.
			CBaseEntity *pEnt = NULL;
			variant_t emptyVariant;
			while ( (pEnt = gEntList.FindEntityByClassname( pEnt, "commentary_auto" )) != NULL )
				pEnt->AcceptInput( "MultiplayerSpawned", this, this, emptyVariant, 0 );
		}
	}

	m_hRagdoll = NULL;
	
	RemoveEffects( EF_NOINTERP );

	m_impactEnergyScale = SDK_PHYSDAMAGE_SCALE;

	//Tony; do the spawn animevent
	DoAnimationEvent( PLAYERANIMEVENT_SPAWN );
	
	BaseClass::Spawn();

	m_bTeamChanged	= false;

	// update this counter, used to not interp players when they spawn
	m_bSpawnInterpCounter = !m_bSpawnInterpCounter;

	InitSpeeds(); //Tony; initialize player speeds.

	ClearDamagerHistory();// clear damager History

	//SetArmorValue( SpawnArmorValue() );

	SetContextThink( &CSDKPlayer::SDKPushawayThink, gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL, SDK_PUSHAWAY_THINK_CONTEXT );
	pl.deadflag = false;

}

bool CSDKPlayer::SelectSpawnSpot( const char *pEntClassName, CBaseEntity* &pSpot )
{
	// Find the next spawn spot.
	pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );

	if ( pSpot == NULL ) // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );

	CBaseEntity *pFirstSpot = pSpot;
	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetAbsOrigin() == Vector( 0, 0, 0 ) )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
					continue;
				}

				// if so, go to pSpot
				return true;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pEntClassName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	DevMsg("CSDKPlayer::SelectSpawnSpot: couldn't find valid spawn point.\n");

	return true;
}


CBaseEntity* CSDKPlayer::EntSelectSpawnPoint()
{
	CBaseEntity *pSpot = NULL;

	const char *pSpawnPointName = "";

	switch( GetTeamNumber() )
	{
	case SDK_TEAM_BLUE:
		{
			pSpawnPointName = "info_player_blue";
			pSpot = g_pLastBlueSpawn;
			if ( SelectSpawnSpot( pSpawnPointName, pSpot ) )
			{
				g_pLastBlueSpawn = pSpot;
			}
		}
		break;
	case SDK_TEAM_RED:
		{
			pSpawnPointName = "info_player_red";
			pSpot = g_pLastRedSpawn;
			if ( SelectSpawnSpot( pSpawnPointName, pSpot ) )
			{
				g_pLastRedSpawn = pSpot;
			}
		}		
		break;
	case SDK_TEAM_GREEN:
		{
			pSpawnPointName = "info_player_green";
			pSpot = g_pLastGreenSpawn;
			if ( SelectSpawnSpot( pSpawnPointName, pSpot ) )
			{
				g_pLastGreenSpawn = pSpot;
			}
		}
		break;
	case SDK_TEAM_YELLOW:
		{
			pSpawnPointName = "info_player_yellow";
			pSpot = g_pLastYellowSpawn;
			if ( SelectSpawnSpot( pSpawnPointName, pSpot ) )
			{
				g_pLastYellowSpawn = pSpot;
			}
		}		
		break;
	case TEAM_UNASSIGNED:
	case TEAM_SPECTATOR:
	default:
		{
			pSpot = CBaseEntity::Instance( INDEXENT(0) );
		}
		break;		
	}

	if ( !pSpot )
	{
		pSpawnPointName = "info_player_deathmatch";
		pSpot = g_pLastDMSpawn;
		if ( pSpot ) 
		{
			if ( SelectSpawnSpot( pSpawnPointName, pSpot ) )
				g_pLastDMSpawn = pSpot;

			return pSpot;
		}

		Warning( "PutClientInServer: no %s on level\n", pSpawnPointName );
		return CBaseEntity::Instance(INDEXENT(0));
	}

	return pSpot;
} 

//=========================================================
//=========================================================
void CSDKPlayer::CheatImpulseCommands( int iImpulse )
{
	if ( !sv_cheats->GetBool() && State_Get() == STATE_ACTIVE )
		return;

	switch ( iImpulse )
	{
	case 101:
		gEvilImpulse101 = true;

		CBasePlayer::GiveAmmo( 255,	"sniper");
		CBasePlayer::GiveAmmo( 255,	"nail");
		CBasePlayer::GiveAmmo( 255,	"shell" );
		CBasePlayer::GiveAmmo( 255,	"cell" );
		CBasePlayer::GiveAmmo( 255,	"explosive" );
		CBasePlayer::GiveAmmo( 255,	"fireball" );
		CBasePlayer::GiveAmmo( 255,	"shotgun" );
		CBasePlayer::GiveAmmo( 255,	"plasma" );
		CBasePlayer::GiveAmmo( 4,	"grenades" );
		CBasePlayer::GiveAmmo( 4,	"concussion" );
		CBasePlayer::GiveAmmo( 4,	"emp" );
		CBasePlayer::GiveAmmo( 4,	"napalm" );

		GiveNamedItem( "weapon_crowbar" );
		GiveNamedItem( "weapon_umbrella" );
		GiveNamedItem( "weapon_wrench" );
		GiveNamedItem( "weapon_medkit" );
		GiveNamedItem( "weapon_knife" );
		GiveNamedItem( "weapon_tranq" );
		GiveNamedItem( "weapon_12gauge" );
		GiveNamedItem( "weapon_shotgun" );
		GiveNamedItem( "weapon_sniperrifle" );
		GiveNamedItem( "weapon_autorifle" );
		GiveNamedItem( "weapon_nailgun" );
		GiveNamedItem( "weapon_railgun" );
		GiveNamedItem( "weapon_supernailgun" );
		GiveNamedItem( "weapon_rpg" );
		GiveNamedItem( "weapon_ic" );
		GiveNamedItem( "weapon_ac" );
		GiveNamedItem( "weapon_grenade" );
		//GiveNamedItem( "weapon_grenade_concussion" );
		//GiveNamedItem( "weapon_grenade_napalm" );
		//GiveNamedItem( "weapon_grenade_emp" );
		//GiveNamedItem( "weapon_grenade_nail" );
		//GiveNamedItem( "weapon_grenade_mirv" );
		//GiveNamedItem( "weapon_grenade_hallucination" );
		//GiveNamedItem( "weapon_grenade_caltrop" );

		if ( GetHealth() < 100 )
			TakeHealth( 25, DMG_GENERIC );

		if ( GetArmorValue() < 100 )
			IncrementArmorValue( 25 );
		
		gEvilImpulse101 = false;

		break;

	default:
		BaseClass::CheatImpulseCommands( iImpulse );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CSDKPlayer::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
			UTIL_Remove( pWeapon );

		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
		return false;

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Put the player in the specified team
//-----------------------------------------------------------------------------
void CSDKPlayer::ChangeTeam( int iTeamNum )
{
	if ( !GetGlobalTeam( iTeamNum ) )
	{
		Warning( "CSDKPlayer::ChangeTeam( %d ) - invalid team index.\n", iTeamNum );
		return;
	}

	int iOldTeam = GetTeamNumber();

	// if this is our current team, just abort
	if ( iTeamNum == iOldTeam )
		return;
	
	m_bTeamChanged = true;

	// do the team change:
	BaseClass::ChangeTeam( iTeamNum );

	// update client state 
	if ( iTeamNum == TEAM_UNASSIGNED )
	{
		State_Transition( STATE_OBSERVER_MODE );
	}
	else if ( iTeamNum == TEAM_SPECTATOR )
	{
		RemoveAllItems( true );
		
		State_Transition( STATE_OBSERVER_MODE );
	}
	else // active player
	{
		if ( !IsDead() )
		{
			// Kill player if switching teams while alive
			CommitSuicide();

			// add 1 to frags to balance out the 1 subtracted for killing yourself
			IncrementFragCount( 1 );
		}

		if( iOldTeam == TEAM_SPECTATOR )
			SetMoveType( MOVETYPE_NONE );

		// Put up the class selection menu.
		State_Transition( STATE_PICKINGCLASS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKPlayer::CommitSuicide( bool bExplode /* = false */, bool bForce /*= false*/ )
{
	// Don't suicide if we haven't picked a class for the first time, or we're not in active state
	if ( m_Shared.PlayerClass() == PLAYERCLASS_UNDEFINED || State_Get() != STATE_ACTIVE )
		return;

	m_iSuicideCustomKillFlags = SDK_DMG_CUSTOM_SUICIDE;

	BaseClass::CommitSuicide( bExplode, bForce );
}

void CSDKPlayer::InitialSpawn( void )
{
	BaseClass::InitialSpawn();

	State_Enter( STATE_WELCOME );

}

int CSDKPlayer::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pAttacker = info.GetAttacker();
	float flArmorBonus = 0.5f;
	float flArmorRatio = 0.5f;
	float flDamage = info.GetDamage();
	bool bFriendlyFire = friendlyfire.GetBool();

	if ( !pInflictor )
		return 0;

	// Already dead
	if ( !IsAlive() )
		return 0;

	// Early out if there's no damage
	if ( !info.GetDamage() )
		return 0;

	if ( GetFlags() & FL_GODMODE )
		return 0;

	if ( GetMoveType() == MOVETYPE_NOCLIP || GetMoveType() == MOVETYPE_OBSERVER )
		return 0;

	// if the player's team does not match the inflictor's team
	// the player has changed teams between when they started the attack
	if( pInflictor->GetTeamNumber() != TEAM_UNASSIGNED && 
		info.GetAttacker() != NULL &&
		pInflictor->GetTeamNumber() != info.GetAttacker()->GetTeamNumber() )
	{
		info.SetDamage( 0 );
		info.SetDamageType( 0 );
	}

	if ( m_debugOverlays & OVERLAY_BUDDHA_MODE ) 
	{
		if ( ( m_iHealth - info.GetDamage() ) <= 0 )
		{
			m_iHealth = 1;
			return 0;
		}
	}

	if( info.GetDamageType() & DMG_BLAST_SURFACE )
	{
		if( GetWaterLevel() > 2 )
		{
			// Don't take blast damage from anything above the surface.
			if( info.GetInflictor()->GetWaterLevel() == 0 )
				return 0;
		}
	}

	IServerVehicle *pVehicle = GetVehicle();
	if ( pVehicle )
	{
		// Let the vehicle decide if we should take this damage or not
		if ( pVehicle->PassengerShouldReceiveDamage( info ) == false )
			return 0;
	}

	if ( IsInCommentaryMode() )
	{
		if( !ShouldTakeDamageInCommentaryMode( info ) )
			return 0;
	}

	// blasts damage armor more unless whe aren't Heavy or soldier.
	if ( ( info.GetDamageType() & DMG_BLAST ) && !( m_Shared.DesiredPlayerClass() == PLAYERCLASS_HEAVY || m_Shared.DesiredPlayerClass() == PLAYERCLASS_SOLDIER ) )
		flArmorBonus *= 2;

	// Refuse the damage
	if ( !g_pGameRules->FPlayerCanTakeDamage( this, info.GetAttacker(), inputInfo ) )
		return 0;

	if ( bFriendlyFire ||
		info.GetAttacker()->GetTeamNumber() != GetTeamNumber() ||
		pInflictor == this ||
		info.GetAttacker() == this ||
		info.GetDamageType() & DMG_BLAST )
	{
		/*if ( bFriendlyFire && (info.GetDamageType() & DMG_BLAST) == 0 )
		{
			if ( pInflictor->GetTeamNumber() == GetTeamNumber() && bCheckFriendlyFire)
			{
				flDamage *= 0.35; // bullets hurt teammates less
			}
		}*/

		AddDamagerToHistory( pAttacker );

		// keep track of amount of damage last sustained
		m_lastDamageAmount = flDamage;

		// Deal with Armour
		if ( GetArmorValue() && !( info.GetDamageType() & ( DMG_FALL | DMG_DROWN | DMG_POISON | DMG_RADIATION ) ) )	// armor doesn't protect against fall or drown damage!
		{
			float flNew = flDamage * flArmorRatio;
			float flArmor = (flDamage - flNew) * flArmorBonus;

			// Does this use more armor than we have?
			if ( flArmor > GetArmorValue() )
			{
				//armorHit = (int)(flArmor);

				flArmor = GetArmorValue();
				flArmor *= (1/flArmorBonus);
				flNew = flDamage - flArmor;
				SetArmorValue( 0 );
			}
			else
			{
				int oldValue = (int)( GetArmorValue() );
			
				if ( flArmor < 0 )
					 flArmor = 1;

				SetArmorValue( oldValue - flArmor );
				//armorHit = oldValue - (int)(pev->armorvalue);
			}
			
			flDamage = flNew;
			
			info.SetDamage( flDamage );
		}

		// round damage to integer
		info.SetDamage( (int)flDamage );

		if ( info.GetDamage() <= 0 )
			return 0;

		// Modify the amount of damage the player takes, based on skill.
		CTakeDamageInfo playerDamage = info;

		if ( GetVehicleEntity() != NULL )
		{
			if( playerDamage.GetDamage() > 30 && playerDamage.GetInflictor() == GetVehicleEntity() && ( playerDamage.GetDamageType() & DMG_CRUSH ) )
				playerDamage.ScaleDamage( 0.3f / playerDamage.GetDamage() );
		}

		m_vecTotalBulletForce += inputInfo.GetDamageForce();

		CSingleUserRecipientFilter user( this );
		user.MakeReliable();
		UserMessageBegin( user, "Damage" );
			WRITE_BYTE( (int)info.GetDamage() );
			WRITE_VEC3COORD( info.GetInflictor()->WorldSpaceCenter() );
		MessageEnd();

		gamestats->Event_PlayerDamage( this, info );

		return CBaseCombatCharacter::OnTakeDamage( playerDamage );
	}
	else
	{
		return 0;
	}
}

int CSDKPlayer::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// set damage type sustained
	m_bitsDamageType |= info.GetDamageType();

	if ( !CBaseCombatCharacter::OnTakeDamage_Alive( info ) )
		return 0;

	CBaseEntity * attacker = info.GetAttacker();
	if ( !attacker )
		return 0;

	// don't apply damage forces if we are the heavy
	if ( !( m_Shared.DesiredPlayerClass() == PLAYERCLASS_HEAVY ) )
	{
		Vector vecDir = vec3_origin;
		if ( info.GetInflictor() )
		{
			vecDir = info.GetInflictor()->WorldSpaceCenter() - Vector ( 0, 0, 10 ) - WorldSpaceCenter();
			VectorNormalize( vecDir );
		}

		if ( info.GetInflictor() && ( GetMoveType() == MOVETYPE_WALK ) && 
			( !attacker->IsSolidFlagSet( FSOLID_TRIGGER ) ) )
		{
			Vector force = vecDir * -DamageForce( WorldAlignSize(), info.GetBaseDamage() );
			if ( force.z > 250.0f )
				force.z = 250.0f;

			// Do special explosion damage effect
			if ( info.GetDamageType() & DMG_BLAST )
			{
				if ( m_Shared.DesiredPlayerClass() == PLAYERCLASS_SOLDIER || 
					 m_Shared.DesiredPlayerClass() == PLAYERCLASS_DEMOMAN )
					force.z = 750.0f; // Flyyy
			}

			CWeaponSDKBase *pWeapon = (CWeaponSDKBase *)GetActiveWeapon();
			if ( pWeapon )
			{
				CBaseSDKGrenade *pGrenade = dynamic_cast<CBaseSDKGrenade *>( pWeapon );
				if( pGrenade )
					force.z = 350.0f; // Flyyy
			}

			ApplyAbsVelocityImpulse( force );
		}	
	}

	// Drown
	if( info.GetDamageType() & DMG_DROWN )
	{
		if( m_idrowndmg == m_idrownrestored )
			EmitSound( "Player.DrownStart" );
		else
			EmitSound( "Player.DrownContinue" );
	}

	// Burnt
	if ( info.GetDamageType() & DMG_BURN )
		EmitSound( "Player.BurnPain" );

	// fire global game event
	IGameEvent * event = gameeventmanager->CreateEvent( "player_hurt" );
	if ( event )
	{
		event->SetInt("userid", GetUserID() );
		event->SetInt("health", max( 0, m_iHealth ) );
		event->SetInt("armor", max( 0, GetArmorValue() ) );

		if ( info.GetDamageType() & DMG_BLAST )
			event->SetInt( "hitgroup", HITGROUP_GENERIC );
		else
			event->SetInt( "hitgroup", LastHitGroup() );

		CBaseEntity * attacker = info.GetAttacker();
		const char *weaponName = "";

		if ( attacker->IsPlayer() )
		{
			CBasePlayer *player = ToBasePlayer( attacker );
			event->SetInt("attacker", player->GetUserID() ); // hurt by other player

			CBaseEntity *pInflictor = info.GetInflictor();
			if ( pInflictor )
			{
				if ( pInflictor == player )
				{
					// If the inflictor is the killer,  then it must be their current weapon doing the damage
					if ( player->GetActiveWeapon() )
						weaponName = player->GetActiveWeapon()->GetClassname();
				}
				else
					weaponName = STRING( pInflictor->m_iClassname );  // it's just that easy
			}
		}
		else
			event->SetInt("attacker", 0 ); // hurt by "world"

		if ( strncmp( weaponName, "weapon_", 7 ) == 0 )
			weaponName += 7;
		else if( strncmp( weaponName, "grenade", 9 ) == 0 )	//"grenade_projectile"	
			weaponName = "grenade";

		event->SetString( "weapon", weaponName );
		event->SetInt( "priority", 5 );

		gameeventmanager->FireEvent( event );
	}
	
	return 1;
}

void CSDKPlayer::OnDamagedByExplosion( const CTakeDamageInfo &info )
{
	if ( info.GetInflictor() && info.GetDamageType() & DMG_SONIC )
	{
		// No ear ringing for concussion grenade
		UTIL_ScreenShake( info.GetInflictor()->GetAbsOrigin(), 4.0, 1.0, 0.5, 1000, SHAKE_START, false );
		return;
	}
	BaseClass::OnDamagedByExplosion( info );
}


void CSDKPlayer::Event_Killed( const CTakeDamageInfo &info )
{
	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	ThrowActiveWeapon();

	FlashlightTurnOff();

	// show killer in death cam mode
	// chopped down version of SetObserverTarget without the team check
	if( info.GetAttacker() && info.GetAttacker()->IsPlayer() )
	{
		// set new target
		m_hObserverTarget.Set( info.GetAttacker() ); 

		// reset fov to default
		SetFOV( this, 0 );
	}
	else
		m_hObserverTarget.Set( NULL );


	State_Transition( STATE_DEATH_ANIM );	// Transition into the dying state.

	//Tony; after transition, remove remaining items
	RemoveAllItems( true );

	if ( info.GetDamageType() & ( DMG_BUCKSHOT | DMG_BLAST ) || info.GetDamage() >= ( GetMaxHealth() * 0.75f ) )
	{
		// Release our gibs
		BecomeAGibs( info );
	}
	else
	{
		// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
		// because we still want to transmit to the clients in our PVS.
		CreateRagdollEntity();
	}

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & ( DMG_DISSOLVE | DMG_PLASMA ) )
	{
		if ( m_hRagdoll )
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
	}

	if( info.GetDamageType() & ( DMG_BLAST| DMG_BURN ) )
	{
		if( m_hRagdoll )
		{
			CBaseAnimating *pRagdoll = (CBaseAnimating *)CBaseEntity::Instance( m_hRagdoll );
			if( info.GetDamageType() & ( DMG_BURN | DMG_BLAST ) )
				pRagdoll->Ignite( 45, false, 10 );
		}
	}

}

void CSDKPlayer::BecomeAGibs( const CTakeDamageInfo &info )
{
	Vector vecDamageDir = info.GetDamageForce();

	EmitSound( "Player.FallGib" );

	UTIL_BloodSpray( WorldSpaceCenter(), vecDamageDir, BLOOD_COLOR_RED, 13, FX_BLOODSPRAY_ALL );

	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/pgib_p1.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/pgib_p2.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/pgib_p3.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/pgib_p4.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/pgib_p5.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/hgibs_jaw.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/hgibs_scapula.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/hgibs_scapula.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/rgib_p1.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/rgib_p2.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/rgib_p3.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/rgib_p4.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/rgib_p5.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/rgib_p6.mdl", 5 );
	CGib::SpawnSpecificGibs( this, 1, 750, 1500, "models/gibs/gibhead.mdl", 5 );
}

Class_T CSDKPlayer::Classify ( void )
{
	if( IsInAVehicle() )
	{
		IServerVehicle *pVehicle = GetVehicle();
		return pVehicle->ClassifyPassenger( this, CLASS_PLAYER );
	}
	else
		return CLASS_PLAYER;
}

void CSDKPlayer::ClearDamagerHistory()
{
	for ( int i = 0; i < ARRAYSIZE( m_DamagerHistory ); i++ )
		m_DamagerHistory[i].Reset();
}

void CSDKPlayer::AddDamagerToHistory(EHANDLE hDamager)
{
	//Check if same team
	CSDKPlayer *pDamager = ToSDKPlayer(hDamager);

	if (!pDamager || pDamager == this || InSameTeam(pDamager))
		return;
	//If most recent damager is different from most recent then shift our list down.
	if (m_DamagerHistory[0].hDamager != hDamager)
	{
		for (int i = 1; i < ARRAYSIZE(m_DamagerHistory); i++)
		{
			m_DamagerHistory[i] = m_DamagerHistory[i - 1];
		}
	}
	m_DamagerHistory[0].hDamager = hDamager;
	m_DamagerHistory[0].flTimeDamage = gpGlobals->curtime;
}

void CSDKPlayer::ThrowActiveWeapon( void )
{
	CWeaponSDKBase *pWeapon = (CWeaponSDKBase *)GetActiveWeapon();

	if( pWeapon && pWeapon->CanWeaponBeDropped() )
	{
		QAngle gunAngles;
		VectorAngles( BodyDirection2D(), gunAngles );

		Vector vecForward;
		AngleVectors( gunAngles, &vecForward, NULL, NULL );

		float flDiameter = sqrt( CollisionProp()->OBBSize().x * CollisionProp()->OBBSize().x + CollisionProp()->OBBSize().y * CollisionProp()->OBBSize().y );

		pWeapon->Holster(NULL);
		SwitchToNextBestWeapon( pWeapon );
		SDKThrowWeapon( pWeapon, vecForward, gunAngles, flDiameter );
	}
}
void CSDKPlayer::Weapon_Equip( CBaseCombatWeapon *pWeapon )
{
	BaseClass::Weapon_Equip( pWeapon );
	dynamic_cast<CWeaponSDKBase*>(pWeapon)->SetDieThink( false );	//Make sure the context think for removing is gone!!

}
void CSDKPlayer::SDKThrowWeapon( CWeaponSDKBase *pWeapon, const Vector &vecForward, const QAngle &vecAngles, float flDiameter  )
{
	Vector vecOrigin;
	CollisionProp()->RandomPointInBounds( Vector( 0.5f, 0.5f, 0.5f ), Vector( 0.5f, 0.5f, 1.0f ), &vecOrigin );

	// Nowhere in particular; just drop it.
	Vector vecThrow;
	SDKThrowWeaponDir( pWeapon, vecForward, &vecThrow );

	Vector vecOffsetOrigin;
	VectorMA( vecOrigin, flDiameter, vecThrow, vecOffsetOrigin );

	trace_t	tr;
	UTIL_TraceLine( vecOrigin, vecOffsetOrigin, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
		
	if ( tr.startsolid || tr.allsolid || ( tr.fraction < 1.0f && tr.m_pEnt != pWeapon ) )
	{
		//FIXME: Throw towards a known safe spot?
		vecThrow.Negate();
		VectorMA( vecOrigin, flDiameter, vecThrow, vecOffsetOrigin );
	}

	vecThrow *= random->RandomFloat( 150.0f, 240.0f );

	pWeapon->SetAbsOrigin( vecOrigin );
	pWeapon->SetAbsAngles( vecAngles );
	pWeapon->Drop( vecThrow );
	pWeapon->SetRemoveable( false );
	Weapon_Detach( pWeapon );

	pWeapon->SetDieThink( true );
}

void CSDKPlayer::SDKThrowWeaponDir( CWeaponSDKBase *pWeapon, const Vector &vecForward, Vector *pVecThrowDir )
{
	VMatrix zRot;
	MatrixBuildRotateZ( zRot, random->RandomFloat( -60.0f, 60.0f ) );

	Vector vecThrow;
	Vector3DMultiply( zRot, vecForward, *pVecThrowDir );

	pVecThrowDir->z = random->RandomFloat( -0.5f, 0.5f );
	VectorNormalize( *pVecThrowDir );
}

void CSDKPlayer::PlayerDeathThink()
{
	//overridden, do nothing - our states handle this now
}

//-----------------------------------------------------------------------------
// Purpose: Makes a splash when the player transitions between water states
//-----------------------------------------------------------------------------
void CSDKPlayer::Splash( void )
{
	CEffectData data;
	data.m_fFlags = 0;
	data.m_vOrigin = GetAbsOrigin();
	data.m_vNormal = Vector(0,0,1);
	data.m_vAngles = QAngle( 0, 0, 0 );
	
	if ( GetWaterType() & CONTENTS_SLIME )
		data.m_fFlags |= FX_WATER_IN_SLIME;

	float flSpeed = GetAbsVelocity().Length();
	if ( flSpeed < 300 )
	{
		data.m_flScale = random->RandomFloat( 10, 12 );
		DispatchEffect( "waterripple", data );
	}
	else
	{
		data.m_flScale = random->RandomFloat( 6, 8 );
		DispatchEffect( "watersplash", data );
	}
}

//=========================================================
// TraceAttack
//=========================================================
void CSDKPlayer::TraceAttack( const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	if ( m_takedamage != DAMAGE_YES )
		return;

	CSDKPlayer *pAttacker = (CSDKPlayer*)ToSDKPlayer( inputInfo.GetAttacker() );
	if ( pAttacker )
	{
		// Prevent team damage here so blood doesn't appear
		if ( inputInfo.GetAttacker()->IsPlayer() )
		{
			if ( !g_pGameRules->FPlayerCanTakeDamage( this, inputInfo.GetAttacker(), inputInfo ) )
				return;
		}
	}

	// Save this bone for the ragdoll.
	m_nForceBone = ptr->physicsbone;

	SetLastHitGroup( ptr->hitgroup );

	// Ignore hitboxes for all weapons except the sniper rifle
	CTakeDamageInfo info = inputInfo;

	// Since this code only runs on the server, make sure it shows the tempents it creates.
	CDisablePredictionFiltering disabler;

	// This does smaller splotches on the guy and splats blood on the world.
	TraceBleed( info.GetDamage(), vecDir, ptr, info.GetDamageType() );

	AddMultiDamage( info, this );
}

void CSDKPlayer::InitVCollision( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity )
{
	BaseClass::InitVCollision( vecAbsOrigin, vecAbsVelocity );

	// Setup the SDK specific callback.
	IPhysicsPlayerController *pPlayerController = GetPhysicsController();
	if ( pPlayerController )
		pPlayerController->SetEventHandler( &playerCallback );
}

void CSDKPlayer::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );

	FireBulletsInfo_t modinfo = info;

	CWeaponSDKBase *pWeapon = dynamic_cast<CWeaponSDKBase *>( GetActiveWeapon() );
	if ( pWeapon )
		modinfo.m_iPlayerDamage = modinfo.m_flDamage = pWeapon->GetSDKWpnData().m_iDamage;

	NoteWeaponFired();

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( this );
}

void CSDKPlayer::CreateRagdollEntity()
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	CSDKRagdoll *pRagdoll = dynamic_cast< CSDKRagdoll* >( m_hRagdoll.Get() );
	
	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CSDKRagdoll* >( CreateEntityByName( "sdk_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );

		switch ( GetTeamNumber() )
		{
			case SDK_TEAM_RED:
				pRagdoll->m_nSkin = 0;
				break;

			case SDK_TEAM_BLUE:
				pRagdoll->m_nSkin = 1;
				break;

			case SDK_TEAM_GREEN:
				pRagdoll->m_nSkin = 2;
				break;

			case SDK_TEAM_YELLOW:
				pRagdoll->m_nSkin = 3;
				break;
		}
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}
//-----------------------------------------------------------------------------
// Purpose: Destroy's a ragdoll, called when a player is disconnecting.
//-----------------------------------------------------------------------------
void CSDKPlayer::DestroyRagdoll( void )
{
	CSDKRagdoll *pRagdoll = dynamic_cast<CSDKRagdoll*>( m_hRagdoll.Get() );	
	if( pRagdoll )
	{
		UTIL_Remove( pRagdoll );
	}
}

void CSDKPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	m_PlayerAnimState->DoAnimationEvent( event, nData );
	TE_PlayerAnimEvent( this, event, nData );	// Send to any clients who can see this guy.
}

//-----------------------------------------------------------------------------
// Purpose: Override setup bones so that is uses the render angles from
//			the SDK animation state to setup the hitboxes.
//-----------------------------------------------------------------------------
void CSDKPlayer::SetupBones( matrix3x4_t *pBoneToWorld, int boneMask )
{
	VPROF_BUDGET( "CSDKPlayer::SetupBones", VPROF_BUDGETGROUP_SERVER_ANIM );

	// Set the mdl cache semaphore.
	MDLCACHE_CRITICAL_SECTION();

	// Get the studio header.
	Assert( GetModelPtr() );

	CStudioHdr *pStudioHdr = GetModelPtr( );
	if ( !pStudioHdr )
		return;

	Vector pos[MAXSTUDIOBONES];
	Quaternion q[MAXSTUDIOBONES];

	// Adjust hit boxes based on IK driven offset.
	Vector adjOrigin = GetAbsOrigin() + Vector( 0, 0, m_flEstIkOffset );

	// FIXME: pass this into Studio_BuildMatrices to skip transforms
	CBoneBitList boneComputed;
	if ( m_pIk )
	{
		m_iIKCounter++;
		m_pIk->Init( pStudioHdr, GetAbsAngles(), adjOrigin, gpGlobals->curtime, m_iIKCounter, boneMask );
		GetSkeleton( pStudioHdr, pos, q, boneMask );

		m_pIk->UpdateTargets( pos, q, pBoneToWorld, boneComputed );
		CalculateIKLocks( gpGlobals->curtime );
		m_pIk->SolveDependencies( pos, q, pBoneToWorld, boneComputed );
	}
	else
	{
		GetSkeleton( pStudioHdr, pos, q, boneMask );
	}

	CBaseAnimating *pParent = dynamic_cast< CBaseAnimating* >( GetMoveParent() );
	if ( pParent )
	{
		// We're doing bone merging, so do special stuff here.
		CBoneCache *pParentCache = pParent->GetBoneCache();
		if ( pParentCache )
		{
			BuildMatricesWithBoneMerge( 
				pStudioHdr, 
				m_PlayerAnimState->GetRenderAngles(),
				adjOrigin, 
				pos, 
				q, 
				pBoneToWorld, 
				pParent, 
				pParentCache );

			return;
		}
	}

	Studio_BuildMatrices( 
		pStudioHdr, 
		m_PlayerAnimState->GetRenderAngles(),
		adjOrigin, 
		pos, 
		q, 
		-1,
		GetModelScale(),
		pBoneToWorld,
		boneMask );
}

CWeaponSDKBase* CSDKPlayer::GetActiveSDKWeapon() const
{
	return dynamic_cast< CWeaponSDKBase* >( GetActiveWeapon() );
}

void CSDKPlayer::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		switch (GetTeamNumber())
		{
		case SDK_TEAM_RED:
			vm->m_nSkin = 0;
			break;

		case SDK_TEAM_BLUE:
			vm->m_nSkin = 1;
			break;

		case SDK_TEAM_GREEN:
			vm->m_nSkin = 2;
			break;

		case SDK_TEAM_YELLOW:
			vm->m_nSkin = 3;
			break;
		}
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
		
		
		
	}
}

void CSDKPlayer::FlashlightTurnOn( void )
{
	if( flashlight.GetInt() > 0 && IsAlive() )
	{
		AddEffects( EF_DIMLIGHT );
		EmitSound( "Player.FlashlightOn" );
	}
}

void CSDKPlayer::FlashlightTurnOff( void )
{
	if( IsEffectActive(EF_DIMLIGHT) )
	{
		RemoveEffects( EF_DIMLIGHT );

		if( m_iHealth > 0 )
			EmitSound( "Player.FlashlightOff" );
	}
}

int CSDKPlayer::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

bool CSDKPlayer::ClientCommand( const CCommand &args )
{
	const char *pcmd = args[0];
	if ( FStrEq( pcmd, "jointeam" ) ) 
	{
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad jointeam syntax\n" );
		}

		int iTeam = atoi( args[1] );
		HandleCommand_JoinTeam( iTeam );
		return true;
	}
	else if( !Q_strncmp( pcmd, "cls_", 4 ) )
	{
		CSDKTeam *pTeam = GetGlobalSDKTeam( GetTeamNumber() );

		Assert( pTeam );

		int iClassIndex = PLAYERCLASS_UNDEFINED;

		if( pTeam->IsClassOnTeam( pcmd, iClassIndex ) )
		{
			HandleCommand_JoinClass( iClassIndex );
		}
		else
		{
			DevMsg( "player tried to join a class that isn't on this team ( %s )\n", pcmd );
			ShowClassSelectMenu();
		}

		return true;
	}
	else if ( FStrEq( pcmd, "spectate" ) )
	{
		// instantly join spectators
		HandleCommand_JoinTeam( TEAM_SPECTATOR );
		return true;
	}
	else if ( FStrEq( pcmd, "joingame" ) )
	{
		// player just closed MOTD dialog
		if ( m_iPlayerState == STATE_WELCOME )
		{
			State_Transition( STATE_PICKINGTEAM );
			//State_Transition( STATE_PICKINGCLASS );
		}
		
		return true;
	}
	else if ( FStrEq( pcmd, "joinclass" ) ) 
	{
		if ( args.ArgC() < 2 )
			Warning( "Player sent bad joinclass syntax\n" );

		int iClass = atoi( args[1] );
		HandleCommand_JoinClass( iClass );

		return true;
	}
	else if ( FStrEq( pcmd, "menuopen" ) )
	{
		SetClassMenuOpen( true );

		return true;
	}
	else if ( FStrEq( pcmd, "menuclosed" ) )
	{
		SetClassMenuOpen( false );
		return true;
	}

	return BaseClass::ClientCommand( args );
}

// returns true if the selection has been handled and the player's menu 
// can be closed...false if the menu should be displayed again
bool CSDKPlayer::HandleCommand_JoinTeam( int team )
{
	CSDKGameRules *mp = SDKGameRules();
	int iOldTeam = GetTeamNumber();
	if ( !GetGlobalTeam( team ) )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}

	// If we already died and changed teams once, deny
	if( m_bTeamChanged && team != TEAM_SPECTATOR && iOldTeam != TEAM_SPECTATOR )
	{
		ClientPrint( this, HUD_PRINTCENTER, "#game_switch_teams_once" );
		return true;
	}

	if ( team == TEAM_UNASSIGNED )
	{
		// Attempt to auto-select a team, may set team to T, CT or SPEC
		team = mp->SelectDefaultTeam();

		if ( team == TEAM_UNASSIGNED )
		{
			// still team unassigned, try to kick a bot if possible	
			 
			ClientPrint( this, HUD_PRINTTALK, "#All_Teams_Full" );

			team = TEAM_SPECTATOR;
		}
	}

	if ( team == iOldTeam )
		return true;	// we wouldn't change the team

	if (mp->TeamFull(team))
	{
		if (team == SDK_TEAM_BLUE)
		{
			ClientPrint(this, HUD_PRINTTALK, "#BlueTeam_Full");
		}
		else if (team == SDK_TEAM_RED)
		{
			ClientPrint(this, HUD_PRINTTALK, "#RedTeam_Full");
		}
		else if (team == SDK_TEAM_YELLOW)
		{
			ClientPrint(this, HUD_PRINTTALK, "#YellowTeam_Full");
		}
		else if (team == SDK_TEAM_GREEN)
		{
			ClientPrint(this, HUD_PRINTTALK, "#GreenTeam_Full");
		}
		ShowViewPortPanel(PANEL_TEAM);
		return false;
	}

	if ( team == TEAM_SPECTATOR )
	{
		// Prevent this if the cvar is set
		if ( !mp_allowspectators.GetInt() && !IsHLTV() )
		{
			ClientPrint( this, HUD_PRINTTALK, "#Cannot_Be_Spectator" );
			ShowViewPortPanel( PANEL_TEAM );
			return false;
		}

		ChangeTeam( TEAM_SPECTATOR );

		return true;
	}
	
	// If the code gets this far, the team is not TEAM_UNASSIGNED

	// Player is switching to a new team (It is possible to switch to the
	// same team just to choose a new appearance)
	if (mp->TeamStacked( team, GetTeamNumber() ))//players are allowed to change to their own team so they can just change their model
	{
		// The specified team is full
		ClientPrint( 
			this,
			HUD_PRINTCENTER,
			( team == SDK_TEAM_BLUE ) ? "#BlueTeam_full" : "#RedTeam_full" ? "#YellowTeam_full" : "#GreenTeam_full" );

		ShowViewPortPanel( PANEL_TEAM );
		return false;
	}

	// Switch their actual team...
	ChangeTeam( team );

	// Force them to choose a new class
	m_Shared.SetDesiredPlayerClass( PLAYERCLASS_UNDEFINED );
	m_Shared.SetPlayerClass( PLAYERCLASS_UNDEFINED );

	return true;
}

bool CSDKPlayer::HandleCommand_JoinClass( int iClass )
{
	Assert( GetTeamNumber() != TEAM_SPECTATOR );
	Assert( GetTeamNumber() != TEAM_UNASSIGNED );

	if( GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	if( iClass == PLAYERCLASS_UNDEFINED )
		return false;	//they typed in something weird

	int iOldPlayerClass = m_Shared.DesiredPlayerClass();

	// See if we're joining the class we already are
	if( iClass == iOldPlayerClass )
		return true;

	if( !SDKGameRules()->IsPlayerClassOnTeam( iClass, GetTeamNumber() ) )
		return false;

	const char *classname = SDKGameRules()->GetPlayerClassName( iClass, GetTeamNumber() );

	if( SDKGameRules()->CanPlayerJoinClass( this, iClass ) )
	{
		m_Shared.SetDesiredPlayerClass( iClass );	//real class value is set when the player spawns

//Tony; don't do this until we have a spawn timer!!
//		if( State_Get() == STATE_PICKINGCLASS )
//			State_Transition( STATE_OBSERVER_MODE );

		if( iClass == PLAYERCLASS_RANDOM )
		{
			if( IsAlive() )
			{
				ClientPrint(this, HUD_PRINTTALK, "#game_respawn_asrandom" );
			}
			else
			{
				ClientPrint(this, HUD_PRINTTALK, "#game_spawn_asrandom" );
			}
		}
		else
		{
			if( IsAlive() )
			{
				ClientPrint(this, HUD_PRINTTALK, "#game_respawn_as", classname );
			}
			else
			{
				ClientPrint(this, HUD_PRINTTALK, "#game_spawn_as", classname );
			}
		}

		IGameEvent * event = gameeventmanager->CreateEvent( "player_changeclass" );
		if ( event )
		{
			event->SetInt( "userid", GetUserID() );
			event->SetInt( "class", iClass );

			gameeventmanager->FireEvent( event );
		}
	}
	else
	{
		ClientPrint(this, HUD_PRINTTALK, "#game_class_limit", classname );
		ShowClassSelectMenu();
	}

	// Incase we don't get the class menu message before the spawn timer
	// comes up, fake that we've closed the menu.
	SetClassMenuOpen( false );

	//Tony; TODO; this is temp, I may integrate with the teamplayroundrules; If I do, there will be wavespawn too.
	if ( State_Get() == STATE_PICKINGCLASS /*|| IsDead()*/ )	//Tony; undone, don't transition if dead; only go into active state at this point if we were picking class.
		State_Transition( STATE_ACTIVE ); //Done picking stuff and we're in the pickingclass state, or dead, so we can spawn now.

	return true;
}

void CSDKPlayer::ShowClassSelectMenu()
{
	if ( GetTeamNumber() == SDK_TEAM_BLUE )
		ShowViewPortPanel( PANEL_CLASS_BLUE );
	else if ( GetTeamNumber() == SDK_TEAM_RED	)
		ShowViewPortPanel( PANEL_CLASS_RED );
	else if ( GetTeamNumber() == SDK_TEAM_YELLOW )
		ShowViewPortPanel( PANEL_CLASS_YELLOW );
	else if ( GetTeamNumber() == SDK_TEAM_GREEN )
		ShowViewPortPanel( PANEL_CLASS_GREEN );
}

void CSDKPlayer::SetClassMenuOpen( bool bOpen )
{
	m_bIsClassMenuOpen = bOpen;
}

bool CSDKPlayer::IsClassMenuOpen( void )
{
	return m_bIsClassMenuOpen;
}

// ------------------------------------------------------------------------------------------------ //
// Player state management.
// ------------------------------------------------------------------------------------------------ //
void CSDKPlayer::State_Transition( SDKPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}

void CSDKPlayer::State_Enter( SDKPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	if ( SDK_ShowStateTransitions.GetInt() == -1 || SDK_ShowStateTransitions.GetInt() == entindex() )
	{
		if ( m_pCurStateInfo )
			Msg( "ShowStateTransitions: entering '%s'\n", m_pCurStateInfo->m_pStateName );
		else
			Msg( "ShowStateTransitions: entering #%d\n", newState );
	}
	
	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CSDKPlayer::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CSDKPlayer::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CSDKPlayerStateInfo* CSDKPlayer::State_LookupInfo( SDKPlayerState state )
{
	// This table MUST match the 
	static CSDKPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CSDKPlayer::State_Enter_ACTIVE, NULL, &CSDKPlayer::State_PreThink_ACTIVE },
		{ STATE_WELCOME,		"STATE_WELCOME",		&CSDKPlayer::State_Enter_WELCOME, NULL, &CSDKPlayer::State_PreThink_WELCOME },
		{ STATE_PICKINGTEAM,	"STATE_PICKINGTEAM",	&CSDKPlayer::State_Enter_PICKINGTEAM, NULL,	&CSDKPlayer::State_PreThink_WELCOME },
		{ STATE_PICKINGCLASS,	"STATE_PICKINGCLASS",	&CSDKPlayer::State_Enter_PICKINGCLASS, NULL,	&CSDKPlayer::State_PreThink_WELCOME },
		{ STATE_DEATH_ANIM,		"STATE_DEATH_ANIM",		&CSDKPlayer::State_Enter_DEATH_ANIM,	NULL, &CSDKPlayer::State_PreThink_DEATH_ANIM },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CSDKPlayer::State_Enter_OBSERVER_MODE,	NULL, &CSDKPlayer::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}
void CSDKPlayer::PhysObjectSleep()
{
	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj )
		pObj->Sleep();
}


void CSDKPlayer::PhysObjectWake()
{
	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj )
		pObj->Wake();
}
void CSDKPlayer::State_Enter_WELCOME()
{
	// Important to set MOVETYPE_NONE or our physics object will fall while we're sitting at one of the intro cameras.
	SetMoveType( MOVETYPE_NONE );
	AddSolidFlags( FSOLID_NOT_SOLID );

	PhysObjectSleep();

	// Show info panel
	if ( IsBot() )
	{
		// If they want to auto join a team for debugging, pretend they clicked the button.
		CCommand args;
		args.Tokenize( "joingame" );
		ClientCommand( args );
	}
	else
	{
		const ConVar *hostname = cvar->FindVar( "hostname" );
		const char *title = (hostname) ? hostname->GetString() : "MESSAGE OF THE DAY";

		// open info panel on client showing MOTD:
		KeyValues *data = new KeyValues("data");
		data->SetString( "title", title );		// info panel title
		data->SetString( "type", "1" );			// show userdata from stringtable entry
		data->SetString( "msg",	"motd" );		// use this stringtable entry
		//data->SetString( "cmd", TEXTWINDOW_CMD_JOINGAME );// exec this command if panel closed
		CCommand args;
		args.Tokenize( "joingame" );
		ClientCommand( args );

		ShowViewPortPanel( PANEL_INFO, true, data );

		data->deleteThis();

	}	
}

void CSDKPlayer::MoveToNextIntroCamera()
{
	m_pIntroCamera = gEntList.FindEntityByClassname( m_pIntroCamera, "point_viewcontrol" );

	// if m_pIntroCamera is NULL we just were at end of list, start searching from start again
	if(!m_pIntroCamera)
		m_pIntroCamera = gEntList.FindEntityByClassname(m_pIntroCamera, "point_viewcontrol");

	// find the target
	CBaseEntity *Target = NULL;
	
	if( m_pIntroCamera )
	{
		Target = gEntList.FindEntityByName( NULL, STRING(m_pIntroCamera->m_target) );
	}

	// if we still couldn't find a camera, goto T spawn
	if(!m_pIntroCamera)
		m_pIntroCamera = gEntList.FindEntityByClassname(m_pIntroCamera, "info_player_terrorist");

	SetViewOffset( vec3_origin );	// no view offset
	UTIL_SetSize( this, vec3_origin, vec3_origin ); // no bbox

	if( !Target ) //if there are no cameras(or the camera has no target, find a spawn point and black out the screen
	{
		if ( m_pIntroCamera.IsValid() )
			SetAbsOrigin( m_pIntroCamera->GetAbsOrigin() + VEC_VIEW );

		SetAbsAngles( QAngle( 0, 0, 0 ) );
		
		m_pIntroCamera = NULL;  // never update again
		return;
	}
	

	Vector vCamera = Target->GetAbsOrigin() - m_pIntroCamera->GetAbsOrigin();
	Vector vIntroCamera = m_pIntroCamera->GetAbsOrigin();
	
	VectorNormalize( vCamera );
		
	QAngle CamAngles;
	VectorAngles( vCamera, CamAngles );

	SetAbsOrigin( vIntroCamera );
	SetAbsAngles( CamAngles );
	SnapEyeAngles( CamAngles );
	m_fIntroCamTime = gpGlobals->curtime + 6;
}

void CSDKPlayer::State_PreThink_WELCOME()
{
	// Update whatever intro camera it's at.
	if( m_pIntroCamera && (gpGlobals->curtime >= m_fIntroCamTime) )
	{
		MoveToNextIntroCamera();
	}
}

void CSDKPlayer::State_Enter_DEATH_ANIM()
{
	if ( HasWeapons() )
	{
		// we drop the guns here because weapons that have an area effect and can kill their user
		// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
		// player class sometimes is freed. It's safer to manipulate the weapons once we know
		// we aren't calling into any of their code anymore through the player pointer.
		PackDeadPlayerItems();
	}

	// Used for a timer.
	m_flDeathTime = gpGlobals->curtime;

	StartObserverMode( OBS_MODE_DEATHCAM );	// go to observer mode

	RemoveEffects( EF_NODRAW );	// still draw player body
}


void CSDKPlayer::State_PreThink_DEATH_ANIM()
{
	// If the anim is done playing, go to the next state (waiting for a keypress to 
	// either respawn the guy or put him into observer mode).
	if ( GetFlags() & FL_ONGROUND )
	{
		float flForward = GetAbsVelocity().Length() - 20;
		if (flForward <= 0)
		{
			SetAbsVelocity( vec3_origin );
		}
		else
		{
			Vector vAbsVel = GetAbsVelocity();
			VectorNormalize( vAbsVel );
			vAbsVel *= flForward;
			SetAbsVelocity( vAbsVel );
		}
	}

	if ( gpGlobals->curtime >= (m_flDeathTime + SDK_PLAYER_DEATH_TIME ) )	// let the death cam stay going up to min spawn time.
	{
		m_lifeState = LIFE_DEAD;

		StopAnimation();

		AddEffects( EF_NOINTERP );

		if ( GetMoveType() != MOVETYPE_NONE && (GetFlags() & FL_ONGROUND) )
			SetMoveType( MOVETYPE_NONE );
	}

	//Tony; if we're now dead, and not changing classes, spawn
	if ( m_lifeState == LIFE_DEAD )
	{
		//Tony; if the class menu is open, don't respawn them, wait till they're done.
		if (IsClassMenuOpen())
			return;

		State_Transition( STATE_ACTIVE );
	}
}

void CSDKPlayer::State_Enter_OBSERVER_MODE()
{
	// Always start a spectator session in roaming mode
	m_iObserverLastMode = OBS_MODE_ROAMING;

	if( m_hObserverTarget == NULL )
	{
		// find a new observer target
		CheckObserverSettings();
	}

	// Change our observer target to the nearest teammate
	CTeam *pTeam = GetGlobalTeam( GetTeamNumber() );

	CBasePlayer *pPlayer;
	Vector localOrigin = GetAbsOrigin();
	Vector targetOrigin;
	float flMinDist = FLT_MAX;
	float flDist;

	for ( int i=0;i<pTeam->GetNumPlayers();i++ )
	{
		pPlayer = pTeam->GetPlayer(i);

		if ( !pPlayer )
			continue;

		if ( !IsValidObserverTarget(pPlayer) )
			continue;

		targetOrigin = pPlayer->GetAbsOrigin();

		flDist = ( targetOrigin - localOrigin ).Length();

		if ( flDist < flMinDist )
		{
			m_hObserverTarget.Set( pPlayer );
			flMinDist = flDist;
		}
	}

	StartObserverMode( m_iObserverLastMode );
	PhysObjectSleep();
}

void CSDKPlayer::State_PreThink_OBSERVER_MODE()
{

	//Tony; if we're in eye, or chase, validate the target - if it's invalid, find a new one, or go back to roaming
	if (  m_iObserverMode == OBS_MODE_IN_EYE || m_iObserverMode == OBS_MODE_CHASE )
	{
		//Tony; if they're not on a spectating team use the cbaseplayer validation method.
		if ( GetTeamNumber() != TEAM_SPECTATOR )
			ValidateCurrentObserverTarget();
		else
		{
			if ( !IsValidObserverTarget( m_hObserverTarget.Get() ) )
			{
				// our target is not valid, try to find new target
				CBaseEntity * target = FindNextObserverTarget( false );
				if ( target )
				{
					// switch to new valid target
					SetObserverTarget( target );	
				}
				else
				{
					// let player roam around
					ForceObserverMode( OBS_MODE_ROAMING );
				}
			}
		}
	}
}

void CSDKPlayer::State_Enter_PICKINGCLASS()
{
	ShowClassSelectMenu();
	PhysObjectSleep();

}

void CSDKPlayer::State_Enter_PICKINGTEAM()
{
	ShowViewPortPanel( PANEL_TEAM );
	PhysObjectSleep();

}

void CSDKPlayer::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	RemoveSolidFlags( FSOLID_NOT_SOLID );
	m_Local.m_iHideHUD = 0;
	PhysObjectWake();

	//Tony; call spawn again now -- remember; when we add respawn timers etc, to just put them into the spawn queue, and let the queue respawn them.
	Spawn();
}

void CSDKPlayer::State_PreThink_ACTIVE()
{
}

int CSDKPlayer::GetPlayerStance()
{
	if (m_Local.m_bDucking)
		return PINFO_STANCE_DUCKING;
	else
		return PINFO_STANCE_STANDING;
}

void CSDKPlayer::NoteWeaponFired( void )
{
	Assert( m_pCurrentCommand );
	if( m_pCurrentCommand )
	{
		m_iLastWeaponFireUsercmd = m_pCurrentCommand->command_number;
	}
}

bool CSDKPlayer::WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// No need to lag compensate at all if we're not attacking in this command and
	// we haven't attacked recently.
	if ( !( pCmd->buttons & IN_ATTACK ) && (pCmd->command_number - m_iLastWeaponFireUsercmd > 5) )
		return false;

	return BaseClass::WantsLagCompensationOnEntity( pPlayer, pCmd, pEntityTransmitBits );
}

void CSDKPlayer::IncrementHealthValue( int nCount )
{ 
	m_iHealth += nCount;
	if ( m_iMaxHealth > 0 && m_iHealth > m_iMaxHealth )
		m_iHealth = m_iMaxHealth;
}

void CSDKPlayer::SetHealth( int value ) 
{ 
	m_iHealth = value; 
}

void CSDKPlayer::SetMaxHealth( int MaxValue ) 
{ 
	m_iMaxHealth = MaxValue; 
}

void CSDKPlayer::AdjustHealth( int HealthValue ) 
{
	m_iHealth = m_iMaxHealth = HealthValue;
}

void CSDKPlayer::IncrementArmorValue( int nCount, int nMaxValue )
{ 
	nMaxValue = m_MaxArmorValue;
	if ( nMaxValue > 0 )
	{
		if ( m_MaxArmorValue > nMaxValue )
			m_MaxArmorValue = nMaxValue;
	}
}	

void CSDKPlayer::SetArmorValue( int value )
{
	m_ArmorValue = value;
}

void CSDKPlayer::SetMaxArmorValue( int MaxArmorValue )
{
	m_MaxArmorValue = MaxArmorValue;
}

void CSDKPlayer::AdjustArmor( int ArmorValue )
{
	m_ArmorValue = m_MaxArmorValue = ArmorValue;
}
