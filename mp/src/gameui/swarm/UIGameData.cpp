#include "cbase.h"
#include "uigamedata.h"
#include "vgenericconfirmation.h"

//setup in GameUI_Interface.cpp
extern const char *COM_GetModDirectory( void );

namespace BaseModUI
{
	CUIGameData* CUIGameData::m_Singleton = 0;
	bool CUIGameData::m_bModuleShutDown = false;

	CUIGameData* CUIGameData::Get()
	{
		if (!m_Singleton && !m_bModuleShutDown)
			m_Singleton = new CUIGameData();

		return m_Singleton;
	}

	void CUIGameData::Shutdown()
	{
		delete m_Singleton;
		m_Singleton = NULL;
		m_bModuleShutDown = true;
	}

	void CUIGameData::OnGameUIPostInit() {}

	void CUIGameData::NeedConnectionProblemWaitScreen()	{}

	void CUIGameData::ShowPasswordUI(char const*pchCurrentPW) {}

	void CUIGameData::RunFrame() {}

	void CUIGameData::RunFrame_Storage() {}

	char const * CUIGameData::GetPlayerName( XUID playerID, char const *szPlayerNameSpeculative )
	{
		static ConVarRef cl_names_debug( "cl_names_debug" );
		if ( cl_names_debug.GetInt() )
			return "WWWWWWWWWWWWWWW";

	#if !defined( NO_STEAM )
		if ( steamapicontext && steamapicontext->SteamUtils() &&
			steamapicontext->SteamFriends() && steamapicontext->SteamUser() )
		{
			int iIndex = m_mapUserXuidToName.Find( playerID );
			if ( iIndex == m_mapUserXuidToName.InvalidIndex() )
			{
				char const *szName = steamapicontext->SteamFriends()->GetFriendPersonaName( playerID );
				if ( szName && *szName )
					iIndex = m_mapUserXuidToName.Insert( playerID, szName );
			}

			if ( iIndex != m_mapUserXuidToName.InvalidIndex() )
				return m_mapUserXuidToName.Element( iIndex ).Get();
		}
	#endif

		return szPlayerNameSpeculative;
	}

	void CUIGameData::RunFrame_Invite()	{}

	void CUIGameData::OnEvent( KeyValues *pEvent )
	{
		char const *szEvent = pEvent->GetName();

		if ( !Q_stricmp( "OnSysXUIEvent", szEvent ) )
		{
			m_bXUIOpen = !Q_stricmp( "opening", pEvent->GetString( "action", "" ) );
		}
		else if ( !Q_stricmp( "OnProfileUnavailable", szEvent ) )
		{
			// Activate game ui to see the dialog
			if ( !CBaseModPanel::GetSingleton().IsVisible() )
				engine->ExecuteClientCmd( "gameui_activate" );
		}
	}
}
