//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "materialsystem/imaterialproxy.h"
#include "materialsystem/imaterial.h"
#include "functionproxy.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Debug visualization
ConVar	lightedmouth("lightedmouth", "1");

class CLightedmouthProxy : public CResultProxy
{
public:
	CLightedmouthProxy();
	virtual ~CLightedmouthProxy();
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);
	virtual void Release(void) { delete this; }
	virtual IMaterial *GetMaterial() { return NULL; }
};

CLightedmouthProxy::CLightedmouthProxy()
{
	//DevMsg(1, "CLightedmouthProxy::CLightedmouthProxy()\n");
}

CLightedmouthProxy::~CLightedmouthProxy()
{
	//DevMsg(1, "CLightedmouthProxy::~CLightedmouthProxy()\n");
}


bool CLightedmouthProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	//DevMsg(1, "CLightedmouthProxy::Init( material = \"%s\" )\n", pMaterial->GetName());

	return CResultProxy::Init(pMaterial, pKeyValues);
}

void CLightedmouthProxy::OnBind(void *pC_BaseEntity)
{
	//DevMsg(1, "CLightedmouthProxy::OnBind( %p )\n", pC_BaseEntity);

	if (!pC_BaseEntity)
		return;

	//CResultProxy::OnBind(pC_BaseEntity);

	//Get an entity from our input object (some clientside renderable thing)
	//C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);

	m_pResult->SetFloatValue(lightedmouth.GetFloat());
}

EXPOSE_INTERFACE(CLightedmouthProxy, IMaterialProxy, "LightedMouth" IMATERIAL_PROXY_INTERFACE_VERSION);
