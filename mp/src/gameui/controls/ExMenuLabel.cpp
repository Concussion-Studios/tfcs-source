//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "ExMenuLabel.h"
#include <vgui_controls/ScrollBarSlider.h>
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"

using namespace vgui;

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CExMenuLabel, CExMenuLabel );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CExMenuLabel::CExMenuLabel(Panel *parent, const char *name, const char *text) : Label(parent, name, text)
{
	SetScheme( vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/ClientScheme.res", "ClientScheme" ) );

	m_szColor[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CExMenuLabel::CExMenuLabel(Panel *parent, const char *name, const wchar_t *wszText) : Label(parent, name, wszText)
{
	SetScheme( vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/ClientScheme.res", "ClientScheme" ) );

	m_szColor[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExMenuLabel::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy( m_szColor, inResourceData->GetString( "fgcolor", "Label.TextColor" ), sizeof( m_szColor ) );

	InvalidateLayout( false, true ); // force ApplySchemeSettings to run
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExMenuLabel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFgColor( pScheme->GetColor( m_szColor, Color( 255, 255, 255, 255 ) ) );
}