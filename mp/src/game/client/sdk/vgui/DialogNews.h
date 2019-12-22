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
	CDialogNews( vgui::VPANEL parent );
	~CDialogNews();

	virtual void OnClose( void );
	virtual void OnCommand( const char *command );
};

#endif // DIALOGNEWS_H
