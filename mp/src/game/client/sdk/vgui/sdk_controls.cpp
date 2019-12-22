//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include <vgui_controls/ScrollBarSlider.h>
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "sdk_controls.h"

using namespace vgui;

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CExButton, CExButton );
DECLARE_BUILD_FACTORY_DEFAULT_TEXT( CExLabel, CExLabel );
DECLARE_BUILD_FACTORY( CExRichText );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CExButton::CExButton(Panel *parent, const char *name, const char *text) : Button(parent, name, text)
{
	m_szFont[0] = '\0';
	m_szColor[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CExButton::CExButton(Panel *parent, const char *name, const wchar_t *wszText) : Button(parent, name, wszText)
{
	m_szFont[0] = '\0';
	m_szColor[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExButton::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy( m_szFont, inResourceData->GetString( "font", "Default" ), sizeof( m_szFont ) );
	Q_strncpy( m_szColor, inResourceData->GetString( "fgcolor", "Button.TextColor" ), sizeof( m_szColor ) );

	InvalidateLayout( false, true ); // force ApplySchemeSettings to run
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExButton::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFont( pScheme->GetFont( m_szFont, true ) );
	SetFgColor( pScheme->GetColor( m_szColor, Color( 255, 255, 255, 255 ) ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CExLabel::CExLabel(Panel *parent, const char *name, const char *text) : Label(parent, name, text)
{
	m_szColor[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CExLabel::CExLabel(Panel *parent, const char *name, const wchar_t *wszText) : Label(parent, name, wszText)
{
	m_szColor[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExLabel::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy( m_szColor, inResourceData->GetString( "fgcolor", "Label.TextColor" ), sizeof( m_szColor ) );

	InvalidateLayout( false, true ); // force ApplySchemeSettings to run
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExLabel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFgColor( pScheme->GetColor( m_szColor, Color( 255, 255, 255, 255 ) ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CExRichText::CExRichText(Panel *parent, const char *name) : RichText(parent, name)
{
	m_szFont[0] = '\0';
	m_szColor[0] = '\0';

	SetCursor(dc_arrow);

	m_pUpArrow = new CSDKImagePanel( this, "UpArrow" );
	if ( m_pUpArrow )
	{
		m_pUpArrow->SetShouldScaleImage( true );
		m_pUpArrow->SetImage( "chalkboard_scroll_up" );
		m_pUpArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
		m_pUpArrow->SetAlpha( 255 );
		m_pUpArrow->SetVisible( false );
	}

	m_pLine = new ImagePanel( this, "Line" );
	if ( m_pLine )
	{
		m_pLine->SetShouldScaleImage( true );
		m_pLine->SetImage( "chalkboard_scroll_line" );
		m_pLine->SetVisible( false );
	}

	m_pDownArrow = new CSDKImagePanel( this, "DownArrow" );
	if ( m_pDownArrow )
	{
		m_pDownArrow->SetShouldScaleImage( true );
		m_pDownArrow->SetImage( "chalkboard_scroll_down" );
		m_pDownArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
		m_pDownArrow->SetAlpha( 255 );
		m_pDownArrow->SetVisible( false );
	}

	m_pBox = new ImagePanel( this, "Box" );
	if ( m_pBox )
	{
		m_pBox->SetShouldScaleImage( true );
		m_pBox->SetImage( "chalkboard_scroll_box" );
		m_pBox->SetVisible( false );
	}

	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExRichText::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy( m_szFont, inResourceData->GetString( "font", "Default" ), sizeof( m_szFont ) );
	Q_strncpy( m_szColor, inResourceData->GetString( "fgcolor", "RichText.TextColor" ), sizeof( m_szColor ) );

	InvalidateLayout( false, true ); // force ApplySchemeSettings to run
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExRichText::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFont( pScheme->GetFont( m_szFont, true  ) );
	SetFgColor( pScheme->GetColor( m_szColor, Color( 255, 255, 255, 255 ) ) );

	SetBorder( pScheme->GetBorder( "NoBorder" ) );
	SetBgColor( pScheme->GetColor( "Blank", Color( 0,0,0,0 ) ) );
	SetPanelInteractive( false );
	SetUnusedScrollbarInvisible( true );

	if ( m_pDownArrow  )
	{
		m_pDownArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
	}

	if ( m_pUpArrow  )
	{
		m_pUpArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
	}

	SetScrollBarImagesVisible( false );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExRichText::PerformLayout()
{
	BaseClass::PerformLayout();

	if ( _vertScrollBar && _vertScrollBar->IsVisible() )
	{
		int nMin, nMax;
		_vertScrollBar->GetRange( nMin, nMax );
		_vertScrollBar->SetValue( nMin );

		int nScrollbarWide = _vertScrollBar->GetWide();

		int wide, tall;
		GetSize( wide, tall );

		if ( m_pUpArrow )
		{
			m_pUpArrow->SetBounds( wide - nScrollbarWide, 0, nScrollbarWide, nScrollbarWide );
		}

		if ( m_pLine )
		{
			m_pLine->SetBounds( wide - nScrollbarWide, nScrollbarWide, nScrollbarWide, tall - ( 2 * nScrollbarWide ) );
		}

		if ( m_pBox )
		{
			m_pBox->SetBounds( wide - nScrollbarWide, nScrollbarWide, nScrollbarWide, nScrollbarWide );
		}

		if ( m_pDownArrow )
		{
			m_pDownArrow->SetBounds( wide - nScrollbarWide, tall - nScrollbarWide, nScrollbarWide, nScrollbarWide );
		}

		SetScrollBarImagesVisible( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExRichText::SetText(const wchar_t *text)
{
	wchar_t buffer[2048];
	Q_wcsncpy( buffer, text, sizeof( buffer ) );

	// transform '\r' to ' ' to eliminate double-spacing on line returns
	for ( wchar_t *ch = buffer; *ch != 0; ch++ )
	{
		if ( *ch == '\r' )
		{
			*ch = ' ';
		}
	}

	BaseClass::SetText( buffer );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExRichText::SetText(const char *text)
{
	char buffer[2048];
	Q_strncpy( buffer, text, sizeof( buffer ) );

	// transform '\r' to ' ' to eliminate double-spacing on line returns
	for ( char *ch = buffer; *ch != 0; ch++ )
	{
		if ( *ch == '\r' )
		{
			*ch = ' ';
		}
	}

	BaseClass::SetText( buffer );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExRichText::SetScrollBarImagesVisible(bool visible)
{
	if ( m_pDownArrow && m_pDownArrow->IsVisible() != visible )
	{
		m_pDownArrow->SetVisible( visible );
	}

	if ( m_pUpArrow && m_pUpArrow->IsVisible() != visible )
	{
		m_pUpArrow->SetVisible( visible );
	}

	if ( m_pLine && m_pLine->IsVisible() != visible )
	{
		m_pLine->SetVisible( visible );
	}

	if ( m_pBox && m_pBox->IsVisible() != visible )
	{
		m_pBox->SetVisible( visible );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExRichText::OnTick()
{
	if ( !IsVisible() )
		return;

	if ( m_pDownArrow && m_pUpArrow && m_pLine && m_pBox )
	{
		if ( _vertScrollBar && _vertScrollBar->IsVisible() )
		{
			_vertScrollBar->SetZPos( 500 );

			// turn off painting the vertical scrollbar
			_vertScrollBar->SetPaintBackgroundEnabled( false );
			_vertScrollBar->SetPaintBorderEnabled( false );
			_vertScrollBar->SetPaintEnabled( false );
			_vertScrollBar->SetScrollbarButtonsVisible( false );

			// turn on our own images
			SetScrollBarImagesVisible ( true );

			// set the alpha on the up arrow
			int nMin, nMax;
			_vertScrollBar->GetRange( nMin, nMax );
			int nScrollPos = _vertScrollBar->GetValue();
			int nRangeWindow = _vertScrollBar->GetRangeWindow();
			int nBottom = nMax - nRangeWindow;
			if ( nBottom < 0 )
			{
				nBottom = 0;
			}

			// set the alpha on the up arrow
			int nAlpha = ( nScrollPos - nMin <= 0 ) ? 90 : 255;
			m_pUpArrow->SetAlpha( nAlpha );

			// set the alpha on the down arrow
			nAlpha = ( nScrollPos >= nBottom ) ? 90 : 255;
			m_pDownArrow->SetAlpha( nAlpha );

			ScrollBarSlider *pSlider = _vertScrollBar->GetSlider();
			if ( pSlider && pSlider->GetRangeWindow() > 0 )
			{
				int x, y, w, t, min, max;
				m_pLine->GetBounds( x, y, w, t );
				pSlider->GetNobPos( min, max );

				m_pBox->SetBounds( x, y + min, w, ( max - min ) );
			}
		}
		else
		{
			// turn off our images
			SetScrollBarImagesVisible ( false );
		}
	}
}