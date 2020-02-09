#include "cbase.h"
#include "nb_header_footer.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ImagePanel.h"
#include <vgui/ISurface.h>
#include "vgui_video_player.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "EngineInterface.h"
#include "basemodpanel.h"

// UI defines. Include if you want to implement some of them [str]
#include "ui_defines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace VideoPlaybackFlags;
using namespace VideoSystem;

CBackgroundMovie *g_pBackgroundMovie = NULL;

CBackgroundMovie* BackgroundMovie()
{
	if(!g_pBackgroundMovie)
		g_pBackgroundMovie = new CBackgroundMovie();
	return g_pBackgroundMovie;
}

CBackgroundMovie::CBackgroundMovie()
{
	m_pVideoMaterial = NULL;
	m_nBIKMaterial = BIKMATERIAL_INVALID;
	m_nTextureID = -1;
	m_szCurrentMovie[0] = 0;
	m_nLastGameState = -1;
	m_bEnabled = 1;
}

CBackgroundMovie::~CBackgroundMovie()
{

}

void CBackgroundMovie::SetCurrentMovie(const char *szFilename)
{
	ClearCurrentMovie();

	char szMaterialName[MAX_PATH];
	Q_snprintf(szMaterialName, sizeof(szMaterialName), "BackgroundMaterial%i", g_pVideo->GetUniqueMaterialID());
	m_pVideoMaterial = g_pVideo->CreateVideoMaterial(szMaterialName, szFilename, "GAME", NO_AUDIO | LOOP_VIDEO, DETERMINE_FROM_FILE_EXTENSION, true);
	if ( !m_pVideoMaterial )
		return;
	int nWidth, nHeight;
	m_pVideoMaterial->GetVideoImageSize(&nWidth, &nHeight);
	m_pVideoMaterial->GetVideoTexCoordRange(&m_flU, &m_flV);

	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	float flFrameRatio = ( (float) wide / (float) tall );
	float flVideoRatio = ( (float) nWidth / (float) nHeight );

	if ( flVideoRatio > flFrameRatio )
	{
		m_nPlaybackWidth = wide;
		m_nPlaybackHeight = ( wide / flVideoRatio );
	}
	else if ( flVideoRatio < flFrameRatio )
	{
		m_nPlaybackWidth = ( tall * flVideoRatio );
		m_nPlaybackHeight = tall;
	}
	else
	{
		m_nPlaybackWidth = wide;
		m_nPlaybackHeight = tall;
	}

	Q_snprintf(m_szCurrentMovie, sizeof(m_szCurrentMovie), "%s", szFilename);
}

void CBackgroundMovie::ClearCurrentMovie()
{
	if( m_pVideoMaterial != NULL ) 
	{
		// FIXME: Make sure the m_pMaterial is actually destroyed at this point!
		m_pVideoMaterial->StopVideo();
		g_pVideo->DestroyVideoMaterial( m_pVideoMaterial );
		m_pVideoMaterial = NULL;
		g_pMatSystemSurface->DeleteTextureByID( m_nTextureID );
		g_pMatSystemSurface->DestroyTextureID( m_nTextureID );
		m_nTextureID = -1;
	}
}

int CBackgroundMovie::SetTextureMaterial()
{
	if( m_pVideoMaterial == NULL )
		return -1;

	if( m_nTextureID == -1 )
		m_nTextureID = g_pMatSystemSurface->CreateNewTextureID( true );

	g_pMatSystemSurface->DrawSetTextureMaterial( m_nTextureID, m_pVideoMaterial->GetMaterial() );
	return m_nTextureID;
}

