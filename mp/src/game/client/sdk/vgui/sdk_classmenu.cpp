//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//
#include "cbase.h"
#include <stdio.h>
#include <cdll_client_int.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/RichText.h>
#include <vgui/IVGUI.h>
#include <vgui_controls/Panel.h>
#include "cdll_util.h"
#include <game/client/iviewport.h>
#include "sdk_backgroundpanel.h"
#include "sdk_gamerules.h"
#include "c_sdk_player.h"
#include "c_sdk_team.h"
#include "sdk_classmenu.h"
#include "IGameUIFuncs.h" // for key bindings

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar _cl_classmenuopen( "_cl_classmenuopen", "0", FCVAR_CLIENTCMD_CAN_EXECUTE, "internal cvar used to tell server when class menu is open" );

extern ConVar mp_allowspecialclass;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CSDKClassInfoPanel::CreateControlByName( const char *controlName )
{
	return BaseClass::CreateControlByName( controlName );
}

void CSDKClassInfoPanel::ApplySchemeSettings( IScheme *pScheme )
{
	m_hFont = pScheme->GetFont( "DefaultVerySmall", true );

	RichText *pClassInfo = dynamic_cast<RichText*>( FindChildByName( "classInfo" ) );
	if ( pClassInfo )
		pClassInfo->SetFont( m_hFont );

	BaseClass::ApplySchemeSettings( pScheme );
}

