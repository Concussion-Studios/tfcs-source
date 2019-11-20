//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Responsible for drawing the scene
//
// $NoKeywords: $
//=============================================================================//
#ifndef TFC_VIEW_SCENE_H
#define TFC_VIEW_SCENE_H
#ifdef _WIN32
#pragma once
#endif

#include "viewrender.h"
#include "view_scene.h"

class CTFCViewRender : public CViewRender 
{
	DECLARE_CLASS( CTFCViewRender, CViewRender );
public:

	CTFCViewRender();

	virtual void Init();
	virtual void SetupRenderTargets();
	virtual void RenderView( const CViewSetup &view, int nClearFlags, int whatToDraw );
};


#endif	//TFC_VIEW_SCENE_H