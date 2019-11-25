//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef DIALOGTABCHANGELOG_H
#define DIALOGTABCHANGELOG_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/PropertyPage.h"
#include <vgui_controls/RichText.h>

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CDialogTabChangeLog : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CDialogTabChangeLog, vgui::PropertyPage );

public:
	CDialogTabChangeLog( vgui::Panel *parent );
	~CDialogTabChangeLog( void );

	virtual void OnCommand( const char *command );
	virtual void OnClose( void );

private:
	vgui::RichText* m_RichText;
};

#endif // DIALOGTABCHANGELOG_H
