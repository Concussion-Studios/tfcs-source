//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_sdk_player.h"
#include "c_user_message_register.h"
#include "weapon_sdkbase.h"
#include "c_basetempentity.h"
#include "iclientvehicle.h"
#include "prediction.h"
#include "view.h"
#include "iviewrender.h"
#include "ivieweffects.h"
#include "view_scene.h"
#include "fx.h"
#include "collisionutils.h"
#include "c_sdk_team.h"
#include "obstacle_pushaway.h"
#include "bone_setup.h"
#include "cl_animevent.h"
#include "soundenvelope.h"
#include "in_buttons.h"
#include "r_efx.h"
#include "dlight.h"
#include "cam_thirdperson.h"
#include <vgui_controls/Image.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/IScheme.h>
#include "vcollide_parse.h"
#include "iviewrender_beams.h"			// flashlight beam
#include "input.h"
#include "c_team.h"
#include "tfc_viewmodel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if defined( CSDKPlayer )
	#undef CSDKPlayer
#endif

extern ConVar cl_forwardspeed;
extern ConVar cl_backspeed;
extern ConVar cl_sidespeed;
extern ConVar cl_righthand;

ConVar sdk_max_separation_force("sdk_max_separation_force", "256", FCVAR_CHEAT | FCVAR_HIDDEN);

ConVar cl_hl1_rollspeed( "cl_hl1_rollspeed", "300.0", FCVAR_USERINFO | FCVAR_ARCHIVE ); // 300.0
ConVar cl_hl1_rollangle( "cl_hl1_rollangle", "0.65", FCVAR_USERINFO | FCVAR_ARCHIVE ); // 0.65

ConVar cl_hl1_bobcycle( "cl_hl1_bobcycle", "0.8", FCVAR_USERINFO | FCVAR_ARCHIVE );
ConVar cl_hl1_bob( "cl_hl1_bob", "0.01", FCVAR_USERINFO | FCVAR_ARCHIVE );
ConVar cl_hl1_bobup( "cl_hl1_bobup", "0.5", FCVAR_USERINFO | FCVAR_ARCHIVE );

ConVar cl_hl1_iyaw_cycle( "cl_hl1_iyaw_cycle", "2.0", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );
ConVar cl_hl1_iroll_cycle( "cl_hl1_iroll_cycle", "0.5", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );
ConVar cl_hl1_ipitch_cycle( "cl_hl1_ipitch_cycle", "1.0", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );
ConVar cl_hl1_iyaw_level( "cl_hl1_iyaw_level", "0.3", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );
ConVar cl_hl1_iroll_level( "cl_hl1_iroll_level", "0.1", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );
ConVar cl_hl1_ipitch_level( "cl_hl1_ipitch_level", "0.3", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );

ConVar cl_thirdperson_dist( "cl_thirdperson_dist", "64", FCVAR_NOT_CONNECTED | FCVAR_USERINFO | FCVAR_ARCHIVE, "Third person distance."  );
ConVar cl_thirdperson_right( "cl_thirdperson_right", "30", FCVAR_NOT_CONNECTED | FCVAR_USERINFO | FCVAR_ARCHIVE, "Third person distance right."  );
ConVar cl_thirdperson_up( "cl_thirdperson_up", "0", FCVAR_NOT_CONNECTED | FCVAR_USERINFO | FCVAR_ARCHIVE, "Third person distance up."  );

vgui::IImage* GetDefaultAvatarImage( C_BasePlayer *pPlayer )
{
	if ( pPlayer )
	{
		switch ( pPlayer->GetTeamNumber() )
		{
			case SDK_TEAM_RED:
			{
				static vgui::IImage *pRedAvatar = vgui::scheme()->GetImage( "../vgui/avatar_default_red", true );
				return pRedAvatar;
			}
			case SDK_TEAM_BLUE:
			{
				static vgui::IImage *pBlueAvatar = vgui::scheme()->GetImage( "../vgui/avatar_default_blue", true );
				return pBlueAvatar;
			}
			case SDK_TEAM_GREEN:
			{
				static vgui::IImage *pGreenAvatar = vgui::scheme()->GetImage( "../vgui/avatar_default_green", true );
				return pGreenAvatar;
			}
			case SDK_TEAM_YELLOW:
			{
				static vgui::IImage *pYellowAvatar = vgui::scheme()->GetImage( "../vgui/avatar_default_yellow", true );
				return pYellowAvatar;
			}
		}
	}

	return NULL;
}

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //
class C_TEPlayerAnimEvent : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerAnimEvent, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void PostDataUpdate( DataUpdateType_t updateType )
	{
		// Create the effect.
		C_SDKPlayer *pPlayer = dynamic_cast< C_SDKPlayer* >( m_hPlayer.Get() );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			pPlayer->DoAnimationEvent( (PlayerAnimEvent_t)m_iEvent.Get(), m_nData );
		}	
	}

public:
	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_CLIENTCLASS_EVENT( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent, CTEPlayerAnimEvent );

BEGIN_RECV_TABLE_NOBASE( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_iEvent ) ),
	RecvPropInt( RECVINFO( m_nData ) )
END_RECV_TABLE()

void __MsgFunc_ReloadEffect( bf_read &msg )
{
	int iPlayer = msg.ReadShort();
	C_SDKPlayer *pPlayer = dynamic_cast< C_SDKPlayer* >( C_BaseEntity::Instance( iPlayer ) );
	if ( pPlayer )
		pPlayer->PlayReloadEffect();

}
USER_MESSAGE_REGISTER( ReloadEffect );

BEGIN_RECV_TABLE_NOBASE( C_SDKPlayer, DT_SDKLocalPlayerExclusive )
	RecvPropInt( RECVINFO( m_iShotsFired ) ),
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),

	RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
//	RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),
	RecvPropInt( RECVINFO( m_ArmorValue ) ),
	RecvPropInt( RECVINFO( m_MaxArmorValue ) ),
END_RECV_TABLE()

BEGIN_RECV_TABLE_NOBASE( C_SDKPlayer, DT_SDKNonLocalPlayerExclusive )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),

	RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
	RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),
END_RECV_TABLE()

