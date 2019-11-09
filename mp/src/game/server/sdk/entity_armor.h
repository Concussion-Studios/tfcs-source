//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======//
//
// Purpose: TFC Armor.
//
//=============================================================================//
#ifndef ENTITY_ARMOR_H
#define ENTITY_ARMOR_H
#ifdef _WIN32
#pragma once
#endif

#include "items.h"

#define ARMOR_MODEL			"models/items/car_battery01.mdl"
#define ARMOR_PICKUP_SOUND	"Armor.Touch"
#define ARMOR_CAPACITY		50

class CEntityArmor : public CItem
{
public:
	DECLARE_CLASS( CEntityArmor, CItem );

	void Spawn( void );
	void Precache( void );
	bool MyTouch( CBasePlayer *pPlayer );
};

#endif // ENTITY_ARMOR_H


