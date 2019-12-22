//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "DialogTabCredits.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDialogTabCredits::CDialogTabCredits( Panel *parent ) : BaseClass( parent, NULL )
{
	LoadControlSettings( "Resource/UI/TabCredits.res" );

	m_RichText = dynamic_cast< RichText* >( FindChildByName( "Credits" ) );
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
