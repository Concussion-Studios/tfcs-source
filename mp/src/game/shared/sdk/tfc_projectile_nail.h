#ifndef TFC_PROJECTILE_NAIL_H
#define TFC_PROJECTILE_NAIL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "baseprojectile.h"
#include "tfc_projectile_base.h"

//
// projectile base (some generic projectile to derive off)
//
class CTFCProjectileNail : public CTFCProjectileBase
{
public:
	DECLARE_CLASS(CTFCProjectileNail, CTFCProjectileBase);
	//DECLARE_NETWORKCLASS();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif



	CTFCProjectileNail();
	~CTFCProjectileNail();

	void Precache(void);
	void Spawn(void);
	virtual void ProjectileTouch(CBaseEntity* pOther);
	virtual void ProjectileThink(void);
	virtual const char* GetProjectileModelName(void) { return "models/weapons/w_models/w_nail.mdl"; }
	float flDamage;
};

#endif //TFC_PROJECTILE_NAIL_H