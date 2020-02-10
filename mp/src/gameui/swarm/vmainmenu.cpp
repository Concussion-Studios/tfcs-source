//========= Copyright  1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "vmainmenu.h"
#include "EngineInterface.h"
#include "vhybridbutton.h"
#include "vflyoutmenu.h"
#include "vgenericconfirmation.h"
#include "basemodpanel.h"
#include "uigamedata.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Tooltip.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Image.h"
#include "materialsystem/materialsystem_config.h"
#include "ienginevgui.h"
#include "basepanel.h"
#include "vgui/ISurface.h"
#include "tier0/icommandline.h"
#include "fmtstr.h"
#include "FileSystem.h"
#include "engine/IEngineSound.h"
#include "time.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

void AddSubKeyNamed( KeyValues *pKeys, const char *pszName )
{
	KeyValues *pKeyvalToAdd = new KeyValues( pszName );

	if ( pKeyvalToAdd )
		pKeys->AddSubKey( pKeyvalToAdd );
}

ConVar ui_mainmenu_music( "ui_mainmenu_music", "1", FCVAR_ARCHIVE, "Toggle music in the main menu." );

//=============================================================================
MainMenu::MainMenu( Panel *parent, const char *panelName ):	BaseClass( parent, panelName, true, true, false, false )
{
	SetProportional( true );
	SetTitle( "", false );
	SetMoveable( false );
	SetSizeable( false );

	SetLowerGarnishEnabled( true );

	m_psMusicStatus = MUSIC_FIND;
	m_pzMusicLink[0] = '\0';
	m_nSongGuid = 0;

	m_pBGImage = NULL;
	m_pLogoImage = NULL;

	AddFrameListener( this );

	SetDeleteSelfOnClose( true );
}

//=============================================================================
MainMenu::~MainMenu()
{
	RemoveFrameListener( this );
}

