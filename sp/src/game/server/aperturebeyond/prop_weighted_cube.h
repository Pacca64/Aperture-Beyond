#define	CUBETYPE_STANDARD 0
#define	CUBETYPE_COMPANION 1
#define	CUBETYPE_LASER 2
#define	CUBETYPE_SPHERE 3
#define	CUBETYPE_ANTIQUE 4

#define CUBEVARIANT_CLEAN 0
#define CUBEVARIANT_RUSTED 1

#define CUBEPAINT_BOUNCE 0
#define CUBEPAINT_STICK 1
#define CUBEPAINT_SPEED 2
#define CUBEPAINT_PORTAL 3
#define CUBEPAINT_NONE 4

#define CUBE_MODEL_NAME "models/props/metal_box.mdl"
#define CUBE_LASER_MODEL_NAME "models/props/reflection_cube.mdl"
#define CUBE_SPHERE_MODEL_NAME "models/props_gameplay/mp_ball.mdl"
#define CUBE_ANTIQUE_MODEL_NAME "models/props_underground/underground_weighted_cube.mdl"

/**
*	Skin defintions
*	The cube models have LOTS of alternate skins, best to clean things up. Some are direct values, some are to be added too.
*/

#define	CUBESKIN_STANDARD_DEFAULT 0			//"Normal" skin for normal cubes. Works for others too, obviously.
#define	CUBESKIN_STANDARD_ON 2				//"Activated" skin for normal cubes.
#define	CUBESKIN_STANDARD_BOUNCE 6			//Bounce paint skin for normal cubes.
#define	CUBESKIN_STANDARD_SPEED 7			//Speed paint skin for normal cubes.
#define	CUBESKIN_STANDARD_BOUNCE_ON 10		//Bounce paint skin for "Activated" normal cubes.
#define	CUBESKIN_STANDARD_SPEED_ON 11		//Speed paint skin for "Activated" normal cubes.

#define	CUBESKIN_STANDARD_RUST_DEFAULT 3	//"Normal" skin for normal rusty cubes.
#define	CUBESKIN_STANDARD_RUST_ON 5			//"Activated" skin for normal rusty cubes.

#define	CUBESKIN_COMPANION_DEFAULT 1		//"Normal" skin for companion cubes.
#define	CUBESKIN_COMPANION_ON 4				//"Activated" skin for companion cubes.
#define	CUBESKIN_COMPANION_BOUNCE 8			//Bounce paint skin for companion cubes.
#define	CUBESKIN_COMPANION_SPEED 9			//Speed paint skin for companion cubes.

#define CUBESKIN_SPHERE_DEFAULT 0			//Normal Skin for sphere cubes.
#define	CUBESKIN_SPHERE_ON 1				//"Activated" skin for sphere cubes.
#define	CUBESKIN_SPHERE_BOUNCE 2			//Bounce paint skin for sphere cubes.
#define	CUBESKIN_SPHERE_SPEED 3			//Speed paint skin for sphere cubes.

#define	CUBESKIN_LASER_DEFAULT 0			//"Normal" skin for laser cubes. Works for others too, obviously.
#define	CUBESKIN_LASER_BOUNCE 2			//Bounce paint skin for laser cubes.
#define	CUBESKIN_LASER_SPEED 3			//Speed paint skin for laser cubes.

#define	CUBESKIN_LASER_RUST_DEFAULT 1	//"Normal" skin for normal rusty cubes.

#define	CUBESKIN_ANTIQUE_DEFAULT 0			//"Normal" skin for antique cubes. Works for others too, obviously.
#define	CUBESKIN_ANTIQUE_BOUNCE 1			//Bounce paint skin for antique cubes.
#define	CUBESKIN_ANTIQUE_SPEED 2			//Speed paint skin for antique cubes.

class CPortalCube : public CPhysicsProp
{
public:
	DECLARE_CLASS(CPortalCube, CPhysicsProp);

	void Spawn();
	void ActivateCube();	//Turn on this cubes "activated" skin
	void DeactivateCube();	//Turn off this cubes "activated" skin

	void SetActiveCube();	//resets reset time for cubes active skin turning off.

	bool IsCubic();	//Used for prop_floor_box_button. Returns true for all types except sphere.
	bool IsSphere();	//Used for prop_floor_ball_button. Returns true for sphere type cubes.

	bool IsLaser();		//true if this is a laser cube
	bool IsCompanion();	//true if this is a companion cube
	bool IsAntique();

private:
	void Precache(void);
	
	void Think(void);

	int m_isRusted;
	int m_PaintPower;
	int m_CubeType;

	int	m_resetCubeSkinFrames;

	DECLARE_DATADESC();
};