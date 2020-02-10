//========= Copyright  1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "VInGameMainMenu.h"
#include "VGenericConfirmation.h"
#include "VFlyoutMenu.h"
#include "VHybridButton.h"
#include "EngineInterface.h"
#include "fmtstr.h"
#include "game/client/IGameClientExports.h"
#include "GameUI_Interface.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui/ISurface.h"
#include "materialsystem/materialsystem_config.h"
#include "FileSystem.h"
#include "time.h"

// UI defines. Include if you want to implement some of them [str]
#include "ui_defines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern IVEngineClient *engine;

extern void AddSubKeyNamed( KeyValues *pKeys, const char *pszName );

//=============================================================================
InGameMainMenu::InGameMainMenu( Panel *parent, const char *panelName ): BaseClass( parent, panelName, false, true )
{
	SetDeleteSelfOnClose(true);

	SetProportional( true );
	SetTitle( "", false );

	SetLowerGarnishEnabled( true );
}

//=============================================================================
InGameMainMenu::~InGameMainMenu()
{
}

//=============================================================================
void InGameMainMenu::OnCommand( const char *command )
{
	if ( UI_IsDebug() )
		ConColorMsg( Color( 77, 116, 85, 255 ), "[GAMEUI] Handling ingame menu command %s\n", command );

	if ( !Q_strcmp( command, "ReturnToGame" ) )
	{
		engine->ClientCmd( "gameui_hide" );
	}
	else if( !Q_strcmp( command, "MutePlayer" ) )
	{
		CBaseModPanel::GetSingleton().OpenPlayerListDialog( this );
	}
	else if ( !Q_stricmp( command, "DevConsole" ) )
	{
		engine->ClientCmd_Unrestricted( "toggleconsole" );
	}
	else if ( !Q_stricmp( command, "DemoEditor" ) )
	{
		engine->ClientCmd_Unrestricted( "demoui2" );
	}
	else if ( !Q_strcmp(command, "GameOptions" ) )
	{
		CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
	}
	else if ( !Q_strcmp( command, "ServerBrowser" ) )
	{
		CBaseModPanel::GetSingleton().OpenServerBrowser();
	}
	else if( !Q_strcmp( command, "CreateGame" ) )
	{
		CBaseModPanel::GetSingleton().OpenCreateMultiplayerGameDialog( this );
	}
	else if ( !Q_strcmp( command, "Achievements" ) )
	{
		CBaseModPanel::GetSingleton().OpenAchievementsDialog( this );
	}
	else if ( !Q_strcmp( command, "Credits" ) )
	{
		engine->ClientCmd( "WhatsNew" );
	}
	else if ( !Q_strcmp( command, "CallVote" ) )
	{
		engine->ClientCmd( "gameui_hide" );
		engine->ClientCmd( "callvote" );
	}
	else if( !Q_strcmp( command, "ExitToMainMenu" ) )
	{
		MakeGenericDialog( "#TF_MM_Disconnect_Title", 
						   "#TF_MM_Disconnect", 
						   true, 
						   &LeaveGameOkCallback,
						   true,
						   this );
	}
	else if ( !Q_strcmp( command, "QuitGame" ) )
	{
		MakeGenericDialog( "#MMenu_PromptQuit_Title", 
						   "#MMenu_PromptQuit_Body", 
						   true, 
						   &AcceptQuitGameCallback, 
						   true,
						   this );
	}
	else if ( !Q_stricmp( command, "QuitGame_NoConfirm" ) )
	{
		engine->ClientCmd( "quit" );
	}
	else
	{
		const char *pchCommand = command;

		// does this command match a flyout menu?
		BaseModUI::FlyoutMenu *flyout = dynamic_cast< FlyoutMenu* >( FindChildByName( pchCommand ) );
		if ( flyout )
		{
			// If so, enumerate the buttons on the menu and find the button that issues this command.
			// (No other way to determine which button got pressed; no notion of "current" button on PC.)
			for ( int iChild = 0; iChild < GetChildCount(); iChild++ )
			{
				BaseModHybridButton *hybrid = dynamic_cast<BaseModHybridButton *>( GetChild( iChild ) );
				if ( hybrid && hybrid->GetCommand() && !Q_strcmp( hybrid->GetCommand()->GetString( "command"), command ) )
				{
					// open the menu next to the button that got clicked
					flyout->OpenMenu( hybrid );
					break;
				}
			}
		}
	}
}