//=============================================================================
void MainMenu::OnCommand( const char *command )
{
	if ( UI_IsDebug() )
		ConColorMsg( Color( 77, 116, 85, 255 ), "[GAMEUI] Handling main menu command %s\n", command );

	bool bOpeningFlyout = false;

	if ( !Q_strcmp( command, "Achievements" ) )
	{
		CBaseModPanel::GetSingleton().OpenAchievementsDialog( this );
	}
	else if ( !Q_strcmp( command, "Credits" ) )
	{
		engine->ClientCmd( "WhatsNew" );
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
	else if ( !Q_stricmp( command, "DevConsole" ) )
	{
		engine->ClientCmd_Unrestricted( "toggleconsole" );
	}
	else if ( !Q_stricmp( command, "DemoEditor" ) )
	{
		engine->ClientCmd_Unrestricted( "demoui2" );
	}
	else if ( !Q_strcmp( command, "GameOptions" ) )
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
	else if ( !Q_strcmp( command, "randommusic" ) )
	{
		enginesound->StopSoundByGuid( m_nSongGuid );
	}
	else
	{
		// does this command match a flyout menu?
		BaseModUI::FlyoutMenu *flyout = dynamic_cast< FlyoutMenu* >( FindChildByName( command ) );
		if ( flyout )
		{
			bOpeningFlyout = true;

			// If so, enumerate the buttons on the menu and find the button that issues this command.
			// (No other way to determine which button got pressed; no notion of "current" button on PC.)
			for ( int iChild = 0; iChild < GetChildCount(); iChild++ )
			{
				bool bFound = false;

				if ( !bFound )
				{
					BaseModHybridButton *hybrid = dynamic_cast<BaseModHybridButton *>( GetChild( iChild ) );
					if ( hybrid && hybrid->GetCommand() && !Q_strcmp( hybrid->GetCommand()->GetString( "command" ), command ) )
					{
						hybrid->NavigateFrom();
						// open the menu next to the button that got clicked
						flyout->OpenMenu( hybrid );
						flyout->SetListener( this );
						break;
					}
				}
			}
		}
		else
		{
			BaseClass::OnCommand( command );
		}
	}

	if( !bOpeningFlyout )
		FlyoutMenu::CloseActiveMenu(); //due to unpredictability of mouse navigation over keyboard, we should just close any flyouts that may still be open anywhere.
}

//=============================================================================
void MainMenu::OnKeyCodePressed( KeyCode code )
{
	int userId = GetJoystickForCode( code );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( userId );

	switch( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_B:
		// Capture the B key so it doesn't play the cancel sound effect
		break;

	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

//=============================================================================
void MainMenu::OnKeyCodeTyped( KeyCode code )
{
	switch ( code )
	{
	case KEY_G:
		OnCommand( "randommusic" );
		break;
	}

	BaseClass::OnKeyTyped( code );
}

//=============================================================================
void MainMenu::OnThink()
{
	BaseClass::OnThink();
}

//=============================================================================
void MainMenu::OnOpen()
{
	BaseClass::OnOpen();
}

//=============================================================================
void MainMenu::PaintBackground() 
{
	// TODO: Add script settings
	vgui::surface()->DrawSetColor( Color(m_BackgroundColor) );
	vgui::surface()->DrawFilledRectFade( 0, 0, GetWide(), GetTall(), 255, 0, true );

	if ( m_pBGImage )
	{
		int x, y, wide, tall;
		m_pBGImage->GetBounds( x, y, wide, tall );
		surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
		surface()->DrawSetTexture( m_pBGImage->GetImage()->GetID() );
		surface()->DrawTexturedRect( x, y, x+wide, y+tall );
	}
}

//=============================================================================
void MainMenu::RunFrame()
{
	BaseClass::RunFrame();

	if ( ui_mainmenu_music.GetBool() && !CommandLine()->FindParm( "-nostartupsound" ) )
	{
		if ( ( m_psMusicStatus == MUSIC_FIND || m_psMusicStatus == MUSIC_STOP_FIND ) && !enginesound->IsSoundStillPlaying( m_nSongGuid ) )
		{
			GetRandomMusic( m_pzMusicLink, sizeof( m_pzMusicLink ) );
			m_psMusicStatus = MUSIC_PLAY;
		}
		else if ( ( m_psMusicStatus == MUSIC_PLAY || m_psMusicStatus == MUSIC_STOP_PLAY )&& m_pzMusicLink[0] != '\0' )
		{
			enginesound->StopSoundByGuid( m_nSongGuid );
			ConVar *snd_musicvolume = cvar->FindVar( "snd_musicvolume" );
			float fVolume = ( snd_musicvolume ? snd_musicvolume->GetFloat() : 1.0f );
			enginesound->EmitAmbientSound( m_pzMusicLink, fVolume, PITCH_NORM, 0 );			
			m_nSongGuid = enginesound->GetGuidForLastSoundEmitted();
			m_psMusicStatus = MUSIC_FIND;
		}
	}
	else if ( m_psMusicStatus == MUSIC_FIND )
	{
		enginesound->StopSoundByGuid( m_nSongGuid );
		m_psMusicStatus = ( m_nSongGuid == 0 ? MUSIC_STOP_FIND : MUSIC_STOP_PLAY );
	}
}

//=============================================================================
void MainMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	int screenWide, screenTall;
	surface()->GetScreenSize( screenWide, screenTall );

	float aspectRatio = (float)screenWide/(float)screenTall;
	bool bIsWidescreen = aspectRatio >= 1.5999f;

	m_pBGImage = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "Background" ) );
	if ( m_pBGImage )
	{
		// set the correct background image
		char szBGName[MAX_PATH];
		engine->GetMainMenuBackgroundName( szBGName, sizeof( szBGName ) );
		char szImage[MAX_PATH];
		Q_snprintf( szImage, sizeof( szImage ), "../console/%s", szBGName );

		if ( bIsWidescreen )
			Q_strcat( szImage, "_widescreen", sizeof( szImage ) );

		m_pBGImage->SetImage( szImage );

		// we will custom draw
		//m_pBGImage->SetVisible( false );
	}

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

	if ( bIsWidescreen )
		AddSubKeyNamed( pConditions, "if_wider" );

	LoadControlSettings( "Resource/UI/BaseModUI/MainMenu.res", NULL, NULL, pConditions );

	if ( pConditions )
		pConditions->deleteThis();
}

//=============================================================================
void MainMenu::GetRandomMusic( char *pszBuf, int iBufLength )
{
	Assert( iBufLength );

	char szPath[MAX_PATH];

	// Check that there's music available
	if ( !g_pFullFileSystem->FileExists( "sound/ui/music/gamestartup1.mp3" ) )
	{
		Assert(false);
		*pszBuf = '\0';
	}

	// Discover tracks, 1 through n
	int iLastTrack = 0;
	do
	{
		Q_snprintf( szPath, sizeof(szPath), "sound/ui/music/gamestartup%d.mp3", ++iLastTrack );
	} while ( g_pFullFileSystem->FileExists( szPath ) );

	// Pick a random one
	Q_snprintf( szPath, sizeof( szPath ), "ui/music/gamestartup%d.mp3", RandomInt( 1, iLastTrack - 1 ) );
	Q_strncpy( pszBuf, szPath, iBufLength);
}

//=============================================================================
void MainMenu::AcceptQuitGameCallback()
{
	if ( MainMenu *pMainMenu = static_cast< MainMenu* >( CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ) ) )
		pMainMenu->OnCommand( "QuitGame_NoConfirm" );
}