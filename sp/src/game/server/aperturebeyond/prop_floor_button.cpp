#include "cbase.h"
#include "props.h"
#include <string>
#include "prop_weighted_cube.h"
#include "filters.h"

//NEW
class CFloorButton : public CDynamicProp
{
public:
	DECLARE_CLASS(CFloorButton, CDynamicProp);

	void Spawn();
	void Activate();
	virtual void Think();
	virtual bool CanPressButton(CBaseEntity* ent);
	void HandleAnimEvent(animevent_t *pEvent);
	bool HasAnimEvent(int nSequence, int nEvent);

	CHandle<class CBaseFilter>	m_hFilter;

	int m_nBoxDetectSphere;
	bool m_ActivatesCubes;
	bool spherehitbox;

private:
	void Precache(void);
	virtual const char *GetUpAnimName();
	virtual const char *GetUpMoveAnimName();
	virtual const char *GetDownMoveAnimName();
	virtual const char *GetDownIdleAnimName();

	bool m_isPressed;
	bool m_isSilent;
	string_t	m_iFilterName;

	COutputEvent m_OnPressed;
	COutputEvent m_OnUnpressed;

	DECLARE_DATADESC();
};

BEGIN_DATADESC(CFloorButton)

DEFINE_FIELD(m_isPressed, FIELD_BOOLEAN),

DEFINE_KEYFIELD(m_isSilent, FIELD_BOOLEAN, "SuppressAnimSounds"),
DEFINE_KEYFIELD(m_iFilterName, FIELD_STRING, "filtername"),

// Declare our think function
DEFINE_THINKFUNC(Think),

DEFINE_OUTPUT(m_OnPressed, "OnPressed"),
DEFINE_OUTPUT(m_OnUnpressed, "OnUnpressed"),

END_DATADESC()


LINK_ENTITY_TO_CLASS(prop_floor_button, CFloorButton);

#define FLOOR_BUTTON_MODEL_NAME "models/props/portal_button.mdl"

const char *CFloorButton::GetUpAnimName(){
	if (V_strcmp(this->GetClassname(), "prop_floor_button")){
		return "idle";
	}
	
	return "BindPose";
}

const char *CFloorButton::GetUpMoveAnimName(){
	return "up";
}

const char *CFloorButton::GetDownMoveAnimName(){
	return "down";
}

const char *CFloorButton::GetDownIdleAnimName(){
	return "idledown";
}

void CFloorButton::Precache(void)
{
	const char *pModelName = STRING(GetModelName());
	pModelName = (pModelName && pModelName[0] != '\0') ? pModelName : FLOOR_BUTTON_MODEL_NAME;
	PrecacheModel(pModelName);

	PrecacheScriptSound("Portal.button_down");
	PrecacheScriptSound("Portal.button_up");
	PrecacheScriptSound("Portal.ButtonDepress");
	PrecacheScriptSound("Portal.ButtonRelease");

}

