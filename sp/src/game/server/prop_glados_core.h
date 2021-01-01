#include <string>

#define CORE_MODEL_NAME "models/props_bts/glados_ball_reference.mdl"

/**
*	Core Type defintions
*/
#define	CORETYPE_CURIOUSITY 0		//The Curiousity Core.
#define	CORETYPE_ANGER 1			//The Anger Core.
#define	CORETYPE_CAKE 2				//The Cake/logic Core.
#define	CORETYPE_MORALITY 3			//The Morality Core.

/**
*	Skin defintions
*/
#define	CORESKIN_MORALITY 0			//The Morality Core Skin.
#define	CORESKIN_CURIOUSITY 1		//The Curiousity Core Skin.
#define	CORESKIN_ANGER 2			//The Anger Core Skin.
#define	CORESKIN_CAKE 3				//The Cake/logic Core Skin.

/**
*	Core sound name definitions
*/
#define CORE_SOUNDSCRIPTNAME_CURIOUSITY "Curiosity"
#define CORE_SOUNDSCRIPTNAME_ANGER "Aggressive"
#define CORE_SOUNDSCRIPTNAME_CAKE "Crazy"

#define CORE_SOUNDFILENAME_CURIOUSITY "curiosity"
#define CORE_SOUNDFILENAME_ANGER "anger"
#define CORE_SOUNDFILENAME_CAKE "cakemix"

class CPropGladosCore : public CDynamicProp
{
public:
	DECLARE_CLASS(CPropGladosCore, CDynamicProp);

	void Spawn();

private:
	void Precache(void);

	void Think(void);
	void AnimThink(void);
	void OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason);

	void setupCoreVoiceLineDurations(float [], std::string, int);
	void playCoreVoiceLine(std::string, int, bool);
	void precacheCoreVoiceLines(std::string, int, bool);

	float m_speechDelay;
	int m_CoreType;

	float m_lastVoiceLineTime;
	float m_voiceLineDuration;

	int m_voiceLineNumber;

	bool m_isActive;
	bool m_isTurning;
	bool m_animJustEnded;

	float voiceLineDurationsCurious[18];
	float voiceLineDurationsAnger[21];
	float voiceLineDurationsCake[41];

	DECLARE_DATADESC();
};