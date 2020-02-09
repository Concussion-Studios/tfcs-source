//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef __VMAINMENU_H__
#define __VMAINMENU_H__

#include "basemodui.h"
#include "vflyoutmenu.h"

enum MusicStatus
{
	MUSIC_STOP,
	MUSIC_FIND,
	MUSIC_PLAY,
	MUSIC_STOP_FIND,
	MUSIC_STOP_PLAY,
};

namespace BaseModUI {

class MainMenu : public CBaseModFrame, public IBaseModFrameListener, public FlyoutMenuListener
{
	DECLARE_CLASS_SIMPLE( MainMenu, CBaseModFrame );

public:
	MainMenu(vgui::Panel *parent, const char *panelName);
	~MainMenu();

	void UpdateVisibility();

	//flyout menu listener
	virtual void OnNotifyChildFocus( vgui::Panel* child ) {}
	virtual void OnFlyoutMenuClose( vgui::Panel* flyTo ) {}
	virtual void OnFlyoutMenuCancelled() {}

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnCommand( const char *command );
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnKeyCodeTyped( vgui::KeyCode code );
	virtual void OnThink();
	virtual void OnOpen();
	virtual void RunFrame();
	virtual void PaintBackground();

private:
	void GetRandomMusic( char *pszBuf, int iBufLength );

	static void AcceptQuitGameCallback();
	void SetFooterState() {}

	char				m_pzMusicLink[64];	
	int					m_nSongGuid;
	MusicStatus			m_psMusicStatus;

	vgui::ImagePanel	*m_pBGImage;
	vgui::ImagePanel	*m_pLogoImage;

	CPanelAnimationVar( Color, m_BackgroundColor, "background_color", "0 0 0 255" );
};

}

#endif // __VMAINMENU_H__
