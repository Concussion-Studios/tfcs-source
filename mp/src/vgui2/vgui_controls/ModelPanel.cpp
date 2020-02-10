//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//


#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IScheme.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/ISurface.h>
#include <vgui/IImage.h>
#include <vgui_controls/Label.h>
#include "ModelPanel.h"

#include "materialsystem/imaterialsystem.h"
#include "engine/ivmodelinfo.h"


using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModelPanel::CModelPanel( vgui::Panel *pParent, const char *pName ) : vgui::EditablePanel( pParent, pName )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModelPanel::~CModelPanel()
{
	if ( m_pModelInfo )
	{
		delete m_pModelInfo;
		m_pModelInfo = NULL;
	}
}