#include "cbase.h"
#include "props.h"
#include <string>
#include "prop_weighted_cube.h"

BEGIN_DATADESC(CPortalCube)

DEFINE_FIELD(m_resetCubeSkinFrames, FIELD_INTEGER),

DEFINE_KEYFIELD(m_isRusted, FIELD_INTEGER, "SkinType"),
DEFINE_KEYFIELD(m_PaintPower, FIELD_INTEGER, "PaintPower"),
DEFINE_KEYFIELD(m_CubeType, FIELD_INTEGER, "CubeType"),

// Declare our think function
DEFINE_THINKFUNC(Think),

END_DATADESC()


LINK_ENTITY_TO_CLASS(prop_weighted_cube, CPortalCube);

void CPortalCube::Precache(void)
{
	const char *pModelName = STRING(GetModelName());

	if (pModelName && pModelName[0] != '\0'){
		pModelName = pModelName;
	} else {
		switch (m_CubeType){

		case CUBETYPE_STANDARD:
			pModelName = CUBE_MODEL_NAME;
			break;

		case CUBETYPE_COMPANION:
			pModelName = CUBE_MODEL_NAME;
			break;

		case CUBETYPE_LASER:
			pModelName = CUBE_LASER_MODEL_NAME;
			break;

		case CUBETYPE_SPHERE:
			pModelName = CUBE_SPHERE_MODEL_NAME;
			break;

		case CUBETYPE_ANTIQUE:
			pModelName = CUBE_ANTIQUE_MODEL_NAME;
			break;
		}
		
	}

	PrecacheModel(pModelName);

	BaseClass::Precache();
}

void CPortalCube::Spawn()
{
	Precache();

	const char *pModelName = STRING(GetModelName());

	if (pModelName && pModelName[0] != '\0'){
		pModelName = pModelName;
	}
	else {
		switch (m_CubeType){

		case CUBETYPE_STANDARD:
			pModelName = CUBE_MODEL_NAME;
			if (m_isRusted){
				m_nSkin = CUBESKIN_STANDARD_RUST_DEFAULT;
			}
			break;

		case CUBETYPE_COMPANION:
			pModelName = CUBE_MODEL_NAME;
			m_nSkin = CUBESKIN_COMPANION_DEFAULT;
			break;

		case CUBETYPE_LASER:
			pModelName = CUBE_LASER_MODEL_NAME;
			if (m_isRusted){
				m_nSkin = CUBESKIN_LASER_RUST_DEFAULT;
			}
			break;

		case CUBETYPE_SPHERE:
			pModelName = CUBE_SPHERE_MODEL_NAME;
			break;

		case CUBETYPE_ANTIQUE:
			pModelName = CUBE_ANTIQUE_MODEL_NAME;
			break;
		}

	}

	SetModel(pModelName);

	BaseClass::Spawn();

	AddSpawnFlags(SF_PHYSPROP_ENABLE_PICKUP_OUTPUT);	//set flag to enable player pickup for this physics prop

	SetThink(&CPortalCube::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CPortalCube::Think(void){
	if (m_resetCubeSkinFrames > -1){
		m_resetCubeSkinFrames--;
	}

	if (m_resetCubeSkinFrames == 0){
		DeactivateCube();
	}
	
	SetNextThink(gpGlobals->curtime + 0.05f);
}

void CPortalCube::SetActiveCube(){
	m_resetCubeSkinFrames = 2;
}

void CPortalCube::ActivateCube(){
	switch (m_CubeType){

	case CUBETYPE_STANDARD:
		if (m_isRusted){
			m_nSkin = CUBESKIN_STANDARD_RUST_ON;
		}
		else {
			m_nSkin = CUBESKIN_STANDARD_ON;
		}
		break;

	case CUBETYPE_COMPANION:
		m_nSkin = CUBESKIN_COMPANION_ON;
		break;

	case CUBETYPE_LASER:
		break;

	case CUBETYPE_SPHERE:
		m_nSkin = CUBESKIN_SPHERE_ON;
		break;

	case CUBETYPE_ANTIQUE:
		break;
	}
}

void CPortalCube::DeactivateCube(){
	switch (m_CubeType){

	case CUBETYPE_STANDARD:
		if (m_isRusted){
			m_nSkin = CUBESKIN_STANDARD_RUST_DEFAULT;
		}
		else {
			m_nSkin = CUBESKIN_STANDARD_DEFAULT;
		}
		break;

	case CUBETYPE_COMPANION:
		m_nSkin = CUBESKIN_COMPANION_DEFAULT;
		break;

	case CUBETYPE_LASER:
		break;

	case CUBETYPE_SPHERE:
		m_nSkin = CUBESKIN_SPHERE_DEFAULT;
		break;

	case CUBETYPE_ANTIQUE:
		break;
	}
}

//Used for prop_floor_box_button. Returns true for all types except sphere.
bool CPortalCube::IsCubic(void){
	if (m_CubeType == CUBETYPE_SPHERE){
		return false;
	}

	return true;
}	

//Used for prop_floor_ball_button. Returns true for sphere type cubes.
bool CPortalCube::IsSphere(){
	if (m_CubeType == CUBETYPE_SPHERE){
		return true;
	}

	return false;
}

//true if this is a laser cube
bool CPortalCube::IsLaser(){
	if (m_CubeType == CUBETYPE_LASER){
		return true;
	}

	return false;
}

//true if this is a companion cube
bool CPortalCube::IsCompanion(){
	if (m_CubeType == CUBETYPE_COMPANION){
		return true;
	}

	return false;
}

bool CPortalCube::IsAntique(){
	if (m_CubeType == CUBETYPE_ANTIQUE){
		return true;
	}

	return false;
}
