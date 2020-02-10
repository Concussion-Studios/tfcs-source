//========= Copyright  1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "basemodpanel.h"
#include "./GameUI/IGameUI.h"
#include "ienginevgui.h"
#include "engine/IEngineSound.h"
#include "EngineInterface.h"
#include "tier0/dbg.h"
#include "ixboxsystem.h"
#include "GameUI_Interface.h"
#include "game/client/IGameClientExports.h"
#include "GameUI/IGameConsole.h"
#include "inputsystem/iinputsystem.h"
#include "filesystem.h"
#include "tier2/renderutils.h"
#include "vgui_video_player.h"
#include "vgui/ISystem.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/AnimationController.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imesh.h"
#include "tier0/icommandline.h"
#include "fmtstr.h"
#include "smartptr.h"
#include "gameconsole.h"
#include "VGuiSystemModuleLoader.h"

// BaseModUI High-level windows

#include "vgenericconfirmation.h"
#include "vingamemainmenu.h"
#include "vloadingprogress.h"
#include "vmainmenu.h"
#include "nb_header_footer.h"

// UI defines. Include if you want to implement some of them [str]
#include "ui_defines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;
using namespace vgui;

//setup in GameUI_Interface.cpp
extern class IMatchSystem *matchsystem;
extern const char *COM_GetModDirectory( void );
extern class IGameConsole *IGameConsole();
extern ISoundEmitterSystemBase *soundemitterbase;
extern IGameEventManager2 *gameeventmanager;

//=============================================================================
CBaseModPanel* CBaseModPanel::m_CFactoryBasePanel = 0;

ConVar ui_gameui_debug( "ui_gameui_debug", "0", FCVAR_HIDDEN );
int UI_IsDebug()
{
	return (*(int *)(&ui_gameui_debug)) ? ui_gameui_debug.GetInt() : 0;
}

CON_COMMAND( ui_frames_reload, "Reload all GameUI frames." )
{
	CBaseModPanel::GetSingleton().InvalidateFramesLayout( true, true );
}

//=============================================================================
CBaseModPanel::CBaseModPanel(): BaseClass( 0, "CBaseModPanel" ),
	m_bClosingAllWindows( false ),
	m_lastActiveUserId( 0 )
{
#if !defined( NOSTEAM )
	// Set Steam overlay position
	if ( steamapicontext && steamapicontext->SteamUtils() )
		steamapicontext->SteamUtils()->SetOverlayNotificationPosition( k_EPositionTopRight );
#endif

	MakePopup( false );

	Assert(m_CFactoryBasePanel == 0);
	m_CFactoryBasePanel = this;

	g_pVGuiLocalize->AddFile( "Resource/basemodui_%language%.txt");

	m_LevelLoading = false;
	
	for ( int k = 0; k < WPRI_COUNT; ++ k )
	{
		m_ActiveWindow[k] = WT_NONE;
	}

	// delay 3 frames before doing activation on initialization
	// needed to allow engine to exec startup commands (background map signal is 1 frame behind) 
	m_DelayActivation = 3;

	m_UIScheme = vgui::scheme()->LoadSchemeFromFileEx( 0, "resource/MenuScheme.res", "MenuScheme" );
	SetScheme( m_UIScheme );

	// Only one user on the PC, so set it now
	SetLastActiveUserId( IsPC() ? 0 : -1 );

	m_hOptionsDialog = NULL;

	m_bWarmRestartMode = false;
	m_ExitingFrameCount = 0;

	m_flBlurScale = 0;
	m_flLastBlurTime = 0;

	m_iBackgroundImageID = -1;
	m_iProductImageID = -1;

	m_nProductImageWide = 0;
	m_nProductImageTall = 0;
	m_flMovieFadeInTime = 0.0f;
	m_pBackgroundMaterial = NULL;
	m_pBackgroundTexture = NULL;

	// Listen for game events
	if ( gameeventmanager )
		gameeventmanager->AddListener( this, "server_spawn", false );
}

//=============================================================================
CBaseModPanel::~CBaseModPanel()
{
	ReleaseStartupGraphic();

	Assert(m_CFactoryBasePanel == this);
	m_CFactoryBasePanel = 0;

	surface()->DestroyTextureID( m_iBackgroundImageID );
	surface()->DestroyTextureID( m_iProductImageID );

	// Shutdown UI game data
	CUIGameData::Shutdown();
}

//=============================================================================
CBaseModPanel& CBaseModPanel::GetSingleton()
{
	Assert(m_CFactoryBasePanel != 0);
	return *m_CFactoryBasePanel;
}

//=============================================================================
CBaseModPanel* CBaseModPanel::GetSingletonPtr()
{
	return m_CFactoryBasePanel;
}

