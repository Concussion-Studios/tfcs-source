//========= Copyright © 1996-2006, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef SDK_IMAGEPANEL_H
#define SDK_IMAGEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui_controls/ImagePanel.h>

class CSDKImagePanel : public vgui::ImagePanel
{
public:
	DECLARE_CLASS_SIMPLE( CSDKImagePanel, vgui::ImagePanel );

	CSDKImagePanel(vgui::Panel *parent, const char *name);

	virtual void ApplySettings( KeyValues *inResourceData );
	virtual Color GetDrawColor( void );
};

#endif // SDK_IMAGEPANEL_H
