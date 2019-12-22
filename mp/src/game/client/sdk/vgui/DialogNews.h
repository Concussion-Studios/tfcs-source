//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef DIALOGNEWS_H
#define DIALOGNEWS_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/PropertyDialog.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CDialogNews : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE( CDialogNews, vgui::PropertyDialog );

public:
	CDialogNews(vgui::Panel *parent);
	~CDialogNews();

	void Run();
	virtual void Activate();

	MESSAGE_FUNC( OnGameUIHidden, "GameUIHidden" );	// called when the GameUI is hidden
};

#endif // DIALOGNEWS_H