// main table
IMPLEMENT_CLIENTCLASS_DT( C_SDKPlayer, DT_SDKPlayer, CSDKPlayer )
	RecvPropDataTable( RECVINFO_DT( m_Shared ), 0, &REFERENCE_RECV_TABLE( DT_SDKPlayerShared ) ),

	RecvPropDataTable( "sdklocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_SDKLocalPlayerExclusive) ),
	RecvPropDataTable( "sdknonlocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_SDKNonLocalPlayerExclusive) ),

	RecvPropEHandle( RECVINFO( m_hRagdoll ) ),

	RecvPropInt( RECVINFO( m_iPlayerState ) ),

	RecvPropBool( RECVINFO( m_bSpawnInterpCounter ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_SDKPlayer )
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_iShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),   
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( player, C_SDKPlayer );

class C_SDKRagdoll : public C_BaseAnimatingOverlay
{
public:
	DECLARE_CLASS( C_SDKRagdoll, C_BaseAnimatingOverlay );
	DECLARE_CLIENTCLASS();
	
	C_SDKRagdoll();
	~C_SDKRagdoll();

	virtual void OnDataChanged( DataUpdateType_t type );

	int GetPlayerEntIndex() const;
	IRagdoll* GetIRagdoll() const;

	void ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName );
	void UpdateOnRemove( void );
	virtual void SetupWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights );
	
private:
	
	C_SDKRagdoll( const C_SDKRagdoll & ) {}

	void Interp_Copy( C_BaseAnimatingOverlay *pDestinationEntity );
	void CreateSDKRagdoll( void );

private:

	EHANDLE	m_hPlayer;
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_SDKRagdoll, DT_SDKRagdoll, CSDKRagdoll )
	RecvPropVector( RECVINFO(m_vecRagdollOrigin) ),
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_nModelIndex ) ),
	RecvPropInt( RECVINFO(m_nForceBone) ),
	RecvPropVector( RECVINFO(m_vecForce) ),
	RecvPropVector( RECVINFO( m_vecRagdollVelocity ) )
END_RECV_TABLE()

C_SDKRagdoll::C_SDKRagdoll()
{

}

C_SDKRagdoll::~C_SDKRagdoll()
{
	PhysCleanupFrictionSounds( this );

	if ( m_hPlayer )
		m_hPlayer->CreateModelInstance();
}

void C_SDKRagdoll::Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity )
{
	if ( !pSourceEntity )
		return;
	
	VarMapping_t *pSrc = pSourceEntity->GetVarMapping();
	VarMapping_t *pDest = GetVarMapping();
		
	// Find all the VarMapEntry_t's that represent the same variable.
	for ( int i = 0; i < pDest->m_Entries.Count(); i++ )
	{
		VarMapEntry_t *pDestEntry = &pDest->m_Entries[i];
		const char *pszName = pDestEntry->watcher->GetDebugName();
		for ( int j=0; j < pSrc->m_Entries.Count(); j++ )
		{
			VarMapEntry_t *pSrcEntry = &pSrc->m_Entries[j];
			if ( !Q_strcmp( pSrcEntry->watcher->GetDebugName(), pszName ) )
			{
				pDestEntry->watcher->Copy( pSrcEntry->watcher );
				break;
			}
		}
	}
}

void FX_BloodSpray( const Vector &origin, const Vector &normal, float scale, unsigned char r, unsigned char g, unsigned char b, int flags );
void C_SDKRagdoll::ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if( !pPhysicsObject )
		return;

	Vector dir = pTrace->endpos - pTrace->startpos;

	if ( iDamageType == DMG_BLAST )
	{
		dir *= 4000;  // adjust impact strenght
				
		// apply force at object mass center
		pPhysicsObject->ApplyForceCenter( dir );
	}
	else
	{
		Vector hitpos;  

		VectorMA( pTrace->startpos, pTrace->fraction, dir, hitpos );
		VectorNormalize( dir );

		// Blood spray!
		FX_BloodSpray( hitpos, dir, 3, 72, 0, 0, FX_BLOODSPRAY_ALL  );

		dir *= 4000;  // adjust impact strenght

		// apply force where we hit it
		pPhysicsObject->ApplyForceOffset( dir, hitpos );	
		//Tony; throw in some bleeds! - just use a generic value for damage.
		TraceBleed( 40, dir, pTrace, iDamageType );
	}

	m_pRagdoll->ResetRagdollSleepAfterTime();
}


void C_SDKRagdoll::CreateSDKRagdoll( void )
{
	// First, initialize all our data. If we have the player's entity on our client,
	// then we can make ourselves start out exactly where the player is.
	C_SDKPlayer *pPlayer = dynamic_cast< C_SDKPlayer* >( m_hPlayer.Get() );
	
	if ( pPlayer && !pPlayer->IsDormant() )
	{
		// move my current model instance to the ragdoll's so decals are preserved.
		pPlayer->SnatchModelInstance( this );

		VarMapping_t *varMap = GetVarMapping();

		// Copy all the interpolated vars from the player entity.
		// The entity uses the interpolated history to get bone velocity.
		bool bRemotePlayer = (pPlayer != C_BasePlayer::GetLocalPlayer());			
		if ( bRemotePlayer )
		{
			Interp_Copy( pPlayer );

			SetAbsAngles( pPlayer->GetRenderAngles() );
			GetRotationInterpolator().Reset();

			m_flAnimTime = pPlayer->m_flAnimTime;
			SetSequence( pPlayer->GetSequence() );
			m_flPlaybackRate = pPlayer->GetPlaybackRate();
		}
		else
		{
			// This is the local player, so set them in a default
			// pose and slam their velocity, angles and origin
			SetAbsOrigin( m_vecRagdollOrigin );
			
			SetAbsAngles( pPlayer->GetRenderAngles() );

			SetAbsVelocity( m_vecRagdollVelocity );

			int iSeq = pPlayer->GetSequence();
			if ( iSeq == -1 )
			{
				Assert( false );	// missing walk_lower?
				iSeq = 0;
			}
			
			SetSequence( iSeq );	// walk_lower, basic pose
			SetCycle( 0.0 );

			Interp_Reset( varMap );
		}		
	}
	else
	{
		// overwrite network origin so later interpolation will
		// use this position
		SetNetworkOrigin( m_vecRagdollOrigin );

		SetAbsOrigin( m_vecRagdollOrigin );
		SetAbsVelocity( m_vecRagdollVelocity );

		Interp_Reset( GetVarMapping() );
		
	}

	SetModelIndex( m_nModelIndex );

	// Make us a ragdoll..
	m_nRenderFX = kRenderFxRagdoll;

	matrix3x4_t boneDelta0[MAXSTUDIOBONES];
	matrix3x4_t boneDelta1[MAXSTUDIOBONES];
	matrix3x4_t currentBones[MAXSTUDIOBONES];
	const float boneDt = 0.05f;

	if ( pPlayer && !pPlayer->IsDormant() )
		pPlayer->GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
	else
		GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );

	InitAsClientRagdoll( boneDelta0, boneDelta1, currentBones, boneDt );
}


void C_SDKRagdoll::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
		CreateSDKRagdoll();
}

IRagdoll* C_SDKRagdoll::GetIRagdoll() const
{
	return m_pRagdoll;
}

