//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef DIALOGTABCREDITS_H
#define DIALOGTABCREDITS_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/PropertyPage.h"
#include <vgui_controls/RichText.h>

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CDialogTabCredits : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CDialogTabCredits, vgui::PropertyPage );

public:
	CDialogTabCredits( vgui::Panel *parent );
	~CDialogTabCredits( void );

private:

	vgui::RichText* m_RichText;
};

#endif // DIALOGTABCREDITS_H
