//========= Copyright � 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "VLoadingProgress.h"
#include "EngineInterface.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ProgressBar.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/Image.h"
#include "vgui_controls/ImagePanel.h"
#include "KeyValues.h"
#include "fmtstr.h"
#include "FileSystem.h"
#include "GameUI_Interface.h"
#include <vgui/IInput.h>
#include "FileSystem.h"
#include "time.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern IGameEventManager2 *gameeventmanager;

extern void AddSubKeyNamed( KeyValues *pKeys, const char *pszName );

const char *GetMapDisplayName( const char *mapName );
const char *GetMapType( const char *mapName );
const char *GetMapAuthor( const char *mapName );
const char *GetMapDescription( const char *mapName );

//=============================================================================
LoadingProgress::LoadingProgress(Panel *parent, const char *panelName, LoadingWindowType eLoadingType ):
	BaseClass( parent, panelName, false, false, false ),
	m_LoadingWindowType( eLoadingType )
{

	if ( IsPC() && eLoadingType == LWT_LOADINGPLAQUE )
		MakePopup( false );

	SetDeleteSelfOnClose( true );
	SetProportional( true );

	m_flPeakProgress = 0.0f;
	m_pLoadingBar = NULL;

	m_pLoadingProgress = NULL;
	m_pCancelButton = NULL;

	m_pLoadingSpinner = NULL;
	m_LoadingType = LT_UNDEFINED;

	m_pBGImage = NULL;
	m_pFooter = NULL;

	// Listen for game events
	if ( gameeventmanager )
		gameeventmanager->AddListener( this, "player_disconnect", false );

	// purposely not pre-caching the bg images
	// as they do not appear in-game, and are 1MB each, we will demand load them and ALWAYS discard them
	m_pMapInfo = NULL;

	m_textureID_LoadingBar = -1;
	m_textureID_LoadingBarBG = -1;

	m_bDrawBackground = false;
	m_bDrawProgress = false;
	m_bDrawSpinner = false;

	m_flLastEngineTime = 0;

	// marked to indicate the controls exist
	m_bValid = false;

	MEM_ALLOC_CREDIT();
}

//=============================================================================
LoadingProgress::~LoadingProgress()
{
	// Stop listening for events
	if ( gameeventmanager )
		gameeventmanager->RemoveListener( this );
}

//=============================================================================
void LoadingProgress::OnThink()
{
	UpdateLoadingSpinner();
}

//=============================================================================
void LoadingProgress::OnCommand( const char *command )
{
	if ( !stricmp( command, "cancel" ) )
	{
		// disconnect from the server
		engine->ClientCmd_Unrestricted( "disconnect\n" );

		// close
		Close();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

//=============================================================================
void LoadingProgress::FireGameEvent( IGameEvent* event )
{
	if ( !Q_strcmp( event->GetName(), "player_disconnect" ) )
	{
		Close();
	}
}

//=============================================================================
void LoadingProgress::ApplySchemeSettings( IScheme *pScheme )
{
	// will cause the controls to be instanced
	BaseClass::ApplySchemeSettings( pScheme );

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

	LoadControlSettings( "Resource/UI/BaseModUI/LoadingProgress.res", NULL, NULL, pConditions );

	SetPaintBackgroundEnabled( true );
	
	// now have controls, can now do further initing
	m_bValid = true;

	// find or create pattern
	// purposely not freeing these, not worth the i/o hitch for something so small
	const char *pImageName = "vgui/loadingbar";
	m_textureID_LoadingBar = vgui::surface()->DrawGetTextureId( pImageName );
	if ( m_textureID_LoadingBar == -1 )
	{
		m_textureID_LoadingBar = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_LoadingBar, pImageName, true, false );	
	}

	// find or create pattern
	// purposely not freeing these, not worth the i/o hitch for something so small
	pImageName = "vgui/loadingbar_bg";
	m_textureID_LoadingBarBG = vgui::surface()->DrawGetTextureId( pImageName );
	if ( m_textureID_LoadingBarBG == -1 )
	{
		m_textureID_LoadingBarBG = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_LoadingBarBG, pImageName, true, false );	
	}

	SetupControlStates();
}

//=============================================================================
void LoadingProgress::Close()
{
	/*if ( m_pBGImage )
		m_pBGImage->EvictImage();*/

	BaseClass::Close();
}

