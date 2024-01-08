#include "plugin.h"
#include "PlaneTrails.h"
#include <CPlane.h>
#include "CGeneral.h"
#include "CPad.h"
#include "extensions/KeyCheck.h"
#include "IniReader.h"
plugin::CdeclEvent <plugin::AddressList<0x726AD0, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPickNone, void()> movingThingsEvent;
using namespace plugin;

class CPla : public CPlane
{
public:
	bool IsAlreadyFlying() {
		return m_nStartedFlyingTime = CTimer::m_snTimeInMilliseconds - 20000;
	}
};

extern struct tIniData {
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

extern tIniData Plane;

class PlaneTrailsSA {
public:
	PlaneTrailsSA() {

		Events::initRwEvent += []() {
			CPlaneBanners::Init();
			CPlaneTrails2::Init();
			CIniReader ini("BannersAndTrails.ini");
			Plane.ReplaceTrailsInTheSky = ini.ReadBoolean("MAIN", "ReplaceTrailsInTheSky", false);
			Plane.EnableBanners = ini.ReadBoolean("MAIN", "EnableBanner", true);
			Plane.EnableTrails = ini.ReadBoolean("MAIN", "EnableTrail", true);
			Plane.AppearOnlyAtAltitude = ini.ReadBoolean("MAIN", "AppearOnlyAtAltitude", true);
			Plane.AppearWhenPilotIsInside = ini.ReadBoolean("MAIN", "AppearWhenPilotIsInside", false);
			Plane.AlwaysAppear = ini.ReadBoolean("MAIN", "AlwaysAppear", false);
			Plane.Height = ini.ReadFloat("MAIN", "Height", 100.0f);
			Plane.TrailFadeTime = ini.ReadFloat("MAIN", "TrailFadeTime", 10000.0f);
			Plane.HeightForBanner = ini.ReadFloat("MAIN", "HeightForBanner", 100.0f);
			Plane.ModelIDForTrails = ini.ReadInteger("MAIN", "ModelIDForTrail", 577);
			Plane.ModelIDForBanners = ini.ReadInteger("MAIN", "ModelIDForBanner", 593);
			Plane.DrawDistance = ini.ReadFloat("MAIN", "BannerDrawDistance", 300.0f);
			if (Plane.ReplaceTrailsInTheSky) {
				patch::RedirectJump(0x7174F0, CPlaneTrails2::Update);
			}
		};
		

		Events::reInitGameEvent += []() { // Reload settings by loading the save file (Not all settings can be reloaded, you may need to restart the game!)
			CIniReader ini("BannersAndTrails.ini");
			Plane.ReplaceTrailsInTheSky = ini.ReadBoolean("MAIN", "ReplaceTrailsInTheSky", false);
			Plane.EnableBanners = ini.ReadBoolean("MAIN", "EnableBanner", true);
			Plane.EnableTrails = ini.ReadBoolean("MAIN", "EnableTrail", true);
			Plane.AppearOnlyAtAltitude = ini.ReadBoolean("MAIN", "AppearOnlyAtAltitude", true);
			Plane.AppearWhenPilotIsInside = ini.ReadBoolean("MAIN", "AppearWhenPilotIsInside", false);
			Plane.AlwaysAppear = ini.ReadBoolean("MAIN", "AlwaysAppear", false);
			Plane.Height = ini.ReadFloat("MAIN", "Height", 100.0f);
			Plane.TrailFadeTime = ini.ReadFloat("MAIN", "TrailFadeTime", 10000.0f);
			Plane.HeightForBanner = ini.ReadFloat("MAIN", "HeightForBanner", 100.0f);
			Plane.ModelIDForTrails = ini.ReadInteger("MAIN", "ModelIDForTrail", 577);
			Plane.ModelIDForBanners = ini.ReadInteger("MAIN", "ModelIDForBanner", 593);
		};

		Events::gameProcessEvent += []() {
				CPlaneBanners::Update();
			if (Plane.ReplaceTrailsInTheSky) {
				CPlaneTrails2::Update();
			}
			for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
				CVehicle* veh = CPools::ms_pVehiclePool->GetAt(i);
				CPla* plane = (CPla*)veh;
				if (veh) {
					if (!veh || veh->m_nStatus == STATUS_WRECKED) continue; // Plane destroyed? Detach the banner & trail!
					if (veh->m_nModelIndex == Plane.ModelIDForTrails && Plane.EnableTrails && reinterpret_cast<CPla*>(plane->IsAlreadyFlying())) {
						if (veh->GetPosition().z > Plane.Height)
							CPlaneTrails2::RegisterPoint(veh->GetPosition(), i);
					}
					else if (veh->m_nModelIndex == Plane.ModelIDForBanners && Plane.EnableBanners) {
						if (veh->GetPosition().z > Plane.HeightForBanner && Plane.AppearOnlyAtAltitude)
						CPlaneBanners::RegisterPoint(veh->GetPosition(), i);
						else if (Plane.AppearWhenPilotIsInside)
						if (veh->m_pDriver)
						CPlaneBanners::RegisterPoint(veh->GetPosition(), i);
						else if (Plane.AlwaysAppear)
						CPlaneBanners::RegisterPoint(veh->GetPosition(), i);
					}
					
				}
			}
			
		
		};

		Events::shutdownRwEvent += []() {
			CPlaneBanners::Shutdown();
		};

		movingThingsEvent += []() {
			CPlaneBanners::Render();
			CPlaneTrails2::Render();
		};


    }
} _PlaneTrailsSA;
