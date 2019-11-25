//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "DialogTabCredits.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDialogTabCredits::CDialogTabCredits( vgui::Panel *parent, const char *name ) :  PropertyPage( parent, NULL )
{
	LoadControlSettings( "Resource/UI/TabCredits.res" );

	m_RichText = dynamic_cast< vgui::RichText* >( FindChildByName( "Credits" ) );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogTabCredits::~CDialogTabCredits()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDialogTabCredits::OnClose( void )
{
	BaseClass::OnClose();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDialogTabCredits::OnCommand( const char *command )
{
	BaseClass::OnCommand( command );
}