//=============================================================================
void LoadingProgress::Activate()
{
	BaseClass::Activate();
}

//=============================================================================
// this is where the spinner gets updated.
void LoadingProgress::UpdateLoadingSpinner()
{
	if ( m_pLoadingSpinner && m_bDrawSpinner )
	{
		// clock the anim at 10hz
		float time = Plat_FloatTime();
		if ( ( m_flLastEngineTime + 0.1f ) < time )
		{
			m_flLastEngineTime = time;
			m_pLoadingSpinner->SetFrame( m_pLoadingSpinner->GetFrame() + 1 );
		}
	}
}

//=============================================================================
void LoadingProgress::SetProgress( float progress )
{
	if ( m_pLoadingBar && m_bDrawProgress )
	{
		if ( progress > m_flPeakProgress )
			m_flPeakProgress = progress;

		m_pLoadingBar->SetProgress( m_flPeakProgress );
	}

	UpdateLoadingSpinner();
}

//=============================================================================
float LoadingProgress::GetProgress()
{
	float retVal = -1.0f;

	if ( m_pLoadingBar )
		retVal = m_pLoadingBar->GetProgress();

	return retVal;
}

//=============================================================================
void LoadingProgress::PaintBackground()
{
	int screenWide, screenTall;
	surface()->GetScreenSize( screenWide, screenTall );

	if ( m_bDrawBackground && m_pBGImage )
	{
		int x, y, wide, tall;
		m_pBGImage->GetBounds( x, y, wide, tall );
		surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
		surface()->DrawSetTexture( m_pBGImage->GetImage()->GetID() );
		surface()->DrawTexturedRect( x, y, x+wide, y+tall );
	}

	if ( m_pFooter )
	{
		int screenWidth, screenHeight;
		CBaseModPanel::GetSingleton().GetSize( screenWidth, screenHeight );

		int x, y, wide, tall;
		m_pFooter->GetBounds( x, y, wide, tall );
		DrawBoxFade( 0, y, x+screenWidth/2, y+tall, Color( 0, 0, 0, 200 ), 1.0f, 255, 0, true, false );
	}

	// this is where the spinner draws
	bool bRenderSpinner = ( m_bDrawSpinner && m_pLoadingSpinner );
	if ( bRenderSpinner )
	{
		int x, y, wide, tall;

		wide = tall = scheme()->GetProportionalScaledValue( 45 );
		x = scheme()->GetProportionalScaledValue( 45 ) - wide/2;
		y = screenTall - scheme()->GetProportionalScaledValue( 32 ) - tall/2;

		m_pLoadingSpinner->GetImage()->SetFrame( m_pLoadingSpinner->GetFrame() );

		surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
		surface()->DrawSetTexture( m_pLoadingSpinner->GetImage()->GetID() );
		surface()->DrawTexturedRect( x, y, x+wide, y+tall );
	}

	if ( m_bDrawProgress && m_pLoadingBar )
	{
		int x, y, wide, tall;
		m_pLoadingBar->GetBounds( x, y, wide, tall );

		int iScreenWide, iScreenTall;
		surface()->GetScreenSize( iScreenWide, iScreenTall );

		float f = m_pLoadingBar->GetProgress();
		f = clamp( f, 0, 1.0f );

		// Textured bar
		surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );

		// Texture BG
		surface()->DrawSetTexture( m_textureID_LoadingBarBG );
		surface()->DrawTexturedRect( x, y, x + wide, y + tall );

		surface()->DrawSetTexture( m_textureID_LoadingBar );

		// YWB 10/13/2009:  If we don't crop the texture coordinate down then we will see a jitter of the texture as the texture coordinate and the rounded width 
		//  alias

		int nIntegerWide = f * wide;		
		//float flUsedFrac = (float)nIntegerWide / (float)wide;

		surface()->DrawTexturedRect( x, y, x + nIntegerWide, y + tall );
	}
}

//=============================================================================
void LoadingProgress::OnKeyCodeTyped( KeyCode code )
{
	if ( code == KEY_ESCAPE	)
		OnCommand( "cancel" );
	else
		BaseClass::OnKeyCodeTyped( code );
}


//=============================================================================
// Must be called first. Establishes the loading style
//=============================================================================
void LoadingProgress::SetLoadingType( LoadingProgress::LoadingType loadingType )
{
	m_LoadingType = loadingType;

	// the first time initing occurs during ApplySchemeSettings() or if the panel is deleted
	// if the panel is re-used, this is for the second time the panel gets used
	SetupControlStates();
}

