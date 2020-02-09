//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef __UIGAMEDATA_H__
#define __UIGAMEDATA_H__

#include "vgui_controls/Panel.h"
#include "vgui_controls/Frame.h"
#include "vgui_controls/Button.h"
#include "tier1/utllinkedlist.h"
#include "tier1/utlmap.h"
#include "tier1/KeyValues.h"
#include "tier1/fmtstr.h"

#ifndef _X360
#include "steam/steam_api.h"
#endif // _X360

#include "ixboxsystem.h"

#include "basemodpanel.h"
#include "basemodframe.h"

#include "EngineInterface.h"

namespace BaseModUI
{
	class CUIGameData
	{
		static CUIGameData* m_Singleton;
		static bool m_bModuleShutDown;

	public:
		static CUIGameData* Get();
		static void Shutdown();

		void RunFrame();
		void RunFrame_Storage();
		void RunFrame_Invite();

		void OnEvent( KeyValues *pEvent );

		void OnGameUIPostInit();
		void NeedConnectionProblemWaitScreen();
		void ShowPasswordUI(char const*pchCurrentPW);

		vgui::IImage * GetAvatarImage( XUID playerID );
		char const * GetPlayerName( XUID playerID, char const *szPlayerNameSpeculative );

		bool IsXUIOpen(){return m_bXUIOpen;};

	protected:
		
		CUtlMap< XUID, CUtlString > m_mapUserXuidToName;

		//XUI info
		bool m_bXUIOpen;
	};
}

#endif // __UIGAMEDATA_H__
