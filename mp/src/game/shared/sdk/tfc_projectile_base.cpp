#include "cbase.h"
#include "tfc_projectile_base.h"
#ifdef CLIENT_DLL
#include "tempent.h"
#include "c_te_legacytempents.h"
#endif

#ifdef GAME_DLL
#include "sdk_player.h"
#endif

//IMPLEMENT_NETWORKCLASS(CTFCProjectileBase, DT_CTFCProjectileBase)

/*BEGIN_NETWORK_TABLE(CTFCProjectileBase, DT_CTFCProjectileBase)
END_NETWORK_TABLE()*/

#ifdef GAME_DLL
BEGIN_DATADESC(CTFCProjectileBase)
DEFINE_ENTITYFUNC(ProjectileTouch),
DEFINE_THINKFUNC(ProjectileThink),
END_DATADESC()
#endif

CTFCProjectileBase::CTFCProjectileBase()
{
}

CTFCProjectileBase::~CTFCProjectileBase()
{
}

void CTFCProjectileBase::Precache(void)
{
#ifdef GAME_DLL
	PrecacheModel(GetProjectileModelName());
#endif
	BaseClass::Precache();
}

void CTFCProjectileBase::Spawn()
{
	Precache();
	SetModel(GetProjectileModelName());

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	AddEFlags(EFL_NO_WATER_VELOCITY_CHANGE);

	SetGravity(GetGravity());
	m_takedamage = DAMAGE_NO;
	flDamage = 10.0f;
	SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
	SetTouch(&CTFCProjectileBase::ProjectileTouch);
	SetThink(&CTFCProjectileBase::ProjectileThink);
	SetNextThink(gpGlobals->curtime);

}

void CTFCProjectileBase::ProjectileThink()
{
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CTFCProjectileBase::ProjectileTouch(CBaseEntity* pOther)
{
	Assert(pOther);

	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;

	if (!pOther->IsNPC() && !pOther->IsPlayer())
	{
		const trace_t* pTrace = &CBaseEntity::GetTouchTrace();
		trace_t* pNewTrace = const_cast<trace_t*>(pTrace);
		UTIL_ImpactTrace(pNewTrace, DMG_BULLET);
#ifdef CLIENT_DLL
		model_t* pModel = (model_t*)engine->LoadModel("models/weapons/w_models/w_nail.mdl");
		C_LocalTempEntity *m_pWorldModel = tempents->SpawnTempModel(pModel, GetAbsOrigin(), GetAbsAngles(), Vector(0, 0, 0), 30.0f, FTENT_NONE);
		m_pWorldModel->SetParent(pOther);
		Msg("baseproj impact");

#endif
#ifdef GAME_DLL
		CTakeDamageInfo info;
		info.SetAttacker(GetOwnerEntity());
		info.SetDamage(flDamage);
		info.SetDamagePosition(GetAbsOrigin());
		pOther->TakeDamage(info);
		UTIL_Remove(this);
#endif
	}
	else
	{
#ifdef GAME_DLL
		Vector vecDir;
		AngleVectors(GetAbsAngles(), &vecDir);
		CTakeDamageInfo info;
		info.SetAttacker(GetOwnerEntity());
		info.SetDamage(flDamage);
		info.SetDamagePosition(GetAbsOrigin());
		pOther->TakeDamage(info);
		int m_iBloodColor = pOther->BloodColor();
		SpawnBlood(GetAbsOrigin(), vecDir, m_iBloodColor, 5);
		UTIL_Remove(this);
#endif
	}
}

#ifdef GAME_DLL
CTFCProjectileBase *CTFCProjectileBase::Create(const char* pszClassname, const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity* pOwner, const Vector& vecVelocity, float flDamage)
{
	CTFCProjectileBase* pProj = static_cast<CTFCProjectileBase*>(CreateEntityByName(pszClassname));
	Assert(pProj);
	pProj->SetOwnerEntity(pOwner);
	pProj->Spawn();
	pProj->SetAbsVelocity(vecVelocity);
	pProj->SetAbsAngles(vecAngles);
	pProj->SetAbsOrigin(vecOrigin);
	pProj->ChangeTeam(pOwner->GetTeamNumber());
	pProj->flDamage = flDamage;
	return pProj;
}
#endif