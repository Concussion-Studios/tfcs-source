//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include <vgui/isurface.h>
#include <vgui/ISystem.h>
#include "hud_numericdisplay.h"
#include "c_sdk_player.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Armor panel
//-----------------------------------------------------------------------------
class CHudArmor : public CHudElement, public CHudNumericDisplay
{
public:
	DECLARE_CLASS_SIMPLE( CHudArmor, CHudNumericDisplay );

	CHudArmor( const char *name );

	virtual void Paint();
	virtual void Init() {}
	virtual void ApplySchemeSettings( IScheme *scheme );

private:
	CHudTexture *m_pArmorIcon;

	CPanelAnimationVarAliasType( float, icon_xpos, "icon_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, icon_ypos, "icon_ypos", "2", "proportional_float" );

	float icon_wide;
	float icon_tall;
};


DECLARE_HUDELEMENT( CHudArmor );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudArmor::CHudArmor( const char *pName ) : CHudNumericDisplay( NULL, "HudArmor" ), CHudElement( pName )
{
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );

	if( !m_pArmorIcon )
		m_pArmorIcon = gHUD.GetIcon( "item_battery" );

	if( m_pArmorIcon )
	{
		icon_tall = GetTall() - YRES(2);
		float scale = icon_tall / (float)m_pArmorIcon->Height();
		icon_wide = ( scale ) * (float)m_pArmorIcon->Width();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::Paint()
{
	// Update the time.
	auto *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if ( pPlayer )
	{
		if( m_pArmorIcon )
			m_pArmorIcon->DrawSelf( icon_xpos, icon_ypos, icon_wide, icon_tall, GetFgColor() );

		SetDisplayValue( (int)pPlayer->GetArmorValue() );
		SetShouldDisplayValue( true );
		BaseClass::Paint();
	}
}