//=============================================================================
void InGameMainMenu::OnKeyCodePressed( KeyCode code )
{
	int userId = GetJoystickForCode( code );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( userId );

	switch( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_START:
	case KEY_XBUTTON_B:
		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_BACK );
		OnCommand( "ReturnToGame" );
		break;
	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

//=============================================================================
void InGameMainMenu::OnKeyCodeTyped( KeyCode code )
{
	BaseClass::OnKeyTyped( code );
}

//=============================================================================
void InGameMainMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pLogoImage = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "TFC_Logo" ) );

	KeyValues *pConditions = new KeyValues( "conditions" );

	time_t ltime = time(0);
	const time_t *ptime = &ltime;
	struct tm *today = localtime( ptime );
	if ( today )
	{
		if ( ( today->tm_mon == 9 ) && ( today->tm_mday == 26 || today->tm_mday == 27 || today->tm_mday == 28 || today->tm_mday == 29 || today->tm_mday == 30 || today->tm_mday == 31 ) )
			AddSubKeyNamed( pConditions, "if_halloween" );
		else if ( ( today->tm_mon == 11 ) && today->tm_mday == 23 || today->tm_mday == 24 || today->tm_mday == 25 )
			AddSubKeyNamed( pConditions, "if_christmas" );
	}

	LoadControlSettings( "Resource/UI/BaseModUI/InGameMainMenu.res", NULL, NULL, pConditions );

	SetPaintBackgroundEnabled( true );

	if ( pConditions )
		pConditions->deleteThis();
}

//=============================================================================
void InGameMainMenu::OnOpen()
{
	BaseClass::OnOpen();
}

//=============================================================================
void InGameMainMenu::OnClose()
{
	Unpause();

	// During shutdown this calls delete this, so Unpause should occur before this call
	BaseClass::OnClose();
}

//=============================================================================
void InGameMainMenu::OnThink()
{
	BaseClass::OnThink();

	if ( IsVisible() )
	{
		// Yield to generic message box if is present
		WINDOW_TYPE arrYield[] = { WT_GENERICCONFIRMATION };
		for ( int j = 0; j < ARRAYSIZE( arrYield ); ++ j )
		{
			CBaseModFrame *pYield = CBaseModPanel::GetSingleton().GetWindow( arrYield[j] );
			if ( pYield && pYield->IsVisible() && !pYield->HasFocus() )
			{
				pYield->Activate();
				pYield->RequestFocus();
			}
		}
	}
}

//=============================================================================
void InGameMainMenu::PerformLayout( void )
{
	BaseClass::PerformLayout();
}

//=============================================================================
void InGameMainMenu::OnGameUIHidden()
{
	Unpause();
	Close();
}

//=============================================================================
void InGameMainMenu::PaintBackground()
{
	// TODO: Add script settings
	vgui::surface()->DrawSetColor( Color(0, 0, 0, 255) );
	vgui::surface()->DrawFilledRectFade( 0, 0, GetWide(), GetTall(), 255, 0, true );
}

//=============================================================================
void InGameMainMenu::LeaveGameOkCallback()
{
	COM_TimestampedLog( "Exit Game" );

	if ( InGameMainMenu *self = static_cast< InGameMainMenu* >( CBaseModPanel::GetSingleton().GetWindow( WT_INGAMEMAINMENU ) ) )
		self->Close();

	engine->ExecuteClientCmd( "gameui_hide" );

	// On PC people can be playing via console bypassing matchmaking
	// and required session settings, so to leave game duplicate
	// session closure with an extra "disconnect" command.
	engine->ExecuteClientCmd( "disconnect" );

	engine->ExecuteClientCmd( "gameui_activate" );

	CBaseModPanel::GetSingleton().CloseAllWindows();
	CBaseModPanel::GetSingleton().OpenFrontScreen();
}

//=============================================================================
void InGameMainMenu::AcceptQuitGameCallback()
{
	if ( InGameMainMenu *pMainMenu = static_cast< InGameMainMenu* >( CBaseModPanel::GetSingleton().GetWindow( WT_INGAMEMAINMENU ) ) )
		pMainMenu->OnCommand( "QuitGame_NoConfirm" );
}