//=============================================================================
CBaseModFrame* CBaseModPanel::OpenWindow(const WINDOW_TYPE & wt, CBaseModFrame * caller, bool hidePrevious, KeyValues *pParameters)
{
	CBaseModFrame *newNav = m_Frames[ wt ].Get();
	bool setActiveWindow = true;

	// Window priority is used to track which windows are visible at all times
	// it is used to resolve the situations when a game requests an error box to popup
	// while a loading progress bar is active.
	// Windows with a higher priority force all other windows to get hidden.
	// After the high-priority window goes away it falls back to restore the low priority windows.
	WINDOW_PRIORITY nWindowPriority = WPRI_NORMAL;

	switch ( wt )
	{
	case WT_GENERICCONFIRMATION:
		nWindowPriority = WPRI_MESSAGE;
		break;
	case WT_LOADINGPROGRESSBKGND:
		nWindowPriority = WPRI_BKGNDSCREEN;
		break;
	case WT_LOADINGPROGRESS:
		nWindowPriority = WPRI_LOADINGPLAQUE;
		break;
	}

	if ( !newNav )
	{
		switch ( wt )
		{
		case WT_GENERICCONFIRMATION:
			m_Frames[wt] = new GenericConfirmation( this, "GenericConfirmation" );
			break;

		case WT_INGAMEMAINMENU:
			m_Frames[wt] = new InGameMainMenu( this, "InGameMainMenu" );
			break;

		case WT_LOADINGPROGRESSBKGND:
			m_Frames[wt] = new LoadingProgress( this, "LoadingProgress", LoadingProgress::LWT_BKGNDSCREEN );
			break;

		case WT_LOADINGPROGRESS:
			m_Frames[wt] = new LoadingProgress( this, "LoadingProgress", LoadingProgress::LWT_LOADINGPLAQUE );
			break;

		case WT_MAINMENU:
			m_Frames[wt] = new MainMenu( this, "MainMenu" );
			break;

		default:
			Assert( false );	// unknown window type
			break;
		}

		//
		// Finish setting up the window
		//

		newNav = m_Frames[wt].Get();
		if ( !newNav )
			return NULL;

		newNav->SetWindowPriority( nWindowPriority );
		newNav->SetWindowType(wt);
		newNav->SetVisible( false );
	}

	newNav->SetDataSettings( pParameters );

	if (setActiveWindow)
	{
		m_ActiveWindow[ nWindowPriority ] = wt;
		newNav->AddActionSignalTarget(this);
		newNav->SetCanBeActiveWindowType(true);
	}
	else if ( nWindowPriority == WPRI_MESSAGE )
	{
		m_ActiveWindow[ nWindowPriority ] = wt;
	}

	//
	// Now the window has been created, set it up
	//

	if ( UI_IsDebug() && (wt != WT_LOADINGPROGRESS) )
	{
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] OnOpen( `%s`, caller = `%s`, hidePrev = %d, setActive = %d, wt=%d, wpri=%d )\n",
			newNav->GetName(), caller ? caller->GetName() : "<NULL>", int(hidePrevious),
			int( setActiveWindow ), wt, nWindowPriority );
		KeyValuesDumpAsDevMsg( pParameters, 1 );
	}

	newNav->SetNavBack(caller);

	if (hidePrevious && caller != 0)
	{
		caller->SetVisible( false );
	}
	else if (caller != 0)
	{
		caller->FindAndSetActiveControl();
		//caller->SetAlpha(128);
	}

	// Check if a higher priority window is open
	if ( GetActiveWindowPriority() > newNav->GetWindowPriority() )
	{
		if ( UI_IsDebug() )
		{
			CBaseModFrame *pOther = m_Frames[ GetActiveWindowType() ].Get();
			Warning( "[GAMEUI] OpenWindow: Another window %p`%s` is having priority %d, deferring `%s`!\n",
				pOther, pOther ? pOther->GetName() : "<<null>>",
				GetActiveWindowPriority(), newNav->GetName() );
		}

		// There's a higher priority window that was open at the moment,
		// hide our window for now, it will get restored later.
		// newNav->SetVisible( false );
	}
	else
	{
		newNav->InvalidateLayout(false, false);
		newNav->OnOpen();
	}

	if ( UI_IsDebug() && (wt != WT_LOADINGPROGRESS) )
	{
		DbgShowCurrentUIState();
	}

	return newNav;
}

//=============================================================================
CBaseModFrame * CBaseModPanel::GetWindow( const WINDOW_TYPE& wt )
{
	return m_Frames[wt].Get();
}

//=============================================================================
WINDOW_TYPE CBaseModPanel::GetActiveWindowType()
{
	for ( int k = WPRI_COUNT; k -- > 0; )
	{
		if ( m_ActiveWindow[ k ] != WT_NONE )
		{
			CBaseModFrame *pFrame = m_Frames[ m_ActiveWindow[k] ].Get();
			if ( !pFrame || !pFrame->IsVisible() )
				continue;
			
			return m_ActiveWindow[ k ];
		}
	}
	return WT_NONE;
}

//=============================================================================
WINDOW_PRIORITY CBaseModPanel::GetActiveWindowPriority()
{
	for ( int k = WPRI_COUNT; k -- > 0; )
	{
		if ( m_ActiveWindow[ k ] != WT_NONE )
		{
			CBaseModFrame *pFrame = m_Frames[ m_ActiveWindow[k] ].Get();
			if ( !pFrame || !pFrame->IsVisible() )
				continue;

			return WINDOW_PRIORITY(k);
		}
	}
	return WPRI_NONE;
}

//=============================================================================
void CBaseModPanel::SetActiveWindow( CBaseModFrame * frame )
{
	if( !frame )
		return;
	
	m_ActiveWindow[ frame->GetWindowPriority() ] = frame->GetWindowType();

	if ( GetActiveWindowPriority() > frame->GetWindowPriority() )
	{
		if ( UI_IsDebug() )
		{
			CBaseModFrame *pOther = m_Frames[ GetActiveWindowType() ].Get();
			Warning( "[GAMEUI] SetActiveWindow: Another window %p`%s` is having priority %d, deferring `%s`!\n",
				pOther, pOther ? pOther->GetName() : "<<null>>",
				GetActiveWindowPriority(), frame->GetName() );
		}

		// frame->SetVisible( false );
	}
	else
	{
		frame->OnOpen();
	}
}

//=============================================================================
void CBaseModPanel::OnFrameClosed( WINDOW_PRIORITY pri, WINDOW_TYPE wt )
{
	if ( UI_IsDebug() )
	{
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::OnFrameClosed( %d, %d )\n", pri, wt );
		DbgShowCurrentUIState();
	}

	// Mark the frame that just closed as NULL so that nobody could find it
	m_Frames[wt] = NULL;

	if ( m_bClosingAllWindows )
	{
		if ( UI_IsDebug() )
		{
			ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] Closing all windows\n" );
		}
		return;
	}

	if( pri <= WPRI_NORMAL )
		return;

	for ( int k = 0; k < WPRI_COUNT; ++ k )
	{
		if ( m_ActiveWindow[k] == wt )
			m_ActiveWindow[k] = WT_NONE;
	}

	//
	// We only care to resurrect windows of lower priority when
	// higher priority windows close
	//

	for ( int k = WPRI_COUNT; k -- > 0; )
	{
		if ( m_ActiveWindow[ k ] == WT_NONE )
			continue;

		CBaseModFrame *pFrame = m_Frames[ m_ActiveWindow[k] ].Get();
		if ( !pFrame )
			continue;

		// pFrame->AddActionSignalTarget(this);

		pFrame->InvalidateLayout(false, false);
		pFrame->OnOpen();
		pFrame->SetVisible( true );
		pFrame->Activate();

		if ( UI_IsDebug() )
		{
			ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::OnFrameClosed( %d, %d ) -> Activated `%s`, pri=%d\n",
				pri, wt, pFrame->GetName(), pFrame->GetWindowPriority() );
			DbgShowCurrentUIState();
		}

		return;
	}
}

