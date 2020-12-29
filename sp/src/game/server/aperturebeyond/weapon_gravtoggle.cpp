#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"
#include "prop_combine_ball.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponGravToggle : public CBaseHLCombatWeapon
{


public:
	DECLARE_CLASS(CWeaponGravToggle, CBaseHLCombatWeapon);
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();

	CWeaponGravToggle(void);

	DECLARE_SERVERCLASS();

	void			Precache();
	void			ItemPreFrame();
	void			ItemBusyFrame();
	void			ItemPostFrame();
	void			PrimaryAttack();
	void			SecondaryAttack();
	void			AddViewKick();
	void			DryFire();
	virtual bool	Reload(void);


	virtual const Vector& GetBulletSpread()
	{
		static Vector cone = VECTOR_CONE_1DEGREES, npcCone = VECTOR_CONE_1DEGREES;
		if (GetOwner() && GetOwner()->IsNPC()) //Always handle NPCs first
			return npcCone;
		else
			return cone;
	}

private:
	float m_flRateOfFire;
};


//Defines all animations
acttable_t	CWeaponGravToggle::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },
	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },
};

IMPLEMENT_ACTTABLE(CWeaponGravToggle);

PRECACHE_WEAPON_REGISTER(weapon_gravtoggle);

IMPLEMENT_SERVERCLASS_ST(CWeaponGravToggle, DT_WeaponGravToggle)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_gravtoggle, CWeaponGravToggle);

BEGIN_DATADESC(CWeaponGravToggle)
DEFINE_FIELD(m_flRateOfFire, FIELD_FLOAT),
END_DATADESC()

CWeaponGravToggle::CWeaponGravToggle(void)
{
	m_fMinRange1 = 24;
	m_fMaxRange1 = 3000;
	m_bFiresUnderwater = true;
}

//Since we are using the SMG models, we will precache those. This also sets our rate of fire to 10 rounds per second. 
void CWeaponGravToggle::Precache()
{
	m_flRateOfFire = 0.1f;

	PrecacheModel("models/weapons/v_smg1.mdl", true);
	PrecacheModel("models/weapons/w_smg1.mdl", true);
	BaseClass::Precache();


	PrecacheScriptSound("Weapon_Portalgun.fire_blue");
	PrecacheScriptSound("Weapon_Portalgun.fire_red");
	PrecacheScriptSound("Weapon_Portalgun.powerup");
	PrecacheScriptSound("Weapon_Portalgun.HoldSound");
	PrecacheScriptSound("Portal.fizzle_invalid_surface");
}

void CWeaponGravToggle::DryFire()
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//ItemPreFrame is called before the player movement function.
//ItemBusyFrame is called with the player movement function.
//ItemPostFrame is called after the player movement function.
//Out of these three, ItemPostFrame is the most useful.
void CWeaponGravToggle::ItemPreFrame()
{
	BaseClass::ItemPreFrame();
}

void CWeaponGravToggle::ItemBusyFrame()
{
	BaseClass::ItemBusyFrame();
}

void CWeaponGravToggle::ItemPostFrame()
{
	BaseClass::ItemPostFrame();
}

void CWeaponGravToggle::PrimaryAttack()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner()); //This gets the current player holding the weapon
	Vector vecSrc = pPlayer->Weapon_ShootPosition();   //This simply just gets the current position of the player.
	Vector vecAim = pPlayer->GetAutoaimVector(0.0);   //This gets where the player is looking, but also corrected by autoaim.
	//pPlayer->FireBullets(1, vecSrc, vecAim, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, entindex(), 0, 30, GetOwner(), true, true);
	//This is a lengthy one. All of the args in here are data for our bullet. We could simply use
	//BaseClass::PrimaryAttack, but this gives us no control over how our weapon is fired.
	//So instead, we use this and give it all our info for our bullet.
	//The next 2 are just where the bullet is being fired from and where it should go.
	//Next is how far the bullet is fired, and after that is what type of ammo we use.
	//After this is the tracer freq, which really doesnt matter.
	//Next is the id of the entity firing the weapon, and the attachment id. These 2 dont really matter.
	//Next is how much damage each bullet should do, which is 30.
	//Next is what entity is firing the bullet, again.
	//The final 2 define wether our first shot should be accurate, and wether this is the primary attack.
	WeaponSound(SINGLE); //This makes our weapon emit the single show sound.
	SendWeaponAnim(ACT_VM_PRIMARYATTACK); //This sends the animation for us shooting.
	m_flNextPrimaryAttack = gpGlobals->curtime + m_flRateOfFire; //This defines when our next attack should be
	AddViewKick(); //Don't forget to add our viewkick

	ConVar *pGravity = cvar->FindVar("sv_gravity");	//get the server gravity variable
	pGravity->SetValue(0);	//set the server gravity value to 0
}

void CWeaponGravToggle::SecondaryAttack()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner()); //This gets the current player holding the weapon
	Vector vecSrc = pPlayer->Weapon_ShootPosition();   //This simply just gets the current position of the player.
	Vector vecAim = pPlayer->GetAutoaimVector(0.0);   //This gets where the player is looking, but also corrected by autoaim.
	//pPlayer->FireBullets(1, vecSrc, vecAim, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 1, entindex(), 0, 30, GetOwner(), true, true);
	//This is a lengthy one. All of the args in here are data for our bullet. We could simply use
	//BaseClass::PrimaryAttack, but this gives us no control over how our weapon is fired.
	//So instead, we use this and give it all our info for our bullet.
	//The next 2 are just where the bullet is being fired from and where it should go.
	//Next is how far the bullet is fired, and after that is what type of ammo we use.
	//After this is the tracer freq, which really doesnt matter.
	//Next is the id of the entity firing the weapon, and the attachment id. These 2 dont really matter.
	//Next is how much damage each bullet should do, which is 30.
	//Next is what entity is firing the bullet, again.
	//The final 2 define wether our first shot should be accurate, and wether this is the primary attack.
	WeaponSound(SINGLE); //This makes our weapon emit the single show sound.
	SendWeaponAnim(ACT_VM_PRIMARYATTACK); //This sends the animation for us shooting.
	m_flNextPrimaryAttack = gpGlobals->curtime + m_flRateOfFire; //This defines when our next attack should be
	AddViewKick(); //Don't forget to add our viewkick

	ConVar *pGravity = cvar->FindVar("sv_gravity");	//get the server gravity variable
	pGravity->SetValue(600);	//set the server gravity value to 0
}

void CWeaponGravToggle::AddViewKick()
{
	return;
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	QAngle punch;
	punch += QAngle(-0.2, 0.0, 0.0);
	pPlayer->ViewPunch(punch);
}

bool CWeaponGravToggle::Reload()
{
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
		WeaponSound(RELOAD);
	return fRet;
}