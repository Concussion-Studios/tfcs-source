//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "DialogTabChangeLog.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDialogTabChangeLog::CDialogTabChangeLog( vgui::Panel *parent, const char *name ) : PropertyPage( parent, NULL )
{
	LoadControlSettings( "Resource/UI/TabChangeLog.res" );

	m_RichText = dynamic_cast< vgui::RichText* >( FindChildByName( "ChangeLog" ) );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogTabChangeLog::~CDialogTabChangeLog()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDialogTabChangeLog::OnClose( void )
{
	BaseClass::OnClose();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDialogTabChangeLog::OnCommand( const char *command )
{
	BaseClass::OnCommand( command );
}