void CBackgroundMovie::Update()
{
	if ( !m_bEnabled )
	{
		ClearCurrentMovie();
		return;
	}
	
	int nGameState = 0;
	if( nGameState != m_nLastGameState ) 
	{
		const char *pFilename = NULL;

		int nChosenMovie = RandomInt( 0, 4 );
		switch( nChosenMovie ) {
			case 0: pFilename = "media/background01.bik"; break;
			default:
			case 1: pFilename = "media/background02.bik"; break;
			case 2: pFilename = "media/background03.bik"; break;
			case 3: pFilename = "media/background04.bik"; break;
			case 4: pFilename = "media/background05.bik"; break;
		}

		if( pFilename ) {
			SetCurrentMovie( pFilename );
		}
	}
	m_nLastGameState = nGameState;

	if (m_pVideoMaterial == NULL)
	{
		m_nLastGameState = 1;
		return;
	}

	if( !m_pVideoMaterial->IsVideoPlaying() ) {
		if( !m_pVideoMaterial->StartVideo() )
			ClearCurrentMovie();
	}

	if( m_pVideoMaterial->Update() == false )
		ClearCurrentMovie();

}

// ======================================

CNB_Header_Footer::CNB_Header_Footer( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	// == MANAGED_MEMBER_CREATION_START: Do not edit by hand ==
	m_pBackground = new vgui::Panel( this, "Background" );
	m_pBackgroundImage = new vgui::ImagePanel( this, "BackgroundImage" );	
	m_pTitle = new vgui::Label( this, "Title", "" );
	m_pBottomBar = new vgui::Panel( this, "BottomBar" );
	m_pBottomBarLine = new vgui::Panel( this, "BottomBarLine" );
	m_pTopBar = new vgui::Panel( this, "TopBar" );
	m_pTopBarLine = new vgui::Panel( this, "TopBarLine" );
	// == MANAGED_MEMBER_CREATION_END ==
	m_pGradientBar = new CNB_Gradient_Bar( this, "GradientBar" );
	m_pGradientBar->SetZPos( 2 );

	m_bHeaderEnabled = true;
	m_bFooterEnabled = true;
	m_bMovieEnabled = true;
	m_bGradientBarEnabled = 0;
	m_nTitleStyle = NB_TITLE_MEDIUM;
	m_nBackgroundStyle = NB_BACKGROUND_TRANSPARENT_BLUE;
	m_nGradientBarY = 0;
	m_nGradientBarHeight = 480;
}

CNB_Header_Footer::~CNB_Header_Footer()
{

}

ConVar ui_background_color( "ui_background_color", "32 32 32 128", FCVAR_NONE, "Color of background tinting in briefing screens" );

void CNB_Header_Footer::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	
	LoadControlSettings( "resource/ui/nb_header_footer.res" );

	// TODO: Different image in widescreen to avoid stretching
	// this image is no longer used
	//m_pBackgroundImage->SetImage( "lobby/swarm_background01" );

	switch( m_nTitleStyle )
	{
		case NB_TITLE_BRIGHT: m_pTitle->SetFgColor( Color( 255, 255, 255, 255 ) ); break;
		case NB_TITLE_MEDIUM: m_pTitle->SetFgColor( Color( UI_STYLE_NB_TITLE_MEDIUM ) ); break;
	}

	switch( m_nBackgroundStyle )
	{
		case NB_BACKGROUND_DARK:
			{
				m_pBackground->SetVisible( true );
				m_pBackgroundImage->SetVisible( false );
				m_pBackground->SetBgColor( Color( 0, 0, 0, 230 ) );
				break;
			}
		case NB_BACKGROUND_TRANSPARENT_BLUE:
			{
				m_pBackground->SetVisible( true );
				m_pBackgroundImage->SetVisible( false );

				color32 clr32;
				UTIL_StringToColor32(&clr32, ui_background_color.GetString());
				
				Color clr;
				clr.SetColor(clr32.r, clr32.g, clr32.b, clr32.a);

				m_pBackground->SetBgColor(clr);
				break;
			}
		case NB_BACKGROUND_TRANSPARENT_RED:
			{
				m_pBackground->SetVisible( true );
				m_pBackgroundImage->SetVisible( false );
				m_pBackground->SetBgColor( Color( 128, 0, 0, 128 ) );
				break;
			}
		case NB_BACKGROUND_BLUE:
			{
				m_pBackground->SetVisible( true );
				m_pBackgroundImage->SetVisible( false );
				m_pBackground->SetBgColor( Color( 16, 32, 46, 230 ) );
				break;
			}
		case NB_BACKGROUND_IMAGE:
			{
				m_pBackground->SetVisible( false );
				m_pBackgroundImage->SetVisible( true );
				break;
			}

		case NB_BACKGROUND_NONE:
			{
				m_pBackground->SetVisible( false );
				m_pBackgroundImage->SetVisible( false );
			}
	}

	m_pTopBar->SetVisible( m_bHeaderEnabled );
	m_pTopBarLine->SetVisible( m_bHeaderEnabled );
	m_pBottomBar->SetVisible( m_bFooterEnabled );
	m_pBottomBarLine->SetVisible( m_bFooterEnabled );
	m_pGradientBar->SetVisible( m_bGradientBarEnabled );
}

