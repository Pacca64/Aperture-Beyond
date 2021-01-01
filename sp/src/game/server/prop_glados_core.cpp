#include "cbase.h"
#include "props.h"
#include <string>
#include "prop_glados_core.h"

BEGIN_DATADESC(CPropGladosCore)

DEFINE_KEYFIELD(m_speechDelay, FIELD_INTEGER, "DelayBetweenLines"),
DEFINE_KEYFIELD(m_CoreType, FIELD_INTEGER, "CoreType"),

// Declare our think function
DEFINE_THINKFUNC(Think),

END_DATADESC()


LINK_ENTITY_TO_CLASS(prop_glados_core, CPropGladosCore);

void CPropGladosCore::Precache(void)
{
	const char *pModelName = STRING(GetModelName());

	if (pModelName && pModelName[0] != '\0' || strcmp(pModelName, "models/error.mdl") == 0){
		pModelName = pModelName;
		DevMsg("prop_glados_core using custom model: ");
		DevMsg(pModelName);
		DevMsg("\n");
	}
	else {
		pModelName = CORE_MODEL_NAME;
		DevMsg("prop_glados_core using default model\n");
	}

	PrecacheModel(pModelName);
	SetModel(pModelName);

	BaseClass::Precache();
}

void CPropGladosCore::Spawn()
{
	Precache();

	BaseClass::Spawn();

	AddSpawnFlags(SF_PHYSPROP_ENABLE_PICKUP_OUTPUT);	//set flag to enable player pickup for this physics prop

	// Don't allow us to skip animation setup because our attachments are critical to us!
	SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);

	CreateVPhysics();

	if (VPhysicsInitNormal(SOLID_VPHYSICS, 0, false) == NULL)
	{
		DevMsg("npc_personality_core unable to spawn physics object!\n");
	}

	ResetSequence(LookupSequence("drop"));

	switch (m_CoreType){

	case CORESKIN_MORALITY:
		m_nSkin = CORESKIN_MORALITY;
		break;

	case CORESKIN_CURIOUSITY:
		m_nSkin = CORESKIN_CURIOUSITY;
		break;

	case CORESKIN_ANGER:
		m_nSkin = CORESKIN_ANGER;
		break;

	case CORESKIN_CAKE:
		m_nSkin = CORESKIN_CAKE;
		break;
	}

	SetThink(&CPropGladosCore::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CPropGladosCore::Think(void){
	BaseClass::AnimThink();

	// Think at 20Hz
	SetNextThink(gpGlobals->curtime + 0.05f);
}