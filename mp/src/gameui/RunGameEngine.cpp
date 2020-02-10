//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//
#include "IRunGameEngine.h"
#include "EngineInterface.h"
#include "tier1/strtools.h"
#include "IGameUIFuncs.h"
#include "tier1/convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Purpose: Interface to running the engine from the UI dlls
//-----------------------------------------------------------------------------
class CRunGameEngine : public IRunGameEngine
{
public:
	// Returns true if the engine is running, false otherwise.
	virtual bool IsRunning()
	{
		return true;
	}

	// Adds text to the engine command buffer. Only works if IsRunning()
	// returns true on success, false on failure
	virtual bool AddTextCommand(const char *text)
	{
		engine->ClientCmd_Unrestricted((char *)text);
		return true;
	}

	// runs the engine with the specified command line parameters.  Only works if !IsRunning()
	// returns true on success, false on failure
	virtual bool RunEngine(const char *gameName, const char *commandLineParams)
	{
		return false;
	}

	virtual bool RunEngine2(const char *gameDir, const char *commandLineParams, bool isSourceGame)
	{
		return false;
	}

	virtual ERunResult RunEngine( int iAppID, const char *gameDir, const char *commandLineParams )
	{
		return k_ERunResultOkay;
	}
	
	// returns true if the player is currently connected to a game server
	virtual bool IsInGame()
	{
		return engine->GetLevelName() && strlen(engine->GetLevelName()) > 0;
	}

	// gets information about the server the engine is currently connected to
	// returns true on success, false on failure
	virtual bool GetGameInfo(char *infoBuffer, int bufferSize)
	{
		//!! need to implement
		return false;
	}

	virtual void SetTrackerUserID(int trackerID, const char *trackerName)
	{
		gameuifuncs->SetFriendsID(trackerID, trackerName);

		// update the player's name if necessary
		ConVarRef name( "name" );
		if ( name.IsValid() && trackerName && *trackerName && !Q_strcmp( name.GetString(), "unnamed" ) )
		{
			name.SetValue(trackerName);
		}
	}

	// iterates users
	// returns the number of user
	virtual int GetPlayerCount()
	{
		return engine->GetMaxClients();
	}

	// returns a playerID for a player
	// playerIndex is in the range [0, GetPlayerCount)
	virtual unsigned int GetPlayerFriendsID(int playerIndex)
	{
		player_info_t pi;

		if  ( engine->GetPlayerInfo(playerIndex, &pi ) )
			return pi.friendsID;

		return 0;
	}

	// gets the in-game name of another user, returns NULL if that user doesn't exists
	virtual const char *GetPlayerName(int trackerID)
	{
		// find the player by their friendsID
		static player_info_t pi;
		for (int i = 0; i < engine->GetMaxClients(); i++)
		{
			if  (engine->GetPlayerInfo(i, &pi ))
			{
				if (pi.friendsID == (uint)trackerID)
				{
					return pi.name;
				}
			}
		}

		return NULL;
	}

	virtual const char *GetPlayerFriendsName(int trackerID)
	{
		// find the player by their friendsID
		static player_info_t pi;
		for (int i = 0; i < engine->GetMaxClients(); i++)
		{
			if  (engine->GetPlayerInfo(i, &pi ))
			{
				if (pi.friendsID == (uint)trackerID)
				{
					return pi.friendsName;
				}
			}
		}

		return NULL;
	}

	// return the build number of the engine
	virtual unsigned int GetEngineBuildNumber()
	{
		return engine->GetEngineBuildNumber();
	}

	// return the product version of the mod being played (comes from steam.inf)
	virtual const char *GetProductVersionString()
	{
		return engine->GetProductVersionString();
	}
};

EXPOSE_SINGLE_INTERFACE(CRunGameEngine, IRunGameEngine, RUNGAMEENGINE_INTERFACE_VERSION);
