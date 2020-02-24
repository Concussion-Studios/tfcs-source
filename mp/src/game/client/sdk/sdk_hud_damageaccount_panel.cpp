//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "c_baseentity.h"
#include "c_sdk_player.h"
#include "iclientmode.h"
#include "ienginevgui.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ProgressBar.h>
#include "engine/IEngineSound.h"
#include <vgui_controls/AnimationController.h>
#include "iclientmode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

// Floating delta text items, float off the top of the head to 
// show damage done
typedef struct
{
	// amount of delta
	int m_iAmount;

	// die time
	float m_flDieTime;

	EHANDLE m_hEntity;

	// position of damaged player
	Vector m_vDamagePos;

} dmg_account_delta_t;

#define NUM_ACCOUNT_DELTA_ITEMS 10

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CDamageAccountPanel : public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE( CDamageAccountPanel, EditablePanel );

public:
	CDamageAccountPanel( const char *pElementName );

	virtual void	Init();
	virtual void	ApplySchemeSettings( IScheme *scheme );
	virtual void	LevelInit( void );
	virtual bool	ShouldDraw( void );
	virtual void	Paint( void );

	virtual void	FireGameEvent( IGameEvent *event );
	void			OnDamaged( IGameEvent *event );
	void			PlayHitSound( int iAmount, bool bKill );

private:

	float m_flLastHitSound;

	int iAccountDeltaHead;
	dmg_account_delta_t m_AccountDeltaItems[NUM_ACCOUNT_DELTA_ITEMS];

	CPanelAnimationVarAliasType( float, m_flDeltaItemEndPos, "delta_item_end_y", "0", "proportional_float" );
	CPanelAnimationVar( Color, m_DeltaPositiveColor, "PositiveColor", "0 255 0 255" )
	CPanelAnimationVar( float, m_flDeltaLifetime, "delta_lifetime", "2.0" );
	CPanelAnimationVar( vgui::HFont, m_hDeltaItemFont, "delta_item_font", "Default" );
	CPanelAnimationVar( vgui::HFont, m_hDeltaItemFontBig, "delta_item_font_big", "Default" );
};

DECLARE_HUDELEMENT( CDamageAccountPanel );

extern ConVar mp_damagepopup;

ConVar hud_damage_numbers( "hud_damage_numbers", "1", FCVAR_ARCHIVE, "Display damage done as text over your target" );
ConVar hud_damage_numbers_batching( "hud_damage_numbers_batching", "1", FCVAR_ARCHIVE, "If set to 1, numbers that are too close together are merged." );
ConVar hud_damage_numbers_batching_window( "hud_damage_numbers_batching_window", "0.2", FCVAR_ARCHIVE, "Maximum delay between damage events in order to batch numbers." );
ConVar hud_damage_numbers_red( "hud_damage_numbers_red", "255", FCVAR_ARCHIVE, "Red modifier for color of damage indicators", true, 0, true, 255 );
ConVar hud_damage_numbers_green( "hud_damage_numbers_green", "0", FCVAR_ARCHIVE, "Green modifier for color of damage indicators", true, 0, true, 255 );
ConVar hud_damage_numbers_blue( "hud_damage_numbers_blue", "0", FCVAR_ARCHIVE, "Blue modifier for color of damage indicators", true, 0, true, 255 );

ConVar cl_hitsound( "cl_hitsound", "1", FCVAR_ARCHIVE, "If set to 1, play a sound everytime you injure an enemy. The sound can be customized." );
ConVar cl_hitsound_effect( "cl_hitsound_effect", "ui/hitsounds/hitsound_quake3.wav", FCVAR_ARCHIVE, "Which Dingalingaling sound is used" );
ConVar cl_hitsound_volume( "cl_hitsound_volume", "0.75", FCVAR_ARCHIVE, "Desired volume of the hit sound.", true, 0.0, true, 1.0 );
ConVar cl_hitsound_pitchmindmg( "cl_hitsound_pitchmindmg", "100", FCVAR_ARCHIVE, "Desired pitch of the hit sound when a minimal damage hit (<= 10 health) is done.", true, 1, true, 255 );
ConVar cl_hitsound_pitchmaxdmg( "cl_hitsound_pitchmaxdmg", "100", FCVAR_ARCHIVE, "Desired pitch of the hit sound when a maximum damage hit (>= 150 health) is done.", true, 1, true, 255 );
ConVar cl_hitsound_repeat_delay( "cl_hitsound_repeat_delay", "0", FCVAR_ARCHIVE, "Desired repeat delay of the hit sound. Set to 0 to play a sound for every instance of damage dealt." );

