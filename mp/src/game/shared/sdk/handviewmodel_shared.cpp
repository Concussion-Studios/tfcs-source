//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "handviewmodel_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( hand_viewmodel, CHandViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( HandViewModel, DT_HandViewModel )

// for whatever reason the parent doesn't get sent 
// I don't really want to mess with the baseviewmodel
// so now it does
BEGIN_NETWORK_TABLE( CHandViewModel, DT_HandViewModel )
#ifndef CLIENT_DLL
	SendPropEHandle( SENDINFO_NAME( m_hMoveParent, moveparent ) ),
#else
	RecvPropInt( RECVINFO_NAME( m_hNetworkMoveParent, moveparent ), 0, RecvProxy_IntToMoveParent ),
#endif
END_NETWORK_TABLE()