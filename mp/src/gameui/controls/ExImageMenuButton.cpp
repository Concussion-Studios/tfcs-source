//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "vgui/ISurface.h"
#include "ExImageMenuButton.h"

using namespace vgui;

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CExImageMenuButton, CExImageMenuButton );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CExImageMenuButton::CExImageMenuButton( Panel *parent, const char *name, const char *text ) : CExMenuButton( parent, name, text )
{
	SetScheme( vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/ClientScheme.res", "ClientScheme" ) );

	m_clrDraw = Color( 0, 0, 0, 0 );
	m_clrArmed = Color( 0, 0, 0, 0 );
	m_clrSelected = Color( 0, 0, 0, 0 );
	m_pSubImage = new vgui::ImagePanel( this, "SubImage" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CExImageMenuButton::CExImageMenuButton( Panel *parent, const char *name, const wchar_t *wszText ) : CExMenuButton( parent, name, wszText )
{
	SetScheme( vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/ClientScheme.res", "ClientScheme" ) );

	m_clrDraw = Color( 0, 0, 0, 0 );
	m_clrArmed = Color( 0, 0, 0, 0 );
	m_clrSelected = Color( 0, 0, 0, 0 );
	m_pSubImage = new vgui::ImagePanel( this, "SubImage" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExImageMenuButton::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings(inResourceData);

	m_clrDraw = inResourceData->GetColor( "image_drawcolor" );
	m_clrArmed = inResourceData->GetColor( "image_armedcolor" );
	m_clrDepressed = inResourceData->GetColor( "image_depressedcolor" );
	m_clrDisabled = inResourceData->GetColor( "image_disabledcolor" );
	m_clrSelected = inResourceData->GetColor( "image_selectedcolor" );

	KeyValues* SubImage = inResourceData->FindKey("SubImage");
	if ( SubImage )
		m_pSubImage->ApplySettings(SubImage);
	const char* ImageDefault = inResourceData->GetString("image_default");
	if ( ImageDefault )
	{
		Q_strncpy(m_pszImageDefaultName, ImageDefault, sizeof(m_pszImageDefaultName));
		if ( !IsArmed() )
			m_pSubImage->SetImage(m_pszImageDefaultName);
	}
	const char* ImageArmed = inResourceData->GetString("image_armed");
	if ( ImageArmed )
	{
		Q_strncpy(m_pszImageArmedName, ImageArmed, sizeof(m_pszImageArmedName));
		if ( IsArmed() )
			m_pSubImage->SetImage(m_pszImageArmedName);
	}
	const char* ImageSelected = inResourceData->GetString("image_selected");
	if ( ImageSelected )
	{
		Q_strncpy( m_pszImageSelectedName, ImageSelected, sizeof( m_pszImageArmedName ) );
		if ( IsSelected() )
			m_pSubImage->SetImage(m_pszImageArmedName);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExImageMenuButton::SetArmed( bool state )
{
	BaseClass::SetArmed( state );

	Color drawcolor;
	if ( m_pSubImage )
	{
		Color drawcolor;
		if ( IsEnabled() )
		{
			if ( IsSelected() )
				drawcolor = m_clrSelected;
			else if ( IsDepressed() )
				drawcolor = m_clrDepressed;
			else if ( IsArmed() )
				drawcolor = m_clrArmed;
			else 
				drawcolor = m_clrDraw;
		}
		else
		{
			drawcolor = m_clrDisabled;
		}
		m_pSubImage->SetDrawColor( drawcolor );

		char* image = m_pszImageDefaultName;
		if ( IsArmed() )
			image = m_pszImageArmedName;
		if ( image )
			m_pSubImage->SetImage(image);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExImageMenuButton::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pSubImage->SetMouseInputEnabled( false );
	Color drawcolor;
	if ( IsEnabled() )
	{
		if ( IsSelected() )
			drawcolor = m_clrSelected;
		else if ( IsDepressed() )
			drawcolor = m_clrDepressed;
		else if ( IsArmed() )
			drawcolor = m_clrArmed;
		else 
			drawcolor = m_clrDraw;
	}
	else
	{
		drawcolor = m_clrDisabled;
	}
	m_pSubImage->SetDrawColor( drawcolor );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExImageMenuButton::SetImageArmed( const char* image )
{
	Q_strncpy( m_pszImageArmedName, image, sizeof( m_pszImageArmedName ) );
	if ( IsArmed() )
		m_pSubImage->SetImage( m_pszImageArmedName );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExImageMenuButton::SetImageSelected( const char* image )
{
	Q_strncpy( m_pszImageSelectedName, image, sizeof( m_pszImageSelectedName ));
	if ( IsSelected() )
		m_pSubImage->SetImage(m_pszImageSelectedName);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExImageMenuButton::SetSelected( bool state )
{
	BaseClass::SetSelected( state );

	Color drawcolor;
	if ( m_pSubImage )
	{
		Color drawcolor;
		if ( IsEnabled() )
		{
			if ( IsSelected() )
				drawcolor = m_clrSelected;
			else if ( IsDepressed() )
				drawcolor = m_clrDepressed;
			else if ( IsArmed() )
				drawcolor = m_clrArmed;
			else 
				drawcolor = m_clrDraw;
		}
		else
		{
			drawcolor = m_clrDisabled;
		}
		m_pSubImage->SetDrawColor( drawcolor );

		char* image = m_pszImageDefaultName;
		if ( IsSelected() )
			image = m_pszImageSelectedName;
		if ( image )
			m_pSubImage->SetImage(image);
	}
}