//=============================================================================
LoadingProgress::LoadingType LoadingProgress::GetLoadingType()
{
	return m_LoadingType;
}

//=============================================================================
void LoadingProgress::SetupControlStates()
{
	m_flPeakProgress = 0.0f;

	// haven't been functionally initialized yet
	// can't set or query control states until they get established
	if ( !m_bValid )
		return;

	m_bDrawBackground = false;
	m_bDrawSpinner = false;
	m_bDrawProgress = false;

	switch( m_LoadingType )
	{
	case LT_MAINMENU:
		m_bDrawBackground = true;
		m_bDrawProgress = true;
		m_bDrawSpinner = false;
		break;
	case LT_TRANSITION:
		m_bDrawBackground = true;
		m_bDrawProgress = true;
		m_bDrawSpinner = false;
		break;
	default:
		break;
	}

	m_pLogoImage = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "TFC_Logo" ) );
	m_pLoadingProgress = dynamic_cast< vgui::Label* >( FindChildByName( "LoadingProgressText" ) );
	m_pCancelButton = dynamic_cast< CExMenuButton* >( FindChildByName( "Cancel" ) );
	m_pFooter = dynamic_cast< vgui::Panel* >( FindChildByName( "Footer" ) );

	m_pBGImage = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "Background" ) );
	if ( m_pBGImage )
	{
		// set the correct background image
		int screenWide, screenTall;
		surface()->GetScreenSize( screenWide, screenTall );

		char szBGName[MAX_PATH];
		engine->GetMainMenuBackgroundName( szBGName, sizeof( szBGName ) );
		char szImage[MAX_PATH];
		Q_snprintf( szImage, sizeof( szImage ), "../console/%s", szBGName );

		float aspectRatio = (float)screenWide/(float)screenTall;
		bool bIsWidescreen = aspectRatio >= 1.5999f;
		if ( bIsWidescreen )
			Q_strcat( szImage, "_widescreen", sizeof( szImage ) );

		m_pBGImage->SetImage( szImage );

		// we will custom draw
		m_pBGImage->SetVisible( false );
	}

	// we will custom draw
	m_pLoadingBar = dynamic_cast< vgui::ProgressBar* >( FindChildByName( "LoadingBar" ) );
	if ( m_pLoadingBar )
		m_pLoadingBar->SetVisible( false );

	// we will custom draw
	m_pLoadingSpinner = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "LoadingSpinner" ) );
	if ( m_pLoadingSpinner )
		m_pLoadingSpinner->SetVisible( false );

	vgui::Label *pLoadingLabel = dynamic_cast< vgui::Label *>( FindChildByName( "LoadingText" ) );
	if ( pLoadingLabel )
		pLoadingLabel->SetVisible( m_bDrawProgress );

	vgui::Label *pMapTypeLabel = dynamic_cast<vgui::Label*>( FindChildByName( "MapType" ) );
		if ( pMapTypeLabel )
			pMapTypeLabel->SetScheme( "ClientScheme" );

	vgui::Label *pMapAuthorLabel = dynamic_cast<vgui::Label*>( FindChildByName( "MapAuthor" ) );
	if ( pMapAuthorLabel ) 
		pMapAuthorLabel->SetScheme( "ClientScheme" );

	vgui::Label *pMapDescLabel = dynamic_cast<vgui::Label*>( FindChildByName( "MapDesc" ) );
	if ( pMapDescLabel ) 
		pMapDescLabel->SetScheme( "ClientScheme" );

	SetupMapInfo();

	// Hold on to start frame slightly
	m_flLastEngineTime = Plat_FloatTime() + 0.2f;
}

//=============================================================================
void LoadingProgress::SetMapData( KeyValues *pMapInfo )
{
	m_pMapInfo = pMapInfo;
}

