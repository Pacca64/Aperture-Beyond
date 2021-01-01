#define CORE_MODEL_NAME "models/props_bts/glados_ball_reference.mdl"

/**
*	Skin defintions
*/
#define	CORESKIN_MORALITY 0			//The Morality Core Skin.
#define	CORESKIN_CURIOUSITY 1		//The Curiousity Core Skin.
#define	CORESKIN_ANGER 2			//The Anger Core Skin.
#define	CORESKIN_CAKE 3				//The Cake/logic Core Skin.

class CPropGladosCore : public CDynamicProp
{
public:
	DECLARE_CLASS(CPropGladosCore, CDynamicProp);

	void Spawn();

private:
	void Precache(void);

	void Think(void);

	int m_speechDelay;
	int m_CoreType;

	DECLARE_DATADESC();
};