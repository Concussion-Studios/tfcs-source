//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		ZM:R C_Arms system
//
// $NoKeywords: $
//=============================================================================//
#ifndef TFC_VIEWMODEL_H
#define TFC_VIEWMODEL_H
#ifdef _WIN32
#pragma once
#endif

#include "predicted_viewmodel.h"

#ifdef CLIENT_DLL
#define CTFCViewModel C_TFCViewModel
#endif

class CTFCViewModel : public CPredictedViewModel
{
public:
	DECLARE_CLASS( CTFCViewModel, CPredictedViewModel );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFCViewModel();
	~CTFCViewModel();

#ifdef CLIENT_DLL
	virtual int CalcOverrideModelIndex() OVERRIDE;
	virtual int DrawModel( int flags ) OVERRIDE;
	virtual bool    ShouldReceiveProjectedTextures( int flags ) OVERRIDE;
	virtual C_BaseAnimating*    FindFollowedEntity() OVERRIDE;
#endif

	void SetWeaponModelEx( const char* psTFCodel, CBaseCombatWeapon* pWep, bool bOverriden );

	virtual CBaseCombatWeapon* GetOwningWeapon() OVERRIDE;

#ifndef GAME_DLL
	void SetDrawVM( bool state ) { m_bDrawVM = state; };
#endif

private:

#ifdef CLIENT_DLL
	bool m_bDrawVM; // We have to override this so the client can decide whether to draw it.

	int m_iOverrideModelIndex;
	CBaseCombatWeapon* m_pOverrideModelWeapon;
	CBaseCombatWeapon* m_pLastWeapon;
#endif
};

#endif // TFC_VIEWMODEL_H
