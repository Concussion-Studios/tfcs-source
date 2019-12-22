//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "DialogNews.h"

#include "vgui_controls/Button.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/QueryBox.h"

#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"

#include "KeyValues.h"
#include "ienginevgui.h"

#include "DialogTabChangeLog.h"
#include "DialogTabCredits.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

static DHANDLE<CDialogNews> g_hCreditsMenu;

CON_COMMAND( WhatsNew, "Show/hide News UI." )
{
	if ( !g_hCreditsMenu.Get() )
	{
		VPANEL parent = enginevgui->GetPanel( PANEL_GAMEUIDLL );
		if ( parent == NULL )
		{
			Assert( 0 );
			return;
		}

		auto* pPanel = new CDialogNews( parent );

		g_hCreditsMenu.Set( pPanel );
	}


	auto* pPanel = g_hCreditsMenu.Get();


	// Center
	int x, y, w, h;
	vgui::surface()->GetWorkspaceBounds( x, y, w, h );
	
	int mw = pPanel->GetWide();
	int mh = pPanel->GetTall();
	pPanel->SetPos( x + w / 2 - mw / 2, y + h / 2 - mh / 2 );


	pPanel->Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CDialogNews::CDialogNews( VPANEL parent ) : PropertyDialog( nullptr, "DialogNews" )
{
	SetParent( parent );

	SetDeleteSelfOnClose( true );
	SetBounds( 0, 0, 512, 406 );
	SetSizeable( false );

	SetTitle( "#GameUI_News", true );

	AddPage( new CDialogTabChangeLog( this ), "#GameUI_Changelog" );
	AddPage( new CDialogTabCredits( this ), "#GameUI_Credits" );

	SetApplyButtonVisible( false );
	SetCancelButtonVisible( false );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogNews::~CDialogNews()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDialogNews::OnClose( void )
{
	BaseClass::OnClose();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDialogNews::OnCommand( const char *command )
{
		BaseClass::OnCommand( command );
}