//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef SDK_PLAYERCLASS_INFO_PARSE_H
#define SDK_PLAYERCLASS_INFO_PARSE_H
#ifdef _WIN32
#pragma once
#endif

#include "playerclass_info_parse.h"
#include "networkvar.h"

//--------------------------------------------------------------------------------------------------------
class CSDKPlayerClassInfo : public FilePlayerClassInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT( CSDKPlayerClassInfo, FilePlayerClassInfo_t );
	
	CSDKPlayerClassInfo();
	
	virtual void Parse( ::KeyValues *pKeyValuesData, const char *szWeaponName );

	char m_szLimitCvar[64];	//which cvar controls the class limit for this class

	float m_flMaxSpeed;
	int m_iMaxArmor;
	int m_iMaxHealth;
	int m_iSpawnArmor;
	float m_flArmorClass;
	int m_aMaxAmmo[AMMO_LAST];
	int m_aSpawnAmmo[AMMO_LAST];

	//Weapons
	int m_iWeapon1;
	int m_iWeapon2;
	int m_iWeapon3;
	int m_iWeapon4;
	int m_iWeapon5;
	int m_iWeapon6;
	int m_iWeapon7;
	int m_iWeapon8;
	int m_iWeapon9;
	int m_iWeapon10;

	// Grenades
	int m_iNumGrensType1;
	int m_iGrenType1;
	int m_iNumGrensType2;
	int m_iGrenType2;
};

#endif // SDK_PLAYERCLASS_INFO_PARSE_H