void CBaseModPanel::DbgShowCurrentUIState()
{
	if ( UI_IsDebug() < 2 )
		return;

	ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] Priorities WT: " );
	for ( int i = 0; i < WPRI_COUNT; ++ i )
	{
		ConColorMsg( Color( 77, 116, 85, 255 ),  " %d ", m_ActiveWindow[i] );
	}
	ConColorMsg( Color( 77, 116, 85, 255 ),  "\n" );
	for ( int i = 0; i < WT_WINDOW_COUNT; ++ i )
	{
		CBaseModFrame *pFrame = m_Frames[i].Get();
		if ( pFrame )
		{
			ConColorMsg( Color( 77, 116, 85, 255 ),  "        %2d. `%s` pri%d vis%d\n",
				i, pFrame->GetName(), pFrame->GetWindowPriority(), pFrame->IsVisible() );
		}
		else
		{
			ConColorMsg( Color( 77, 116, 85, 255 ),  "        %2d. NULL\n", i );
		}
	}
}

bool CBaseModPanel::IsLevelLoading()
{
	return m_LevelLoading;
}

//=============================================================================
void CBaseModPanel::CloseAllWindows( int ePolicyFlags )
{
	CAutoPushPop< bool > auto_m_bClosingAllWindows( m_bClosingAllWindows, true );

	if ( UI_IsDebug() )
	{
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::CloseAllWindows( 0x%x )\n", ePolicyFlags );
	}

	// make sure we also close any active flyout menus that might be hanging out.
	FlyoutMenu::CloseActiveMenu();

	for (int i = 0; i < WT_WINDOW_COUNT; ++i)
	{
		CBaseModFrame *pFrame = m_Frames[i].Get();
		if ( !pFrame )
			continue;

		int nPriority = pFrame->GetWindowPriority();

		switch ( nPriority )
		{
		case WPRI_LOADINGPLAQUE:
			if ( !(ePolicyFlags & CLOSE_POLICY_EVEN_LOADING) )
			{
				if ( UI_IsDebug() )
				{
					ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::CloseAllWindows() - Keeping loading type %d of priority %d.\n", i, nPriority );
				}

				continue;
				m_ActiveWindow[ WPRI_LOADINGPLAQUE ] = WT_NONE;
			}
			break;

		case WPRI_MESSAGE:
			if ( !(ePolicyFlags & CLOSE_POLICY_EVEN_MSGS) )
			{
				if ( UI_IsDebug() )
				{
					ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::CloseAllWindows() - Keeping msgbox type %d of priority %d.\n", i, nPriority );
				}

				continue;
				m_ActiveWindow[ WPRI_MESSAGE ] = WT_NONE;
			}
			break;

		case WPRI_BKGNDSCREEN:
			if ( ePolicyFlags & CLOSE_POLICY_KEEP_BKGND )
			{
				if ( UI_IsDebug() )
				{
					ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::CloseAllWindows() - Keeping bkgnd type %d of priority %d.\n", i, nPriority );
				}

				continue;
				m_ActiveWindow[ WPRI_BKGNDSCREEN ] = WT_NONE;
			}
			break;
		}

		// Close the window
		pFrame->Close();
		m_Frames[i] = NULL;
	}

	if ( UI_IsDebug() )
	{
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] After close all windows:\n" );
		DbgShowCurrentUIState();
	}

	m_ActiveWindow[ WPRI_NORMAL ] = WT_NONE;
}

//=============================================================================
void CBaseModPanel::OnGameUIActivated()
{
	if ( UI_IsDebug() )
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::OnGameUIActivated( delay = %d )\n", m_DelayActivation );

	if ( m_DelayActivation )
		return;

	COM_TimestampedLog( "CBaseModPanel::OnGameUIActivated()" );

	SetVisible( true );

	// Achievement dialog refreshes it's data if the player looks at the pause menu
	if ( m_hAchievementsDialog.Get() )
		//m_hAchievementsDialog->OnCommand( "OnGameUIActivated" );
		PostMessage( m_hAchievementsDialog.Get(), new KeyValues( "GameUIActivated" ) );

	if ( WT_LOADINGPROGRESS == GetActiveWindowType() )
	{
		// Ignore UI activations when loading poster is up
		return;
	}
	else if ( ( !m_LevelLoading && !engine->IsConnected() ) || GameUI().IsInBackgroundLevel() )
	{
		bool bForceReturnToFrontScreen = false;
		WINDOW_TYPE wt = GetActiveWindowType();
		switch ( wt )
		{
		default:
			break;
		case WT_NONE:
		case WT_INGAMEMAINMENU:
		case WT_GENERICCONFIRMATION:
			// bForceReturnToFrontScreen = !g_pMatchFramework->GetMatchmaking()->ShouldPreventOpenFrontScreen();
			bForceReturnToFrontScreen = true; // this used to be some magic about mid-disconnecting-states on PC...
			break;
		}
		if ( !IsPC() || bForceReturnToFrontScreen )
		{
			OpenFrontScreen();
		}
	}
	else if ( engine->IsConnected() && !m_LevelLoading )
	{
		CBaseModFrame *pInGameMainMenu = m_Frames[ WT_INGAMEMAINMENU ].Get();

		if ( !pInGameMainMenu || !pInGameMainMenu->IsAutoDeleteSet() )
		{
			// Prevent in game menu from opening if it already exists!
			// It might be hiding behind a modal window that needs to keep focus
			OpenWindow( WT_INGAMEMAINMENU, 0 );

			LoadingProgress *pLoadingProgress = static_cast<LoadingProgress*>( GetWindow( WT_LOADINGPROGRESS ) );
			if ( pLoadingProgress )
				pLoadingProgress->Close();
		}
	}
}

//=============================================================================
void CBaseModPanel::OnGameUIHidden()
{
	if ( UI_IsDebug() )
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::OnGameUIHidden()\n" );

	SetVisible(false);
	
	// Notify the options dialog that game UI is closing
	if ( m_hOptionsDialog.Get() )
		PostMessage( m_hOptionsDialog.Get(), new KeyValues( "GameUIHidden" ) );

	// Notify the achievements dialog that game UI is closing
	if ( m_hAchievementsDialog.Get() )
		PostMessage( m_hAchievementsDialog.Get(), new KeyValues( "GameUIHidden" ) );

	// Notify the in game menu that game UI is closing
	CBaseModFrame *pInGameMainMenu = GetWindow( WT_INGAMEMAINMENU );
	if ( pInGameMainMenu )
		PostMessage( pInGameMainMenu, new KeyValues( "GameUIHidden" ) );

}