void C_SDKRagdoll::UpdateOnRemove( void )
{
	VPhysicsSetObject( NULL );

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: clear out any face/eye values stored in the material system
//-----------------------------------------------------------------------------
void C_SDKRagdoll::SetupWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights )
{
	BaseClass::SetupWeights( pBoneToWorld, nFlexWeightCount, pFlexWeights, pFlexDelayedWeights );

	static float destweight[128];
	static bool bIsInited = false;

	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
		return;

	int nFlexDescCount = hdr->numflexdesc();
	if ( nFlexDescCount )
	{
		Assert( !pFlexDelayedWeights );
		memset( pFlexWeights, 0, nFlexWeightCount * sizeof(float) );
	}

	if ( m_iEyeAttachment > 0 )
	{
		matrix3x4_t attToWorld;
		if ( GetAttachment( m_iEyeAttachment, attToWorld ) )
		{
			Vector local, tmp;
			local.Init( 1000.0f, 0.0f, 0.0f );
			VectorTransform( local, attToWorld, tmp );
			modelrender->SetViewTarget( GetModelPtr(), GetBody(), tmp );
		}
	}
}

void C_SDKPlayer::ItemPreFrame( void )
{
	if ( GetFlags() & FL_FROZEN )
		 return;

	BaseClass::ItemPreFrame();
}
	
void C_SDKPlayer::ItemPostFrame( void )
{
	if ( GetFlags() & FL_FROZEN )
		 return;

	BaseClass::ItemPostFrame();
}

C_BaseAnimating *C_SDKPlayer::BecomeRagdollOnClient()
{
	// Let the C_SDKRagdoll entity do this.
	return NULL;
}

IRagdoll* C_SDKPlayer::GetRepresentativeRagdoll() const
{
	if ( m_hRagdoll.Get() )
	{
		C_SDKRagdoll *pRagdoll = (C_SDKRagdoll*)m_hRagdoll.Get();

		return pRagdoll->GetIRagdoll();
	}
	else
		return NULL;
}

C_SDKPlayer::C_SDKPlayer() : m_iv_angEyeAngles( "C_SDKPlayer::m_iv_angEyeAngles" )
{
	m_PlayerAnimState = CreateSDKPlayerAnimState( this );
	m_Shared.Init(this);

	m_angEyeAngles.Init();
	AddVar( &m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR );

	m_fNextThinkPushAway = 0.0f;

	m_pFlashlightBeam = NULL;
	
	ConVarRef scissor( "r_flashlightscissor" );
	scissor.SetValue( "0" );
}

C_SDKPlayer::~C_SDKPlayer()
{
	ReleaseFlashlight();

	m_PlayerAnimState->Release();
}

C_SDKPlayer* C_SDKPlayer::GetLocalSDKPlayer()
{
	return ToSDKPlayer( C_BasePlayer::GetLocalPlayer() );
}

const QAngle &C_SDKPlayer::EyeAngles()
{
	if( IsLocalPlayer() )
		return BaseClass::EyeAngles();
	else
		return m_angEyeAngles;

}

const QAngle& C_SDKPlayer::GetRenderAngles()
{
	if ( IsRagdoll() )
		return vec3_angle;
	else
		return m_PlayerAnimState->GetRenderAngles();
}

void C_SDKPlayer::ProcessMuzzleFlashEvent()
{
	if ( this != C_BasePlayer::GetLocalPlayer() )
	{
		Vector vAttachment;
		QAngle dummyAngles;
		
		C_WeaponSDKBase *pWeapon = m_Shared.GetActiveSDKWeapon();		
		if ( pWeapon )
		{
			int iAttachment = pWeapon->LookupAttachment( "muzzle_flash" );
			if ( iAttachment > 0 )
			{
				float flScale = 1;
				pWeapon->GetAttachment( iAttachment, vAttachment, dummyAngles );
				
				// The way the models are setup, the up vector points along the barrel.
				Vector vForward, vRight, vUp;
				AngleVectors( dummyAngles, &vForward, &vRight, &vUp );
				VectorAngles( vUp, dummyAngles );

				FX_MuzzleEffect( vAttachment, dummyAngles, flScale, INVALID_EHANDLE_INDEX, NULL, true );
			}
		}
	}

	Vector vAttachment;
	QAngle dummyAngles;
	
	bool bFoundAttachment = GetAttachment( 1, vAttachment, dummyAngles );

	// If we have an attachment, then stick a light on it.
	if ( bFoundAttachment )
	{
		dlight_t *el = effects->CL_AllocDlight( LIGHT_INDEX_MUZZLEFLASH + index );
		el->origin = vAttachment;
		el->radius = 24; 
		el->decay = el->radius / 0.05f;
		el->die = gpGlobals->curtime + 0.05f;
		el->color.r = 255;
		el->color.g = 192;
		el->color.b = 64;
		el->color.exponent = 5;
	}
}

