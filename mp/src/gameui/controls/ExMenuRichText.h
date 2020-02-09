//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef EXMENURICHTEXT_H
#define EXMENURICHTEXT_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <KeyValues.h>
#include <vgui/IVGui.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/RichText.h>
#include "ExMenuImagePanel.h"
#include <vgui_controls/ImagePanel.h>

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CExMenuRichText : public vgui::RichText
{
public:
	DECLARE_CLASS_SIMPLE( CExMenuRichText, vgui::RichText );

	CExMenuRichText(vgui::Panel *parent, const char *panelName);

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();
	virtual void SetText( const char *text );
	virtual void SetText( const wchar_t *text );

	virtual void OnTick( void );
	void SetScrollBarImagesVisible( bool visible );

private:
	char		m_szFont[64];
	char		m_szColor[64];

	CExMenuImagePanel		*m_pUpArrow;
	vgui::ImagePanel	*m_pLine;
	CExMenuImagePanel		*m_pDownArrow;
	vgui::ImagePanel	*m_pBox;
protected:
	char			pDefaultScrollImage[64];
	char			pDefaultScrollLineImage[64];
};

#endif // EXMENURICHTEXT_H
