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
#include "particle_parse.h"
#include <string>
#include "collisionutils.h"
#include "decals.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sv_portal_placement_never_fail("sv_portal_placement_never_fail", "0", FCVAR_REPLICATED | FCVAR_CHEAT);

class CWeaponPortalgun : public CBaseHLCombatWeapon
{


public:
	DECLARE_CLASS(CWeaponPortalgun, CBaseHLCombatWeapon);
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();

	CWeaponPortalgun(void);

	DECLARE_SERVERCLASS();

	void			Precache();
	void			ItemPreFrame();
	void			ItemBusyFrame();
	void			ItemPostFrame();
	void			PrimaryAttack();
	void			SecondaryAttack();
	void			AddViewKick();
	void			DryFire();
	void			PlacePortal(CBaseEntity*, QAngle, Vector, CBaseEntity*);	//Fires a portal (passed as the first entity) from the given angles and vector. Owner/ignore is last entity.
	bool			CanPlacePortal(Vector, QAngle, CBaseEntity*);	//Checks if a portal fits in the given space. vector is pos, angles are portal angles, entity is portal itself.
	void			InputFirePortal1(inputdata_t &inputData);
	void			InputFirePortal2(inputdata_t &inputData);
	virtual bool	Reload(void);
	bool			IsNoPortalSurface(const csurface_t &);
	void			BadPortalFX(QAngle, Vector, const char*);


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
	bool m_canFirePortal1;
	bool m_canFirePortal2;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponPortalgun, DT_WeaponPortalgun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_portalgun, CWeaponPortalgun);
PRECACHE_WEAPON_REGISTER(weapon_portalgun);

BEGIN_DATADESC(CWeaponPortalgun)

// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_canFirePortal1, FIELD_BOOLEAN, "CanFirePortal1"),
DEFINE_KEYFIELD(m_canFirePortal2, FIELD_BOOLEAN, "CanFirePortal2"),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "FirePortal1", InputFirePortal1),
DEFINE_INPUTFUNC(FIELD_VOID, "FirePortal2", InputFirePortal2),

DEFINE_FIELD(m_flRateOfFire, FIELD_FLOAT),
END_DATADESC()

//Defines all animations
acttable_t	CWeaponPortalgun::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },
	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },
};

IMPLEMENT_ACTTABLE(CWeaponPortalgun);

CWeaponPortalgun::CWeaponPortalgun(void)
{
	m_fMinRange1 = 24;
	m_fMaxRange1 = 3000;
	m_bFiresUnderwater = true;

	if (!m_canFirePortal1 && !m_canFirePortal2){	//if we can't fire portal 1 and can't fire portal 2...
		//that's dumb, enable both portals regardless.
		m_canFirePortal1 = true;
		m_canFirePortal2 = true;
	}
}

//Since we are using the SMG models, we will precache those. This also sets our rate of fire to 10 rounds per second. 
void CWeaponPortalgun::Precache()
{
	m_flRateOfFire = 0.5f;

	PrecacheModel("models/weapons/v_portalgun.mdl", true);
	PrecacheModel("models/weapons/w_portalgun.mdl", true);

	PrecacheParticleSystem("portal_1_badsurface");
	PrecacheParticleSystem("portal_2_badsurface");

	PrecacheParticleSystem("portal_1_badvolume");
	PrecacheParticleSystem("portal_2_badvolume");

	BaseClass::Precache();
}

void CWeaponPortalgun::DryFire()
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//ItemPreFrame is called before the player movement function.
//ItemBusyFrame is called with the player movement function.
//ItemPostFrame is called after the player movement function.
//Out of these three, ItemPostFrame is the most useful.
void CWeaponPortalgun::ItemPreFrame()
{
	BaseClass::ItemPreFrame();
}

void CWeaponPortalgun::ItemBusyFrame()
{
	BaseClass::ItemBusyFrame();
}

void CWeaponPortalgun::ItemPostFrame()
{
	BaseClass::ItemPostFrame();
}

