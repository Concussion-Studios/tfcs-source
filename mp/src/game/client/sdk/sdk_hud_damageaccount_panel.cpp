//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
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
#include "gamevars_shared.h"

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

	//bool bHeadshot;
} dmg_account_delta_t;

#define NUM_ACCOUNT_DELTA_ITEMS 10

extern ConVar mp_damagepopup;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CDamageAccountPanel : public CHudElement, public EditablePanel
{
	DECLARE_CLASS_SIMPLE(CDamageAccountPanel, EditablePanel);

public:
	CDamageAccountPanel(const char *pElementName);

	virtual void	ApplySchemeSettings(IScheme *scheme);
	virtual void	LevelInit(void);
	virtual bool	ShouldDraw(void);
	virtual void	Paint(void);

	virtual void	FireGameEvent(IGameEvent *event);
	void			OnDamaged(IGameEvent *event);

private:
	int iAccountDeltaHead;
	dmg_account_delta_t m_AccountDeltaItems[NUM_ACCOUNT_DELTA_ITEMS];

	//CPanelAnimationVarAliasType( float, m_flDeltaItemStartPos, "delta_item_start_y", "100", "proportional_float" );
	CPanelAnimationVarAliasType(float, m_flDeltaItemEndPos, "delta_item_end_y", "0", "proportional_float");

	//CPanelAnimationVarAliasType( float, m_flDeltaItemX, "delta_item_x", "0", "proportional_float" );

	CPanelAnimationVar(Color, m_DeltaPositiveColor, "PositiveColor", "0 255 0 255");
	CPanelAnimationVar(Color, m_DeltaNegativeColor, "NegativeColor", "255 0 0 255");
	//CPanelAnimationVar(Color, m_DeltaHeadshotColor, "HeadshotColor", "255 255 0 255");

	CPanelAnimationVar(Color, m_DeltaNegativeFlashColor, "NegativeFlashColor", "255 128 128 255");
	//CPanelAnimationVar(Color, m_DeltaHeadshotFlashColor, "HeadshotFlashColor", "255 255 196 255");

	CPanelAnimationVar(float, m_flDeltaLifetime, "delta_lifetime", "2.0");

	CPanelAnimationVar(vgui::HFont, m_hDeltaItemFont, "delta_item_font", "Default");
	//CPanelAnimationVar(vgui::HFont, m_hDeltaItemFontBig, "delta_item_font_big", "Default");
};

DECLARE_HUDELEMENT(CDamageAccountPanel);

