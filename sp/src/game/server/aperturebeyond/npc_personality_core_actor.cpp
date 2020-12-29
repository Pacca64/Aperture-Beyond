//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//=========================================================
// Generic NPC - purely for scripted sequence work.
//=========================================================
#include "cbase.h"
#include "shareddefs.h"
#include "npcevent.h"
#include "player_pickup.h"
#include "ai_basenpc.h"
#include "ai_hull.h"
#include "ai_baseactor.h"
#include "tier1/strtools.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar physcannon_tracelength;

//Debug visualization
ConVar	DisableCoreUse("DisableCoreUse", "false");

//---------------------------------------------------------
// Sounds
//---------------------------------------------------------


//=========================================================
// NPC's Anim Events Go Here
//=========================================================

class CNPC_PersonalityCore2 : public CAI_BaseActor, public CDefaultPlayerPickupVPhysics
{
public:
	DECLARE_CLASS(CNPC_PersonalityCore2, CAI_BaseActor);

	void	Spawn(void);
	void	Precache(void);
	float	MaxYawSpeed(void);
	Class_T Classify(void);
	void	HandleAnimEvent(animevent_t *pEvent);
	int		GetSoundInterests(void);

	virtual bool	CreateVPhysics(void);
	virtual void	PlayerPenetratingVPhysics(void);
	virtual int		VPhysicsTakeDamage(const CTakeDamageInfo &info);

	virtual void	OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason);
	virtual void	OnPhysGunDrop(CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason);
	virtual bool	HasPreferredCarryAnglesForPlayer(CBasePlayer *pPlayer);
	virtual QAngle	PreferredCarryAngles(void);
	virtual bool	OnAttemptPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason);

	virtual int	ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE | FCAP_WCEDIT_POSITION; };
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	bool	ShouldSavePhysics() { return true; }

	void	TempGunEffect(void);

	void InputSkin(inputdata_t &inputData);
	void InputForcePlayerPickup(inputdata_t &inputData);
	void InputEnablePickup(inputdata_t &inputData);
	void InputDisablePickup(inputdata_t &inputData);

	string_t			m_strHullName;

	CHandle<CBasePlayer>	m_hPhysicsAttacker;
	float					m_flLastPhysicsInfluenceTime;

protected:
	float	m_flNextActivateSoundTime;
	bool	m_bCarriedByPlayer;
	bool	m_bUseCarryAngles;
	float	m_flPlayerDropTime;
	int		m_iKeySkin;
	bool	m_gagged;
	bool	m_canPickup;

	COutputEvent m_OnPhysGunPickup;
	COutputEvent m_OnPhysGunDrop;

	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(npc_personality_core, CNPC_PersonalityCore2);

BEGIN_DATADESC(CNPC_PersonalityCore2)

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_INTEGER, "skin", InputSkin),
DEFINE_INPUTFUNC(FIELD_VOID, "ForcePickup", InputForcePlayerPickup),
DEFINE_INPUTFUNC(FIELD_VOID, "EnablePickup", InputEnablePickup),
DEFINE_INPUTFUNC(FIELD_VOID, "DisablePickup", InputDisablePickup),

DEFINE_FIELD(m_canPickup, FIELD_BOOLEAN),

DEFINE_KEYFIELD(m_nSkin, FIELD_INTEGER, "skin"),

DEFINE_OUTPUT(m_OnPhysGunDrop, "OnPlayerDrop"),
DEFINE_OUTPUT(m_OnPhysGunPickup, "OnPlayerPickup"),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Sets the amount of trailing zeroes our appended number should have.
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore2::InputSkin(inputdata_t &inputData)
{
	m_nSkin = inputData.value.Int();
}

//-----------------------------------------------------------------------------
// Purpose: Force player to pick us up
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore2::InputForcePlayerPickup(inputdata_t &inputData)
{
	PlayerPickupObject(UTIL_GetLocalPlayer(), this);
}

