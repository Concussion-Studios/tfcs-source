//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#ifndef SDK_TEAMMENU_H
#define SDK_TEAMMENU_H

#include "teammenu.h"

class CSDKTeamMenu : public CTeamMenu
{
private:
	DECLARE_CLASS_SIMPLE( CSDKTeamMenu, CTeamMenu );

	CSDKTeamMenu( IViewPort *pViewPort );
	virtual ~CSDKTeamMenu();

	void Update();
	virtual void SetVisible( bool state );

public:

	// VGUI2 override
	void OnCommand( const char* command );

	// helper functions
	void SetVisibleButton( const char* textEntryName, bool state );

	// Background panel -------------------------------------------------------

public:
	virtual void PaintBackground( void ) { /* Draw nothing */ }
	virtual void PerformLayout();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	bool m_backgroundLayoutFinished;

	// End background panel ---------------------------------------------------
};

#endif //SDK_CLASSMENU_H