//=============================================================================
void LoadingProgress::SetupMapInfo( void )
{	
	vgui::Label *pMapLabel = dynamic_cast<vgui::Label*>( FindChildByName( "MapLabel" ) );

	if ( !m_pMapInfo )
	{
		if ( pMapLabel )
		{
			pMapLabel->SetText( "#UI_MapName_Unknown" );
			pMapLabel->SetVisible( false );
		}
		return;
	}

	const char *pMapName = m_pMapInfo->GetString( "mapname" );
	if ( pMapName )
	{
		// If we're loading a background map, don't display anything
		// HACK: Client doesn't get gpGlobals->eLoadType, so just do string compare for now.
		if ( Q_stristr( pMapName, "background" ) )
		{
		}
		else
		{
			// set the map name in the UI
			wchar_t wzMapName[255] = L"";
			g_pVGuiLocalize->ConvertANSIToUnicode( GetMapDisplayName( pMapName ), wzMapName, sizeof( wzMapName ) );

			if ( pMapLabel )
				pMapLabel->SetText( wzMapName );

			// set the map type in the UI
			const char *szMapType = GetMapType( pMapName );
			vgui::Label *pMapTypeLabel = dynamic_cast<vgui::Label*>( FindChildByName( "MapType" ) );
			if ( pMapTypeLabel )
				pMapTypeLabel->SetText( szMapType );

			// set the map author name in the UI
			const char *szMapAuthor = GetMapAuthor( pMapName );
			if ( !V_stricmp( "", szMapAuthor ) || !V_stricmp( "Valve", szMapAuthor ) )
			{
				vgui::Label *pMapAuthorLabel = dynamic_cast<vgui::Label*>( FindChildByName( "MapAuthor" ) );
				if ( pMapAuthorLabel ) 
				{
					pMapAuthorLabel->SetText( szMapAuthor );
					if ( !pMapAuthorLabel->IsVisible() )
						pMapAuthorLabel->SetVisible( true );
				}
			}

			// set the map desc name in the UI
			const char *szMapDesc = GetMapDescription( pMapName );
			if ( !V_stricmp( "", szMapDesc ) )
			{
				vgui::Label *pMapDescLabel = dynamic_cast<vgui::Label*>( FindChildByName( "MapDesc" ) );
				if ( pMapDescLabel ) 
				{
					pMapDescLabel->SetText( szMapDesc );
					if ( !pMapDescLabel->IsVisible() )
						pMapDescLabel->SetVisible( true );
				}
			}

			/*int screenWide, screenTall;
			surface()->GetScreenSize( screenWide, screenTall );

			float aspectRatio = (float)screenWide/(float)screenTall;
			bool bIsWidescreen = aspectRatio >= 1.5999f;

			char szMapBackground[ MAX_PATH ];
			Q_snprintf( szMapBackground, sizeof( szMapBackground ),  bIsWidescreen ? "VGUI/maps/menu_loading_%s_widescreen" : "VGUI/maps/menu_loading_%s", pMapName );
			Q_strlower( szMapBackground );
			IMaterial *pMapBGMaterial = materials->FindMaterial( szMapBackground, TEXTURE_GROUP_VGUI, false );
			if ( pMapBGMaterial && !IsErrorMaterial( pMapBGMaterial ) )
			{
				if ( !m_pBGImage->IsVisible() )
					m_pBGImage->SetVisible( true );

				// take off the vgui/ at the beginning when we set the image
				Q_snprintf( szMapBackground, sizeof( szMapBackground ), bIsWidescreen ? "maps/menu_loading_%s_widescreen" : "maps/menu_loading_%s", pMapName );
				Q_strlower( szMapBackground );

				m_pBGImage->SetImage( szMapBackground );
			}
			else
			{
				if ( m_pBGImage->IsVisible() )
					m_pBGImage->SetVisible( false );

				vgui::ImagePanel *pStampBackground = dynamic_cast<ImagePanel *>( FindChildByName( "Background" ) );
				if ( pStampBackground && !pStampBackground->IsVisible() )
					pStampBackground->SetVisible( true );
			}

			// set the map photos
			vgui::ImagePanel *pMapImage = dynamic_cast<ImagePanel *>( FindChildByName( "MapImage" ) );

			char szMapImage[ MAX_PATH ];
			Q_snprintf( szMapImage, sizeof( szMapImage ),  "VGUI/maps/menu_photos_%s", pMapName );
			Q_strlower( szMapImage );
			IMaterial *pMapMaterial = materials->FindMaterial( szMapImage, TEXTURE_GROUP_VGUI, false );
			if ( pMapMaterial && !IsErrorMaterial( pMapMaterial ) )
			{
				if ( !pMapImage->IsVisible() )
					pMapImage->SetVisible( true );

				// take off the vgui/ at the beginning when we set the image
				Q_snprintf( szMapImage, sizeof( szMapImage ), "maps/menu_photos_%s", pMapName );
				Q_strlower( szMapImage );

				pMapImage->SetImage( szMapImage );
			}*/
		}
	}
	else
	{
		if ( pMapLabel )
			pMapLabel->SetText( "#UI_MapName_Unknown" );
	}
}