void CBaseModPanel::OpenFrontScreen()
{
	WINDOW_TYPE frontWindow = WT_NONE;

	frontWindow = WT_MAINMENU;

	if( frontWindow != WT_NONE )
	{
		if( GetActiveWindowType() != frontWindow )
		{
			CloseAllWindows();
			OpenWindow( frontWindow, NULL );
		}
	}
}

//=============================================================================
void CBaseModPanel::RunFrame()
{
	if ( s_NavLock > 0 )
		--s_NavLock;

	GetAnimationController()->UpdateAnimations( Plat_FloatTime() );

	CBaseModFrame::RunFrameOnListeners();

	CUIGameData::Get()->RunFrame();

	if ( m_DelayActivation )
	{
		m_DelayActivation--;
		if ( !m_LevelLoading && !m_DelayActivation )
		{
			if ( UI_IsDebug() )
				ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] Executing delayed UI activation\n");

			OnGameUIActivated();
		}
	}

	bool bDoBlur = true;
	WINDOW_TYPE wt = GetActiveWindowType();
	switch ( wt )
	{
	case WT_NONE:
	case WT_MAINMENU:
	case WT_LOADINGPROGRESSBKGND:
	case WT_LOADINGPROGRESS:
		bDoBlur = false;
		break;
	}
	if ( enginevguifuncs && !enginevguifuncs->IsGameUIVisible() )
	{
		bDoBlur = false;
	}

	if ( !bDoBlur )
	{
		bDoBlur = false /*GameClientExports()->ClientWantsBlurEffect()*/;
	}

	float nowTime = Plat_FloatTime();
	float deltaTime = nowTime - m_flLastBlurTime;
	if ( deltaTime > 0 )
	{
		m_flLastBlurTime = nowTime;
		m_flBlurScale += deltaTime * bDoBlur ? 0.05f : -0.05f;
		m_flBlurScale = clamp( m_flBlurScale, 0, 0.85f );
		//engine->SetBlurFade( m_flBlurScale );
	}
}


//=============================================================================
void CBaseModPanel::OnLevelLoadingStarted( bool bShowProgressDialog )
{
	Assert( !m_LevelLoading );

	CloseAllWindows();

	if ( UI_IsDebug() )
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] OnLevelLoadingStarted - opening loading progress (%s)...\n" );

	LoadingProgress *pLoadingProgress = static_cast<LoadingProgress*>( OpenWindow( WT_LOADINGPROGRESS, 0 ) );
	LoadingProgress::LoadingType type;

	if ( GameUI().IsInLevel() && !GameUI().IsInBackgroundLevel() )
		type = LoadingProgress::LT_TRANSITION;	// Transitions between levels 
	else
		type = LoadingProgress::LT_MAINMENU;	// Loading the menu the first time

	if ( UI_IsDebug() )
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] Loading Map with %i...\n", type);

	pLoadingProgress->SetLoadingType( type );

	pLoadingProgress->SetProgress( 0.0f );

	m_LevelLoading = true;
}

void CBaseModPanel::OnEngineLevelLoadingSession( void )
{
	// We must keep the default loading poster because it will be replaced by
	// the real campaign loading poster shortly
	/*float flProgress = 0.0f;
	if ( LoadingProgress *pLoadingProgress = static_cast<LoadingProgress*>( GetWindow( WT_LOADINGPROGRESS ) ) )
	{
		flProgress = pLoadingProgress->GetProgress();
		pLoadingProgress->Close();
		m_Frames[ WT_LOADINGPROGRESS ] = NULL;
	}
	CloseAllWindows( CLOSE_POLICY_DEFAULT );*/

	/*if ( LoadingProgress *pLoadingProgress = static_cast<LoadingProgress*>( OpenWindow( WT_LOADINGPROGRESSBKGND, NULL ) ) )
	{
		KeyValues *pMapInfo = new KeyValues( "MapInfo" );
		pMapInfo->SetString( "mapname", engine->GetLevelName() );
		pLoadingProgress->SetMapData( pMapInfo );

		//pLoadingProgress->SetProgress( flProgress );
		pLoadingProgress->SetupMapInfo();

		pMapInfo->deleteThis();
	}*/

	if ( UI_IsDebug() )
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::OnEngineLevelLoadingSession...\n");
}

//=============================================================================
void CBaseModPanel::OnLevelLoadingFinished( KeyValues *kvEvent )
{
	int bError = kvEvent->GetInt( "error" );
	const char *failureReason = kvEvent->GetString( "reason" );
	
	Assert( m_LevelLoading );

	if ( UI_IsDebug() )
		ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::OnLevelLoadingFinished( %s, %s )\n", bError ? "Had Error" : "No Error", failureReason );

	LoadingProgress *pLoadingProgress = static_cast<LoadingProgress*>( GetWindow( WT_LOADINGPROGRESS ) );
	if ( pLoadingProgress )
	{
		pLoadingProgress->SetProgress( 1.0f );

		// always close loading progress, this frees costly resources
		pLoadingProgress->Close();
	}

	m_LevelLoading = false;

	CBaseModFrame *pFrame = CBaseModPanel::GetSingleton().GetWindow( WT_GENERICCONFIRMATION );
	if ( !pFrame )
		GameUI().HideGameUI();	// no confirmation up, hide the UI
}

void CBaseModPanel::FireGameEvent( IGameEvent* event )
{
	char const *szEvent = event->GetName();
	if ( !Q_stricmp( "server_spawn", szEvent ) )
		OnEngineLevelLoadingSession();
}