//-----------------------------------------------------------------------------
// Purpose: Allows players to pick us up!
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore2::InputEnablePickup(inputdata_t &inputData)
{
	m_canPickup = true;
}

//-----------------------------------------------------------------------------
// Purpose: Allows players to pick us up!
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore2::InputDisablePickup(inputdata_t &inputData)
{
	m_canPickup = false;
}

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_PersonalityCore2::Classify(void)
{
	return	CLASS_NONE;
}

//=========================================================
// MaxYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
float CNPC_PersonalityCore2::MaxYawSpeed(void)
{
	return 90;
}

//=========================================================
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CNPC_PersonalityCore2::HandleAnimEvent(animevent_t *pEvent)
{
	BaseClass::HandleAnimEvent(pEvent);
}

//=========================================================
// GetSoundInterests - generic NPC can't hear.
//=========================================================
int CNPC_PersonalityCore2::GetSoundInterests(void)
{
	return	NULL;
}

//=========================================================
// Spawn
//=========================================================
void CNPC_PersonalityCore2::Spawn()
{
	m_canPickup = true;	//can be picked up by default

	Precache();

	SetModel(STRING(GetModelName()));

	/*
	if ( FStrEq( STRING( GetModelName() ), "models/player.mdl" ) )
	UTIL_SetSize(this, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
	else
	UTIL_SetSize(this, VEC_HULL_MIN, VEC_HULL_MAX);
	*/

	if (FStrEq(STRING(GetModelName()), "models/player.mdl") ||
		FStrEq(STRING(GetModelName()), "models/holo.mdl") ||
		FStrEq(STRING(GetModelName()), "models/blackout.mdl"))
	{
		UTIL_SetSize(this, VEC_HULL_MIN, VEC_HULL_MAX);
	}
	else
	{
		UTIL_SetSize(this, NAI_Hull::Mins(HULL_HUMAN), NAI_Hull::Maxs(HULL_HUMAN));
	}

	if (!FStrEq(STRING(GetModelName()), "models/blackout.mdl"))
	{
		SetSolid(SOLID_BBOX);
		AddSolidFlags(FSOLID_NOT_STANDABLE);
	}
	else
	{
		SetSolid(SOLID_NONE);
	}

	SetMoveType(MOVETYPE_NONE);
	SetBloodColor(BLOOD_COLOR_MECH);
	m_iHealth = 99999;
	m_flFieldOfView = 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;

	//CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS);

	CapabilitiesAdd(bits_CAP_FRIENDLY_DMG_IMMUNE);

	// remove head turn if no eyes or forward attachment
	if (LookupAttachment("eyes") > 0 && LookupAttachment("forward") > 0)
	{
		CapabilitiesAdd(bits_CAP_TURN_HEAD | bits_CAP_ANIMATEDFACE);
	}

	if (m_strHullName != NULL_STRING)
	{
		SetHullType(NAI_Hull::LookupId(STRING(m_strHullName)));
	}
	else
	{
		SetHullType(HULL_HUMAN);
	}

	SetHullType(HULL_SMALL_CENTERED);	//Always set hull to this, allows grabing to work properly. Same setting used by combine scanners!

	SetHullSizeNormal();

	NPCInit();
}

//=========================================================
// Precache - precaches all resources this NPC needs
//=========================================================
void CNPC_PersonalityCore2::Precache()
{
	PrecacheModel(STRING(GetModelName()));
}