void CWeaponPortalgun::PrimaryAttack()
{
	if (!m_canFirePortal1){	//if set not to fire portal 1...
		return;
	}

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner()); //This gets the current player holding the weapon
	Vector vecSrc = pPlayer->Weapon_ShootPosition();   //This simply just gets the current position of the player.
	//Vector vecAim = pPlayer->GetAutoaimVector(0.0);   //This gets where the player is looking, but also corrected by autoaim.

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

	//Vector vecAbsEnd = vecSrc + (vecSrc * MAX_TRACE_LENGTH);

	CBaseEntity* portal1 = gEntList.FindEntityByName(NULL, "portal1");

	PlacePortal(portal1, pPlayer->EyeAngles(), vecSrc, pPlayer);

	WeaponSound(SINGLE); //This makes our weapon emit the single show sound.
	SendWeaponAnim(ACT_VM_PRIMARYATTACK); //This sends the animation for us shooting.
	m_flNextPrimaryAttack = gpGlobals->curtime + m_flRateOfFire; //This defines when our next attack should be
	AddViewKick(); //Don't forget to add our viewkick
}

void CWeaponPortalgun::SecondaryAttack()
{
	if (!m_canFirePortal2){	//if set not to fire portal 2...
		return;
	}

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner()); //This gets the current player holding the weapon
	Vector vecSrc = pPlayer->Weapon_ShootPosition();   //This simply just gets the current position of the player.
	//Vector vecAim = pPlayer->GetAutoaimVector(0.0);   //This gets where the player is looking, but also corrected by autoaim.

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

	CBaseEntity* portal2 = gEntList.FindEntityByName(NULL, "portal2");

	PlacePortal(portal2, pPlayer->EyeAngles(), vecSrc, pPlayer);

	//Old portal placement code, kept here for reference
	/*
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward); //Assign vecForward a vector facing the EyeAngles

	CBaseEntity* portal2 = gEntList.FindEntityByName(NULL, "portal2");

	trace_t tr; // Create our trace_t class to hold the end result
	UTIL_TraceLine(vecSrc, vecSrc + (vecForward * MAX_TRACE_LENGTH), MASK_SHOT_PORTAL, pPlayer, COLLISION_GROUP_NONE, &tr);

	QAngle angles;
	VectorAngles(tr.plane.normal, angles);	//get the normal of the plane we hit, and convert it to angles

	if (portal2){
		if (&tr.plane != NULL && strcmp(tr.m_pEnt->GetClassname(), "worldspawn") == 0){
			portal2->Teleport(&tr.endpos, &angles, NULL);
		}
		else {
			// Create at world co-ords
			DispatchParticleEffect("portal_2_badsurface", tr.endpos, angles, this);
		}
	}
	else {
		ConWarning("portal 2 not found! Portal gun will not function.");
	}
	*/

	WeaponSound(WPN_DOUBLE); //This makes our weapon emit the single show sound.
	SendWeaponAnim(ACT_VM_PRIMARYATTACK); //This sends the animation for us shooting.
	m_flNextSecondaryAttack = gpGlobals->curtime + m_flRateOfFire; //This defines when our next attack should be
	AddViewKick(); //Don't forget to add our viewkick
}

void CWeaponPortalgun::AddViewKick()
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	QAngle punch;
	punch += QAngle(-0.2, 0.0, 0.0);
	pPlayer->ViewPunch(punch);
}

bool CWeaponPortalgun::Reload()
{
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
		WeaponSound(RELOAD);
	return fRet;
}

