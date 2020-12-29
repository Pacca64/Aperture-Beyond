#include "cbase.h"
#include "props.h"
#include <string>
#include "prop_weighted_cube.h"
#include "filters.h"

//NEW
class CPortalUseButton : public CDynamicProp
{
public:
	DECLARE_CLASS(CPortalUseButton, CDynamicProp);

	void Spawn();
	void Activate();
	void Think();
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual void HandleAnimEvent(animevent_t *pEvent);
	virtual bool HasAnimEvent(int nSequence, int nEvent);

	virtual int	ObjectCaps(void)
	{
		int flags = BaseClass::ObjectCaps();
		//if (HasSpawnFlags(SF_DOOR_PUSE))
			return flags | FCAP_IMPULSE_USE | FCAP_USE_IN_RADIUS;

		return flags;
	};

	virtual const char *GetUpIdleAnimName();
	virtual const char *GetUpMoveAnimName();
	virtual const char *GetDownMoveAnimName();
	virtual const char *GetDownIdleAnimName();

	CHandle<class CBaseFilter>	m_hFilter;

private:
	void Precache(void);

	bool m_isPressed;
	bool m_isSilent;
	string_t	m_iFilterName;

	COutputEvent m_OnPressed;
	COutputEvent m_OnUnpressed;

	DECLARE_DATADESC();
};

BEGIN_DATADESC(CPortalUseButton)

DEFINE_FIELD(m_isPressed, FIELD_BOOLEAN),

DEFINE_KEYFIELD(m_isSilent, FIELD_BOOLEAN, "SuppressAnimSounds"),
DEFINE_KEYFIELD(m_iFilterName, FIELD_STRING, "filtername"),

// Declare our think function
DEFINE_THINKFUNC(Think),

DEFINE_OUTPUT(m_OnPressed, "OnPressed"),
DEFINE_OUTPUT(m_OnUnpressed, "OnButtonReset"),

DEFINE_USEFUNC(Use)

END_DATADESC()


LINK_ENTITY_TO_CLASS(prop_button, CPortalUseButton);

#define USE_BUTTON_MODEL_NAME "models/props/switch001.mdl"

const char *CPortalUseButton::GetUpIdleAnimName(){
	return "idle";
}

const char *CPortalUseButton::GetUpMoveAnimName(){
	return "up";
}

const char *CPortalUseButton::GetDownMoveAnimName(){
	return "down";
}

const char *CPortalUseButton::GetDownIdleAnimName(){
	return "idle_down";
}

void CPortalUseButton::Precache(void)
{
	const char *pModelName = STRING(GetModelName());
	pModelName = (pModelName && pModelName[0] != '\0') ? pModelName : USE_BUTTON_MODEL_NAME;
	PrecacheModel(pModelName);

	PrecacheScriptSound("Portal.button_down");
	PrecacheScriptSound("Portal.button_up");
	PrecacheScriptSound("Portal.ButtonDepress");
	PrecacheScriptSound("Portal.ButtonRelease");

}

void CPortalUseButton::Spawn()
{
	Precache();

	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_VPHYSICS);
	//CreateVPhysics();

	const char *pModelName = STRING(GetModelName());
	SetModel((pModelName && pModelName[0] != '\0') ? pModelName : USE_BUTTON_MODEL_NAME);
	//AddEffects(EF_NOSHADOW);

	ResetSequence(LookupSequence(GetUpIdleAnimName()));

	CreateVPhysics();

	m_isPressed = false;

	SetUse(&CPortalUseButton::Use);

	SetThink(&CPortalUseButton::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

//------------------------------------------------------------------------------
// Activate
//------------------------------------------------------------------------------
void CPortalUseButton::Activate(void)
{
	// Get a handle to my filter entity if there is one
	if (m_iFilterName != NULL_STRING)
	{
		m_hFilter = dynamic_cast<CBaseFilter *>(gEntList.FindEntityByName(NULL, m_iFilterName));
	}

	BaseClass::Activate();
}

void CPortalUseButton::Think(){
	BaseClass::AnimThink();

	// Think at 20Hz
	SetNextThink(gpGlobals->curtime + 0.05f);
}

void CPortalUseButton::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CBasePlayer *pPlayer = ToBasePlayer(pActivator);

	DevMsg("Button used!\n");

	Vector vecSrc = pPlayer->Weapon_ShootPosition();

	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward); //Assign vecForward a vector facing the EyeAngles

	trace_t tr; // Create our trace_t class to hold the end result
	UTIL_TraceLine(vecSrc, vecSrc + (vecForward * MAX_TRACE_LENGTH), MASK_ALL, pPlayer, COLLISION_GROUP_NONE, &tr);

	tr.endpos;

	if (tr.endpos.z > this->GetAbsOrigin().z + 40 && tr.endpos.z < this->GetAbsOrigin().z + 60){
		DevMsg("And Pressed even! Nice\n");
		PropSetAnim(GetDownMoveAnimName());
		m_iszDefaultAnim = castable_string_t(GetDownIdleAnimName());
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPortalUseButton::HandleAnimEvent(animevent_t *pEvent){
	//if (!m_isSilent)
	return;

	BaseClass::HandleAnimEvent(pEvent);
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the given sequence has the anim event, false if not.
// Input  : nSequence - sequence number to check
//			nEvent - anim event number to look for
//-----------------------------------------------------------------------------
bool CPortalUseButton::HasAnimEvent(int nSequence, int nEvent)
{
	return false;
	if (!m_isSilent){
		return BaseClass::HasAnimEvent(nSequence, nEvent);
	}
	else {
		return false;	//pretend we have no anim events.
	}
}