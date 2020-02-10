//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Extended Verson Of vgui::Button For GameUI
//
//=============================================================================//
#include "cbase.h"
#include "ExMenuButton.h"
#include <vgui/IScheme.h>
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CExMenuButton, CExMenuButton );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CExMenuButton::CExMenuButton( Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget, const char *pCmd ) 
	: BaseClass( parent, panelName, text )
{
	SetScheme( vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/ClientScheme.res", "ClientScheme" ) );

	m_szFont[0] = '\0';
	m_szColor[0] = '\0';

	if ( pActionSignalTarget && pCmd )
	{
		AddActionSignalTarget( pActionSignalTarget );
		SetCommand( pCmd );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CExMenuButton::CExMenuButton( Panel *parent, const char *panelName, const wchar_t *wszText, Panel *pActionSignalTarget, const char *pCmd ) 
	: BaseClass( parent, panelName, wszText )
{
	SetScheme( vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/ClientScheme.res", "ClientScheme" ) );

	m_szFont[0] = '\0';
	m_szColor[0] = '\0';

	if ( pActionSignalTarget && pCmd )
	{
		AddActionSignalTarget( pActionSignalTarget );
		SetCommand( pCmd );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CExMenuButton::~CExMenuButton()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExMenuButton::OnCommand( const char *command )
{
	BaseClass::OnCommand( command );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CExMenuButton::OnCursorEntered()
{
	if ( IsEnabled() )
		surface()->PlaySound( "ui/buttonrollover.wav" );

	BaseClass::OnCursorEntered();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExMenuButton::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy( m_szFont, inResourceData->GetString( "font", "Default" ), sizeof( m_szFont ) );
	Q_strncpy( m_szColor, inResourceData->GetString( "fgcolor", "Button.TextColor" ), sizeof( m_szColor ) );

	InvalidateLayout( false, true ); // force ApplySchemeSettings to run
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CExMenuButton::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFont( pScheme->GetFont( m_szFont, true ) );
	SetFgColor( pScheme->GetColor( m_szColor, Color( 255, 255, 255, 255 ) ) );

	SetReleasedSound( "ui/buttonclick.wav" );
}