CSDKClassMenu::CSDKClassMenu(IViewPort *pViewPort) : CClassMenu( pViewPort )
{
	Panel *pClassInfo = dynamic_cast<Panel*>( FindChildByName( "classInfo" ) );
	if ( pClassInfo )
		pClassInfo->MarkForDeletion();

	m_mouseoverButtons.RemoveAll();

	m_iClassMenuKey = BUTTON_CODE_INVALID;
	m_pInitialButton = NULL;

	CreateBackground( this );
	m_backgroundLayoutFinished = false;

	m_pClassInfoPanel = new CSDKClassInfoPanel( this, "ClassInfoPanel" );
	
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_iActivePlayerClass = -1;
	m_iLastPlayerClassCount = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSDKClassMenu::CSDKClassMenu( IViewPort *pViewPort, const char *panelName ) : CClassMenu( pViewPort, panelName )
{
	Panel *pClassInfo = dynamic_cast<Panel*>( FindChildByName( "classInfo" ) );
	if ( pClassInfo )
		pClassInfo->MarkForDeletion();

	m_mouseoverButtons.RemoveAll();

	m_iClassMenuKey = BUTTON_CODE_INVALID;
	m_pInitialButton = NULL;

	CreateBackground( this );
	m_backgroundLayoutFinished = false;

	m_pClassInfoPanel = new CSDKClassInfoPanel( this, "ClassInfoPanel" );
	
	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_iActivePlayerClass = -1;
	m_iLastPlayerClassCount = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CSDKClassMenu::~CSDKClassMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKClassMenu::ShowPanel( bool bShow )
{
	if ( bShow )
	{
		engine->CheckPoint( "ClassMenu" );
		m_iClassMenuKey = gameuifuncs->GetButtonCodeForBind( "changeclass" );
	}

	for( int i = 0; i< GetChildCount(); i++ ) 
	{
		//Tony; using mouse over button for now, later we'll use CModelButton when I get it implemented!!
		MouseOverButton<CSDKClassInfoPanel> *button = dynamic_cast<MouseOverButton<CSDKClassInfoPanel> *>(GetChild(i));
		if ( button )
		{
			if( button == m_pInitialButton && bShow == true )
				button->ShowPage();
			else
				button->HidePage();
		}
	}

	MouseOverButton<CSDKClassInfoPanel> *pRandom =	dynamic_cast<MouseOverButton<CSDKClassInfoPanel> *>( FindChildByName("random") );
	if ( pRandom )
		pRandom->HidePage();

	BaseClass::ShowPanel( bShow );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKClassMenu::OnKeyCodePressed( KeyCode code )
{
	if ( m_iClassMenuKey != BUTTON_CODE_INVALID && m_iClassMenuKey == code )
		ShowPanel( false );
	else
		BaseClass::OnKeyCodePressed( code );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKClassMenu::Update()
{
	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	if ( pPlayer && pPlayer->m_Shared.DesiredPlayerClass() == PLAYERCLASS_UNDEFINED )
		SetVisibleButton( "CancelButton", false );
	else
		SetVisibleButton( "CancelButton", true ); 

	if ( mp_allowspecialclass.GetBool() )
	{
		SetVisibleButton( "blue_class10", true );
		SetVisibleButton( "red_class10", true );
		SetVisibleButton( "green_class10", true );
		SetVisibleButton( "yellow_class10", true );

		Label *pCivLabel = dynamic_cast<Label*>( FindChildByName( "class_10_num" ) );
		if ( pCivLabel )
			pCivLabel->SetVisible( true );
	}
	else
	{
		SetVisibleButton( "blue_class10", false );
		SetVisibleButton( "red_class10", false );
		SetVisibleButton( "green_class10", false );
		SetVisibleButton( "yellow_class10", false );

		Label *pCivLabel = dynamic_cast<Label*>( FindChildByName( "class_10_num" ) );
		if ( pCivLabel )
			pCivLabel->SetVisible( false );
	}

	MoveToCenterOfScreen();

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Panel *CSDKClassMenu::CreateControlByName( const char *controlName )
{
	if ( !Q_stricmp( "SDKButton", controlName ) )
	{
		MouseOverButton<CSDKClassInfoPanel> *newButton = new MouseOverButton<CSDKClassInfoPanel>( this, NULL, m_pClassInfoPanel );

		if( !m_pInitialButton )
			m_pInitialButton = newButton;

		return newButton;
	}
	else
		return BaseClass::CreateControlByName( controlName );
}

//-----------------------------------------------------------------------------
// Catch the mouseover event and set the active class
//-----------------------------------------------------------------------------
void CSDKClassMenu::OnShowPage( const char *pagename )
{
	// change which class we are counting based on class name
	// turn the button name into a classname
	char buf[64];

	Q_snprintf( buf, sizeof(buf), "cls_%s", pagename );

	C_SDKTeam *pTeam = dynamic_cast<C_SDKTeam *>( GetGlobalTeam(GetTeamNumber()) );
	if( !pTeam )
		return;

	// Pull the index of this class via IsClassOnTeam
	if ( !pTeam->IsClassOnTeam( buf, m_iActivePlayerClass ) )
		Assert( !"bad class name on class button" );
}

//-----------------------------------------------------------------------------
// Do things that should be done often, eg number of players in the 
// selected class
//-----------------------------------------------------------------------------
void CSDKClassMenu::OnTick( void )
{
	//When a player changes teams, their class and team values don't get here 
	//necessarily before the command to update the class menu. This leads to the cancel button 
	//being visible and people cancelling before they have a class. check for class == PLAYERCLASS_UNASSIGNED and if so
	//hide the cancel button
	if ( !IsVisible() )
		return;

	C_SDKPlayer *pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if( pPlayer && pPlayer->m_Shared.PlayerClass() == PLAYERCLASS_UNDEFINED )
		SetVisibleButton("CancelButton", false);

	BaseClass::OnTick();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSDKClassMenu::SetVisible( bool state )
{
	BaseClass::SetVisible( state );

	if ( state )
	{
		engine->ServerCmd( "menuopen" );			// to the server
		engine->ClientCmd( "_cl_classmenuopen 1" );	// for other panels
	}
	else
	{
		engine->ServerCmd( "menuclosed" );	
		engine->ClientCmd( "_cl_classmenuopen 0" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the visibility of a button by name
//-----------------------------------------------------------------------------
void CSDKClassMenu::SetVisibleButton( const char *textEntryName, bool state )
{
	Button *entry = dynamic_cast<Button *>( FindChildByName( textEntryName ) );
	if ( entry )
		entry->SetVisible( state );
}

//-----------------------------------------------------------------------------
// Purpose: Scale / center the window
//-----------------------------------------------------------------------------
void CSDKClassMenu::PerformLayout()
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
void CSDKClassMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	ApplyBackgroundSchemeSettings( this, pScheme );
}