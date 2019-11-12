//======== Copyright � 1996-2008, Valve Corporation, All rights reserved. =========//
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
};

class CSDKClassMenu : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CSDKClassMenu, CClassMenu );

public:
	CSDKClassMenu(IViewPort *pViewPort);
	CSDKClassMenu(IViewPort *pViewPort, const char *panelName);
	virtual ~CSDKClassMenu();

	virtual void Update( void );
	void MoveToCenterOfScreen();
	virtual Panel *CreateControlByName( const char *controlName );
	virtual void OnTick( void );
	virtual void OnKeyCodePressed(KeyCode code);
	virtual void SetVisible( bool state );
	// helper functions
	void SetVisibleButton(const char *textEntryName, bool state);
	virtual void ShowPanel(bool bShow);

	MESSAGE_FUNC_CHARPTR( OnShowPage, "ShowPage", page );

	void UpdateNumClassLabel( void );

	MESSAGE_FUNC_PTR( OnSuicideOptionChanged, "CheckButtonChecked", panel );

	virtual int GetTeamNumber( void ) = 0;


private:
	CSDKClassInfoPanel *m_pClassInfoPanel;
	MouseOverButton<CSDKClassInfoPanel> *m_pInitialButton;
	CheckButton *m_pSuicideOption;

	int m_iActivePlayerClass;
	int m_iLastPlayerClassCount;
	int	m_iLastClassLimit;

	ButtonCode_t m_iClassMenuKey;

	vgui::Label *m_pClassNumLabel[SDK_NUM_PLAYERCLASSES];
	vgui::Label *m_pClassFullLabel[SDK_NUM_PLAYERCLASSES];

protected:
	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	// rounded corners
	Color m_bgColor;
	Color m_borderColor;
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