void CWeaponPortalgun::PlacePortal(CBaseEntity* portal1, QAngle eyeangles, Vector vecSrc, CBaseEntity* pPlayer)
{
	Vector vecForward;
	AngleVectors(eyeangles, &vecForward); //Assign vecForward a vector facing the EyeAngles

	trace_t tr; // Create our trace_t class to hold the end result
	UTIL_TraceLine(vecSrc, vecSrc + (vecForward * MAX_TRACE_LENGTH), MASK_SHOT_PORTAL, pPlayer, COLLISION_GROUP_NONE, &tr);

	if (!tr.DidHit()){	//if trace did not hit world...
		DevMsg("Trace never hit anything! Can't place portal on nothing!\n");
		return;
	}

	QAngle angles;
	VectorAngles(tr.plane.normal, angles);	//get the normal of the plane we hit, and convert it to angles

	if (portal1){//if portal exists...
		if (&tr.plane != NULL && strcmp(tr.m_pEnt->GetClassname(), "worldspawn") == 0 && !IsNoPortalSurface(tr.surface)){//and the trace hit a static world brush/prop...

			//Do a second trace to ensure we never collide with the floor!
			//This ensures objects going through the portal won't get stuck in floors.
			Vector vecPortalForward, vecPortalRight, vecPortalUp;	//get various direction vectors for our portal
			AngleVectors(angles, &vecPortalForward, &vecPortalRight, &vecPortalUp);

			Vector vecPortalDown;	//make variable for a downward facing vector for portal floor collision
			VectorRotate(vecPortalUp, QAngle(180, 0, 0), vecPortalDown);	//rotate vector to get downward facing portal vector

			Vector vecPortalLeft;	//make variable for a downward facing vector for portal floor collision
			VectorRotate(vecPortalRight, QAngle(180, 0, 0), vecPortalLeft);	//rotate vector to get downward facing portal vector

			trace_t trFloor; // Create our trace_t class to hold the end result

			//Do the actual trace. Trace distance is set to 56, as thats what we want in hammer units; hopefully, they apply here.
			UTIL_TraceLine(tr.endpos, tr.endpos + (vecPortalDown * 56), MASK_SOLID, portal1, COLLISION_GROUP_NONE, &trFloor);

			if (trFloor.DidHit()){//if we did get stuck in the floor...
				DevMsg("Portal 1 stuck in floor! Fixing!\n");

				trace_t trFloorBump; // Create our trace_t class to hold the end result

				//Do the next trace to determine portal bump position. Trace distance is set to 56, as thats what we want in hammer units; hopefully, they apply here.
				UTIL_TraceLine(trFloor.endpos, trFloor.endpos + (vecPortalUp * 56), MASK_SOLID, portal1, COLLISION_GROUP_NONE, &trFloorBump);

				//check if portal fits i new space
				if (CanPlacePortal(trFloorBump.endpos, angles, portal1)){
					DevMsg("Portal fits in bump space! Teleporting!\n");
					portal1->Teleport(&trFloorBump.endpos, &angles, NULL);//Teleport the portal to the new bumped location!
				}
				else {
					DevMsg("Portal does not fit in bump space! Nothing will happen now.\n");
					BadPortalFX(angles, tr.endpos, "portal_1_badvolume");
				}
			} else {
				//When a portal bump is unnecessary, just place the portal as is!

				//check if portal fits i new space
				if (CanPlacePortal(tr.endpos, angles, portal1)){
					DevMsg("Portal fits in space! Teleporting!\n");
					portal1->Teleport(&tr.endpos, &angles, NULL);//Teleport the portal directly to wall where we are facing!
				}
				else {
					DevMsg("Portal does not fit in space! Nothing will happen now.\n");
					BadPortalFX(angles, tr.endpos, "portal_1_badvolume");
				}
			}
		}
		else {
			BadPortalFX(angles, tr.endpos, "portal_1_badsurface");	//we hit a bad surface (metal, etc.)
		}
	}
	else {
		ConWarning("portal 1 not found! Portal gun will not function.\n");
	}
}

