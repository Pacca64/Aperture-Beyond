#include <string>

#define CORE_MODEL_NAME "models/props_bts/glados_ball_reference.mdl"

/**
*	Core Type defintions
*/
#define	CORETYPE_CURIOUSITY 0		//The Curiousity Core.
#define	CORETYPE_ANGER 1			//The Anger Core.
#define	CORETYPE_CAKE 2				//The Cake/logic Core.
#define	CORETYPE_MORALITY 3			//The Morality Core.
#define	CORETYPE_CUSTOM 4			//Custom. Enables use of extra keyvalues.

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

class CPropGladosCore : public CPhysicsProp
{
public:
	DECLARE_CLASS(CPropGladosCore, CPhysicsProp);

	void Spawn();

	//Custom stuff for hammer users to play with
	//custom animation names
	const char *m_customTurnAnim;	
	const char *m_customLookAnim;
	const char *m_customDropAnim;
	//custom sound file names
	const char *m_customFileName;	//the custom file name. Should be the part before the number.
	const char *m_customFileNameEnding;	//file name ending. FGD should default to ".wav". Don't think any other file type works.
	bool	m_customFileName_HasLeadingZero;//whether the number in the file name should have a leading zero
	//custom game sound names
	const char *m_customSoundName;	//the custom game sound name. Should be the part before the number.
	const char *m_customSoundNameEnding;	//game sound name ending. Can be left blank.
	bool	m_customSoundName_HasLeadingZero; //whether the number in the sound name should have a leading zero
	//Other sound info
	int m_customNumberOfSoundsInLoop;


	//copied prop_dynamic routines
	void	PropSetSequence(int nSequence);

	// Input handlers
	void InputSetAnimation(inputdata_t &inputdata);
	void InputSetDefaultAnimation(inputdata_t &inputdata);
	void InputTurnOn(inputdata_t &inputdata);
	void InputTurnOff(inputdata_t &inputdata);
	void InputDisableCollision(inputdata_t &inputdata);
	void InputEnableCollision(inputdata_t &inputdata);
	void InputSetPlaybackRate(inputdata_t &inputdata);

	COutputEvent		m_pOutputAnimBegun;
	COutputEvent		m_pOutputAnimOver;

	string_t			m_iszDefaultAnim;

	int					m_iGoalSequence;
	int					m_iTransitionDirection;

	// Random animations
	bool				m_bRandomAnimator;
	float				m_flNextRandAnim;
	float				m_flMinRandAnimTime;
	float				m_flMaxRandAnimTime;
	short				m_nPendingSequence;

	bool				m_bStartDisabled;
	bool				m_bDisableBoneFollowers;

private:
	void Precache(void);

	void Think(void);
	void AnimThink(void);
	void OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason);

	void setupCoreVoiceLineDurations(float [], std::string, int);
	void playCoreVoiceLine(std::string, int, bool);
	void precacheCoreVoiceLines(std::string, int, bool);

	//copied prop_dynamic routines
	void FinishSetSequence(int nSequence);
	void PropSetAnim(const char *szAnim);

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
	float voiceLineDurationsCustom[200];

	DECLARE_DATADESC();
};