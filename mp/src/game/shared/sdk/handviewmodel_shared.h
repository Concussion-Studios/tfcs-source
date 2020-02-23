//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef HANDVIEWMODEL_SHARED_H
#define HANDVIEWMODEL_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "baseviewmodel_shared.h"

#if defined( CLIENT_DLL )
	#define CHandViewModel C_HandViewModel
#endif

class CHandViewModel : public CBaseViewModel
{
	DECLARE_CLASS( CHandViewModel, CBaseViewModel );

public:
	DECLARE_NETWORKCLASS();
};

#endif // HANDVIEWMODEL_SHARED_H