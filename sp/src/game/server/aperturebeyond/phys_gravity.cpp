//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========
//
// Purpose: Simple logical entity that counts up to a threshold value, then
//			fires an output when reached.
//
//=============================================================================

#include "cbase.h"
#include "string.h"
#include <iostream>
#include <string>

class CPhysGravity : public CLogicalEntity
{
public:
	DECLARE_CLASS(CPhysGravity, CLogicalEntity);
	DECLARE_DATADESC();

	// Constructor
	CPhysGravity()
	{
		m_defaultGravZ = -600;
		m_defaultGrav = Vector(0, 0, -600);

		m_lastGrav = m_defaultGrav;
		m_lastGravZ = m_defaultGravZ;
	}

	// Input function
	void DisableGravity(inputdata_t &inputData);
	void EnableGravity(inputdata_t &inputData);
	void SetGravity(inputdata_t &inputData);
	void SetGravityVector(inputdata_t &inputData);
	void GetCurrentGravity(inputdata_t &inputData);
	void SetPlayerGravity(inputdata_t &inputData);

private:

	Vector m_defaultGrav;	//holds the default gravity value.
	int	m_defaultGravZ;	// holds the default Z axis gravity value.
	
	Vector m_lastGrav;	//holds the last set gravity vector.
	int m_lastGravZ;	//holds the last set gravity value on the z axis.

	COutputEvent	m_OnGravityChanged;	// Output event when the gravity setting is changed
	COutputEvent	m_OnGravityDisabled;	// Output event when the gravity is disabled
	COutputEvent	m_OnGravityEnabled;	// Output event when the gravity is enabled
	COutputVector	m_OnGravityGet;	// Output event when gravity is requested via input
	COutputInt	m_OnGravityGetZ;	// Output event when gravity Z axis is requested via input
};

LINK_ENTITY_TO_CLASS(phys_gravity, CPhysGravity);

// Start of our data description for the class
BEGIN_DATADESC(CPhysGravity)

// For save/load
DEFINE_FIELD(m_defaultGrav, FIELD_VECTOR),
DEFINE_FIELD(m_defaultGravZ, FIELD_INTEGER),

DEFINE_FIELD(m_lastGrav, FIELD_VECTOR),
DEFINE_FIELD(m_lastGravZ, FIELD_INTEGER),

// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_defaultGravZ, FIELD_INTEGER, "defaultgravity"),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "DisableGravity", DisableGravity),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "EnableGravity", EnableGravity),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "GetCurrentGravity", GetCurrentGravity),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetGravity", SetGravity),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VECTOR, "SetGravityVector", SetGravityVector),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetPlayerGravity", SetPlayerGravity),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnGravityEnabled, "OnGravityEnabled"),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnGravityDisabled, "OnGravityDisabled"),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnGravityChanged, "OnGravityChanged"),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnGravityGet, "OnGravityGet"),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_OnGravityGetZ, "OnGravityGetZ"),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Disables gravity for all physics objects.
//-----------------------------------------------------------------------------
void CPhysGravity::DisableGravity(inputdata_t &inputData)
{
	Vector gravVector;
	physenv->GetGravity(&gravVector);
	/*
	int gravX = gravVector.x;
	int gravY = gravVector.y;
	int gravZ = gravVector.z;

	DevMsg("Original Gravity is %i, %i, %i", gravX, gravY, gravZ);
	*/
	physenv->SetGravity(Vector(0,0,0));
	m_OnGravityDisabled.FireOutput(inputData.pActivator, this);
	m_OnGravityChanged.FireOutput(inputData.pActivator, this);
}

//-----------------------------------------------------------------------------
// Purpose: Reenables gravity for all physics objects.
//-----------------------------------------------------------------------------
void CPhysGravity::EnableGravity(inputdata_t &inputData)
{
	physenv->SetGravity(Vector(0, 0, -600));
	m_OnGravityEnabled.FireOutput(inputData.pActivator, this);
	m_OnGravityChanged.FireOutput(inputData.pActivator, this);
}

//-----------------------------------------------------------------------------
// Purpose: Sets gravity to the given value. Only affects Z axis (up/down).
//-----------------------------------------------------------------------------
void CPhysGravity::SetGravity(inputdata_t &inputData)
{
	int inputNum = inputData.value.Int();
	physenv->SetGravity(Vector(0, 0, inputNum));
	m_OnGravityChanged.FireOutput(inputData.pActivator, this);
}

//-----------------------------------------------------------------------------
// Purpose: Sets gravity to the given vector. Can be set to make gravity go in any direction, theoretically.
//-----------------------------------------------------------------------------
void CPhysGravity::SetGravityVector(inputdata_t &inputData)
{
	Vector inputVector;
	inputData.value.Vector3D(inputVector);	//get input datas' vector in our Vector variable

	physenv->SetGravity(inputVector);
	m_OnGravityChanged.FireOutput(inputData.pActivator, this);
}

//-----------------------------------------------------------------------------
// Purpose: Outputs current gravity vector and gravity value on Z axis
//-----------------------------------------------------------------------------
void CPhysGravity::GetCurrentGravity(inputdata_t &inputData)
{
	Vector gravVector;
	physenv->GetGravity(&gravVector);

	m_OnGravityGet.Set(gravVector, inputData.pActivator, this);	//this line set our input vector as the output data and fires it for us!
	m_OnGravityGetZ.Set(gravVector.z, inputData.pActivator, this);	//this line set our input vector as the output data and fires it for us!
}

//-----------------------------------------------------------------------------
// Purpose: Sets the players gravity. Doesn't support full vectors in this case, only z axis movement.
//			Should be called alongside the others if you want to sync player and physics objects gravity.
//-----------------------------------------------------------------------------
void CPhysGravity::SetPlayerGravity(inputdata_t &inputData){
	/*
	CBasePlayer *pPlayer = NULL;
	pPlayer = UTIL_GetLocalPlayer();	//get current single player player object. We need something else for multiplayer...
	pPlayer->SetGravity(5);
	*/

	ConVar *pGravity = cvar->FindVar("sv_gravity");	//get the server gravity variable
	int oldgrav = pGravity->GetInt();

	pGravity->SetValue(inputData.value.Int());	//set the server gravity variable to our objects input value.
	
	if (oldgrav * inputData.value.Int() < 0){	//if oldgrav * new grav is less then zero (gravity has been reversed)...
		CBasePlayer *pPlayer = NULL;
		pPlayer = UTIL_GetLocalPlayer();	//get current single player player object. We need something else for multiplayer...

		pPlayer->RemoveFlag(FL_ONGROUND);	//make player no longer grounded
	}
}