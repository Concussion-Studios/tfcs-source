//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//====================================================================================//

#include "cbase.h"
#include "sdk_textwindow.h"
#include <cdll_client_int.h>
#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVgui.h>
#include "sdk_backgroundpanel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSDKTextWindow::CSDKTextWindow(IViewPort *pViewPort) : CTextWindow( pViewPort )
{
	SetProportional( true );
	CreateBackground( this );

	m_backgroundLayoutFinished = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKTextWindow::Update()
{
	BaseClass::Update();

	m_pOK->RequestFocus();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKTextWindow::SetVisible( bool state )
{
	BaseClass::SetVisible( state );

	if ( state )
		m_pOK->RequestFocus();
}

//-----------------------------------------------------------------------------
// Purpose: Scale / center the window
//-----------------------------------------------------------------------------
void CSDKTextWindow::PerformLayout()
{
	BaseClass::PerformLayout();

	// stretch the window to fullscreen
	if ( !m_backgroundLayoutFinished )
		LayoutBackgroundPanel( this );

	m_backgroundLayoutFinished = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKTextWindow::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	ApplyBackgroundSchemeSettings( this, pScheme );
}