//=============================================================================
bool CBaseModPanel::UpdateProgressBar( float progress, const char *statusText )
{
	if ( !m_LevelLoading )
	{
		// Assert( m_LevelLoading );
		// Warning( "WARN: CBaseModPanel::UpdateProgressBar called outside of level loading, discarded!\n" );
		return false;
	}

	LoadingProgress *loadingProgress = static_cast<LoadingProgress*>( OpenWindow( WT_LOADINGPROGRESS, 0 ) );

	// Even if the progress hasn't advanced, we want to go ahead and refresh if it has been more than 1/10 seconds since last refresh to keep the spinny thing going.
	static float s_LastEngineTime = -1.0f;
	// clock the anim at 10hz
	float time = Plat_FloatTime();
	float deltaTime = time - s_LastEngineTime;

	if ( loadingProgress && ( ( loadingProgress->IsDrawingProgressBar() && ( loadingProgress->GetProgress() < progress ) ) || ( deltaTime > 0.06f ) ) )
	{
		// update progress
		loadingProgress->SetProgress( progress );
		loadingProgress->SetStatusText( statusText );
		s_LastEngineTime = time;

		if ( UI_IsDebug() )
		{
			ConColorMsg( Color( 77, 116, 85, 255 ),  "[GAMEUI] CBaseModPanel::UpdateProgressBar(%.2f %s)\n", loadingProgress->GetProgress(), statusText );
		}
		return true;
	}

	// no update required
	return false;
}

//=============================================================================
bool CBaseModPanel::SetProgressBarStatusText( const char *statusText )
{
	LoadingProgress *pLoadingProgress = static_cast<LoadingProgress*>( GetWindow( WT_LOADINGPROGRESS ) );
	if ( !pLoadingProgress )
		return false;

	pLoadingProgress->SetStatusText( statusText );
	Q_strncpy( GameUI().GetPreviousStatusText(), statusText, sizeof( GameUI().GetPreviousStatusText() ) );
	return true;
}

//=============================================================================
void CBaseModPanel::SetLastActiveUserId( int userId )
{
	if ( m_lastActiveUserId != userId )
	{
		DevWarning( "SetLastActiveUserId: %d -> %d\n", m_lastActiveUserId, userId );
	}

	m_lastActiveUserId = userId;
}

//=============================================================================
int CBaseModPanel::GetLastActiveUserId( )
{
	return m_lastActiveUserId;
}

//=============================================================================
void CBaseModPanel::OpenPlayerListDialog( Panel *parent )
{
	CPlayerListDialog *plist = new CPlayerListDialog( this );
	plist->Activate();
	
	//Put this thing in the middle of the screen
	int x, y;
	x = GetWide()/2;
	y = GetTall()/2;
	x -= plist->GetWide() / 2;
	y -= plist->GetTall() / 2;
	plist->SetPos(x, y);
}

//=============================================================================
void CBaseModPanel::OpenOptionsDialog( Panel *parent )
{
	COptionsDialog *options = new COptionsDialog( this );
	options->Activate();

	//Put this thing in the middle of the screen
	int x, y;
	x = GetWide() / 2;
	y = GetTall() / 2;
	x -= options->GetWide() / 2;
	y -= options->GetTall() / 2;
	options->SetPos(x, y);
}

//=============================================================================
void CBaseModPanel::OpenCreateMultiplayerGameDialog( Panel *parent )
{
	CCreateMultiplayerGameDialog *create = new CCreateMultiplayerGameDialog( this );
	create->Activate();

	//Put this thing in the middle of the screen
	int x, y;
	x = GetWide() / 2;
	y = GetTall() / 2;
	x -= create->GetWide() / 2;
	y -= create->GetTall() / 2;
	create->SetPos( x, y );
}

//=============================================================================
void CBaseModPanel::OpenAchievementsDialog( Panel *parent )
{
	CAchievementsDialog *ach = new CAchievementsDialog( this );
	ach->Activate();

	//Put this thing in the middle of the screen
	int x, y;
	x = GetWide() / 2;
	y = GetTall() / 2;
	x -= ach->GetWide() / 2;
	y -= ach->GetTall() / 2;
	ach->SetPos( x, y );
}

//=============================================================================
void CBaseModPanel::OpenServerBrowser()
{
	g_VModuleLoader.ActivateModule( "Servers" );
}

//=============================================================================
void CBaseModPanel::OnNavigateTo( const char* panelName )
{
	CBaseModFrame* currentFrame = 
		static_cast<CBaseModFrame*>(FindChildByName(panelName, false));

	if (currentFrame && currentFrame->GetCanBeActiveWindowType())
	{
		m_ActiveWindow[ currentFrame->GetWindowPriority() ] = currentFrame->GetWindowType();
	}
}

//=============================================================================
void CBaseModPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBgColor( pScheme->GetColor( "Blank", Color(0, 0, 0, 0) ) );

	int screenWide, screenTall;
	surface()->GetScreenSize( screenWide, screenTall );

	char szBGName[MAX_PATH];
	engine->GetMainMenuBackgroundName( szBGName, sizeof( szBGName ) );
	char szImage[MAX_PATH];
	Q_snprintf( szImage, sizeof( szImage ), "console/%s", szBGName );

	float aspectRatio = (float)screenWide/(float)screenTall;
	bool bIsWidescreen = aspectRatio >= 1.5999f;
	if ( bIsWidescreen )
		Q_strcat( szImage, "_widescreen", sizeof( szImage ) );

	m_iBackgroundImageID = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iBackgroundImageID, szImage, true, false );

	m_iProductImageID = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile( m_iProductImageID, "console/startup_loading", true, false );

	int logoW = 384;
	int logoH = 192;

	if ( !bIsWidescreen )
	{
		// smaller in standard res
		logoW = 320;
		logoH = 160;
	}

	m_nProductImageWide = vgui::scheme()->GetProportionalScaledValue( logoW );
	m_nProductImageTall = vgui::scheme()->GetProportionalScaledValue( logoH );

	if ( aspectRatio >= 1.6f )
	{
		// use the widescreen version
		Q_snprintf( m_szFadeFilename, sizeof( m_szFadeFilename ), "materials/console/%s_widescreen.vtf", "background01" );
	}
	else
	{
		Q_snprintf( m_szFadeFilename, sizeof( m_szFadeFilename ), "materials/console/%s_widescreen.vtf", "background01" );
	}
}

