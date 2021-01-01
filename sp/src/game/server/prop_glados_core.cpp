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

	if (pModelName && pModelName[0] != '\0'){
		pModelName = pModelName;
	}
	else {
		pModelName = CORE_MODEL_NAME;
	}

	pModelName = CORE_MODEL_NAME;

	PrecacheModel(pModelName);

	BaseClass::Precache();
}

void CPropGladosCore::Spawn()
{
	Precache();

	const char *pModelName = STRING(GetModelName());

	if (pModelName && pModelName[0] != '\0'){
		pModelName = pModelName;
	}
	else {
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
	}

	pModelName = CORE_MODEL_NAME;

	SetModel(pModelName);

	BaseClass::Spawn();

	AddSpawnFlags(SF_PHYSPROP_ENABLE_PICKUP_OUTPUT);	//set flag to enable player pickup for this physics prop

	SetThink(&CPropGladosCore::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CPropGladosCore::Think(void){

	SetNextThink(gpGlobals->curtime + 0.05f);
}