void C_SDKPlayer::AddEntity( void )
{
	BaseClass::AddEntity();

	//Tony; modified so third person can do the beam too.
	if ( IsEffectActive( EF_DIMLIGHT ) )
	{
		//Tony; if local player, not in third person, and there's a beam, destroy it. It will get re-created if they go third person again.
		if ( this == C_BasePlayer::GetLocalPlayer() && !::input->CAM_IsThirdPerson() && m_pFlashlightBeam != NULL )
		{
			ReleaseFlashlight();
			return;
		}
		else if( this != C_BasePlayer::GetLocalPlayer() || ::input->CAM_IsThirdPerson() )
		{
			int iAttachment = LookupAttachment( "muzzle_flash" );
			if ( iAttachment < 0 )
				return;

			Vector vecOrigin;
			//Tony; EyeAngles will return proper whether it's local player or not.
			QAngle eyeAngles = EyeAngles();

			GetAttachment( iAttachment, vecOrigin, eyeAngles );

			Vector vForward;
			AngleVectors( eyeAngles, &vForward );

			trace_t tr;
			UTIL_TraceLine( vecOrigin, vecOrigin + (vForward * 200), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

			if( !m_pFlashlightBeam )
			{
				BeamInfo_t beamInfo;
				beamInfo.m_nType = TE_BEAMPOINTS;
				beamInfo.m_vecStart = tr.startpos;
				beamInfo.m_vecEnd = tr.endpos;
				beamInfo.m_pszModelName = "sprites/glow01.vmt";
				beamInfo.m_pszHaloName = "sprites/glow01.vmt";
				beamInfo.m_flHaloScale = 3.0;
				beamInfo.m_flWidth = 8.0f;
				beamInfo.m_flEndWidth = 35.0f;
				beamInfo.m_flFadeLength = 300.0f;
				beamInfo.m_flAmplitude = 0;
				beamInfo.m_flBrightness = 60.0;
				beamInfo.m_flSpeed = 0.0f;
				beamInfo.m_nStartFrame = 0.0;
				beamInfo.m_flFrameRate = 0.0;
				beamInfo.m_flRed = 255.0;
				beamInfo.m_flGreen = 255.0;
				beamInfo.m_flBlue = 255.0;
				beamInfo.m_nSegments = 8;
				beamInfo.m_bRenderable = true;
				beamInfo.m_flLife = 0.5;
				beamInfo.m_nFlags = FBEAM_FOREVER | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

				m_pFlashlightBeam = beams->CreateBeamPoints( beamInfo );
			}

			if( m_pFlashlightBeam )
			{
				BeamInfo_t beamInfo;
				beamInfo.m_vecStart = tr.startpos;
				beamInfo.m_vecEnd = tr.endpos;
				beamInfo.m_flRed = 255.0;
				beamInfo.m_flGreen = 255.0;
				beamInfo.m_flBlue = 255.0;

				beams->UpdateBeamInfo( m_pFlashlightBeam, beamInfo );

				//Tony; local players don't make the dlight.
				if( this != C_BasePlayer::GetLocalPlayer() )
				{
					dlight_t *el = effects->CL_AllocDlight( 0 );
					el->origin = tr.endpos;
					el->radius = 50; 
					el->color.r = 200;
					el->color.g = 200;
					el->color.b = 200;
					el->die = gpGlobals->curtime + 0.1;
				}
			}
		}
	}
	else if ( m_pFlashlightBeam )
	{
		ReleaseFlashlight();
	}
}

void C_SDKPlayer::ReleaseFlashlight( void )
{
	if( m_pFlashlightBeam )
	{
		m_pFlashlightBeam->flags = 0;
		m_pFlashlightBeam->die = gpGlobals->curtime - 1;

		m_pFlashlightBeam = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates, destroys, and updates the flashlight effect as needed.
//-----------------------------------------------------------------------------
void C_SDKPlayer::UpdateFlashlight()
{
	// The dim light is the flashlight.
	if ( IsEffectActive( EF_DIMLIGHT ) )
	{
		if ( !m_pSDKFlashLightEffect )
		{
			// Turned on the headlight; create it.
			m_pSDKFlashLightEffect = new CSDKFlashlightEffect(index);

			if (!m_pSDKFlashLightEffect)
				return;

			m_pSDKFlashLightEffect->TurnOn();
		}

		Vector vecForward, vecRight, vecUp;
		Vector position = EyePosition();

		if ( ::input->CAM_IsThirdPerson() )
		{
			int iAttachment = LookupAttachment( "muzzle_flash" );
			if ( iAttachment >= 0 )
			{
				Vector vecOrigin;
				//Tony; EyeAngles will return proper whether it's local player or not.
				QAngle eyeAngles = EyeAngles();

				GetAttachment( iAttachment, vecOrigin, eyeAngles );

				Vector vForward;
				AngleVectors( eyeAngles, &vecForward, &vecRight, &vecUp );
				position = vecOrigin;
			}
			else
				EyeVectors( &vecForward, &vecRight, &vecUp );
		}
		else
			EyeVectors( &vecForward, &vecRight, &vecUp );


		// Update the light with the new position and direction.		
		m_pSDKFlashLightEffect->UpdateLight( position, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE );
	}
	else if ( m_pSDKFlashLightEffect )
	{
		// Turned off the flashlight; delete it.
		delete m_pSDKFlashLightEffect;
		m_pSDKFlashLightEffect = NULL;
	}
}

void CSDKFlashlightEffect::UpdateLight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance )
{
	CFlashlightEffect::UpdateLight( vecPos, vecDir, vecRight, vecUp, nDistance );
}

//=========================================================
// Autoaim
// set crosshair position to point to enemey
//=========================================================
Vector C_SDKPlayer::GetAutoaimVector( float flDelta )
{
	// Never autoaim a predicted weapon (for now)
	Vector	forward;
	AngleVectors( EyeAngles() + m_Local.m_vecPunchAngle, &forward );
	return	forward;
}

void C_SDKPlayer::UpdateClientSideAnimation()
{
	m_PlayerAnimState->Update( EyeAngles()[YAW], EyeAngles()[PITCH] );
	BaseClass::UpdateClientSideAnimation();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_SDKPlayer::ThirdPersonSwitch( bool bThirdPerson )
{
	BaseClass::ThirdPersonSwitch( bThirdPerson );

	if ( bThirdPerson )
	{
		if ( g_ThirdPersonManager.WantToUseGameThirdPerson() )
		{
			Vector vecOffset( cl_thirdperson_dist.GetInt(), cl_thirdperson_right.GetInt(), cl_thirdperson_up.GetInt() );

			// Flip the angle if viewmodels are flipped.
			if ( cl_righthand.GetBool() )
				vecOffset.y *= -1.0f;

			g_ThirdPersonManager.SetDesiredCameraOffset( vecOffset );
		}
	}

	if ( GetViewModel() )
		GetViewModel()->UpdateVisibility();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_SDKPlayer::OnSpawn()
{
	if ( IsLocalPlayer() )
	{
		// By default display hands.
		// This hack has to be here because SetWeaponVisible isn't called on client when the player spawns.
		C_TFCViewModel* pHands = static_cast< C_TFCViewModel* >( GetViewModel( VMINDEX_HANDS, false ) );
		if ( pHands )
			pHands->SetDrawVM( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_SDKPlayer::TeamChange( int iNewTeam )
{
	BaseClass::TeamChange( iNewTeam );

	// The team number hasn't been updated yet.
	int iOldTeam = GetTeamNumber();
	C_BaseEntity::ChangeTeam( iNewTeam );

	// Reset back to old team just in case something uses it.
	C_BaseEntity::ChangeTeam( iOldTeam );

	TeamChangeStatic( iNewTeam );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_SDKPlayer::TeamChangeStatic( int iNewTeam )
{
	// It's possible to receive events from the server before our local player is created.
	// All crucial things that don't rely on local player
	// should be put here.
	const char *pTeamConfig = "exec team_red.cfg";

	switch ( iNewTeam )
	{
	case SDK_TEAM_RED:
		pTeamConfig = "exec team_red.cfg";
		break;
	case SDK_TEAM_BLUE:
		pTeamConfig = "exec team_blue.cfg";
		break;
	case SDK_TEAM_GREEN:
		pTeamConfig = "exec team_green.cfg";
		break;
	case SDK_TEAM_YELLOW:
		pTeamConfig = "exec team_yellow.cfg";
		break;
	}

	if ( !( iNewTeam == TEAM_UNASSIGNED || iNewTeam == TEAM_SPECTATOR ) )
		engine->ClientCmd_Unrestricted( pTeamConfig );

}

//-----------------------------------------------------------------------------
// Purpose: Should this object cast shadows?
//-----------------------------------------------------------------------------
ShadowType_t C_SDKPlayer::ShadowCastType()
{
	return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
}

//-----------------------------------------------------------------------------
// Should this object receive shadows?
//-----------------------------------------------------------------------------
bool C_SDKPlayer::ShouldReceiveProjectedTextures( int flags )
{
	if ( IsEffectActive( EF_NODRAW ) )
		 return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CStudioHdr *C_SDKPlayer::OnNewModel( void )
{
	CStudioHdr *hdr = BaseClass::OnNewModel();

	InitializePoseParams();

	// Reset the players animation states, gestures
	if ( m_PlayerAnimState )
	{
		m_PlayerAnimState->OnNewModel();
	}

	return hdr;
}
//-----------------------------------------------------------------------------
// Purpose: Clear all pose parameters
//-----------------------------------------------------------------------------
void C_SDKPlayer::InitializePoseParams( void )
{
	CStudioHdr *hdr = GetModelPtr();
	Assert( hdr );
	if ( !hdr )
		return;

	m_headYawPoseParam = LookupPoseParameter( "head_yaw" );
	GetPoseParameterRange( m_headYawPoseParam, m_headYawMin, m_headYawMax );

	m_headPitchPoseParam = LookupPoseParameter( "head_pitch" );
	GetPoseParameterRange( m_headPitchPoseParam, m_headPitchMin, m_headPitchMax );

	for ( int i = 0; i < hdr->GetNumPoseParameters() ; i++ )
	{
		SetPoseParameter( hdr, i, 0.0 );
	}

}

void C_SDKPlayer::DoImpactEffect( trace_t &tr, int nDamageType )
{
	if ( GetActiveWeapon() )
	{
		GetActiveWeapon()->DoImpactEffect( tr, nDamageType );
		return;
	}

	BaseClass::DoImpactEffect( tr, nDamageType );
}

void C_SDKPlayer::PostDataUpdate( DataUpdateType_t updateType )
{
	// C_BaseEntity assumes we're networking the entity's angles, so pretend that it
	// networked the same value we already have.
	SetNetworkAngles( GetLocalAngles() );
	
	BaseClass::PostDataUpdate( updateType );

	bool bIsLocalPlayer = IsLocalPlayer();

	if( m_bSpawnInterpCounter != m_bSpawnInterpCounterCache )
	{
		MoveToLastReceivedPosition( true );
		ResetLatched();

		if ( bIsLocalPlayer )
		{
			LocalPlayerRespawn();
		}
		m_bSpawnInterpCounterCache = m_bSpawnInterpCounter.m_Value;
	}

}
// Called every time the player respawns
void C_SDKPlayer::LocalPlayerRespawn( void )
{
	ResetToneMapping(1.0);

	InitSpeeds(); //Tony; initialize player speeds.
}

void C_SDKPlayer::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	UpdateVisibility();
}

void C_SDKPlayer::PlayReloadEffect()
{
	// Only play the effect for other players.
	if ( this == C_SDKPlayer::GetLocalSDKPlayer() )
	{
		Assert( false ); // We shouldn't have been sent this message.
		return;
	}

	// Get the view model for our current gun.
	CWeaponSDKBase *pWeapon = GetActiveSDKWeapon();
	if ( !pWeapon )
		return;

	// The weapon needs two models, world and view, but can only cache one. Synthesize the other.
	const CSDKWeaponInfo &info = pWeapon->GetSDKWpnData();
	const model_t *pModel = modelinfo->GetModel( modelinfo->GetModelIndex( info.szViewModel ) );
	if ( !pModel )
		return;
	CStudioHdr studioHdr( modelinfo->GetStudiomodel( pModel ), mdlcache );
	if ( !studioHdr.IsValid() )
		return;

	// Find the reload animation.
	for ( int iSeq=0; iSeq < studioHdr.GetNumSeq(); iSeq++ )
	{
		mstudioseqdesc_t *pSeq = &studioHdr.pSeqdesc( iSeq );

		if ( pSeq->activity == ACT_VM_RELOAD )
		{
			float poseParameters[MAXSTUDIOPOSEPARAM];
			memset( poseParameters, 0, sizeof( poseParameters ) );
			float cyclesPerSecond = Studio_CPS( &studioHdr, *pSeq, iSeq, poseParameters );

			// Now read out all the sound events with their timing
			for ( int iEvent=0; iEvent < pSeq->numevents; iEvent++ )
			{
				mstudioevent_t *pEvent = pSeq->pEvent( iEvent );

				if ( pEvent->event == CL_EVENT_SOUND )
				{
					CSDKSoundEvent event;
					event.m_SoundName = pEvent->options;
					event.m_flEventTime = gpGlobals->curtime + pEvent->cycle / cyclesPerSecond;
					m_SoundEvents.AddToTail( event );
				}
			}

			break;
		}
	}	
}

void C_SDKPlayer::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	if ( IsLocalPlayer() )
	{
		if ( ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() ) )
			return;
	}

	MDLCACHE_CRITICAL_SECTION();
	m_PlayerAnimState->DoAnimationEvent( event, nData );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_SDKPlayer::CalculateIKLocks( float currentTime )
{
	if (!m_pIk) 
		return;

	int targetCount = m_pIk->m_target.Count();
	if ( targetCount == 0 )
		return;

	// In TF, we might be attaching a player's view to a walking model that's using IK. If we are, it can
	// get in here during the view setup code, and it's not normally supposed to be able to access the spatial
	// partition that early in the rendering loop. So we allow access right here for that special case.
	SpatialPartitionListMask_t curSuppressed = partition->GetSuppressedLists();
	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );
	CBaseEntity::PushEnableAbsRecomputations( false );

	for (int i = 0; i < targetCount; i++)
	{
		trace_t trace;
		CIKTarget *pTarget = &m_pIk->m_target[i];

		if (!pTarget->IsActive())
			continue;

		switch( pTarget->type)
		{
		case IK_GROUND:
			{
				pTarget->SetPos( Vector( pTarget->est.pos.x, pTarget->est.pos.y, GetRenderOrigin().z ));
				pTarget->SetAngles( GetRenderAngles() );
			}
			break;

		case IK_ATTACHMENT:
			{
				C_BaseEntity *pEntity = NULL;
				float flDist = pTarget->est.radius;

				// FIXME: make entity finding sticky!
				// FIXME: what should the radius check be?
				for ( CEntitySphereQuery sphere( pTarget->est.pos, 64 ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
				{
					C_BaseAnimating *pAnim = pEntity->GetBaseAnimating( );
					if (!pAnim)
						continue;

					int iAttachment = pAnim->LookupAttachment( pTarget->offset.pAttachmentName );
					if (iAttachment <= 0)
						continue;

					Vector origin;
					QAngle angles;
					pAnim->GetAttachment( iAttachment, origin, angles );

					// debugoverlay->AddBoxOverlay( origin, Vector( -1, -1, -1 ), Vector( 1, 1, 1 ), QAngle( 0, 0, 0 ), 255, 0, 0, 0, 0 );

					float d = (pTarget->est.pos - origin).Length();

					if ( d >= flDist)
						continue;

					flDist = d;
					pTarget->SetPos( origin );
					pTarget->SetAngles( angles );
					// debugoverlay->AddBoxOverlay( pTarget->est.pos, Vector( -pTarget->est.radius, -pTarget->est.radius, -pTarget->est.radius ), Vector( pTarget->est.radius, pTarget->est.radius, pTarget->est.radius), QAngle( 0, 0, 0 ), 0, 255, 0, 0, 0 );
				}

				if (flDist >= pTarget->est.radius)
				{
					// debugoverlay->AddBoxOverlay( pTarget->est.pos, Vector( -pTarget->est.radius, -pTarget->est.radius, -pTarget->est.radius ), Vector( pTarget->est.radius, pTarget->est.radius, pTarget->est.radius), QAngle( 0, 0, 0 ), 0, 0, 255, 0, 0 );
					// no solution, disable ik rule
					pTarget->IKFailed( );
				}
			}
			break;
		}
	}

	CBaseEntity::PopEnableAbsRecomputations();
	partition->SuppressLists( curSuppressed, true );
}

bool C_SDKPlayer::ShouldDraw( void )
{
	// If we're dead, our ragdoll will be drawn for us instead.
	if ( !IsAlive() )
		return false;

	if( GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	if ( State_Get() == STATE_WELCOME )
		return false;

	if ( State_Get() == STATE_PICKINGTEAM )
		return false;

	if ( State_Get() == STATE_PICKINGCLASS )
		return false;

	if( IsLocalPlayer() && IsRagdoll() )
		return true;

	if ( IsRagdoll() )
		return false;

	return BaseClass::ShouldDraw();
}

void C_SDKPlayer::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	if ( state == SHOULDTRANSMIT_END )
	{
		if( m_pFlashlightBeam != NULL )
			ReleaseFlashlight();
	}

	BaseClass::NotifyShouldTransmit( state );
}

CWeaponSDKBase* C_SDKPlayer::GetActiveSDKWeapon() const
{
	return dynamic_cast< CWeaponSDKBase* >( GetActiveWeapon() );
}

//-----------------------------------------------------------------------------
// Purpose: HL1's view bob, roll and idle effects.
//-----------------------------------------------------------------------------
void C_SDKPlayer::CalcVehicleView(IClientVehicle* pVehicle, Vector& eyeOrigin, QAngle& eyeAngles, float& zNear, float& zFar, float& fov)
{
	BaseClass::CalcVehicleView(pVehicle, eyeOrigin, eyeAngles, zNear, zFar, fov);

	if (pVehicle != nullptr)
	{
		if (pVehicle->GetVehicleEnt() != nullptr)
		{
			Vector Velocity;
			pVehicle->GetVehicleEnt()->EstimateAbsVelocity(Velocity);

			if (Velocity.Length() == 0)
			{
				IdleScale += gpGlobals->frametime * 0.05;
				if (IdleScale > 1.0)
					IdleScale = 1.0;
			}
			else
			{
				IdleScale -= gpGlobals->frametime;
				if (IdleScale < 0.0)
					IdleScale = 0.0;
			}

			CalcViewIdle(eyeAngles);
		}
	}
}

void C_SDKPlayer::CalcPlayerView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov)
{
	BaseClass::CalcPlayerView(eyeOrigin, eyeAngles, fov);

	Vector Velocity;
	EstimateAbsVelocity(Velocity);

	if (Velocity.Length() == 0)
	{
		IdleScale += gpGlobals->frametime * 0.05;
		if (IdleScale > 1.0)
			IdleScale = 1.0;
	}
	else
	{
		IdleScale -= gpGlobals->frametime;
		if (IdleScale < 0.0)
			IdleScale = 0.0;
	}

	CalcViewBob(eyeOrigin);
	CalcViewIdle(eyeAngles);
}

void C_SDKPlayer::CalcViewRoll( QAngle& eyeAngles )
{
	if ( GetMoveType() == MOVETYPE_NOCLIP )
		return;

	float Side = CalcRoll(GetAbsAngles(), GetAbsVelocity(), cl_hl1_rollangle.GetFloat(), cl_hl1_rollspeed.GetFloat()) * 4.0;
	eyeAngles[ROLL] += Side;

	if (GetHealth() <= 0)
	{
		eyeAngles[ROLL] = 80;
		return;
	}
}

void C_SDKPlayer::CalcViewBob( Vector& eyeOrigin )
{
	if ( !IsAlive() )
		return;

	if ( m_Shared.IsSniperZoomed() )
		return;

	float Cycle;
	Vector Velocity;

	if (GetGroundEntity() == nullptr || gpGlobals->curtime == BobLastTime)
	{
		eyeOrigin.z += ViewBob;
		return;
	}

	BobLastTime = gpGlobals->curtime;
	BobTime += gpGlobals->frametime;

	Cycle = BobTime - (int)(BobTime / cl_hl1_bobcycle.GetFloat()) * cl_hl1_bobcycle.GetFloat();
	Cycle /= cl_hl1_bobcycle.GetFloat();

	if (Cycle < cl_hl1_bobup.GetFloat())
		Cycle = M_PI * Cycle / cl_hl1_bobup.GetFloat();
	else
		Cycle = M_PI + M_PI * (Cycle - cl_hl1_bobup.GetFloat()) / (1.0 - cl_hl1_bobup.GetFloat());

	EstimateAbsVelocity(Velocity);
	Velocity.z = 0;

	ViewBob = sqrt(Velocity.x * Velocity.x + Velocity.y * Velocity.y) * cl_hl1_bob.GetFloat();
	ViewBob = ViewBob * 0.3 + ViewBob * 0.7 * sin(Cycle);
	ViewBob = min(ViewBob, 4);
	ViewBob = max(ViewBob, -7);

	eyeOrigin.z += ViewBob;
}

void C_SDKPlayer::CalcViewIdle(QAngle& eyeAngles)
{
	if ( !IsAlive() )
		return;

	if ( m_Shared.IsSniperZoomed() )
		return;

	eyeAngles[ROLL] += IdleScale * sin(gpGlobals->curtime * cl_hl1_iroll_cycle.GetFloat()) * cl_hl1_iroll_level.GetFloat();
	eyeAngles[PITCH] += IdleScale * sin(gpGlobals->curtime * cl_hl1_ipitch_cycle.GetFloat()) * cl_hl1_ipitch_level.GetFloat();
	eyeAngles[YAW] += IdleScale * sin(gpGlobals->curtime * cl_hl1_iyaw_cycle.GetFloat()) * cl_hl1_iyaw_level.GetFloat();
}

bool C_SDKPlayer::CanShowClassMenu( void )
{
	return ( GetTeamNumber() == SDK_TEAM_BLUE || 
			 GetTeamNumber() == SDK_TEAM_RED || 
			 GetTeamNumber() == SDK_TEAM_GREEN || 
			 GetTeamNumber() == SDK_TEAM_YELLOW );
}

bool C_SDKPlayer::CanShowTeamMenu( void )
{
	return true;
}

void C_SDKPlayer::ClientThink()
{
	UpdateSoundEvents();

	// Pass on through to the base class.
	BaseClass::ClientThink();

	bool bFoundViewTarget = false;
	
	Vector vForward;
	AngleVectors( GetLocalAngles(), &vForward );

	for( int iClient = 1; iClient <= gpGlobals->maxClients; ++iClient )
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex( iClient );
		if(!pEnt || !pEnt->IsPlayer())
			continue;

		if ( pEnt->entindex() == entindex() )
			continue;

		Vector vTargetOrigin = pEnt->GetAbsOrigin();
		Vector vMyOrigin =  GetAbsOrigin();

		Vector vDir = vTargetOrigin - vMyOrigin;
		
		if ( vDir.Length() > 128 ) 
			continue;

		VectorNormalize( vDir );

		if ( DotProduct( vForward, vDir ) < 0.0f )
			 continue;

		m_vLookAtTarget = pEnt->EyePosition();
		bFoundViewTarget = true;
		break;
	}

	if ( bFoundViewTarget == false )
	{
		m_vLookAtTarget = GetAbsOrigin() + vForward * 512;
	}

	UpdateIDTarget();

	// Avoidance
	if ( gpGlobals->curtime >= m_fNextThinkPushAway )
	{
		PerformObstaclePushaway( this );
		m_fNextThinkPushAway =  gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_SDKPlayer::UpdateLookAt( void )
{
	// head yaw
	if (m_headYawPoseParam < 0 || m_headPitchPoseParam < 0)
		return;

	// orient eyes
	m_viewtarget = m_vLookAtTarget;

	// Figure out where we want to look in world space.
	QAngle desiredAngles;
	Vector to = m_vLookAtTarget - EyePosition();
	VectorAngles( to, desiredAngles );

	// Figure out where our body is facing in world space.
	QAngle bodyAngles( 0, 0, 0 );
	bodyAngles[YAW] = GetLocalAngles()[YAW];


	float flBodyYawDiff = bodyAngles[YAW] - m_flLastBodyYaw;
	m_flLastBodyYaw = bodyAngles[YAW];
	

	// Set the head's yaw.
	float desired = AngleNormalize( desiredAngles[YAW] - bodyAngles[YAW] );
	desired = clamp( desired, m_headYawMin, m_headYawMax );
	m_flCurrentHeadYaw = ApproachAngle( desired, m_flCurrentHeadYaw, 130 * gpGlobals->frametime );

	// Counterrotate the head from the body rotation so it doesn't rotate past its target.
	m_flCurrentHeadYaw = AngleNormalize( m_flCurrentHeadYaw - flBodyYawDiff );
	desired = clamp( desired, m_headYawMin, m_headYawMax );
	
	SetPoseParameter( m_headYawPoseParam, m_flCurrentHeadYaw );

	
	// Set the head's yaw.
	desired = AngleNormalize( desiredAngles[PITCH] );
	desired = clamp( desired, m_headPitchMin, m_headPitchMax );
	
	m_flCurrentHeadPitch = ApproachAngle( desired, m_flCurrentHeadPitch, 130 * gpGlobals->frametime );
	m_flCurrentHeadPitch = AngleNormalize( m_flCurrentHeadPitch );
	SetPoseParameter( m_headPitchPoseParam, m_flCurrentHeadPitch );
}



int C_SDKPlayer::GetIDTarget() const
{
	return m_iIDEntIndex;
}

//-----------------------------------------------------------------------------
// Purpose: Update this client's target entity
//-----------------------------------------------------------------------------
void C_SDKPlayer::UpdateIDTarget()
{
	if ( !IsLocalPlayer() )
		return;

	// Clear old target and find a new one
	m_iIDEntIndex = 0;

	// don't show id's in any state but active.
	if ( State_Get() != STATE_ACTIVE )
		return;

	trace_t tr;
	Vector vecStart, vecEnd;
	VectorMA( MainViewOrigin(), 1500, MainViewForward(), vecEnd );
	VectorMA( MainViewOrigin(), 10,   MainViewForward(), vecStart );
	UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	if ( !tr.startsolid && tr.DidHitNonWorldEntity() )
	{
		C_BaseEntity *pEntity = tr.m_pEnt;

		if ( pEntity && (pEntity != this) )
		{
			m_iIDEntIndex = pEntity->entindex();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Try to steer away from any players and objects we might interpenetrate
//-----------------------------------------------------------------------------
void C_SDKPlayer::AvoidPlayers( CUserCmd *pCmd )
{
	// Don't test if the player doesn't exist or is dead.
	if ( IsAlive() == false )
		return;

	C_SDKTeam *pTeam = ( C_SDKTeam * )GetTeam();
	if ( !pTeam )
		return;

	// Up vector.
	static Vector vecUp( 0.0f, 0.0f, 1.0f );

	Vector vecSDKPlayerCenter = GetAbsOrigin();
	Vector vecSDKPlayerMin = GetPlayerMins();
	Vector vecSDKPlayerMax = GetPlayerMaxs();
	float flZHeight = vecSDKPlayerMax.z - vecSDKPlayerMin.z;
	vecSDKPlayerCenter.z += 0.5f * flZHeight;
	VectorAdd( vecSDKPlayerMin, vecSDKPlayerCenter, vecSDKPlayerMin );
	VectorAdd( vecSDKPlayerMax, vecSDKPlayerCenter, vecSDKPlayerMax );

	// Find an intersecting player or object.
	int nAvoidPlayerCount = 0;
	C_SDKPlayer *pAvoidPlayerList[MAX_PLAYERS];

	C_SDKPlayer *pIntersectPlayer = NULL;
	float flAvoidRadius = 0.0f;

	Vector vecAvoidCenter, vecAvoidMin, vecAvoidMax;
	for ( int i = 0; i < pTeam->GetNumPlayers(); ++i )
	{
		C_SDKPlayer *pAvoidPlayer = static_cast< C_SDKPlayer * >( pTeam->GetPlayer( i ) );
		if ( pAvoidPlayer == NULL )
			continue;
		// Is the avoid player me?
		if ( pAvoidPlayer == this )
			continue;

		// Save as list to check against for objects.
		pAvoidPlayerList[nAvoidPlayerCount] = pAvoidPlayer;
		++nAvoidPlayerCount;

		// Check to see if the avoid player is dormant.
		if ( pAvoidPlayer->IsDormant() )
			continue;

		// Is the avoid player solid?
		if ( pAvoidPlayer->IsSolidFlagSet( FSOLID_NOT_SOLID ) )
			continue;

		Vector t1, t2;

		vecAvoidCenter = pAvoidPlayer->GetAbsOrigin();
		vecAvoidMin = pAvoidPlayer->GetPlayerMins();
		vecAvoidMax = pAvoidPlayer->GetPlayerMaxs();
		flZHeight = vecAvoidMax.z - vecAvoidMin.z;
		vecAvoidCenter.z += 0.5f * flZHeight;
		VectorAdd( vecAvoidMin, vecAvoidCenter, vecAvoidMin );
		VectorAdd( vecAvoidMax, vecAvoidCenter, vecAvoidMax );

		if ( IsBoxIntersectingBox( vecSDKPlayerMin, vecSDKPlayerMax, vecAvoidMin, vecAvoidMax ) )
		{
			// Need to avoid this player.
			if ( !pIntersectPlayer )
			{
				pIntersectPlayer = pAvoidPlayer;
				break;
			}
		}
	}

	// Anything to avoid?
	if ( !pIntersectPlayer )
		return;

	// Calculate the push strength and direction.
	Vector vecDelta;

	// Avoid a player - they have precedence.
	if ( pIntersectPlayer )
	{
		VectorSubtract( pIntersectPlayer->WorldSpaceCenter(), vecSDKPlayerCenter, vecDelta );

		Vector vRad = pIntersectPlayer->WorldAlignMaxs() - pIntersectPlayer->WorldAlignMins();
		vRad.z = 0;

		flAvoidRadius = vRad.Length();
	}

	float flPushStrength = RemapValClamped( vecDelta.Length(), flAvoidRadius, 0, 0, sdk_max_separation_force.GetInt() ); //flPushScale;

	// Check to see if we have enough push strength to make a difference.
	if ( flPushStrength < 0.01f )
		return;

	Vector vecPush;
	if ( GetAbsVelocity().Length2DSqr() > 0.1f )
	{
		Vector vecVelocity = GetAbsVelocity();
		vecVelocity.z = 0.0f;
		CrossProduct( vecUp, vecVelocity, vecPush );
		VectorNormalize( vecPush );
	}
	else
	{
		// We are not moving, but we're still intersecting.
		QAngle angView = pCmd->viewangles;
		angView.x = 0.0f;
		AngleVectors( angView, NULL, &vecPush, NULL );
	}

	// Move away from the other player/object.
	Vector vecSeparationVelocity;
	if ( vecDelta.Dot( vecPush ) < 0 )
		vecSeparationVelocity = vecPush * flPushStrength;
	else
		vecSeparationVelocity = vecPush * -flPushStrength;

	// Don't allow the max push speed to be greater than the max player speed.
	float flMaxPlayerSpeed = MaxSpeed();
	float flCropFraction = 1.33333333f;

	if ( ( GetFlags() & FL_DUCKING ) && ( GetGroundEntity() != NULL ) )
		flMaxPlayerSpeed *= flCropFraction;

	float flMaxPlayerSpeedSqr = flMaxPlayerSpeed * flMaxPlayerSpeed;

	if ( vecSeparationVelocity.LengthSqr() > flMaxPlayerSpeedSqr )
	{
		vecSeparationVelocity.NormalizeInPlace();
		VectorScale( vecSeparationVelocity, flMaxPlayerSpeed, vecSeparationVelocity );
	}

	QAngle vAngles = pCmd->viewangles;
	vAngles.x = 0;
	Vector currentdir;
	Vector rightdir;

	AngleVectors( vAngles, &currentdir, &rightdir, NULL );

	Vector vDirection = vecSeparationVelocity;

	VectorNormalize( vDirection );

	float fwd = currentdir.Dot( vDirection );
	float rt = rightdir.Dot( vDirection );

	float forward = fwd * flPushStrength;
	float side = rt * flPushStrength;

	pCmd->forwardmove	+= forward;
	pCmd->sidemove		+= side;

	float flForwardScale = 1.0f;
	if ( pCmd->forwardmove > fabs( cl_forwardspeed.GetFloat() ) )
		flForwardScale = fabs( cl_forwardspeed.GetFloat() ) / pCmd->forwardmove;
	else if ( pCmd->forwardmove < -fabs( cl_backspeed.GetFloat() ) )
		flForwardScale = fabs( cl_backspeed.GetFloat() ) / fabs( pCmd->forwardmove );

	float flSideScale = 1.0f;
	if ( fabs( pCmd->sidemove ) > fabs( cl_sidespeed.GetFloat() ) )
		flSideScale = fabs( cl_sidespeed.GetFloat() ) / fabs( pCmd->sidemove );

	float flScale = min( flForwardScale, flSideScale );
	pCmd->forwardmove *= flScale;
	pCmd->sidemove *= flScale;
}

bool C_SDKPlayer::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{	
	static QAngle angMoveAngle( 0.0f, 0.0f, 0.0f );

	VectorCopy( pCmd->viewangles, angMoveAngle );

	BaseClass::CreateMove( flInputSampleTime, pCmd );

	AvoidPlayers( pCmd );

	return true;
}

void C_SDKPlayer::UpdateSoundEvents()
{
	int iNext;
	for ( int i=m_SoundEvents.Head(); i != m_SoundEvents.InvalidIndex(); i = iNext )
	{
		iNext = m_SoundEvents.Next( i );

		CSDKSoundEvent *pEvent = &m_SoundEvents[i];
		if ( gpGlobals->curtime >= pEvent->m_flEventTime )
		{
			CLocalPlayerFilter filter;
			EmitSound( filter, GetSoundSourceIndex(), STRING( pEvent->m_SoundName ) );

			m_SoundEvents.Remove( i );
		}
	}

	if ( ( m_afButtonPressed & IN_SAVEME ) && IsAlive() )
	{
		C_SDKPlayer *pPlayer = ToSDKPlayer( C_SDKPlayer::GetLocalSDKPlayer() );
		CPASAttenuationFilter filter( pPlayer, "Player.Medic" ); // Filters
		EmitSound( filter, pPlayer->entindex(), "Player.Medic" ); // Play player.medic sound
	}
}