//=============================================================================
void CBaseModPanel::PaintBackground()
{
	if ( !m_LevelLoading &&
		!GameUI().IsInLevel() &&
		!GameUI().IsInBackgroundLevel() )
	{
		int wide, tall;
		GetSize( wide, tall );

		if ( true /*engine->IsTransitioningToLoad()*/ )
		{
			surface()->DrawSetColor( 255, 255, 255, 255 );
			surface()->DrawSetTexture( m_iBackgroundImageID );
			surface()->DrawTexturedRect( 0, 0, wide, tall );

			if ( BackgroundMovie() )
			{
				BackgroundMovie()->Update();
				if ( BackgroundMovie()->SetTextureMaterial() != -1 )
				{
					surface()->DrawSetColor( 255, 255, 255, 255 );
					int x, y, w, h;
					GetBounds( x, y, w, h );

					// center, 16:9 aspect ratio
					int width_at_ratio = h * (16.0f / 9.0f);
					x = ( w * 0.5f ) - ( width_at_ratio * 0.5f );

					surface()->DrawTexturedRect( x, y, x + width_at_ratio, y + h );

					if ( !m_flMovieFadeInTime )
					{
						// do the fade a little bit after the movie starts (needs to be stable)
						// the product overlay will fade out
						m_flMovieFadeInTime	= Plat_FloatTime() + TRANSITION_TO_MOVIE_DELAY_TIME;
					}

					float flFadeDelta = RemapValClamped( Plat_FloatTime(), m_flMovieFadeInTime, m_flMovieFadeInTime + TRANSITION_TO_MOVIE_FADE_TIME, 1.0f, 0.0f );
					if ( flFadeDelta > 0.0f )
					{
						if ( !m_pBackgroundMaterial )
							PrepareStartupGraphic();

						DrawStartupGraphic( flFadeDelta );
					}
				}
			}
		}
		else
		{
			if ( BackgroundMovie() )
			{
				BackgroundMovie()->Update();

				if ( BackgroundMovie()->GetVideoMaterial() )
				{
					// Draw the polys to draw this out
					CMatRenderContextPtr pRenderContext( materials );
	
					pRenderContext->MatrixMode( MATERIAL_VIEW );
					pRenderContext->PushMatrix();
					pRenderContext->LoadIdentity();

					pRenderContext->MatrixMode( MATERIAL_PROJECTION );
					pRenderContext->PushMatrix();
					pRenderContext->LoadIdentity();

					pRenderContext->Bind( BackgroundMovie()->GetVideoMaterial()->GetMaterial(), NULL );

					CMeshBuilder meshBuilder;
					IMesh* pMesh = pRenderContext->GetDynamicMesh( true );
					meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

					int xpos = 0;
					int ypos = 0;
					vgui::ipanel()->GetAbsPos(GetVPanel(), xpos, ypos);

					float flLeftX = xpos;
					float flRightX = xpos + ( BackgroundMovie()->m_nPlaybackWidth-1 );

					float flTopY = ypos;
					float flBottomY = ypos + ( BackgroundMovie()->m_nPlaybackHeight-1 );

					// Map our UVs to cut out just the portion of the video we're interested in
					float flLeftU = 0.0f;
					float flTopV = 0.0f;

					// We need to subtract off a pixel to make sure we don't bleed
					float flRightU = BackgroundMovie()->m_flU - ( 1.0f / (float) BackgroundMovie()->m_nPlaybackWidth );
					float flBottomV = BackgroundMovie()->m_flV - ( 1.0f / (float) BackgroundMovie()->m_nPlaybackHeight );

					// Get the current viewport size
					int vx, vy, vw, vh;
					pRenderContext->GetViewport( vx, vy, vw, vh );

					// map from screen pixel coords to -1..1
					flRightX = FLerp( -1, 1, 0, vw, flRightX );
					flLeftX = FLerp( -1, 1, 0, vw, flLeftX );
					flTopY = FLerp( 1, -1, 0, vh ,flTopY );
					flBottomY = FLerp( 1, -1, 0, vh, flBottomY );

					float alpha = ((float)GetFgColor()[3]/255.0f);

					for ( int corner=0; corner<4; corner++ )
					{
						bool bLeft = (corner==0) || (corner==3);
						meshBuilder.Position3f( (bLeft) ? flLeftX : flRightX, (corner & 2) ? flBottomY : flTopY, 0.0f );
						meshBuilder.Normal3f( 0.0f, 0.0f, 1.0f );
						meshBuilder.TexCoord2f( 0, (bLeft) ? flLeftU : flRightU, (corner & 2) ? flBottomV : flTopV );
						meshBuilder.TangentS3f( 0.0f, 1.0f, 0.0f );
						meshBuilder.TangentT3f( 1.0f, 0.0f, 0.0f );
						meshBuilder.Color4f( 1.0f, 1.0f, 1.0f, alpha );
						meshBuilder.AdvanceVertex();
					}
	
					meshBuilder.End();
					pMesh->Draw();

					pRenderContext->MatrixMode( MATERIAL_VIEW );
					pRenderContext->PopMatrix();

					pRenderContext->MatrixMode( MATERIAL_PROJECTION );
					pRenderContext->PopMatrix();
				}
			}
		}
	}
}

IVTFTexture *LoadVTF( CUtlBuffer &temp, const char *szFileName )
{
	if ( !g_pFullFileSystem->ReadFile( szFileName, NULL, temp ) )
		return NULL;

	IVTFTexture *texture = CreateVTFTexture();
	if ( !texture->Unserialize( temp ) )
	{
		Error( "Invalid or corrupt background texture %s\n", szFileName );
		return NULL;
	}
	texture->ConvertImageFormat( IMAGE_FORMAT_RGBA8888, false );
	return texture;
}

void CBaseModPanel::PrepareStartupGraphic()
{
	CUtlBuffer buf;
	// load in the background vtf
	buf.Clear();
	m_pBackgroundTexture = LoadVTF( buf, m_szFadeFilename );
	if ( !m_pBackgroundTexture )
	{
		Error( "Can't find background image '%s'\n", m_szFadeFilename );
		return;
	}

	// Allocate a white material
	m_pVMTKeyValues = new KeyValues( "UnlitGeneric" );
	m_pVMTKeyValues->SetString( "$basetexture", m_szFadeFilename + 10 );
	m_pVMTKeyValues->SetInt( "$ignorez", 1 );
	m_pVMTKeyValues->SetInt( "$nofog", 1 );
	m_pVMTKeyValues->SetInt( "$no_fullbright", 1 );
	m_pVMTKeyValues->SetInt( "$nocull", 1 );
	m_pVMTKeyValues->SetInt( "$vertexalpha", 1 );
	m_pVMTKeyValues->SetInt( "$vertexcolor", 1 );
	m_pBackgroundMaterial = g_pMaterialSystem->CreateMaterial( "__background", m_pVMTKeyValues );
}