ConVar cl_damagepopup( "cl_damagepopup", "1", FCVAR_ARCHIVE, "Enable damage popups." );
ConVar cl_damagepopup_batching( "cl_damagepopup_batching", "1", FCVAR_ARCHIVE, "If set to 1, numbers that are too close together are merged." );
ConVar cl_damagepopup_batching_window( "cl_damagepopup_batching_window", "0.2", FCVAR_ARCHIVE, "Maximum delay between damage events in order to batch numbers." );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CDamageAccountPanel::CDamageAccountPanel(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "CDamageAccountPanel")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_MISCSTATUS);

	iAccountDeltaHead = 0;

	for (int i = 0; i<NUM_ACCOUNT_DELTA_ITEMS; i++)
		m_AccountDeltaItems[i].m_flDieTime = 0.0f;

	ListenForGameEvent("player_hurt");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDamageAccountPanel::FireGameEvent(IGameEvent *event)
{
	const char * type = event->GetName();

	if ( Q_strcmp( type, "player_hurt" ) == 0 )
		OnDamaged( event );
	else
		CHudElement::FireGameEvent( event );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDamageAccountPanel::ApplySchemeSettings(IScheme *pScheme)
{
	// load control settings...
	LoadControlSettings( "resource/UI/HudDamageAccount.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

//-----------------------------------------------------------------------------
// Purpose: called whenever a new level's starting
//-----------------------------------------------------------------------------
void CDamageAccountPanel::LevelInit(void)
{
	iAccountDeltaHead = 0;

	CHudElement::LevelInit();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CDamageAccountPanel::ShouldDraw(void)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer || !pPlayer->IsAlive())
		return false;

	return CHudElement::ShouldDraw();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDamageAccountPanel::OnDamaged(IGameEvent *event)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (pPlayer && pPlayer->IsAlive())
	{
		int iAttacker = event->GetInt("attacker");
		int iVictim = event->GetInt("userid");
		int iDmgAmount = event->GetInt("damageamount");

		// Did we shoot the guy?
		if (iAttacker != pPlayer->GetUserID())
			return;

		// No self-damage notifications.
		if (iAttacker == iVictim)
			return;

		// Don't show anything if no damage was done.
		if (iDmgAmount == 0)
			return;

		// Currently only supporting players.
		C_BasePlayer *pVictim = UTIL_PlayerByUserId(iVictim);

		if (!pVictim)
			return;

		// Stop here if we chose not to show hit numbers, or server disallows it.
		if ( !cl_damagepopup.GetBool() || !mp_damagepopup.GetBool() )
			return;

		// Don't show the numbers if we can't see the victim.
		trace_t tr;
		UTIL_TraceLine(pPlayer->EyePosition(), pVictim->WorldSpaceCenter(), MASK_VISIBLE, NULL, COLLISION_GROUP_NONE, &tr);
		if (tr.fraction != 1.0f)
			return;

		if (cl_damagepopup_batching.GetBool())
		{
			// Cycle through deltas and search for one that belongs to this player.
			for (int i = 0; i < NUM_ACCOUNT_DELTA_ITEMS; i++)
			{
				if (m_AccountDeltaItems[i].m_hEntity.Get() == pVictim)
				{
					// See if it's lifetime is inside batching window.
					float flCreateTime = m_AccountDeltaItems[i].m_flDieTime - m_flDeltaLifetime;
					if (gpGlobals->curtime - flCreateTime < cl_damagepopup_batching_window.GetFloat())
					{
						// Update it's die time and damage.
						m_AccountDeltaItems[i].m_flDieTime = gpGlobals->curtime + m_flDeltaLifetime;
						m_AccountDeltaItems[i].m_iAmount += iDmgAmount;
						m_AccountDeltaItems[i].m_vDamagePos = pVictim->EyePosition() + Vector(0, 0, 18);
						//m_AccountDeltaItems[i].bHeadshot = event->GetBool("headshot");
						return;
					}
				}
			}
		}

		// create a delta item that floats off the top
		dmg_account_delta_t *pNewDeltaItem = &m_AccountDeltaItems[iAccountDeltaHead];

		iAccountDeltaHead++;
		iAccountDeltaHead %= NUM_ACCOUNT_DELTA_ITEMS;

		pNewDeltaItem->m_flDieTime = gpGlobals->curtime + m_flDeltaLifetime;
		pNewDeltaItem->m_iAmount = iDmgAmount;
		pNewDeltaItem->m_hEntity = pVictim;
		pNewDeltaItem->m_vDamagePos = pVictim->EyePosition() + Vector(0, 0, 18);
		//pNewDeltaItem->bHeadshot = event->GetBool("headshot");
	}
}

Color FadeColor(Color color1, Color color2, float percent)
{
	Color FinalColor;

	if (percent > 1.0) // Clamp
		percent = 1.0;

	if (percent < 0.0) // Clamp
		percent = 0.0; 

	FinalColor[0] = (int)(((color2[0] - color1[0]) * percent) + color1[0]);
	FinalColor[1] = (int)(((color2[1] - color1[1]) * percent) + color1[1]);
	FinalColor[2] = (int)(((color2[2] - color1[2]) * percent) + color1[2]);
	FinalColor[3] = 255; // [Striker] Alpha is ignored, handled by Paint.
	return FinalColor;
}

//-----------------------------------------------------------------------------
// Purpose: Paint the deltas
//-----------------------------------------------------------------------------
void CDamageAccountPanel::Paint(void)
{
	BaseClass::Paint();

	for (int i = 0; i<NUM_ACCOUNT_DELTA_ITEMS; i++)
	{
		// update all the valid delta items
		if (m_AccountDeltaItems[i].m_flDieTime > gpGlobals->curtime)
		{
			// position and alpha are determined from the lifetime
			// color is determined by the delta - green for positive, red for negative

			Color c = /* m_AccountDeltaItems[i].bHeadshot ? m_DeltaHeadshotColor : */ m_DeltaNegativeColor;
			Color flashcolor = /* m_AccountDeltaItems[i].bHeadshot ? m_DeltaHeadshotFlashColor : */ m_DeltaNegativeFlashColor;
			Color finalcolor = c;

			float flLifetimePercent = (m_AccountDeltaItems[i].m_flDieTime - gpGlobals->curtime) / m_flDeltaLifetime;

			if (flLifetimePercent >= 0.75)
			{
				finalcolor = FadeColor(c, flashcolor, RemapVal(flLifetimePercent, 1.0, 0.75, 1.0, 0.0));
			}

			// fade out after half our lifetime
			if (flLifetimePercent < 0.5)
			{
				finalcolor[3] = (int)(255.0f * (flLifetimePercent / 0.5));
			}

			int iX, iY;
			bool bOnscreen = GetVectorInScreenSpace(m_AccountDeltaItems[i].m_vDamagePos, iX, iY);

			if (!bOnscreen)
				continue;

			float flHeight = 50.0f;
			float flYPos = (float)iY - (1.0 - flLifetimePercent) * flHeight;

			// Use BIGGER font for crits.
			vgui::surface()->DrawSetTextFont( /* m_AccountDeltaItems[i].bHeadshot ? m_hDeltaItemFontBig : */ m_hDeltaItemFont );
			vgui::surface()->DrawSetTextColor(finalcolor);
			vgui::surface()->DrawSetTextPos(iX, (int)flYPos);

			wchar_t wBuf[20];

			_snwprintf(wBuf, sizeof(wBuf) / sizeof(wchar_t), L"-%d", m_AccountDeltaItems[i].m_iAmount);

			vgui::surface()->DrawPrintText(wBuf, wcslen(wBuf), FONT_DRAW_DEFAULT);
		}
	}
}