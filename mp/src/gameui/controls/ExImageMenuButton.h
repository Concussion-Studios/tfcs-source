//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef EXIMAGEMENUBUTTON_H
#define EXIMAGEMENUBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include "ExMenuButton.h"
#include <vgui/IScheme.h>
#include <vgui/KeyCode.h>
#include <vgui/IVGui.h>
#include <vgui_controls/ImagePanel.h>

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CExImageMenuButton : public CExMenuButton
{
public:
	DECLARE_CLASS_SIMPLE( CExImageMenuButton, CExMenuButton );

	CExImageMenuButton( vgui::Panel *parent, const char *name, const char *text );
	CExImageMenuButton( vgui::Panel *parent, const char *name, const wchar_t *wszText );

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void SetArmed( bool state );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void SetImageArmed( const char* image );
	virtual void SetImageSelected( const char* image );
	virtual void SetSelected( bool state );
protected:
	vgui::ImagePanel *m_pSubImage;
	char	m_pszImageDefaultName[260];
	char	m_pszImageArmedName[260];
	char	m_pszImageSelectedName[260];
	Color	m_clrDraw;
	Color	m_clrArmed;
	Color	m_clrDepressed;
	Color	m_clrDisabled;
	Color	m_clrSelected;
};

#endif // EXIMAGEMENUBUTTON_H
