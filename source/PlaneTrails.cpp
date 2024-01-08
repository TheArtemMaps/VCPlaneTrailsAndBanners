#include "PlaneTrails.h"
#include "plugin.h"
#include "CCamera.h"
#include "RenderBuffer.h"
#include "CCoronas.h"
#include "CWeather.h"
#include "CClock.h"
#include "CTimeCycle.h"
#include "Utility.h"
#include "CTxdStore.h"
#include "CCutsceneMgr.h"
#include "ePedBones.h"
#define FIX_BUGS // Undefine to play with bugs
#define PI (float)M_PI
#define TWOPI (PI*2)
#define ARRAY_SIZE(array)                (sizeof(array) / sizeof(array[0]))
RwTexture* gpBannerTexture;
// Trails from vc look better in my opinion
struct tIniData {
	bool EnableBanners;
	bool EnableTrails;
	float Height;
	int ModelIDForTrails;
	int ModelIDForBanners;
	bool ReplaceTrailsInTheSky;
	float HeightForBanner;
	bool AppearOnlyAtAltitude;
	bool AppearWhenPilotIsInside;
	bool AlwaysAppear;
	float DrawDistance;
	float TrailFadeTime;
};

tIniData Plane;
CPlaneBanner CPlaneBanners::aArray[50];
void
CPlaneBanner::Init(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(m_pos); i++) {
		m_pos[i].x = i;
		m_pos[i].y = 0.0f;
		m_pos[i].z = -60.0f;
	}
	CTxdStore::PushCurrentTxd();
	int32_t slot2 = CTxdStore::AddTxdSlot("Banner");
	CTxdStore::LoadTxd(slot2, PLUGIN_PATH((char*)"MODELS\\BANNER.TXD"));
	int32_t slot = CTxdStore::FindTxdSlot("Banner");
	CTxdStore::SetCurrentTxd(slot);
	gpBannerTexture = RwTextureRead("Banner", NULL);
	CTxdStore::PopCurrentTxd();
}

void
CPlaneBanner::Update(void)
{
	int i;
	if (m_pos[0].z > -50.0f) {
		m_pos[0].z -= 0.05f * CTimer::ms_fTimeStep;
		m_pos[0].z = max(m_pos[0].z, -100.0f);
		for (i = 1; i < ARRAY_SIZE(m_pos); i++) {
			CVector dist = m_pos[i] - m_pos[i - 1];
			float len = dist.Magnitude();
			if (len > 8.0f)
				m_pos[i] = m_pos[i - 1] + dist / len * 8.0f;
		}
	}
}

void
CPlaneBanner::Render(void)
{
	int i;
	if (m_pos[0].z > -50.0f) {
		float camDist = (TheCamera.GetPosition() - m_pos[0]).Magnitude();
		if (TheCamera.IsSphereVisible(m_pos[4], 32.0f) && camDist < Plane.DrawDistance) {
			TempBufferVerticesStored = 0;
			TempBufferIndicesStored = 0;
			int alpha = camDist < 250.0f ? 160 : (Plane.DrawDistance - camDist) / (Plane.DrawDistance - 250.0f) * 160;

			TempBufferVerticesStored += 2;
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[0], 255, 255, 255, alpha);
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[1], 255, 255, 255, alpha);
			RwIm3DVertexSetPos(&TempBufferRenderVertices[0], m_pos[2].x, m_pos[2].y, m_pos[2].z);
			RwIm3DVertexSetPos(&TempBufferRenderVertices[1], m_pos[2].x, m_pos[2].y, m_pos[2].z - 4.0f);
			RwIm3DVertexSetU(&TempBufferRenderVertices[0], 0.0f);
			RwIm3DVertexSetV(&TempBufferRenderVertices[0], 0.0f);
			RwIm3DVertexSetU(&TempBufferRenderVertices[1], 0.0f);
			RwIm3DVertexSetV(&TempBufferRenderVertices[1], 1.0f);
			for (i = 2; i < 8; i++) {
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored + 0], 255, 255, 255, alpha);
				RwIm3DVertexSetRGBA(&TempBufferRenderVertices[TempBufferVerticesStored + 1], 255, 255, 255, alpha);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored + 0], m_pos[i].x, m_pos[i].y, m_pos[i].z);
				RwIm3DVertexSetPos(&TempBufferRenderVertices[TempBufferVerticesStored + 1], m_pos[i].x, m_pos[i].y, m_pos[i].z - 4.0f);
				RwIm3DVertexSetU(&TempBufferRenderVertices[TempBufferVerticesStored + 0], (i - 2) / 5.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[TempBufferVerticesStored + 0], 0.0f);
				RwIm3DVertexSetU(&TempBufferRenderVertices[TempBufferVerticesStored + 1], (i - 2) / 5.0f);
				RwIm3DVertexSetV(&TempBufferRenderVertices[TempBufferVerticesStored + 1], 1.0f);
				TempBufferRenderIndexList[TempBufferIndicesStored + 0] = TempBufferVerticesStored - 2;
				TempBufferRenderIndexList[TempBufferIndicesStored + 1] = TempBufferVerticesStored - 1;
				TempBufferRenderIndexList[TempBufferIndicesStored + 2] = TempBufferVerticesStored + 1;
				TempBufferRenderIndexList[TempBufferIndicesStored + 3] = TempBufferVerticesStored - 2;
				TempBufferRenderIndexList[TempBufferIndicesStored + 4] = TempBufferVerticesStored + 1;
				TempBufferRenderIndexList[TempBufferIndicesStored + 5] = TempBufferVerticesStored;
				TempBufferVerticesStored += 2;
				TempBufferIndicesStored += 6;
			}
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
			RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
			RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
			RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
			RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
			RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpBannerTexture));

