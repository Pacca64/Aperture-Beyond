#include "cbase.h"
#include "props.h"
#include <string>
#include "filters.h"
#include "trigger_portal.h"

//NEW
class CPropPortal : public CTriggerPortal
{
public:
	DECLARE_CLASS(CPropPortal, CTriggerPortal);

	void Spawn();

private:
	void Precache(void);

	DECLARE_DATADESC();
};

BEGIN_DATADESC(CPropPortal)

END_DATADESC()


LINK_ENTITY_TO_CLASS(prop_portal, CPropPortal);

#define USE_BUTTON_MODEL_NAME "models/props_borealis/borealis_door001a.mdl"

void CPropPortal::Precache(void)
{
	const char *pModelName = STRING(GetModelName());
	pModelName = (pModelName && pModelName[0] != '\0') ? pModelName : USE_BUTTON_MODEL_NAME;
	PrecacheModel(pModelName);

	PrecacheScriptSound("Portal.button_down");
	PrecacheScriptSound("Portal.button_up");
	PrecacheScriptSound("Portal.ButtonDepress");
	PrecacheScriptSound("Portal.ButtonRelease");
}

void CPropPortal::Spawn()
{
	const char *pModelName = STRING(GetModelName());
	SetModel((pModelName && pModelName[0] != '\0') ? pModelName : USE_BUTTON_MODEL_NAME);

	BaseClass::Spawn();
	Precache();

	//const char *pModelName = STRING(GetModelName());
	//SetModel((pModelName && pModelName[0] != '\0') ? pModelName : USE_BUTTON_MODEL_NAME);

	//CPropPortal::InitTrigger();
	//CPropPortal::Enable();
}