void CNB_Header_Footer::PerformLayout()
{
	BaseClass::PerformLayout();

	m_pGradientBar->SetBounds( 0, YRES( m_nGradientBarY ), ScreenWidth(), YRES( m_nGradientBarHeight ) );
}

void CNB_Header_Footer::OnThink()
{
	BaseClass::OnThink();
}

void CNB_Header_Footer::SetTitle( const char *pszTitle )
{
	m_pTitle->SetText( pszTitle );
}

void CNB_Header_Footer::SetTitle( wchar_t *pwszTitle )
{
	m_pTitle->SetText( pwszTitle );
}

void CNB_Header_Footer::SetHeaderEnabled( bool bEnabled )
{
	m_pTopBar->SetVisible( bEnabled );
	m_pTopBarLine->SetVisible( bEnabled );
	m_bHeaderEnabled = bEnabled;
}

void CNB_Header_Footer::SetFooterEnabled( bool bEnabled )
{
	m_pBottomBar->SetVisible( bEnabled );
	m_pBottomBarLine->SetVisible( bEnabled );
	m_bFooterEnabled = bEnabled;
}

void CNB_Header_Footer::SetGradientBarEnabled( bool bEnabled )
{
	m_pGradientBar->SetVisible( bEnabled );
	m_bGradientBarEnabled = bEnabled;
}

void CNB_Header_Footer::SetGradientBarPos( int nY, int nHeight )
{
	m_nGradientBarY = nY;
	m_nGradientBarHeight = nHeight;
	m_pGradientBar->SetBounds( 0, YRES( m_nGradientBarY ), ScreenWidth(), YRES( m_nGradientBarHeight ) );
}

void CNB_Header_Footer::SetTitleStyle( NB_Title_Style nTitleStyle )
{
	m_nTitleStyle = nTitleStyle;
	InvalidateLayout( false, true );
}

void CNB_Header_Footer::SetBackgroundStyle( NB_Background_Style nBackgroundStyle )
{
	m_nBackgroundStyle = nBackgroundStyle;
	InvalidateLayout( false, true );
}

void CNB_Header_Footer::SetMovieEnabled( bool bMovieEnabled )
{
	m_bMovieEnabled = bMovieEnabled;
	InvalidateLayout( false, true );
}