void CBaseModPanel::ReleaseStartupGraphic()
{
	if ( m_pBackgroundMaterial )
	{
		m_pBackgroundMaterial->Release();
	}

	if ( m_pBackgroundTexture )
	{
		DestroyVTFTexture( m_pBackgroundTexture );
		m_pBackgroundTexture = NULL;
	}
}

// we have to draw the startup fade graphic using this function so it perfectly matches the one drawn by the engine during load
void DrawScreenSpaceRectangleAlpha( IMaterial *pMaterial, 
							  int nDestX, int nDestY, int nWidth, int nHeight,	// Rect to draw into in screen space
							  float flSrcTextureX0, float flSrcTextureY0,		// which texel you want to appear at destx/y
							  float flSrcTextureX1, float flSrcTextureY1,		// which texel you want to appear at destx+width-1, desty+height-1
							  int nSrcTextureWidth, int nSrcTextureHeight,		// needed for fixup
							  void *pClientRenderable,							// Used to pass to the bind proxies
							  int nXDice, int nYDice,							// Amount to tessellate the mesh
							  float fDepth, float flAlpha )									// what Z value to put in the verts (def 0.0)
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );

	if ( ( nWidth <= 0 ) || ( nHeight <= 0 ) )
		return;

	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	pRenderContext->Bind( pMaterial, pClientRenderable );

	int xSegments = MAX( nXDice, 1);
	int ySegments = MAX( nYDice, 1);

	CMeshBuilder meshBuilder;

	IMesh* pMesh = pRenderContext->GetDynamicMesh( true );
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, xSegments * ySegments );

	int nScreenWidth, nScreenHeight;
	pRenderContext->GetRenderTargetDimensions( nScreenWidth, nScreenHeight );
	float flLeftX = nDestX - 0.5f;
	float flRightX = nDestX + nWidth - 0.5f;

	float flTopY = nDestY - 0.5f;
	float flBottomY = nDestY + nHeight - 0.5f;

	float flSubrectWidth = flSrcTextureX1 - flSrcTextureX0;
	float flSubrectHeight = flSrcTextureY1 - flSrcTextureY0;

	float flTexelsPerPixelX = ( nWidth > 1 ) ? flSubrectWidth / ( nWidth - 1 ) : 0.0f;
	float flTexelsPerPixelY = ( nHeight > 1 ) ? flSubrectHeight / ( nHeight - 1 ) : 0.0f;

	float flLeftU = flSrcTextureX0 + 0.5f - ( 0.5f * flTexelsPerPixelX );
	float flRightU = flSrcTextureX1 + 0.5f + ( 0.5f * flTexelsPerPixelX );
	float flTopV = flSrcTextureY0 + 0.5f - ( 0.5f * flTexelsPerPixelY );
	float flBottomV = flSrcTextureY1 + 0.5f + ( 0.5f * flTexelsPerPixelY );

	float flOOTexWidth = 1.0f / nSrcTextureWidth;
	float flOOTexHeight = 1.0f / nSrcTextureHeight;
	flLeftU *= flOOTexWidth;
	flRightU *= flOOTexWidth;
	flTopV *= flOOTexHeight;
	flBottomV *= flOOTexHeight;

	// Get the current viewport size
	int vx, vy, vw, vh;
	pRenderContext->GetViewport( vx, vy, vw, vh );

	// map from screen pixel coords to -1..1
	flRightX = FLerp( -1, 1, 0, vw, flRightX );
	flLeftX = FLerp( -1, 1, 0, vw, flLeftX );
	flTopY = FLerp( 1, -1, 0, vh ,flTopY );
	flBottomY = FLerp( 1, -1, 0, vh, flBottomY );

	// Dice the quad up...
	if ( xSegments > 1 || ySegments > 1 )
	{
		// Screen height and width of a subrect
		float flWidth  = (flRightX - flLeftX) / (float) xSegments;
		float flHeight = (flTopY - flBottomY) / (float) ySegments;

		// UV height and width of a subrect
		float flUWidth  = (flRightU - flLeftU) / (float) xSegments;
		float flVHeight = (flBottomV - flTopV) / (float) ySegments;

		for ( int x=0; x < xSegments; x++ )
		{
			for ( int y=0; y < ySegments; y++ )
			{
				// Top left
				meshBuilder.Position3f( flLeftX   + (float) x * flWidth, flTopY - (float) y * flHeight, fDepth );
				meshBuilder.Normal3f( 0.0f, 0.0f, 1.0f );
				meshBuilder.TexCoord2f( 0, flLeftU   + (float) x * flUWidth, flTopV + (float) y * flVHeight);
				meshBuilder.TangentS3f( 0.0f, 1.0f, 0.0f );
				meshBuilder.TangentT3f( 1.0f, 0.0f, 0.0f );
				meshBuilder.Color4ub( 255, 255, 255, 255.0f * flAlpha );
				meshBuilder.AdvanceVertex();

				// Top right (x+1)
				meshBuilder.Position3f( flLeftX   + (float) (x+1) * flWidth, flTopY - (float) y * flHeight, fDepth );
				meshBuilder.Normal3f( 0.0f, 0.0f, 1.0f );
				meshBuilder.TexCoord2f( 0, flLeftU   + (float) (x+1) * flUWidth, flTopV + (float) y * flVHeight);
				meshBuilder.TangentS3f( 0.0f, 1.0f, 0.0f );
				meshBuilder.TangentT3f( 1.0f, 0.0f, 0.0f );
				meshBuilder.Color4ub( 255, 255, 255, 255.0f * flAlpha );
				meshBuilder.AdvanceVertex();

				// Bottom right (x+1), (y+1)
				meshBuilder.Position3f( flLeftX   + (float) (x+1) * flWidth, flTopY - (float) (y+1) * flHeight, fDepth );
				meshBuilder.Normal3f( 0.0f, 0.0f, 1.0f );
				meshBuilder.TexCoord2f( 0, flLeftU   + (float) (x+1) * flUWidth, flTopV + (float)(y+1) * flVHeight);
				meshBuilder.TangentS3f( 0.0f, 1.0f, 0.0f );
				meshBuilder.TangentT3f( 1.0f, 0.0f, 0.0f );
				meshBuilder.Color4ub( 255, 255, 255, 255.0f * flAlpha );
				meshBuilder.AdvanceVertex();

				// Bottom left (y+1)
				meshBuilder.Position3f( flLeftX   + (float) x * flWidth, flTopY - (float) (y+1) * flHeight, fDepth );
				meshBuilder.Normal3f( 0.0f, 0.0f, 1.0f );
				meshBuilder.TexCoord2f( 0, flLeftU   + (float) x * flUWidth, flTopV + (float)(y+1) * flVHeight);
				meshBuilder.TangentS3f( 0.0f, 1.0f, 0.0f );
				meshBuilder.TangentT3f( 1.0f, 0.0f, 0.0f );
				meshBuilder.Color4ub( 255, 255, 255, 255.0f * flAlpha );
				meshBuilder.AdvanceVertex();
			}
		}
	}
	else // just one quad
	{
		for ( int corner=0; corner<4; corner++ )
		{
			bool bLeft = (corner==0) || (corner==3);
			meshBuilder.Position3f( (bLeft) ? flLeftX : flRightX, (corner & 2) ? flBottomY : flTopY, fDepth );
			meshBuilder.Normal3f( 0.0f, 0.0f, 1.0f );
			meshBuilder.TexCoord2f( 0, (bLeft) ? flLeftU : flRightU, (corner & 2) ? flBottomV : flTopV );
			meshBuilder.TangentS3f( 0.0f, 1.0f, 0.0f );
			meshBuilder.TangentT3f( 1.0f, 0.0f, 0.0f );
			meshBuilder.Color4ub( 255, 255, 255, 255.0f * flAlpha );
			meshBuilder.AdvanceVertex();
		}
	}

	meshBuilder.End();
	pMesh->Draw();

	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->PopMatrix();

	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->PopMatrix();
}

