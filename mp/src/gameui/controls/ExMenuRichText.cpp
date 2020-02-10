//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//


#include "cbase.h"
#include "ExMenuRichText.h"
#include <vgui_controls/ScrollBarSlider.h>
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"

using namespace vgui;

DECLARE_BUILD_FACTORY( CExMenuRichText );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CExMenuRichText::CExMenuRichText(Panel *parent, const char *name) : RichText(parent, name)
{
	SetScheme( vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/ClientScheme.res", "ClientScheme" ) );

	m_szFont[0] = '\0';
	m_szColor[0] = '\0';

	SetCursor(dc_arrow);

	m_pUpArrow = new CExMenuImagePanel( this, "UpArrow" );
	if ( m_pUpArrow )
	{
	//	m_pUpArrow->Shouldsca( true );
	//	m_pUpArrow->SetImage( "chalkboard_scroll_up" );
		m_pUpArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
		m_pUpArrow->SetAlpha( 255 );
		m_pUpArrow->SetVisible( false );
	}

	Q_strncpy( pDefaultScrollLineImage, "chalkboard_scroll_line", sizeof( pDefaultScrollLineImage ) );

	m_pLine = new ImagePanel( this, "Line" );
	if ( m_pLine )
	{
		m_pLine->SetShouldScaleImage( true );
		m_pLine->SetImage( pDefaultScrollLineImage );
		m_pLine->SetVisible( false );
	}

	m_pDownArrow = new CExMenuImagePanel( this, "DownArrow" );
	if ( m_pDownArrow )
	{
//		m_pDownArrow->SetShouldScaleImage( true );
//		m_pDownArrow->SetImage( "chalkboard_scroll_down" );
		m_pDownArrow->SetFgColor( Color( 255, 255, 255, 255 ) );
		m_pDownArrow->SetAlpha( 255 );
		m_pDownArrow->SetVisible( false );
	}

	Q_strncpy( pDefaultScrollImage, "chalkboard_scroll_box", sizeof(pDefaultScrollImage) );

	m_pBox = new ImagePanel( this, "Box" );
	if ( m_pBox )
	{
		m_pBox->SetShouldScaleImage( true );
		m_pBox->SetImage( pDefaultScrollImage );
		m_pBox->SetVisible( false );
	}

	vgui::ivgui()->AddTickSignal( GetVPanel() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExMenuRichText::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );

	Q_strncpy( m_szFont, inResourceData->GetString( "font", "Default" ), sizeof( m_szFont ) );
	Q_strncpy( m_szColor, inResourceData->GetString( "fgcolor", "RichText.TextColor" ), sizeof( m_szColor ) );
	Q_strncpy( pDefaultScrollImage, inResourceData->GetString( "scroll_box_image", "chalkboard_scroll_box" ), sizeof( pDefaultScrollImage ));
	Q_strncpy( pDefaultScrollLineImage, inResourceData->GetString( "scroll_line_image", "chalkboard_scroll_line" ), sizeof( pDefaultScrollLineImage ));

	InvalidateLayout( false, true ); // force ApplySchemeSettings to run
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExMenuRichText::ApplySchemeSettings(IScheme *pScheme)
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
void CExMenuRichText::PerformLayout()
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
void CExMenuRichText::SetText(const wchar_t *text)
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
void CExMenuRichText::SetText(const char *text)
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
void CExMenuRichText::SetScrollBarImagesVisible(bool visible)
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
void CExMenuRichText::OnTick()
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
