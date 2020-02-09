//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef __VLOADINGPROGRESS_H__
#define __VLOADINGPROGRESS_H__

#include "basemodui.h"
#include "vgui/IScheme.h"
#include "const.h"
#include "igameevents.h"
#include "ExMenuButton.h"

namespace BaseModUI 
{
	class LoadingProgress : public CBaseModFrame, public IGameEventListener2
	{
		DECLARE_CLASS_SIMPLE( LoadingProgress, CBaseModFrame );

	public:
		enum LoadingType
		{
			LT_UNDEFINED = 0,
			LT_MAINMENU,
			LT_TRANSITION,
		};

		enum LoadingWindowType
		{
			LWT_LOADINGPLAQUE,
			LWT_BKGNDSCREEN,
		};

	public:
		LoadingProgress( vgui::Panel *parent, const char *panelName, LoadingWindowType eLoadingType );
		~LoadingProgress();

		virtual void		Activate();
		virtual void		Close();

		void				FireGameEvent( IGameEvent* event ) OVERRIDE;

		void				SetProgress( float progress );
		float				GetProgress();
		bool				IsDrawingProgressBar( void ) { return m_bDrawProgress; }

		void				SetLoadingType( LoadingType loadingType );
		LoadingType			GetLoadingType();

		void				SetStatusText( const char *statusText );

		void				SetMapData( KeyValues *pMapInfo );
		void				SetupMapInfo( void );
	protected:
		virtual void		OnThink();
		virtual void		OnCommand(const char *command);
		virtual void		ApplySchemeSettings( vgui::IScheme *pScheme );
		virtual void		PaintBackground();
		virtual void		OnKeyCodeTyped( vgui::KeyCode code );

	private:
		void				SetupControlStates( void );
		void				UpdateLoadingSpinner();

		vgui::ProgressBar	*m_pLoadingBar;
		vgui::ImagePanel	*m_pLoadingSpinner;
		vgui::ImagePanel	*m_pBGImage;
		vgui::Panel			*m_pFooter;
		vgui::ImagePanel	*m_pLogoImage;
		vgui::Label			*m_pLoadingProgress;
		CExMenuButton		*m_pCancelButton;

		LoadingType			m_LoadingType;
		LoadingWindowType	m_LoadingWindowType;

		bool				m_bLoadedFully;

		// Map Data
		KeyValues			*m_pMapInfo;
		bool				m_bValid;

		int					m_textureID_LoadingBar;
		int					m_textureID_LoadingBarBG;

		bool				m_bDrawBackground;
		bool				m_bDrawProgress;
		bool				m_bDrawSpinner;

		float				m_flPeakProgress;

		float				m_flLastEngineTime;
	};
};

#endif // __VLOADINGPROGRESS_H__