//=============================================================================
void LoadingProgress::SetStatusText( const char *statusText )
{
	if ( m_pLoadingProgress )
		m_pLoadingProgress->SetText( statusText );
}

struct s_LMapInfo
{
	const char	*pDiskName;
	const char	*pDisplayName;
	const char	*pGameType;
	const char	*pAuthor;
	const char	*pDescription;
};

struct s_LMapTypeInfo
{
	const char	*pDiskPrefix;
	int			iLength;
	const char	*pGameType;
};

static s_LMapInfo s_LMaps[] = 
{
	"dummy",			"aaaa",				"aaaa",				"aaa",		"aaaa",
};

static s_LMapTypeInfo s_MapTypes[] = 
{
	"cp_",		3, "#Gametype_CP",
	"ctf_",		4, "#Gametype_CTF",
	"pl_",		3, "#Gametype_Escort",
	"plr_",		4, "#Gametype_EscortRace",
	"koth_",	5, "#Gametype_Koth",
	"arena_",	6, "#Gametype_Arena",
	"tc_",		3, "#Gametype_TerritoryControl",
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *GetMapDisplayName( const char *mapName )
{
	static char szDisplayName[256];
	char szTempName[256];
	const char *pszSrc = NULL;

	szDisplayName[0] = '\0';

	if ( !mapName )
		return szDisplayName;

	// check our lookup table
	Q_strncpy( szTempName, mapName, sizeof( szTempName ) );
	Q_strlower( szTempName );

	for ( int i = 0; i < ARRAYSIZE(s_LMaps); ++i )
	{
		if ( !Q_stricmp( s_LMaps[i].pDiskName, szTempName ) )
			return s_LMaps[i].pDisplayName;
	}

	// we haven't found a "friendly" map name, so let's just clean up what we have
	pszSrc = szTempName;

	for ( int i = 0; i < ARRAYSIZE(s_MapTypes); ++i )
	{
		if ( !Q_strncmp( mapName, s_MapTypes[i].pDiskPrefix, s_MapTypes[i].iLength ) )
		{
			pszSrc = szTempName + s_MapTypes[i].iLength;
			break;
		}
	}

	Q_strncpy( szDisplayName, pszSrc, sizeof(szDisplayName) );
	Q_strupr( szDisplayName );

	return szDisplayName;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *GetMapType( const char *mapName )
{
	if ( mapName )
	{
		// Have we got a registered map named that?
		for ( int i = 0; i < ARRAYSIZE(s_LMaps); ++i )
		{
			if ( !Q_stricmp(s_LMaps[i].pDiskName, mapName ) )
			{
				// If so, return the registered gamemode
				return s_LMaps[i].pGameType;
			}
		}
		// If not, see what the prefix is and try and guess from that
		for ( int i = 0; i < ARRAYSIZE(s_MapTypes); ++i )
		{
			if ( !Q_strncmp( mapName, s_MapTypes[i].pDiskPrefix, s_MapTypes[i].iLength ) )
				return s_MapTypes[i].pGameType;
		}
	}

	return "";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *GetMapAuthor( const char *mapName )
{
	if ( mapName )
	{
		// Have we got a registered map named that?
		for ( int i = 0; i < ARRAYSIZE(s_LMaps); ++i )
		{
			if ( !Q_stricmp(s_LMaps[i].pDiskName, mapName ) )
			{
				// If so, return the registered author
				return s_LMaps[i].pAuthor;
			}
		}
	}

	return ""; // Otherwise, return NULL
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *GetMapDescription( const char *mapName )
{
	if ( mapName )
	{
		// Have we got a registered map named that?
		for ( int i = 0; i < ARRAYSIZE(s_LMaps); ++i )
		{
			if ( !Q_stricmp(s_LMaps[i].pDiskName, mapName ) )
				return s_LMaps[i].pDescription;
		}
	}

	return ""; // Otherwise, return NULL
}