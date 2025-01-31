#include "cbase.h"

class CMyModelEntity : public CBaseAnimating
{
public:
	DECLARE_CLASS(CMyModelEntity, CBaseAnimating);
	DECLARE_DATADESC();

	CMyModelEntity()
	{
		m_bActive = false;
	}

	void Spawn(void);
	void Precache(void);

	void MoveThink(void);

	// Input function
	void InputToggle(inputdata_t &inputData);

private:

	bool	m_bActive;
	float	m_flNextChangeTime;
};

LINK_ENTITY_TO_CLASS(my_model_entity, CMyModelEntity);

// Start of our data description for the class
BEGIN_DATADESC(CMyModelEntity)

// Save/restore our active state
DEFINE_FIELD(m_bActive, FIELD_BOOLEAN),
DEFINE_FIELD(m_flNextChangeTime, FIELD_TIME),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

// Declare our think function
DEFINE_THINKFUNC(MoveThink),

END_DATADESC()

// Name of our entity's model
#define	ENTITY_MODEL	"models/gibs/airboat_broken_engine.mdl"

//-----------------------------------------------------------------------------
// Purpose: Precache assets used by the entity
//-----------------------------------------------------------------------------
void CMyModelEntity::Precache(void)
{
	PrecacheModel(ENTITY_MODEL);

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CMyModelEntity::Spawn(void)
{
	Precache();

	SetModel(ENTITY_MODEL);
	SetSolid(SOLID_BBOX);
	UTIL_SetSize(this, -Vector(20, 20, 20), Vector(20, 20, 20));
}

//-----------------------------------------------------------------------------
// Purpose: Think function to randomly move the entity
//-----------------------------------------------------------------------------
void CMyModelEntity::MoveThink(void)
{
	// See if we should change direction again
	if (m_flNextChangeTime < gpGlobals->curtime)
	{
		// Randomly take a new direction and speed
		SetAbsVelocity(RandomVector(-64.0f, 64.0f));

		// Randomly change it again within one to three seconds
		m_flNextChangeTime = gpGlobals->curtime + random->RandomFloat(1.0f, 3.0f);
	}

	// Snap our facing to where we're heading
	Vector velFacing = GetAbsVelocity();
	QAngle angFacing;
	VectorAngles(velFacing, angFacing);
	SetAbsAngles(angFacing);

	// Think at 20Hz
	SetNextThink(gpGlobals->curtime + 0.05f);
}

//-----------------------------------------------------------------------------
// Purpose: Toggle the movement of the entity
//-----------------------------------------------------------------------------
void CMyModelEntity::InputToggle(inputdata_t &inputData)
{
	// Toggle our active state
	if (!m_bActive)
	{
		// Start thinking
		SetThink(&CMyModelEntity::MoveThink);

		SetNextThink(gpGlobals->curtime + 0.05f);

		// Start moving
		SetMoveType(MOVETYPE_FLY);

		// Force MoveThink() to choose a new speed and direction immediately
		m_flNextChangeTime = gpGlobals->curtime;

		// Update m_bActive to reflect our new state
		m_bActive = true;
	}
	else
	{
		// Stop thinking
		SetThink(NULL);

		// Stop moving
		SetAbsVelocity(vec3_origin);
		SetMoveType(MOVETYPE_NONE);

		m_bActive = false;
	}
}

CON_COMMAND(create_sdk_modelentity, "Creates an instance of the sdk model entity in front of the player.")
{
	Vector vecForward;
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
	{
		Warning("Could not determine calling player!\n");
		return;
	}

	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	CBaseEntity *pEnt = CreateEntityByName("my_model_entity");
	if (pEnt)
	{
		Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 256 + Vector(0, 0, 64);
		QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 90, 0);
		pEnt->SetAbsOrigin(vecOrigin);
		pEnt->SetAbsAngles(vecAngles);
		DispatchSpawn(pEnt);
	}
}