void CFloorButton::Spawn()
{
	Precache();

	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_VPHYSICS);
	//CreateVPhysics();

	const char *pModelName = STRING(GetModelName());
	SetModel((pModelName && pModelName[0] != '\0') ? pModelName : FLOOR_BUTTON_MODEL_NAME);
	//AddEffects(EF_NOSHADOW);

	ResetSequence(LookupSequence(GetUpAnimName()));

	CreateVPhysics();

	m_isPressed = false;

	m_nBoxDetectSphere = 256;

	m_ActivatesCubes = true;

	spherehitbox = true;

	SetThink(&CFloorButton::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

//------------------------------------------------------------------------------
// Activate
//------------------------------------------------------------------------------
void CFloorButton::Activate(void)
{
	// Get a handle to my filter entity if there is one
	if (m_iFilterName != NULL_STRING)
	{
		m_hFilter = dynamic_cast<CBaseFilter *>(gEntList.FindEntityByName(NULL, m_iFilterName));
	}

	BaseClass::Activate();
}

ConVar	button_radius("button_radius", "15.0", FCVAR_REPLICATED | FCVAR_CHEAT);

void CFloorButton::Think(){
	BaseClass::AnimThink();

	//What this does, is look through entities in a sphere, and then
	//checks to see if they are valid, and if they are
	//adds them to a second list of valid entities.
	//Create an array of CBaseEntity pointers
	CBaseEntity *ppEnts[256];

	//Get the center of the object
	Vector vecCenter = WorldSpaceCenter();

	float flRadius = button_radius.GetFloat();
	//vecCenter.z -= flRadius * 0.8f;

	int nEntCount;

	if (spherehitbox){
		nEntCount = UTIL_EntitiesInSphere(ppEnts, m_nBoxDetectSphere, vecCenter, flRadius, 0);
	}
	else {
		//Do a second trace to ensure we never collide with the floor!
		//This ensures objects going through the portal won't get stuck in floors.
		Vector vecForward, vecRight, vecUp;	//get various direction vectors for our portal
		AngleVectors(GetAbsAngles(), &vecForward, &vecRight, &vecUp);

		Vector vecDown;	//make variable for a downward facing vector for portal floor collision
		VectorRotate(vecUp, QAngle(180, 0, 0), vecDown);	//rotate vector to get downward facing portal vector

		Vector vecLeft;	//make variable for a downward facing vector for portal floor collision
		VectorRotate(vecRight, QAngle(180, 0, 0), vecLeft);	//rotate vector to get downward facing portal vector

		Vector vecBack;	//make variable for a downward facing vector for portal floor collision
		VectorRotate(vecForward, QAngle(180, 0, 0), vecBack);	//rotate vector to get downward facing portal vector

		Vector rangelow(40, 40, 0);
		Vector rangehigh(40, 40, 20);

		nEntCount = UTIL_EntitiesInBox(ppEnts, 256, GetAbsOrigin() - rangelow, GetAbsOrigin() + rangehigh, 0);
	}

	CBaseEntity *ppCandidates[256];
	int nCandidateCount = 0;
	int i;
	for (i = 0; i < nEntCount; i++)
	{
		//Look through the entities it found
		if (ppEnts[i] == NULL)
			continue;

		if (ppEnts[i] == this)
			continue;

		CBaseEntity* ent = ppEnts[i];

		// Check if given entity can press potentially this button
		if (!this->CanPressButton(ent))
			continue;

		//Check if the activating object is a prop_weighted_cube
		CPortalCube* cube = dynamic_cast<CPortalCube*>(ent);
		if (cube && m_ActivatesCubes) {
			cube->ActivateCube();	//use cubes "on" skin
			cube->SetActiveCube();	//set cooldown to reset skin
		}

		//Add them to the new, valid only, Canidate list.
		ppCandidates[nCandidateCount++] = ppEnts[i];

		//DevMsg(ppEnts[i]->GetClassname());
		//DevMsg("\n");
	}

	if (nCandidateCount == 0){
		//DevMsg("button not pressed\n");
		if (!m_isPressed){
			PropSetAnim(GetUpMoveAnimName());
			m_iszDefaultAnim = castable_string_t(GetUpAnimName());
			m_nSkin = 0;
			m_isPressed = true;
			m_OnUnpressed.FireOutput(NULL, this);
		}
	}
	else {
		//DevMsg("button pressed\n");
		if (m_isPressed){
			PropSetAnim(GetDownMoveAnimName());
			m_iszDefaultAnim = castable_string_t(GetDownIdleAnimName());
			m_nSkin = 1;
			m_isPressed = false;
			m_OnPressed.FireOutput(NULL, this);
		}
	}

	// Think at 20Hz
	SetNextThink(gpGlobals->curtime + 0.05f);
}

bool CFloorButton::CanPressButton(CBaseEntity* ent){
	if (!ent->IsSolid())
		return	false;

	//get our filter
	CBaseFilter *pFilter = m_hFilter.Get();

	//if filter is not null...
	if (pFilter){
		return pFilter->PassesFilter(this, ent);	//use the filter for all checks instead of the built in check.
	}

	//Check if the activating object is a prop_weighted_cube
	CPortalCube* cube = dynamic_cast<CPortalCube*>(ent);

	if (ent->IsPlayer() || cube){	//if entity is a player or is a cube
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFloorButton::HandleAnimEvent(animevent_t *pEvent){
	//if (!m_isSilent)
	return;

	BaseClass::HandleAnimEvent(pEvent);
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the given sequence has the anim event, false if not.
// Input  : nSequence - sequence number to check
//			nEvent - anim event number to look for
//-----------------------------------------------------------------------------
bool CFloorButton::HasAnimEvent(int nSequence, int nEvent)
{
	return false;
	if (!m_isSilent){
		return BaseClass::HasAnimEvent(nSequence, nEvent);
	}
	else {
		return false;	//pretend we have no anim events.
	}
}


//NEW
class CFloorButtonBox : public CFloorButton
{
public:
	DECLARE_CLASS(CFloorButtonBox, CFloorButton);

	void Spawn();
	bool CanPressButton(CBaseEntity* ent);

private:

	DECLARE_DATADESC();
};

BEGIN_DATADESC(CFloorButtonBox)

END_DATADESC()


LINK_ENTITY_TO_CLASS(prop_floor_cube_button, CFloorButtonBox);

#define FLOOR_BUTTON_BOX_MODEL_NAME "models/props/box_socket.mdl"

void CFloorButtonBox::Spawn()
{
	SetModelName(castable_string_t(FLOOR_BUTTON_BOX_MODEL_NAME));

	BaseClass::Spawn();
}

bool CFloorButtonBox::CanPressButton(CBaseEntity* ent){
	if (!ent->IsSolid())
		return	false;

	//get our filter
	CBaseFilter *pFilter = m_hFilter.Get();

	//if filter is not null...
	if (pFilter){
		return pFilter->PassesFilter(this, ent);	//use the filter for all checks instead of the built in check.
	}

	//Check if the activating object is a prop_weighted_cube
	CPortalCube* cube = dynamic_cast<CPortalCube*>(ent);

	if (cube && cube->IsCubic()){	//if entity is a cube, and not a sphere type cube
		return true;
	}

	return false;
}

//NEW
class CFloorButtonBall : public CFloorButton
{
public:
	DECLARE_CLASS(CFloorButtonBall, CFloorButton);

	void Spawn();
	bool CanPressButton(CBaseEntity* ent);

private:
	virtual const char *GetUpAnimName();
	virtual const char *GetUpMoveAnimName();
	virtual const char *GetDownMoveAnimName();
	virtual const char *GetDownIdleAnimName();

	DECLARE_DATADESC();
};

BEGIN_DATADESC(CFloorButtonBall)

END_DATADESC()


LINK_ENTITY_TO_CLASS(prop_floor_ball_button, CFloorButtonBall);

#define FLOOR_BUTTON_BALL_MODEL_NAME "models/props/ball_button.mdl"

const char *CFloorButtonBall::GetUpAnimName(){
	return "idle";
}

const char *CFloorButtonBall::GetUpMoveAnimName(){
	return "idle";
}

const char *CFloorButtonBall::GetDownMoveAnimName(){
	return "down";
}

const char *CFloorButtonBall::GetDownIdleAnimName(){
	return "downIdle";
}

void CFloorButtonBall::Spawn()
{
	SetModelName(castable_string_t(FLOOR_BUTTON_BALL_MODEL_NAME));

	BaseClass::Spawn();
}

bool CFloorButtonBall::CanPressButton(CBaseEntity* ent){
	if (!ent->IsSolid())
		return	false;

	//get our filter
	CBaseFilter *pFilter = m_hFilter.Get();

	//if filter is not null...
	if (pFilter){
		return pFilter->PassesFilter(this, ent);	//use the filter for all checks instead of the built in check.
	}

	//Check if the activating object is a prop_weighted_cube
	CPortalCube* cube = dynamic_cast<CPortalCube*>(ent);

	if (cube && cube->IsSphere()){	//if entity is a cube, and not a sphere type cube
		return true;
	}

	return false;
}


//NEW
class CFloorButtonUnder : public CFloorButton
{
public:
	DECLARE_CLASS(CFloorButtonUnder, CFloorButton);

	void Spawn();

private:
	virtual const char *GetUpAnimName();
	virtual const char *GetUpMoveAnimName();
	virtual const char *GetDownMoveAnimName();
	virtual const char *GetDownIdleAnimName();

	DECLARE_DATADESC();
};

BEGIN_DATADESC(CFloorButtonUnder)

END_DATADESC()


LINK_ENTITY_TO_CLASS(prop_under_floor_button, CFloorButtonUnder);

#define FLOOR_BUTTON_UNDER_MODEL_NAME "models/props_underground/underground_floor_button.mdl"

const char *CFloorButtonUnder::GetUpAnimName(){
	return "release_idle";
}

const char *CFloorButtonUnder::GetUpMoveAnimName(){
	return "release";
}

const char *CFloorButtonUnder::GetDownMoveAnimName(){
	return "press";
}

const char *CFloorButtonUnder::GetDownIdleAnimName(){
	return "press_idle";
}

void CFloorButtonUnder::Spawn()
{
	SetModelName(castable_string_t(FLOOR_BUTTON_UNDER_MODEL_NAME));

	BaseClass::Spawn();

	m_nBoxDetectSphere = 512;

	m_ActivatesCubes = false;

	spherehitbox = false;
}