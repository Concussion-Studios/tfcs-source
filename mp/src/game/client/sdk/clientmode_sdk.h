//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SDK_CLIENTMODE_H
#define SDK_CLIENTMODE_H
#ifdef _WIN32
#pragma once
#endif

#include "clientmode_shared.h"
#include "sdkviewport.h"

class ClientModeSDKNormal : public ClientModeShared 
{
DECLARE_CLASS( ClientModeSDKNormal, ClientModeShared );

private:

// IClientMode overrides.
public:

					ClientModeSDKNormal();
	virtual			~ClientModeSDKNormal();

	virtual void	Init();
	virtual void	InitViewport();
	virtual void	FireGameEvent( IGameEvent *event );
	virtual void	OverrideView( CViewSetup *pSetup );
	virtual bool	DoPostScreenSpaceEffects( const CViewSetup *pSetup );
	virtual float	GetViewModelFOV( void );
	virtual int		GetDeathMessageStartHeight( void );
	virtual void	PostRenderVGui() {}
	virtual bool	CanRecordDemo( char *errorMsg, int length ) const;
	virtual int		KeyInput( int down, ButtonCode_t keynum, const char* pszCurrentBinding );
};

extern IClientMode *GetClientModeNormal();
extern ClientModeSDKNormal* GetClientModeSDKNormal();

#endif // SDK_CLIENTMODE_H