#ifdef FIX_BUGS
				if (RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, NULL, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXUV | rwIM3D_VERTEXRGBA)) {
#else
				if (RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, NULL, 0)) {
#endif
					RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored);
					RwIm3DEnd();
				}

				RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
				RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
				RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
				RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);

				TempBufferVerticesStored = 0;
				TempBufferIndicesStored = 0;
			}
		}
	}


void
CPlaneBanner::RegisterPoint(CVector pos)
{
	m_pos[0] = pos;
}

void
CPlaneBanners::Init(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(aArray); i++)
		aArray[i].Init();
}

void
CPlaneBanners::Update(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(aArray); i++)
		aArray[i].Update();
}

void
CPlaneBanners::Render(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(aArray); i++)
		aArray[i].Render();
}

void CPlaneBanners::Shutdown(void) {
	gpBannerTexture = nullptr;
	RwTextureDestroy(gpBannerTexture);
}

void
CPlaneBanners::RegisterPoint(CVector pos, uint32_t id)
{
	aArray[id].RegisterPoint(pos);
}

CPlaneTrail2 CPlaneTrails2::aArray[50];
RwImVertexIndex TrailIndices[32] = {
	0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
	10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16
};

void
CPlaneTrail2::Init(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(m_time); i++)
		m_time[i] = 0;
}

void
CPlaneTrail2::Render(float visibility)
{
	int i;
	int numVerts = 0;
	if (!TheCamera.IsSphereVisible(m_pos[0], 1000.0f))
		return;

	int alpha = visibility * 110.0f;
	if (alpha == 0)
		return;

	for (i = 0; i < ARRAY_SIZE(m_pos); i++) {
		int32_t time = CTimer::m_snTimeInMilliseconds - m_time[i];
		if (time > 30000)
			m_time[i] = 0;
		if (m_time[i] != 0) {
			float fade = (30000.0f - time) / Plane.TrailFadeTime;
			fade = min(fade, 1.0f);
			RwIm3DVertexSetRGBA(&TempBufferRenderVertices[numVerts], 255, 255, 255, (int)(alpha * fade));
			RwIm3DVertexSetPos(&TempBufferRenderVertices[numVerts], m_pos[i].x, m_pos[i].y, m_pos[i].z);
			numVerts++;
		}
	}
	if (numVerts > 1) {
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

		if (RwIm3DTransform(TempBufferRenderVertices, numVerts, NULL, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA)) {
			RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, TrailIndices, (numVerts - 1) * 2);
			RwIm3DEnd();
		}
	}
}

void
CPlaneTrail2::RegisterPoint(CVector pos)
{
	int i;
	bool bNewPoint = false;
	if (m_time[0] != 0 && CTimer::m_snTimeInMilliseconds - m_time[0] > 2000) {
		bNewPoint = true;
		for (i = ARRAY_SIZE(m_pos) - 1; i > 0; i--) {
			m_pos[i] = m_pos[i - 1];
			m_time[i] = m_time[i - 1];
		}
	}
	m_pos[0] = pos;
	if (bNewPoint || m_time[0] == 0)
		m_time[0] = CTimer::m_snTimeInMilliseconds;
}

