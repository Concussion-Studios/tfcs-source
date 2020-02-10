//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef EXMENUBUTTON_H
#define EXMENUBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Button.h>

//-----------------------------------------------------------------------------
// Purpose: Extended Verson Of vgui::Button For GameUI
//-----------------------------------------------------------------------------
class CExMenuButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CExMenuButton, Button );

public:
	// You can optionally pass in the panel to send the click message to and the name of the command to send to that panel.
	CExMenuButton( vgui::Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL );
	CExMenuButton( vgui::Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget=NULL, const char *pCmd=NULL );

	~CExMenuButton();

	virtual void OnCommand( const char *command );
	virtual void OnCursorEntered();
	virtual void ApplySettings( KeyValues* inResourceData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	typedef vgui::Button BaseClass;

	char		m_szFont[64];
	char		m_szColor[64];
};


#endif // EXMENUBUTTON_H