#include "triggers.h"

//////////////////////////////////////////////////////////////////////////
// CTriggerPortal
// Moves touched entity to a target location, changing the model's orientation
// to match the exit target. It differs from CTriggerTeleport in that it
// reorients physics and has inputs to enable/disable its function.
//////////////////////////////////////////////////////////////////////////
class CTriggerPortal : public CBaseTrigger
{
public:
	DECLARE_DATADESC();
	DECLARE_CLASS(CTriggerPortal, CBaseTrigger);
	//DECLARE_SERVERCLASS();

	virtual void Spawn(void);
	virtual void Activate();

	void Touch(CBaseEntity *pOther);
	void EndTouch(CBaseEntity *pOther);
	void DisableForIncomingEntity(CBaseEntity *pEntity);
	bool IsTouchingPortal(CBaseEntity *pEntity);

	void DisabledThink(void);

	// TEMP: Since brushes have no directionality, give this wall a forward face specified in hammer
	QAngle						m_qFaceAngles;
	bool						m_useDirection;

private:
	string_t					m_strRemotePortal;
	CNetworkHandle(CTriggerPortal, m_hRemotePortal);
	CUtlVector<EHANDLE>			m_hDisabledForEntities;

	// Input for setting remote portal entity (for teleporting to it)
	void SetRemotePortal(const char* strRemotePortalName);
	void InputSetRemotePortal(inputdata_t &inputdata);

};