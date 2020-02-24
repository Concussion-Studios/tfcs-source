//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		ZM:R C_Arms system
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "tfc_viewmodel.h"
#ifdef CLIENT_DLL
	#include "c_sdk_player.h"
#else
	#include "sdk_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS( tfc_viewmodel, CTFCViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( TFCViewModel, DT_TFC_ViewModel )

BEGIN_NETWORK_TABLE( CTFCViewModel, DT_TFC_ViewModel )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( C_TFCViewModel )
END_PREDICTION_DATA()

CTFCViewModel::CTFCViewModel()
{
#ifdef CLIENT_DLL
	m_bDrawVM = true;
	m_iOverrideModelIndex = -1;
	m_pOverrideModelWeapon = nullptr;
	m_pLastWeapon = nullptr;
#endif
}

CTFCViewModel::~CTFCViewModel()
{
}

CBaseCombatWeapon* CTFCViewModel::GetOwningWeapon()
{
	auto* pOwner = BaseClass::GetOwningWeapon();
	if ( pOwner )
		return pOwner;
 
	if ( ViewModelIndex() == VMINDEX_HANDS )
	{
		CSDKPlayer* pPlayer = ToSDKPlayer( GetOwner() );
		if ( pPlayer )
		{
			CBaseViewModel* vm = pPlayer->GetViewModel( VMINDEX_WEP, false );

			// Apparently this is possible...
			// ???
			if ( vm && vm->ViewModelIndex() == VMINDEX_WEP )
				return vm->GetOwningWeapon();
		}
	}

	return nullptr;
}

void CTFCViewModel::SetWeaponModelEx( const char* psTFCodel, CBaseCombatWeapon* pWep, bool bOverriden )
{
#ifdef CLIENT_DLL
	// Set override model
	auto* pCurWeapon = GetWeapon();

	int newIndex = modelinfo->GetModelIndex( psTFCodel );

	if ( pWep != pCurWeapon )
	{
		m_iOverrideModelIndex = bOverriden ? newIndex : -1;
		m_pOverrideModelWeapon = pWep;

		m_pLastWeapon = pCurWeapon;
	}

#endif

	SetWeaponModel( psTFCodel, pWep );
}

#ifdef CLIENT_DLL
int C_TFCViewModel::CalcOverrideModelIndex()
{
	if ( m_iOverrideModelIndex != -1 )
	{
		// HACK: Check if we changed weapons.
		auto* pCurWeapon = GetWeapon();
		if ( pCurWeapon != m_pOverrideModelWeapon && pCurWeapon != m_pLastWeapon )
		{
			// Stop overriding.
			m_iOverrideModelIndex = -1;
			m_pOverrideModelWeapon = nullptr;
		}
	}


	return m_iOverrideModelIndex;
}

int C_TFCViewModel::DrawModel( int flags )
{
	if ( m_bDrawVM )
		return BaseClass::DrawModel( flags );

	return 0;
}

bool C_TFCViewModel::ShouldReceiveProjectedTextures( int flags )
{
	//
	// IMPORTANT: There's a common crash is caused by things that return true in ShouldReceiveProjectedTextures
	// I've not been able to debug it, so fuck it.
	//
	return false;
}

C_BaseAnimating* C_TFCViewModel::FindFollowedEntity()
{
	if ( ViewModelIndex() == VMINDEX_HANDS )
	{
		C_SDKPlayer* pPlayer = ToSDKPlayer( GetOwner() );
		if ( pPlayer )
		{
			C_BaseViewModel* vm = pPlayer->GetViewModel( VMINDEX_WEP );
			if ( vm )
				return vm;
		}
	}

	return C_BaseAnimating::FindFollowedEntity();
}
#endif
