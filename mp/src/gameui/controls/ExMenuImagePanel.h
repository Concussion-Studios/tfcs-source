//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef EXMENUIMAGEPANEL_H
#define EXMENUIMAGEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui_controls/ImagePanel.h>
#include "vgui_controls/ScalableImagePanel.h"

class CExMenuImagePanel : public vgui::ScalableImagePanel
{
public:
	DECLARE_CLASS_SIMPLE( CExMenuImagePanel, vgui::ScalableImagePanel );

	CExMenuImagePanel( vgui::Panel *parent, const char *name );

	virtual void ApplySettings( KeyValues *inResourceData );

	virtual Color GetDrawColor( void );

};

#endif // EXMENUIMAGEPANEL_H