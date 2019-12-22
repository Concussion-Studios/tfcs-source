//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "DialogTabChangeLog.h"
#include "cdll_client_int.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDialogTabChangeLog::CDialogTabChangeLog( Panel *parent ) : BaseClass( parent, NULL )
{
	LoadControlSettings( "Resource/UI/TabChangeLog.res" );

	m_RichText = dynamic_cast< RichText* >( FindChildByName( "ChangeLog" ) );
	if ( m_RichText )
	{
		auto hndl = filesystem->Open( "changelog.txt", "rb", "MOD" );
		if ( hndl )
		{
			unsigned int size = filesystem->Size( hndl );

			char* buf = new char[size];
			buf[size-1] = NULL;

			filesystem->Read( buf, size-1, hndl );

			m_RichText->InsertString( buf );

			delete[] buf;

			filesystem->Close( hndl );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CDialogTabChangeLog::~CDialogTabChangeLog()
{
}