#include "cbase.h"
#include "tfc_projectile_nail.h"
#ifdef CLIENT_DLL
#include "tempent.h"
#include "c_te_legacytempents.h"
#endif

#ifdef GAME_DLL
#include "sdk_player.h"
#endif

//IMPLEMENT_NETWORKCLASS(CTFCProjectileNail, DT_CTFCProjectileNail)

/*BEGIN_NETWORK_TABLE(CTFCProjectileNail, DT_CTFCProjectileNail)
END_NETWORK_TABLE()*/

#ifdef GAME_DLL
BEGIN_DATADESC(CTFCProjectileNail)
DEFINE_ENTITYFUNC(ProjectileTouch),
DEFINE_THINKFUNC(ProjectileThink),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS(tf_proj_nail, CTFCProjectileNail);

CTFCProjectileNail::CTFCProjectileNail()
{
}

CTFCProjectileNail::~CTFCProjectileNail()
{
}

void CTFCProjectileNail::Precache(void)
{
	BaseClass::Precache();
}

void CTFCProjectileNail::Spawn()
{
	BaseClass::Spawn();
}

void CTFCProjectileNail::ProjectileThink()
{
	BaseClass::ProjectileThink();
}

void CTFCProjectileNail::ProjectileTouch(CBaseEntity* pOther)
{
	BaseClass::ProjectileTouch(pOther);
}
