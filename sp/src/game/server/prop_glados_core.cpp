#include "cbase.h"
#include "props.h"
#include <string>
#include "prop_glados_core.h"
#include "engine/IEngineSound.h"


BEGIN_DATADESC(CPropGladosCore)

DEFINE_FIELD(m_isActive, FIELD_BOOLEAN),
DEFINE_FIELD(m_isTurning, FIELD_BOOLEAN),
DEFINE_FIELD(m_animJustEnded, FIELD_BOOLEAN),

DEFINE_FIELD(m_voiceLineDuration, FIELD_FLOAT),
DEFINE_FIELD(m_lastVoiceLineTime, FIELD_FLOAT),

//Keyfields that replicate original entity
DEFINE_KEYFIELD(m_speechDelay, FIELD_FLOAT, "DelayBetweenLines"),
DEFINE_KEYFIELD(m_CoreType, FIELD_INTEGER, "CoreType"),

//new "custom" fields, for mapping
DEFINE_KEYFIELD(m_customTurnAnim, FIELD_STRING, "CustomTurnAnim"),
DEFINE_KEYFIELD(m_customDropAnim, FIELD_STRING, "CustomDropAnim"),
DEFINE_KEYFIELD(m_customLookAnim, FIELD_STRING, "CustomLookAnim"),

DEFINE_KEYFIELD(m_customFileName, FIELD_STRING, "CustomSoundFileName"),
DEFINE_KEYFIELD(m_customFileNameEnding, FIELD_STRING, "CustomSoundFileNameEnding"),
DEFINE_KEYFIELD(m_customFileName_HasLeadingZero, FIELD_BOOLEAN, "CustomSoundFileNameHasLeadingZero"),
//custom game sound names
DEFINE_KEYFIELD(m_customSoundName, FIELD_STRING, "CustomGameSoundName"),
DEFINE_KEYFIELD(m_customSoundNameEnding, FIELD_STRING, "CustomGameSoundNameEnding"),
DEFINE_KEYFIELD(m_customSoundName_HasLeadingZero, FIELD_BOOLEAN, "CustomGameSoundNameHasLeadingZero"),

DEFINE_KEYFIELD(m_customNumberOfSoundsInLoop, FIELD_INTEGER, "CustomNumberOfSounds"),

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

	//PrecacheScriptSound("Portal.Glados_core.Curiosity_1");

	precacheCoreVoiceLines(CORE_SOUNDSCRIPTNAME_CURIOUSITY, 17, false);
	precacheCoreVoiceLines(CORE_SOUNDSCRIPTNAME_ANGER, 21, true);
	precacheCoreVoiceLines(CORE_SOUNDSCRIPTNAME_CAKE, 41, true);
	precacheCoreVoiceLines("custom", m_customNumberOfSoundsInLoop, m_customSoundName_HasLeadingZero);

	BaseClass::Precache();
}