bool CWeaponPortalgun::CanPlacePortal(Vector endpos, QAngle angles, CBaseEntity* portal1){
	//Do a second trace to ensure we never collide with the floor!
	//This ensures objects going through the portal won't get stuck in floors.
	Vector vecPortalForward, vecPortalRight, vecPortalUp;	//get various direction vectors for our portal
	AngleVectors(angles, &vecPortalForward, &vecPortalRight, &vecPortalUp);

	Vector vecPortalDown;	//make variable for a downward facing vector for portal floor collision
	VectorRotate(vecPortalUp, QAngle(180, 0, 0), vecPortalDown);	//rotate vector to get downward facing portal vector

	Vector vecPortalLeft;	//make variable for a left facing vector for portal wall collision
	VectorRotate(vecPortalRight, QAngle(180, 0, 0), vecPortalLeft);	//rotate vector to get leftward facing portal vector

	//TESTING FOR THE FLOOR

	trace_t trFloor; // Create our trace_t class to hold the end result

	//Do the actual trace. Trace distance is set to 56, as thats what we want in hammer units; hopefully, they apply here.
	UTIL_TraceLine(endpos, endpos + (vecPortalDown * 56), MASK_SOLID, portal1, COLLISION_GROUP_NONE, &trFloor);

	if (trFloor.DidHit()){//if we did get stuck in the floor...
		return false;	//return false, we did not fit...
	}

	//TESTING FOR THE CEILING

	trace_t trCeiling; // Create our trace_t class to hold the end result

	//Do the actual trace. Trace distance is set to 56, as thats what we want in hammer units; hopefully, they apply here.
	UTIL_TraceLine(endpos, endpos + (vecPortalUp * 56), MASK_SOLID, portal1, COLLISION_GROUP_NONE, &trCeiling);

	if (trCeiling.DidHit()){//if we did get stuck in the floor...
		return false;	//return false, we did not fit...
	}

	//TESTING FOR THE RIGHT WALL

	trace_t trRWall; // Create our trace_t class to hold the end result

	//Do the actual trace. Trace distance is set to 32, as thats what we want in hammer units; hopefully, they apply here.
	UTIL_TraceLine(endpos, endpos + (vecPortalRight * 32), MASK_SOLID, portal1, COLLISION_GROUP_NONE, &trRWall);

	if (trRWall.DidHit()){//if we did get stuck in the floor...
		return false;	//return false, we did not fit...
	}

	//TESTING FOR THE LEFT WALL

	trace_t trLWall; // Create our trace_t class to hold the end result

	//Do the actual trace. Trace distance is set to 32, as thats what we want in hammer units; hopefully, they apply here.
	UTIL_TraceLine(endpos, endpos + (vecPortalLeft * 32), MASK_SOLID, portal1, COLLISION_GROUP_NONE, &trLWall);

	if (trLWall.DidHit()){//if we did get stuck in the floor...
		return false;	//return false, we did not fit...
	}

	return true;
}

//Fires a portal directly from the gun object itself!
//Enables pedestals to function, and mimics the original object!
void CWeaponPortalgun::InputFirePortal1(inputdata_t &inputData){
	CBaseEntity* portal1 = gEntList.FindEntityByName(NULL, "portal1");//get the first portal entity

	QAngle shootAngles = QAngle(this->GetAbsAngles());//get our angles and copy into a new object
	shootAngles.y = shootAngles.y + 180;	//rotate 180 degrees so we shoot from the front of the gun (otherwise would fire from the butt end)

	PlacePortal(portal1, shootAngles, this->GetAbsOrigin(), this);
}

//Fires a portal directly from the gun object itself!
//Enables pedestals to function, and mimics the original object!
void CWeaponPortalgun::InputFirePortal2(inputdata_t &inputData){
	CBaseEntity* portal2 = gEntList.FindEntityByName(NULL, "portal2");//get the first portal entity

	QAngle shootAngles = QAngle(this->GetAbsAngles());//get our angles and copy into a new object
	shootAngles.y = shootAngles.y + 180;	//rotate 180 degrees so we shoot from the front of the gun (otherwise would fire from the butt end)

	PlacePortal(portal2, shootAngles, this->GetAbsOrigin(), this);
}

//Returns true if surface is non portalable
bool CWeaponPortalgun::IsNoPortalSurface(const csurface_t &surface)
{
	if (sv_portal_placement_never_fail.GetBool()){
		return false;	//allow placement on all surface types if convar is set.
	}

	if (surface.flags & SURF_SKY || surface.flags & SURF_SKY2D || surface.flags & SURF_NOPORTAL){
		return true;
	}

	const surfacedata_t *pdata = physprops->GetSurfaceData(surface.surfaceProps);

	if (pdata->game.material == CHAR_TEX_GLASS)
		return true;

	return false;
}

//ConVar	fx_x("fx_x", "0", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar	fx_y("fx_y", "0", FCVAR_REPLICATED | FCVAR_CHEAT);
//ConVar	fx_z("fx_z", "0", FCVAR_REPLICATED | FCVAR_CHEAT);

void CWeaponPortalgun::BadPortalFX(QAngle angles, Vector endpos, const char* effect){
	// Create at world co-ords
	angles.x -= 0; //Rotate effect upwards pitch by 90 degrees, effect for some reason faces down by default!
	angles.y -= 90; //Rotate effect upwards pitch by 90 degrees, effect for some reason faces down by default!
	angles.z -= 90; //Rotate effect upwards pitch by 90 degrees, effect for some reason faces down by default!
	DispatchParticleEffect(effect, endpos, angles, this);
}