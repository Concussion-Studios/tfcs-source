#include "cbase.h"
#include "tfc_projectile_rocket.h"
#ifdef CLIENT_DLL
#include "tempent.h"
#include "c_te_legacytempents.h"
#endif

#ifdef GAME_DLL
#include "sdk_player.h"
#endif

//IMPLEMENT_NETWORKCLASS(CTFCProjectileRocket, DT_TFCProjectileRocket)

/*BEGIN_NETWORK_TABLE(CTFCProjectileRocket, DT_TFCProjectileNail)
END_NETWORK_TABLE()*/

#ifdef GAME_DLL
BEGIN_DATADESC(CTFCProjectileRocket)
DEFINE_ENTITYFUNC(ProjectileTouch),
DEFINE_THINKFUNC(ProjectileThink),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS(tf_proj_rocket, CTFCProjectileRocket);

CTFCProjectileRocket::CTFCProjectileRocket()
{
}

CTFCProjectileRocket::~CTFCProjectileRocket()
{
}

void CTFCProjectileRocket::Precache(void)
{
	BaseClass::Precache();
}

void CTFCProjectileRocket::Spawn()
{
	BaseClass::Spawn();
}

void CTFCProjectileRocket::ProjectileThink()
{
	BaseClass::ProjectileThink();
}

void CTFCProjectileRocket::ProjectileTouch(CBaseEntity* pOther)
{
	BaseClass::ProjectileTouch(pOther);
}