void CPropGladosCore::Spawn()
{

	//If custom sound name is set as the string "null"...
	if (strcmp(m_customSoundName, "null") == 0){
		m_customSoundName = "";	//set sound name to an empty string. Workaround for a crash related to setting empty string values.
	}

	//If custom sound name is set as the string "null"...
	if (strcmp(m_customSoundNameEnding, "null") == 0){
		m_customSoundNameEnding = "";	//set sound name to an empty string. Workaround for a crash related to setting empty string values.
	}

	//If custom sound name is set as the string "null"...
	if (strcmp(m_customFileName, "null") == 0){
		m_customFileName = "";	//set sound name to an empty string. Workaround for a crash related to setting empty string values.
	}

	//If custom sound name is set as the string "null"...
	if (strcmp(m_customFileNameEnding, "null") == 0){
		m_customFileNameEnding = "";	//set sound name to an empty string. Workaround for a crash related to setting empty string values.
	}

	Precache();

	BaseClass::Spawn();

	AddSpawnFlags(SF_PHYSPROP_ENABLE_PICKUP_OUTPUT);	//set flag to enable player pickup for this physics prop

	// Don't allow us to skip animation setup because our attachments are critical to us!
	//SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);

	//CreateVPhysics();

	//if (VPhysicsInitNormal(SOLID_VPHYSICS, 0, false) == NULL)
	//{
	//	DevMsg("prop_glados_core unable to spawn physics object!\n");
	//}

	ResetSequence(LookupSequence("drop"));
	PropSetAnim("drop");

	switch (m_CoreType){

	case CORETYPE_MORALITY:
		m_nSkin = CORESKIN_MORALITY;
		break;

	case CORETYPE_CURIOUSITY:
		m_nSkin = CORESKIN_CURIOUSITY;
		break;

	case CORETYPE_ANGER:
		m_nSkin = CORESKIN_ANGER;
		break;

	case CORETYPE_CAKE:
		m_nSkin = CORESKIN_CAKE;
		break;

	case CORETYPE_CUSTOM:
		PropSetAnim(m_customDropAnim);
		break;
	}

	m_isActive = false;
	m_isTurning = false;

	//initialize arrays of voice line durations
	setupCoreVoiceLineDurations(voiceLineDurationsCurious, std::string(CORE_SOUNDFILENAME_CURIOUSITY), 18);
	setupCoreVoiceLineDurations(voiceLineDurationsAnger, std::string(CORE_SOUNDFILENAME_ANGER), 21);
	setupCoreVoiceLineDurations(voiceLineDurationsCake, std::string(CORE_SOUNDFILENAME_CAKE), 41);
	setupCoreVoiceLineDurations(voiceLineDurationsCustom, std::string(CORE_SOUNDFILENAME_CAKE), m_customNumberOfSoundsInLoop);

	SetThink(&CPropGladosCore::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CPropGladosCore::Think(void){
	AnimThink();

	//This if activates after the turn animation has ended.
	//It sets each cores special animation, and toggles the isActive flag on.
	if (m_isTurning == true && m_animJustEnded == true && m_isActive == false){
		m_isTurning = false;
		m_isActive = true;

		switch (m_CoreType){

		case CORETYPE_MORALITY:
			PropSetAnim("idle");
			m_iszDefaultAnim = castable_string_t("idle");
			break;

		case CORETYPE_CURIOUSITY:
			PropSetAnim("look_02");
			m_iszDefaultAnim = castable_string_t("look_02");
			break;

		case CORETYPE_ANGER:
			PropSetAnim("look_03");
			m_iszDefaultAnim = castable_string_t("look_03");
			break;

		case CORETYPE_CAKE:
			PropSetAnim("look_04");
			m_iszDefaultAnim = castable_string_t("look_04");
			break;

		case CORETYPE_CUSTOM:
			PropSetAnim(m_customLookAnim);
			m_iszDefaultAnim = castable_string_t(m_customLookAnim);
			break;
		}
	}

	//If we are active, and last voice line time + voice line delay is less then current time, AND we are not the morality core
	if (m_isActive && (gpGlobals->curtime) >= m_lastVoiceLineTime + m_speechDelay + m_voiceLineDuration && m_CoreType != CORETYPE_MORALITY){
		
		//setup core name string variable
		std::string coreNameString;

		//Setup bool for whether file has a leading zero
		bool hasLeadingZero = true;

		//setup for each vocal core type. Resets voiceLineNumber, save correct line duration, and sets core name string.
		if (m_CoreType == CORETYPE_CURIOUSITY){
			coreNameString = CORE_SOUNDSCRIPTNAME_CURIOUSITY;

			if (m_voiceLineNumber >= 17){
				m_voiceLineNumber = 0;
			}
			//The curiousity cores voice lines are strange
			//line 14 is missing, and line 15 is a panic input line, not normal speech.
			//keep in mind that the code uses voice line number - 1
			else if (m_voiceLineNumber == 13){
				m_voiceLineNumber = 15;
			}

			hasLeadingZero = false;
			m_voiceLineDuration = voiceLineDurationsCurious[m_voiceLineNumber];
		}
		else if (m_CoreType == CORETYPE_ANGER){
			coreNameString = CORE_SOUNDSCRIPTNAME_ANGER;

			if (m_voiceLineNumber >= 21){
				m_voiceLineNumber = 0;
			}

			hasLeadingZero = true;
			m_voiceLineDuration = voiceLineDurationsAnger[m_voiceLineNumber];
		}
		else if (m_CoreType == CORETYPE_CAKE){
			coreNameString = CORE_SOUNDSCRIPTNAME_CAKE;

			if (m_voiceLineNumber >= 41){
				m_voiceLineNumber = 0;
			}

			hasLeadingZero = true;
			m_voiceLineDuration = voiceLineDurationsCake[m_voiceLineNumber];
		}
		else if (m_CoreType == CORETYPE_CUSTOM){

			if (m_voiceLineNumber >= m_customNumberOfSoundsInLoop){
				m_voiceLineNumber = 0;
			}

			hasLeadingZero = m_customSoundName_HasLeadingZero;
			m_voiceLineDuration = voiceLineDurationsCustom[m_voiceLineNumber];
		}

		//play the appropriate voice line based on correct core name string.
		playCoreVoiceLine(coreNameString, m_voiceLineNumber, hasLeadingZero);
		m_voiceLineNumber++;	//increment voice line number

		//save the time we played this line at to help time the next one.
		m_lastVoiceLineTime = gpGlobals->curtime;
	}

	BaseClass::ClearFlagsThink();

	// Think at 20Hz
	SetNextThink(gpGlobals->curtime + 0.05f);
}

//when player or physgun picks this up, do something
void CPropGladosCore::OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason){
	if (m_isActive == false && m_isTurning == false){
		
		if (m_CoreType >= CORETYPE_CUSTOM){
			PropSetAnim(m_customTurnAnim);
		}
		else {
			PropSetAnim("turn");
		}

		m_isTurning = true;
	}

	BaseClass::OnPhysGunPickup(pPhysGunUser, reason);
}

//------------------------------------------------------------------------------
// Purpose: Copied from CDynamicProp. Modified to do things when the turn animation is over
//------------------------------------------------------------------------------
void CPropGladosCore::AnimThink(void)
{
	m_animJustEnded = false;

	if (m_nPendingSequence != -1)
	{
		FinishSetSequence(m_nPendingSequence);
		m_nPendingSequence = -1;
	}

	if (m_bRandomAnimator && m_flNextRandAnim < gpGlobals->curtime)
	{
		ResetSequence(SelectWeightedSequence(ACT_IDLE));
		ResetClientsideFrame();

		// Fire output
		m_pOutputAnimBegun.FireOutput(NULL, this);

		m_flNextRandAnim = gpGlobals->curtime + random->RandomFloat(m_flMinRandAnimTime, m_flMaxRandAnimTime);
	}

	if (((m_iTransitionDirection > 0 && GetCycle() >= 0.999f) || (m_iTransitionDirection < 0 && GetCycle() <= 0.0f)) && !SequenceLoops())
	{
		Assert(m_iGoalSequence >= 0);
		if (GetSequence() != m_iGoalSequence)
		{
			PropSetSequence(m_iGoalSequence);
		}
		else
		{
			// Fire output
			m_pOutputAnimOver.FireOutput(NULL, this);
			m_animJustEnded = true;

			// If I'm a random animator, think again when it's time to change sequence
			if (m_bRandomAnimator)
			{
				SetNextThink(gpGlobals->curtime + m_flNextRandAnim + 0.1);
			}
			else
			{
				if (m_iszDefaultAnim != NULL_STRING)
				{
					PropSetAnim(STRING(m_iszDefaultAnim));
				}
			}
		}
	}
	else
	{
		SetNextThink(gpGlobals->curtime + 0.1f);
	}

	StudioFrameAdvance();
	DispatchAnimEvents(this);
	//m_BoneFollowerManager.UpdateBoneFollowers(this);
}

//fills arrays for durations of voice lines
void CPropGladosCore::setupCoreVoiceLineDurations(float voiceLineDurations[], std::string coreNameString, int numOfLines){
	for (int i = 0; i < numOfLines; i++){

		std::string numeralString = std::to_string(i+1);

		if (i + 1 < 10 && m_CoreType != CORETYPE_CUSTOM){
			numeralString = "0" + numeralString;
		}
		else if (i + 1 < 10 && m_customFileName_HasLeadingZero){
			numeralString = "0" + numeralString;
		}

		std::string soundStartString = "vo/aperture_ai/escape_02_sphere_";
		//std::string coreNameString = "curiosity";
		std::string dash = "-";
		//number
		std::string soundEndString = ".wav";

		std::string finalSoundName = soundStartString + coreNameString + dash + numeralString + soundEndString;

		//if custom core type, use hammer values instead.
		if (m_CoreType == CORETYPE_CUSTOM){
			finalSoundName = m_customFileName + numeralString + m_customFileNameEnding;
		}

		voiceLineDurations[i] = enginesound->GetSoundDuration(finalSoundName.c_str());
		//DevMsg(finalSoundName.c_str());
		//DevMsg("\n");
	}
}

void CPropGladosCore::playCoreVoiceLine(std::string coreNameString, int lineNum, bool leadingZero){
	std::string numeralString = std::to_string(lineNum+1);

	if (lineNum+1 < 10 && leadingZero){
		numeralString = "0" + numeralString;
	}

	std::string soundStartString = "Portal.Glados_core.";
	//std::string coreNameString = "curiosity";
	std::string dash = "_";
	//number

	std::string finalSoundName = soundStartString + coreNameString + dash + numeralString;

	//if custom core type, use hammer values instead.
	if (m_CoreType == CORETYPE_CUSTOM){
		finalSoundName = m_customSoundName + numeralString + m_customSoundNameEnding;
	}

	DevMsg("playing glados core sound: ");
	DevMsg(finalSoundName.c_str());
	DevMsg("\n");

	EmitSound(finalSoundName.c_str());
}

void CPropGladosCore::precacheCoreVoiceLines(std::string coreNameString, int lineNum, bool leadingZero){
	for (int i = 0; i < lineNum; i++){
		std::string numeralString = std::to_string(i + 1);

		if (i + 1 < 10 && leadingZero){
			numeralString = std::string("0") + numeralString;
		}

		std::string soundStartString = "Portal.Glados_core.";
		//std::string coreNameString = "curiosity";
		std::string dash = "_";
		//number

		std::string finalSoundName = soundStartString + coreNameString + dash + numeralString;

		//if custom core type, use hammer values instead.
		if (m_CoreType == CORETYPE_CUSTOM){
			finalSoundName = m_customSoundName + numeralString + m_customSoundNameEnding;
		}

		DevMsg("precaching glados core sound: ");
		DevMsg(finalSoundName.c_str());
		DevMsg("\n");

		PrecacheScriptSound(finalSoundName.c_str());
	}
}

/**
*	Everything below here is copy paste from prop_dynamic. Cross your fingers!
*/

//-----------------------------------------------------------------------------
// Purpose: Helper in case we have to async load the sequence
// Input  : nSequence - 
//-----------------------------------------------------------------------------
void CPropGladosCore::FinishSetSequence(int nSequence)
{
	// Msg("%.2f CPropGladosCore::FinishSetSequence( %d )\n", gpGlobals->curtime, nSequence );
	SetCycle(0);
	m_flAnimTime = gpGlobals->curtime;
	ResetSequence(nSequence);
	ResetClientsideFrame();
	RemoveFlag(FL_STATICPROP);
	SetPlaybackRate(m_iTransitionDirection > 0 ? 1.0f : -1.0f);
	SetCycle(m_iTransitionDirection > 0 ? 0.0f : 0.999f);
}

//-----------------------------------------------------------------------------
// Purpose: Sets the sequence and starts thinking.
// Input  : nSequence - 
//-----------------------------------------------------------------------------
void CPropGladosCore::PropSetSequence(int nSequence)
{
	m_iGoalSequence = nSequence;

	// Msg("%.2f CPropGladosCore::PropSetSequence( %d (%d:%.1f:%.3f)\n", gpGlobals->curtime, nSequence, GetSequence(), GetPlaybackRate(), GetCycle() );

	int nNextSequence;
	float nextCycle;
	//float flInterval = 0.1f;

	if (GotoSequence(GetSequence(), GetCycle(), GetPlaybackRate(), m_iGoalSequence, nNextSequence, nextCycle, m_iTransitionDirection))
	{
		FinishSetSequence(nNextSequence);
	}

	//SetThink(&CPropGladosCore::AnimThink);
	//if (GetNextThink() <= gpGlobals->curtime)
	//	SetNextThink(gpGlobals->curtime + flInterval);
}

// NOTE: To avoid risk, currently these do nothing about collisions, only visually on/off
void CPropGladosCore::InputTurnOn(inputdata_t &inputdata)
{
	RemoveEffects(EF_NODRAW);
}

void CPropGladosCore::InputTurnOff(inputdata_t &inputdata)
{
	AddEffects(EF_NODRAW);
}

void CPropGladosCore::InputDisableCollision(inputdata_t &inputdata)
{
	AddSolidFlags(FSOLID_NOT_SOLID);
}

void CPropGladosCore::InputEnableCollision(inputdata_t &inputdata)
{
	RemoveSolidFlags(FSOLID_NOT_SOLID);
}

//------------------------------------------------------------------------------
// Purpose: Sets an animation by sequence name or activity name.
//------------------------------------------------------------------------------
void CPropGladosCore::PropSetAnim(const char *szAnim)
{
	if (!szAnim)
		return;

	int nSequence = LookupSequence(szAnim);

	// Set to the desired anim, or default anim if the desired is not present
	if (nSequence > ACTIVITY_NOT_AVAILABLE)
	{
		PropSetSequence(nSequence);

		// Fire output
		m_pOutputAnimBegun.FireOutput(NULL, this);
	}
	else
	{
		// Not available try to get default anim
		Warning("Dynamic prop %s: no sequence named:%s\n", GetDebugName(), szAnim);
		SetSequence(0);
	}
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CPropGladosCore::InputSetAnimation(inputdata_t &inputdata)
{
	PropSetAnim(inputdata.value.String());
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CPropGladosCore::InputSetDefaultAnimation(inputdata_t &inputdata)
{
	m_iszDefaultAnim = inputdata.value.StringID();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPropGladosCore::InputSetPlaybackRate(inputdata_t &inputdata)
{
	SetPlaybackRate(inputdata.value.Float());
}