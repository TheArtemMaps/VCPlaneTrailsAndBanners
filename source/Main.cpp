#include "plugin.h"
#include "PlaneTrails.h"
#include <CPlane.h>
#include "CGeneral.h"
#include "CPad.h"
#include "extensions/KeyCheck.h"
#include "IniReader.h"
plugin::CdeclEvent <plugin::AddressList<0x726AD0, plugin::H_CALL>, plugin::PRIORITY_AFTER, plugin::ArgPickNone, void()> movingThingsEvent;
using namespace plugin;
bool EnableBanners;
bool EnableTrails;
float Height;
int ModelIDForTrails;
int ModelIDForBanners;
bool ReplaceTrailsInTheSky;
float HeightForBanner;
class CPla : public CPlane
{
public:
	bool IsAlreadyFlying() {
		return m_nStartedFlyingTime = CTimer::m_snTimeInMilliseconds - 20000;
	}
};
class PlaneTrailsSA {
public:
	PlaneTrailsSA() {

		Events::initRwEvent += []() {
			CPlaneBanners::Init();
			CPlaneTrails2::Init();
			CIniReader ini("BannersAndTrails.ini");
			ReplaceTrailsInTheSky = ini.ReadBoolean("MAIN", "ReplaceTrailsInTheSky", false);
			EnableBanners = ini.ReadBoolean("MAIN", "EnableBanner", true);
			EnableTrails = ini.ReadBoolean("MAIN", "EnableTrail", true);
			Height = ini.ReadFloat("MAIN", "Height", 100.0f);
			HeightForBanner = ini.ReadFloat("MAIN", "HeightForBanner", 100.0f);
			ModelIDForTrails = ini.ReadInteger("MAIN", "ModelIDForTrail", 577);
			ModelIDForBanners = ini.ReadInteger("MAIN", "ModelIDForBanner", 593);
			if (ReplaceTrailsInTheSky) {
				patch::RedirectJump(0x7174F0, CPlaneTrails2::Update);
			}
		};

		Events::reInitGameEvent += []() { // Reload settings by loading the save file (Not all settings can be reloaded, you may need to restart the game!)
			CIniReader ini("BannersAndTrails.ini");
			ReplaceTrailsInTheSky = ini.ReadBoolean("MAIN", "ReplaceTrailsInTheSky", false);
			EnableBanners = ini.ReadBoolean("MAIN", "EnableBanner", true);
			EnableTrails = ini.ReadBoolean("MAIN", "EnableTrail", true);
			Height = ini.ReadFloat("MAIN", "Height", 100.0f);
			HeightForBanner = ini.ReadFloat("MAIN", "HeightForBanner", 100.0f);
			ModelIDForTrails = ini.ReadInteger("MAIN", "ModelIDForTrail", 577);
			ModelIDForBanners = ini.ReadInteger("MAIN", "ModelIDForBanner", 593);
		};

		Events::gameProcessEvent += []() {
				CPlaneBanners::Update();
			if (ReplaceTrailsInTheSky) {
				CPlaneTrails2::Update();
			}
			for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++) {
				CVehicle* veh = CPools::ms_pVehiclePool->GetAt(i);
				CPla* plane = (CPla*)veh;
				if (veh) {
					if (!veh || veh->m_nStatus == STATUS_WRECKED) continue; // Plane destroyed? Detach the banner!
					if (veh->m_nModelIndex == ModelIDForTrails && EnableTrails && reinterpret_cast<CPla*>(plane->IsAlreadyFlying())) {
						if (veh->GetPosition().z > Height)
							CPlaneTrails2::RegisterPoint(veh->GetPosition(), i);
					}
					else if (veh->m_nModelIndex == ModelIDForBanners && EnableBanners) {
						if (veh->GetPosition().z > HeightForBanner)
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
