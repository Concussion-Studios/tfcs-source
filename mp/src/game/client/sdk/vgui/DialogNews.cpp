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
#include "DialogTabChangeLog.h"
#include "DialogTabCredits.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

static void CC_Dialog()
{
	CDialogNews*options = new CDialogNews( NULL );
	options->Activate();
}

static ConCommand WhatsNew( "mdlpickerdialog", CC_Dialog, "Show/hide News UI." );

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CDialogNews::CDialogNews(vgui::Panel *parent) : PropertyDialog( parent, "DialogNews" )
{
	SetDeleteSelfOnClose( true );
	SetBounds( 0, 0, 512, 406 );
	SetSizeable( false );

	SetTitle( "#GameUI_News", true );

	AddPage( new CDialogTabChangeLog( this ), "#GameUI_Changelog" );
	AddPage( new CDialogTabCredits( this ), "#GameUI_Credits" );

	SetApplyButtonVisible( false );
	GetPropertySheet()->SetTabWidth(84);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogNews::~CDialogNews()
{
}

//-----------------------------------------------------------------------------
// Purpose: Brings the dialog to the fore
//-----------------------------------------------------------------------------
void CDialogNews::Activate()
{
	BaseClass::Activate();
	EnableApplyButton( false );
}

//-----------------------------------------------------------------------------
// Purpose: Opens the dialog
//-----------------------------------------------------------------------------
void CDialogNews::Run()
{
	SetTitle( "#GameUI_News", true );
	Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the GameUI is hidden
//-----------------------------------------------------------------------------
void CDialogNews::OnGameUIHidden()
{
	// tell our children about it
	for ( int i = 0 ; i < GetChildCount() ; i++ )
	{
		Panel *pChild = GetChild( i );
		if ( pChild )
			PostMessage( pChild, new KeyValues( "GameUIHidden" ) );
	}
}