void
CPlaneTrails2::Init(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(aArray); i++)
		aArray[i].Init();
}

void
CPlaneTrails2::Update(void)
{
	CVector planePos;

	planePos.x = 1590.0f * sin((float)(CTimer::m_snTimeInMilliseconds & 0x1FFFF) / 0x20000 * TWOPI);
	planePos.y = 1200.0f * cos((float)(CTimer::m_snTimeInMilliseconds & 0x1FFFF) / 0x20000 * TWOPI);
	planePos.z = 550.0f;
	RegisterPoint(planePos, 3);
	if (CClock::ms_nGameClockHours > 22 || CClock::ms_nGameClockHours < 7) {
		if (CTimer::m_snTimeInMilliseconds & 0x200)
			CCoronas::RegisterCorona(
				101,
				nullptr,
				255, 0, 0, 255,
				planePos,
				5.0f,
				2000.0f,
				eCoronaType::CORONATYPE_HEADLIGHT,
				eCoronaFlareType::FLARETYPE_NONE,
				false,
				false,
				false,
				0.0f,
				false,
				1.5f,
				0,
				15.0f,
				false,
				false
			);
		else
			CCoronas::UpdateCoronaCoors(101, planePos, 2000.0f, 0.0f);
	}

	planePos.x = 1000.0f * sin((float)(CTimer::m_snTimeInMilliseconds & 0x1FFFF) / 0x20000 * TWOPI);
	planePos.y = -1600.0f * cos((float)(CTimer::m_snTimeInMilliseconds & 0x1FFFF) / 0x20000 * TWOPI);
	planePos.z = 500.0f;
	RegisterPoint(planePos, 4);
	if (CClock::ms_nGameClockHours > 22 || CClock::ms_nGameClockHours < 7) {
		if (CTimer::m_snTimeInMilliseconds & 0x200)
			CCoronas::RegisterCorona(
				102,
				nullptr,
				255, 0, 0, 255,
				planePos,
				5.0f,
				2000.0f,
				eCoronaType::CORONATYPE_HEADLIGHT,
				eCoronaFlareType::FLARETYPE_NONE,
				false,
				false,
				false,
				0.0f,
				false,
				1.5f,
				0,
				15.0f,
				false,
				false
			);
		else
			CCoronas::UpdateCoronaCoors(102, planePos, 2000.0f, 0.0f);
	}

	planePos.x = 1100.0f * cos((float)(CTimer::m_snTimeInMilliseconds & 0x1FFFF) / 0x20000 * TWOPI);
	planePos.y = 700.0f * sin((float)(CTimer::m_snTimeInMilliseconds & 0x1FFFF) / 0x20000 * TWOPI);
	planePos.z = 600.0f;
	RegisterPoint(planePos, 5);
	if (CClock::ms_nGameClockHours > 22 || CClock::ms_nGameClockHours < 7) {
		if (CTimer::m_snTimeInMilliseconds & 0x200)
			CCoronas::RegisterCorona(
				103,
				nullptr,
				255, 0, 0, 255,
				planePos,
				5.0f,
				2000.0f,
				eCoronaType::CORONATYPE_HEADLIGHT,
				eCoronaFlareType::FLARETYPE_NONE,
				false,
				false,
				false,
				0.0f,
				false,
				1.5f,
				0,
				15.0f,
				false,
				false
			);
		else
			CCoronas::UpdateCoronaCoors(103, planePos, 2000.0f, 0.0f);
	}
}

void
CPlaneTrails2::Render(void)
{
	int i;
	float visibility = min(1.0f - CWeather::Foggyness, 1.0f - CWeather::CloudCoverage);
	visibility = min(visibility, 1.0f - CWeather::Rain);
	visibility = min(max(max(CTimeCycle::m_CurrentColours.m_nSkyTopRed, CTimeCycle::m_CurrentColours.m_nSkyTopGreen), CTimeCycle::m_CurrentColours.m_nSkyTopBlue) / 256.0f, visibility);
	if (visibility > 0.0001f)
		for (i = 0; i < ARRAY_SIZE(aArray); i++)
			aArray[i].Render(visibility);
}

void
CPlaneTrails2::RegisterPoint(CVector pos, uint32_t id)
{
	aArray[id].RegisterPoint(pos);
}