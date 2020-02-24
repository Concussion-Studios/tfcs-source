//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSDK_TEXTWINDOW_H
#define CSDK_TEXTWINDOW_H
#ifdef _WIN32
#pragma once
#endif

#include <vguitextwindow.h>

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CSDKTextWindow : public CTextWindow
{
private:
	DECLARE_CLASS_SIMPLE(CSDKTextWindow, CTextWindow);
	
public:
	CSDKTextWindow( IViewPort *pViewPort );

	virtual void Update();
	virtual void SetVisible( bool state );

	// Background panel -------------------------------------------------------

public:
	virtual void PaintBackground() {}
	virtual void PerformLayout();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	bool m_backgroundLayoutFinished;

	// End background panel ---------------------------------------------------
};


#endif // CSDK_TEXTWINDOW_H
