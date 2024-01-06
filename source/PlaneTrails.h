#pragma once
#include "CVector.h"
class CPlaneBanner
{
	CVector m_pos[8];
public:
	void Init(void);
	void Update(void);
	void Render(void);
	void RegisterPoint(CVector pos);
};

class CPlaneBanners
{
	static CPlaneBanner aArray[50];
public:
	static void Init(void);
	static void Update(void);
	static void Render(void);
	static void RegisterPoint(CVector pos, uint32_t id);
	static void Shutdown();
};

class CPlaneTrail2
{
	CVector m_pos[16];
	int32_t m_time[16];
public:
	void Init(void);
	void Render(float visibility);
	void RegisterPoint(CVector pos);
};

class CPlaneTrails2
{
	static CPlaneTrail2 aArray[50];	// NB: 3 CPlanes and 3 hardcoded far away ones
public:
	static void Init(void);
	static void Update(void);
	static void Render(void);
	static void RegisterPoint(CVector pos, uint32_t id);
};