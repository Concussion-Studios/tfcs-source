#ifndef TFC_PROJECTILE_BASE_H
#define TFC_PROJECTILE_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "baseprojectile.h"

//
// projectile base (some generic projectile to derive off)
//
class CTFCProjectileBase : public CBaseProjectile
{
public:
	DECLARE_CLASS(CTFCProjectileBase, CBaseProjectile);
	//DECLARE_NETWORKCLASS();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif



	CTFCProjectileBase();
	~CTFCProjectileBase();

	void Precache(void);
	void Spawn(void);
#ifdef GAME_DLL
	static CTFCProjectileBase* Create(const char* pszClassname, const Vector& vecOrigin,
		const QAngle& vecAngles, CBaseEntity* pOwner, const Vector& vecVelocity, float flDamage);
#endif
	virtual void ProjectileTouch(CBaseEntity* pOther);
	virtual void ProjectileThink(void);
	virtual const char* GetProjectileModelName(void) { return ""; }
	float flDamage;
};

#endif //TFC_PROJECTILE_BASE_H