//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Responsible for drawing the scene
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "tfc_view_scene.h"

#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CTFCViewRender g_ViewRender;

CTFCViewRender* TFCViewRender()
{
	return &g_ViewRender;
}

CTFCViewRender::CTFCViewRender()
{
	view = ( IViewRender * )&g_ViewRender;
}

void CTFCViewRender::Init()
{
	BaseClass::Init();

	SetupRenderTargets();
}

void CTFCViewRender::SetupRenderTargets()
{
	ITexture *depthOld = materials->FindTexture( "_rt_ResolvedFullFrameDepth", TEXTURE_GROUP_RENDER_TARGET );
	static int flags = TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_RENDERTARGET;
	if ( depthOld )
		flags = depthOld->GetFlags();

	int iW, iH;
	materials->GetBackBufferDimensions( iW, iH );
	materials->BeginRenderTargetAllocation();
	materials->CreateNamedRenderTargetTextureEx(
			"_rt_ResolvedFullFrameDepth_TFC",
			iW, iH, RT_SIZE_NO_CHANGE,
			IMAGE_FORMAT_RGBA16161616F,
			MATERIAL_RT_DEPTH_NONE,
			flags,
			0);
	materials->EndRenderTargetAllocation();
}

void CTFCViewRender::RenderView( const CViewSetup &view, int nClearFlags, int whatToDraw ) 
{
	BaseClass::RenderView( view, nClearFlags, whatToDraw );
}