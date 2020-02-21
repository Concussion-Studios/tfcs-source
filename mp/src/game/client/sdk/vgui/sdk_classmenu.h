//======== Copyright © 1996-2008, Valve Corporation, All rights reserved. =========//
//
// Purpose: 
//
// $NoKeywords: $
//=================================================================================//

#ifndef SDK_CLASSMENU_H
#define SDK_CLASSMENU_H

#include <classmenu.h>
#include <vgui_controls/EditablePanel.h>
#include <FileSystem.h>
#include "iconpanel.h"
#include "mouseoverpanelbutton.h"
#include <vgui_controls/CheckButton.h>

class CSDKClassInfoPanel : public vgui::EditablePanel
{
private:
	DECLARE_CLASS_SIMPLE( CSDKClassInfoPanel, vgui::EditablePanel );

public:
	CSDKClassInfoPanel( vgui::Panel *parent, const char *panelName ) : vgui::EditablePanel( parent, panelName )	{}
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual vgui::Panel *CreateControlByName( const char *controlName );

private:
	vgui::HFont m_hFont;
};

class CSDKClassMenu : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CSDKClassMenu, CClassMenu );

public:
	CSDKClassMenu( IViewPort *pViewPort );
	CSDKClassMenu( IViewPort *pViewPort, const char *panelName );
	virtual ~CSDKClassMenu();

	virtual void Update( void );
	virtual Panel *CreateControlByName( const char *controlName );
	virtual void OnTick( void );
	virtual void OnKeyCodePressed(KeyCode code);
	virtual void SetVisible( bool state );

	// helper functions
	void SetVisibleButton( const char *textEntryName, bool state );
	virtual void ShowPanel( bool bShow );

	MESSAGE_FUNC_CHARPTR( OnShowPage, "ShowPage", page );

	virtual int GetTeamNumber( void ) = 0;

	// Background panel -------------------------------------------------------

public:
	virtual void PaintBackground( void ) { /* Draw nothing */ }
	virtual void PerformLayout();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	bool m_backgroundLayoutFinished;

	// End background panel ---------------------------------------------------

private:
	CSDKClassInfoPanel *m_pClassInfoPanel;
	MouseOverButton<CSDKClassInfoPanel> *m_pInitialButton;

	int m_iActivePlayerClass;
	int m_iLastPlayerClassCount;
	int	m_iLastClassLimit;

	ButtonCode_t m_iClassMenuKey;
};

class CSDKClassMenu_Blue : public CSDKClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CSDKClassMenu_Blue, CSDKClassMenu );

public:
	CSDKClassMenu_Blue::CSDKClassMenu_Blue(IViewPort *pViewPort) : BaseClass(pViewPort, PANEL_CLASS_BLUE) {	LoadControlSettings( "Resource/UI/ClassMenu_Blue.res" ); }
	virtual const char *GetName( void )	{ return PANEL_CLASS_BLUE; }
	virtual int GetTeamNumber( void ) {	return SDK_TEAM_BLUE; }
};

class CSDKClassMenu_Red : public CSDKClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CSDKClassMenu_Red, CSDKClassMenu );

public:
	CSDKClassMenu_Red::CSDKClassMenu_Red(IViewPort *pViewPort) : BaseClass(pViewPort, PANEL_CLASS_RED)	{ LoadControlSettings( "Resource/UI/ClassMenu_Red.res" ); }
	virtual const char *GetName( void )	{ return PANEL_CLASS_RED; }
	virtual int GetTeamNumber( void ) {	return SDK_TEAM_RED; }
};

class CSDKClassMenu_Green : public CSDKClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CSDKClassMenu_Green, CSDKClassMenu );

public:
	CSDKClassMenu_Green::CSDKClassMenu_Green(IViewPort *pViewPort) : BaseClass(pViewPort, PANEL_CLASS_GREEN) {	LoadControlSettings( "Resource/UI/ClassMenu_Green.res" ); }
	virtual const char *GetName( void )	{ return PANEL_CLASS_GREEN; }
	virtual int GetTeamNumber( void ) {	return SDK_TEAM_GREEN; }
};

class CSDKClassMenu_Yellow : public CSDKClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CSDKClassMenu_Yellow, CSDKClassMenu );

public:
	CSDKClassMenu_Yellow::CSDKClassMenu_Yellow(IViewPort *pViewPort) : BaseClass(pViewPort, PANEL_CLASS_YELLOW)	{ LoadControlSettings( "Resource/UI/ClassMenu_Yellow.res" ); }
	virtual const char *GetName( void )	{ return PANEL_CLASS_YELLOW; }
	virtual int GetTeamNumber( void ) {	return SDK_TEAM_YELLOW; }
};

#endif //SDK_CLASSMENU_H