bool CNPC_PersonalityCore2::CreateVPhysics(void)
{
	//Spawn our physics hull
	if (VPhysicsInitNormal(SOLID_VPHYSICS, 0, false) == NULL)
	{
		DevMsg("npc_personality_core unable to spawn physics object!\n");
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore2::OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason)
{

	m_hPhysicsAttacker = pPhysGunUser;
	m_flLastPhysicsInfluenceTime = gpGlobals->curtime;

	// Drop our mass a lot so that we can be moved easily with +USE
	if (reason != PUNTED_BY_CANNON)
	{
		Assert(VPhysicsGetObject());

		m_bCarriedByPlayer = true;
		m_OnPhysGunPickup.FireOutput(this, this);

		// We want to use preferred carry angles if we're not nicely upright
		Vector vecToTurret = pPhysGunUser->GetAbsOrigin() - GetAbsOrigin();
		vecToTurret.z = 0;
		VectorNormalize(vecToTurret);

		// We want to use preferred carry angles if we're not nicely upright
		Vector	forward, up;
		GetVectors(&forward, NULL, &up);

		bool bUpright = DotProduct(up, Vector(0, 0, 1)) > 0.9f;
		bool bBehind = DotProduct(vecToTurret, forward) < 0.85f;

		// Correct our angles only if we're not upright or we're mostly behind the turret
		if (hl2_episodic.GetBool())
		{
			m_bUseCarryAngles = (bUpright == false || bBehind);
		}
		else
		{
			m_bUseCarryAngles = (bUpright == false);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore2::OnPhysGunDrop(CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason)
{
	m_hPhysicsAttacker = pPhysGunUser;
	m_flLastPhysicsInfluenceTime = gpGlobals->curtime;

	m_bCarriedByPlayer = false;
	m_bUseCarryAngles = false;
	m_OnPhysGunDrop.FireOutput(this, this);

	// If this is a friendly turret, remember that it was just dropped
	if (IRelationType(pPhysGunUser) != D_HT)
	{
		m_flPlayerDropTime = gpGlobals->curtime + 2.0;
	}

	// Restore our mass to the original value
	Assert(VPhysicsGetObject());
}

//-----------------------------------------------------------------------------
// Purpose: Whether this should return carry angles
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_PersonalityCore2::HasPreferredCarryAnglesForPlayer(CBasePlayer *pPlayer)
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_PersonalityCore2::OnAttemptPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason)
{
	return true;//m_canPickup;
}

//-----------------------------------------------------------------------------
// Purpose: We override this code because otherwise we start to move into the
//			tricky realm of player avoidance.  Since we don't go through the
//			normal NPC thinking but we ARE an NPC (...) we miss a bunch of 
//			book keeping.  This means we can become invisible and then never
//			reappear.
//-----------------------------------------------------------------------------
void CNPC_PersonalityCore2::PlayerPenetratingVPhysics(void)
{
	// We don't care!
}


//-----------------------------------------------------------------------------
// Purpose: Reduce physics forces from the front
//-----------------------------------------------------------------------------
int CNPC_PersonalityCore2::VPhysicsTakeDamage(const CTakeDamageInfo &info)
{
	return BaseClass::VPhysicsTakeDamage(info);
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : const QAngle
//-----------------------------------------------------------------------------
QAngle CNPC_PersonalityCore2::PreferredCarryAngles(void)
{
	// FIXME: Embed this into the class
	static QAngle g_prefAngles;

	Vector vecUserForward;
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	pPlayer->EyeVectors(&vecUserForward);

	QAngle eyes = pPlayer->LocalEyeAngles();

	return QAngle(0, 90, 0);

	// If we're looking up, then face directly forward
	if (vecUserForward.z >= 0.0f)
		return vec3_angle;

	// Otherwise, stay "upright"
	g_prefAngles.Init();
	g_prefAngles.x = -pPlayer->EyeAngles().x;

	return g_prefAngles;
}

void CNPC_PersonalityCore2::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (DisableCoreUse.GetBool()){
		return;
	}

	//if pickup has been disabled...
	if (!m_canPickup){
		//return; //return.
	}

	CBasePlayer *pPlayer = ToBasePlayer(pActivator);

	if (pPlayer)
	{
		pPlayer->PickupObject(this);
	}
}

//=========================================================
// AI Schedules Specific to this NPC
//=========================================================