ConVar cl_hitsound_lasthit( "cl_hitsound_lasthit", "1", FCVAR_ARCHIVE, "If set to 1, play a sound whenever one of your attacks kills an enemy. The sound can be customized." );
ConVar cl_hitsound_last_effect( "cl_hitsound_last_effect", "ui/hitsounds/killsound_quake3.wav", FCVAR_ARCHIVE, "Which final hit sound to play when the target expires." );
ConVar cl_hitsound_lasthit_volume( "cl_hitsound_lasthit_volume", "0.75", FCVAR_ARCHIVE, "Desired volume of the last hit sound.", true, 0.0, true, 1.0 );
ConVar cl_hitsound_lasthit_pitchmindmg( "cl_hitsound_lasthit_pitchmindmg", "100", FCVAR_ARCHIVE, "Desired pitch of the last hit sound when a minimal damage hit (<= 10 health) is done.", true, 1, true, 255 );
ConVar cl_hitsound_lasthit_pitchmaxdmg( "cl_hitsound_lasthit_pitchmaxdmg", "100", FCVAR_ARCHIVE, "Desired pitch of the last hit sound when a maximum damage hit (>= 150 health) is done.", true, 1, true, 255 );



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CDamageAccountPanel::CDamageAccountPanel( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "CDamageAccountPanel" )
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_MISCSTATUS );

	m_flLastHitSound = 0.0f;

	iAccountDeltaHead = 0;

	for ( int i = 0; i < NUM_ACCOUNT_DELTA_ITEMS; i++ )
		m_AccountDeltaItems[i].m_flDieTime = 0.0f;

	ListenForGameEvent( "player_hurt" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDamageAccountPanel::Init()
{
	enginesound->PrecacheSound( cl_hitsound_effect.GetString() );
	enginesound->PrecacheSound( cl_hitsound_last_effect.GetString() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDamageAccountPanel::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( V_strcmp( type, "player_hurt" ) == 0 )
		OnDamaged( event );
	else
		CHudElement::FireGameEvent( event );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDamageAccountPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// load control settings...
	LoadControlSettings( "resource/UI/HudDamageAccount.res" );
}

//-----------------------------------------------------------------------------
// Purpose: called whenever a new level's starting
//-----------------------------------------------------------------------------
void CDamageAccountPanel::LevelInit( void )
{
	iAccountDeltaHead = 0;

	for ( int i = 0; i < NUM_ACCOUNT_DELTA_ITEMS; i++ )
		m_AccountDeltaItems[i].m_flDieTime = 0.0f;

	m_flLastHitSound = 0.0f;

	CHudElement::LevelInit();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CDamageAccountPanel::ShouldDraw( void )
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer || !pPlayer->IsAlive() )
		return false;

	return CHudElement::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDamageAccountPanel::OnDamaged( IGameEvent *event )
{
	C_SDKPlayer* pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( !pPlayer || !pPlayer->IsAlive() )
		return;

	int iAttacker = event->GetInt( "attacker_index" );
	int iVictim = event->GetInt( "victim_index" );
	int iDmgAmount = event->GetInt( "damageamount" );
	int iHealth = event->GetInt( "health" );

	// Did we shoot the guy?
	if ( iAttacker != pPlayer->entindex() )
		return;

	// No self-damage notifications.
	if ( iAttacker == iVictim )
		return;

	// Don't show anything if no damage was done.
	if ( iDmgAmount == 0 )
		return;

	C_BaseEntity *pVictim = ClientEntityList().GetBaseEntity(iVictim);
	if ( !pVictim )
		return;
		
	// Play hit sound, if appliable.
	bool bDinged = false;

	if ( cl_hitsound_lasthit.GetBool() && iHealth == 0 )
	{
		// This guy is dead, play kill sound.
		PlayHitSound( iDmgAmount, true );
		bDinged = true;
	}

	// Stop here if we chose not to show hit numbers, or server disallows it.
	if ( ( cl_hitsound.GetBool() || mp_damagepopup.GetBool() ) && !bDinged )
	{
		PlayHitSound( iDmgAmount, false );
		bDinged = true;
	}

	// Leftover from old code?
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "DamagedPlayer" );

	// Stop here if we chose not to show hit numbers.
	if ( !hud_damage_numbers.GetBool() )
		return;

	// Don't show the numbers if we can't see the victim.
	trace_t tr;
	UTIL_TraceLine( pPlayer->EyePosition(), pVictim->WorldSpaceCenter(), MASK_VISIBLE, NULL, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 1.0f )
		return;

	Vector vecTextPos;
	if ( pVictim->IsBaseObject() )
		vecTextPos = pVictim->GetAbsOrigin() + Vector( 0, 0, pVictim->WorldAlignMaxs().z );
	else
		vecTextPos = pVictim->WorldSpaceCenter() +  Vector( 0, 0, pVictim->WorldAlignMaxs().z );;

	bool bBatch = false;
	dmg_account_delta_t *pDelta = NULL;

	if ( hud_damage_numbers_batching.GetBool() )
	{
		// Cycle through deltas and search for one that belongs to this player.
		for ( int i = 0; i < NUM_ACCOUNT_DELTA_ITEMS; i++ )
		{
			if ( m_AccountDeltaItems[i].m_hEntity.Get() == pVictim )
			{
				// See if it's lifetime is inside batching window.
				float flCreateTime = m_AccountDeltaItems[i].m_flDieTime - m_flDeltaLifetime;
				if ( gpGlobals->curtime - flCreateTime < hud_damage_numbers_batching_window.GetFloat() )
				{
					pDelta = &m_AccountDeltaItems[i];
					bBatch = true;
					break;
				}
			}
		}
	}

	if ( !pDelta )
	{
		pDelta = &m_AccountDeltaItems[iAccountDeltaHead];
		iAccountDeltaHead++;
		iAccountDeltaHead %= NUM_ACCOUNT_DELTA_ITEMS;
	}

	pDelta->m_flDieTime = gpGlobals->curtime + m_flDeltaLifetime;
	pDelta->m_iAmount = bBatch ? pDelta->m_iAmount - iDmgAmount : -iDmgAmount;
	pDelta->m_hEntity = pVictim;
	pDelta->m_vDamagePos = vecTextPos + Vector( 0, 0, 18 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CDamageAccountPanel::PlayHitSound( int iAmount, bool bKill )
{
	if ( !bKill )
	{
		float flRepeatDelay = cl_hitsound_repeat_delay.GetFloat();
		if ( flRepeatDelay > 0 && gpGlobals->curtime - m_flLastHitSound <= flRepeatDelay )
			return;
	}

	EmitSound_t params;

	if ( bKill )
	{
		params.m_pSoundName = cl_hitsound_last_effect.GetString();
		params.m_flVolume = cl_hitsound_lasthit_volume.GetFloat();
		float flPitchMin = cl_hitsound_lasthit_pitchmindmg.GetFloat();
		float flPitchMax = cl_hitsound_lasthit_pitchmaxdmg.GetFloat();
		params.m_nPitch = RemapValClamped( (float)iAmount, 10, 150, flPitchMin, flPitchMax );
	}
	else
	{
		params.m_pSoundName = cl_hitsound_effect.GetString();
		params.m_flVolume = cl_hitsound_volume.GetFloat();
		float flPitchMin = cl_hitsound_pitchmindmg.GetFloat();
		float flPitchMax = cl_hitsound_pitchmaxdmg.GetFloat();
		params.m_nPitch = RemapValClamped( (float)iAmount, 10, 150, flPitchMin, flPitchMax );
	}

	CLocalPlayerFilter filter;

	C_BaseEntity::EmitSound( filter, SOUND_FROM_LOCAL_PLAYER, params ); // Ding!
	if ( !bKill )
		m_flLastHitSound = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Paint the deltas
//-----------------------------------------------------------------------------
void CDamageAccountPanel::Paint( void )
{
	BaseClass::Paint();

	for ( int i = 0; i < NUM_ACCOUNT_DELTA_ITEMS; i++ )
	{
		// update all the valid delta items
		if ( m_AccountDeltaItems[i].m_flDieTime > gpGlobals->curtime )
		{
			// position and alpha are determined from the lifetime
			// color is determined by the delta
			// Negative damage deltas are determined by convar settings
			// Positive damage deltas are green
			Color m_DeltaNegativeColor( hud_damage_numbers_red.GetInt(), hud_damage_numbers_green.GetInt(), hud_damage_numbers_blue.GetInt(), 255 );

			Color c = m_AccountDeltaItems[i].m_iAmount > 0 ? m_DeltaPositiveColor : m_DeltaNegativeColor;

			float flLifetimePercent = ( m_AccountDeltaItems[i].m_flDieTime - gpGlobals->curtime ) / m_flDeltaLifetime;

			// fade out after half our lifetime
			if ( flLifetimePercent < 0.5 )
				c[3] = (int)( 255.0f * ( flLifetimePercent / 0.5 ) );

			int x, y;
			bool bOnscreen = GetVectorInScreenSpace( m_AccountDeltaItems[i].m_vDamagePos, x, y );

			if ( !bOnscreen )
				continue;

			int flHeight = 50.0f;
			y -= (int)( ( 1.0f - flLifetimePercent ) * flHeight );

			vgui::HFont hFont = m_hDeltaItemFont;

			wchar_t wBuf[20];
			if ( m_AccountDeltaItems[i].m_iAmount > 0 )
				_snwprintf( wBuf, sizeof( wBuf ) / sizeof( wchar_t ), L"+%d", m_AccountDeltaItems[i].m_iAmount );
			else
				_snwprintf( wBuf, sizeof( wBuf ) / sizeof( wchar_t ), L"%d", m_AccountDeltaItems[i].m_iAmount );

			// Offset x pos so the text is centered.
			x -= UTIL_ComputeStringWidth( hFont, wBuf ) / 2;

			vgui::surface()->DrawSetTextFont( hFont );
			vgui::surface()->DrawSetTextColor( c );
			vgui::surface()->DrawSetTextPos( x, y );

			vgui::surface()->DrawPrintText( wBuf, wcslen( wBuf ), FONT_DRAW_NONADDITIVE );
		}
	}
}
