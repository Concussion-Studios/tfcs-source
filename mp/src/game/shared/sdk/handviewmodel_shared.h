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

#include "predicted_viewmodel.h"

#if defined( CLIENT_DLL )
	#define CHandViewModel C_HandViewModel
#endif

class CHandViewModel : public CPredictedViewModel
{
	DECLARE_CLASS( CHandViewModel, CPredictedViewModel );

public:
	DECLARE_NETWORKCLASS();
};

#endif // HANDVIEWMODEL_SHARED_H