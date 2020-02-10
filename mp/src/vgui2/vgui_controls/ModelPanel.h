//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASEMODELPANEL_H
#define BASEMODELPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>
#include "KeyValues.h"

namespace vgui
{
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CModelPanelModelInfo
{
public:
	CModelPanelModelInfo()
	{
		m_vecAbsAngles.Init();
		m_vecOriginOffset.Init();
		m_vecFramedOriginOffset.Init();
	}

public:
	Vector		m_vecAbsAngles;
	Vector		m_vecOriginOffset;
	Vector2D	m_vecViewportOffset;
	Vector		m_vecFramedOriginOffset;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CModelPanel : public EditablePanel
{
public:
	DECLARE_CLASS_SIMPLE( CModelPanel, EditablePanel );

	CModelPanel( Panel *parent, const char *name );
	virtual ~CModelPanel();

	CModelPanelModelInfo			*m_pModelInfo;
};

}
#endif // BASEMODELPANEL_H