void CBaseModPanel::DrawStartupGraphic( float flNormalizedAlpha )
{
	CMatRenderContextPtr pRenderContext( g_pMaterialSystem );
	int w = GetWide();
	int h = GetTall();
	int tw = m_pBackgroundTexture->Width();
	int th = m_pBackgroundTexture->Height();

	float depth = 0.5f;
	int width_at_ratio = h * (16.0f / 9.0f);
	int x = ( w * 0.5f ) - ( width_at_ratio * 0.5f );
	DrawScreenSpaceRectangleAlpha( m_pBackgroundMaterial, x, 0, width_at_ratio, h, 8, 8, tw-8, th-8, tw, th, NULL,1,1,depth,flNormalizedAlpha );
}

void CBaseModPanel::OnCommand(const char *command)
{
	if ( !Q_stricmp( command, "RestartWithNewLanguage" ) )
	{
		const char *pUpdatedAudioLanguage = COptionsSubAudio::GetUpdatedAudioLanguage();

		if ( pUpdatedAudioLanguage[ 0 ] != '\0' )
		{
			char szSteamURL[50];
			char szAppId[50];

			// hide everything while we quit
			SetVisible( false );
			vgui::surface()->RestrictPaintToSinglePanel( GetVPanel() );
			engine->ClientCmd_Unrestricted( "quit\n" );

			// Construct Steam URL. Pattern is steam://run/<appid>/<language>. (e.g. Ep1 In French ==> steam://run/380/french)
			Q_strcpy(szSteamURL, "steam://run/");
			Q_snprintf( szAppId, sizeof(szAppId), "%d", engine->GetAppID() );
			Q_strcat( szSteamURL, szAppId, sizeof( szSteamURL ) );
			Q_strcat( szSteamURL, "/", sizeof( szSteamURL ) );
			Q_strcat( szSteamURL, pUpdatedAudioLanguage, sizeof( szSteamURL ) );

			// Set Steam URL for re-launch in registry. Launcher will check this registry key and exec it in order to re-load the game in the proper language
			vgui::system()->SetRegistryString("HKEY_CURRENT_USER\\Software\\Valve\\Source\\Relaunch URL", szSteamURL );
		}
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

const char *CBaseModPanel::GetUISoundName(  UISound_t UISound )
{
	switch ( UISound )
	{
	case UISOUND_BACK:
		return "ui/buttonclickrelease.wav";
	case UISOUND_ACCEPT:
		return "ui/buttonclick.wav";
	case UISOUND_FOCUS:
		return "ui/buttonrollover.wav";
	case UISOUND_CLICK:
		return "ui/buttonclick.wav";
	default:
		return "ui/buttonclick.wav";
	}
	return NULL;
}

void CBaseModPanel::PlayUISound( UISound_t UISound )
{
	const char *pSound = GetUISoundName( UISound );
	if ( pSound )
	{
		vgui::surface()->PlaySound( pSound );
	}
}

void CBaseModPanel::OnSetFocus()
{
	BaseClass::OnSetFocus();

	//GameConsole().Hide();
}

void CBaseModPanel::OnMovedPopupToFront()
{
	GameConsole().Hide();
}

void CBaseModPanel::SafeNavigateTo( Panel *pExpectedFrom, Panel *pDesiredTo, bool bAllowStealFocus )
{
	Panel *pOriginalFocus = ipanel()->GetPanel( GetCurrentKeyFocus(), GetModuleName() );
	bool bSomeoneElseHasFocus = pOriginalFocus && (pOriginalFocus != pExpectedFrom);
	bool bActuallyChangingFocus = (pExpectedFrom != pDesiredTo);
	bool bNeedToReturnKeyFocus = !bAllowStealFocus && bSomeoneElseHasFocus && bActuallyChangingFocus;

	pDesiredTo->NavigateTo();

	if ( bNeedToReturnKeyFocus )
	{
		pDesiredTo->NavigateFrom();
		pOriginalFocus->NavigateTo();
	}
}

void CBaseModPanel::InvalidateFramesLayout( bool layoutNow, bool reloadScheme )
{	
	for ( int i = 0; i < WT_WINDOW_COUNT; ++ i )
	{
		CBaseModFrame *pFrames = m_Frames[i].Get();
		if ( pFrames )
			pFrames->InvalidateLayout( true, true );
	}
}
