//========= Copyright © 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef EXMENULABEL_H
#define EXMENULABEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <KeyValues.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Label.h>

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CExMenuLabel : public vgui::Label
{
public:
	DECLARE_CLASS_SIMPLE( CExMenuLabel, vgui::Label );

	CExMenuLabel( vgui::Panel *parent, const char *panelName, const char *text );
	CExMenuLabel( vgui::Panel *parent, const char *panelName, const wchar_t *wszText );

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	char		m_szColor[64];
};

#endif // EXMENULABEL_H