void CNB_Header_Footer::PaintBackground()
{
	BaseClass::PaintBackground();

	if ( m_bMovieEnabled && BackgroundMovie() )
	{
		BackgroundMovie()->Update();
		if ( BackgroundMovie()->SetTextureMaterial() != -1 )
		{
			surface()->DrawSetColor( 255, 255, 255, 255 );

			int x, y, w, t;
			GetBounds( x, y, w, t );

			// center, 16:10 aspect ratio
			int width_at_ratio = t * (16.0f / 9.0f);
			x = ( w * 0.5f ) - ( width_at_ratio * 0.5f );
			
			surface()->DrawTexturedRect( x, y, x + width_at_ratio, y + t );
		}
	}

	// test of gradient header/footer
	
	int nScreenWidth = GetWide();
	int nScreenHeight = GetTall();
	int iHalfWide = nScreenWidth * 0.5f;
	int nBarHeight = YRES( 22 );

	//surface()->DrawSetColor( Color( 16, 32, 46, 230 ) );
	surface()->DrawSetColor( Color( 0, 0, 0, 230 ) );
	surface()->DrawFilledRect( 0, 0, nScreenWidth, nScreenHeight );

	if ( m_bHeaderEnabled )
	{
		surface()->DrawSetColor( Color( UI_STYLE_FOOTER_GRADIENT ) );
		surface()->DrawFilledRect( 0, 0, nScreenWidth, nBarHeight );

		surface()->DrawSetColor( Color( UI_STYLE_FOOTER_GRALINE ) );
		surface()->DrawFilledRectFade( iHalfWide, 0, iHalfWide + YRES( 320 ), nBarHeight, 255, 0, true );
		surface()->DrawFilledRectFade( iHalfWide - YRES( 320 ), 0, iHalfWide, nBarHeight, 0, 255, true );
	}

	if ( m_bFooterEnabled )
	{
		surface()->DrawSetColor( Color( UI_STYLE_FOOTER_GRADIENT ) );
		surface()->DrawFilledRect( 0, nScreenHeight - nBarHeight, nScreenWidth, nScreenHeight );

		surface()->DrawSetColor( Color( UI_STYLE_FOOTER_GRALINE ) );
		surface()->DrawFilledRectFade( iHalfWide, nScreenHeight - nBarHeight, iHalfWide + YRES( 320 ), nScreenHeight, 255, 0, true );
		surface()->DrawFilledRectFade( iHalfWide - YRES( 320 ), nScreenHeight - nBarHeight, iHalfWide, nScreenHeight, 0, 255, true );
	}
	
}

// =================

CNB_Gradient_Bar::CNB_Gradient_Bar( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
}

void CNB_Gradient_Bar::PaintBackground()
{
	int wide, tall;
	GetSize( wide, tall );

	int y = 0;
	int iHalfWide = wide * 0.5f;

	float flAlpha = 200.0f / 255.0f;

	// fill bar background
	vgui::surface()->DrawSetColor( Color( 0, 0, 0, 255 * flAlpha ) );
	vgui::surface()->DrawFilledRect( 0, y, wide, y + tall );

	vgui::surface()->DrawSetColor( Color( UI_STYLE_BACKGROUNDFILL * flAlpha ) );

	int nBarPosY = y + YRES( 4 );
	int nBarHeight = tall - YRES( 8 );
	vgui::surface()->DrawFilledRectFade( iHalfWide, nBarPosY, wide, nBarPosY + nBarHeight, 255, 0, true );
	vgui::surface()->DrawFilledRectFade( 0, nBarPosY, iHalfWide, nBarPosY + nBarHeight, 0, 255, true );
	// draw highlights
	nBarHeight = YRES( 2 );
	nBarPosY = y;
	vgui::surface()->DrawSetColor( Color( UI_STYLE_HIGHLIGHTS * flAlpha ) );
	vgui::surface()->DrawFilledRectFade( iHalfWide, nBarPosY, wide, nBarPosY + nBarHeight, 255, 0, true );
	vgui::surface()->DrawFilledRectFade( 0, nBarPosY, iHalfWide, nBarPosY + nBarHeight, 0, 255, true );

	nBarPosY = y + tall - YRES( 2 );
	vgui::surface()->DrawFilledRectFade( iHalfWide, nBarPosY, wide, nBarPosY + nBarHeight, 255, 0, true );
	vgui::surface()->DrawFilledRectFade( 0, nBarPosY, iHalfWide, nBarPosY + nBarHeight, 0, 255, true );
}
