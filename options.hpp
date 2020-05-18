#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "valve_sdk/Misc/Color.hpp"

class Options
{
public:
	//legit
	bool LegitAimbotEnabled = false;
	int LegitAimbotType = 0;
	int LegitAimbotHitbox = 0;
	float LegitAimbotFov = 0.0f;
	int LegitAimbotSmooth = 0;
	int LegitAimbotRcs = 0;
	bool LegitAntiaimEnabled = false;

	bool LegitBacktrackEnabled = false;
	int LegitBacktrackDuration = 200;
	//rage
	bool RageAimbotEnabled = false;
	bool RageAimbotResolver = false;
	int RageAimbotHitchanceAuto = 0;
	int RageAimbotMinDmgAuto = 0;
	int RageAimbotHitchanceScout = 0;
	int RageAimbotMinDmgScout = 0;
	int RageAimbotHitchanceAWP = 0;
	int RageAimbotMinDmgAWP = 0;
	int RageAimbotHitchancePistols = 0;
	int RageAimbotMinDmgPistols = 0;
	int RageAimbotHitchanceDeagle = 0;
	int RageAimbotMinDmgDeagle = 0;
	int RageAimbotHitchanceOther = 0;
	int RageAimbotMinDmgOther = 0;
	int RageAimbotSlowWalk = 0;
	bool RageAimbotHead = false;
	bool RageAimbotBody = false;
	bool RageAimbotLegs = false;
	bool RageAimbotToes = false;
	int RageAimbotHeadScale = 0;
	int RageAimbotBodyScale = 0;
	int RageAimbotBaimAfter = 0;
	bool RageAimbotDelayShot = false;
	bool RageAimbotSafePoint = false;

	bool RageAntiaimEnabled = false;

	//visuals
	bool VisualsBox = false;
	bool VisualsName = false;
	bool VisualsWeapon = false;
	bool VisualsHealth = false;
	bool VisualsBacktrackLine = false;
	bool VisualsBacktrackSkeleton = false;
	bool VisualsNoScope = false;
	bool VisualsNoSmoke = false;
	bool VisualsNoFlash = false;
	bool VisualsRadar = false;
	bool VisualsBombTimer = false;
	bool VisualsBulletTracers = false;
	float VisualsBulletTracersLocal[3] = { 1.f, 1.f, 1.f };
	float VisualsBulletTracersEnemy[3] = { 1.f, 1.f, 1.f };
	float VisualsBulletTracersImpacts[3] = { 1.f, 1.f, 1.f };

	

	bool VisualsChamsEnabled = false;
	float VisualsChamsColor[3] = { 1.f, 1.f, 1.f };
	int VisualsChamsMaterial = 2;
	bool VisualsChamsIgnoreZ = false;
	float VisualsChamsColorIgnoreZ[3] = { 1.f, 1.f, 1.f };
	int VisualsChamsAlpha = 255;
	int VisualsChamsBacktrack = 0;
	int VisualsChamsBacktrackMaterial = 2;
	float VisualsChamsBacktrackColor[3] = { 1.f, 1.f, 1.f };
	int VisualsChamsBacktrackAlpha = 255;

	bool VisualsGlowEnabled = false;
	int VisualsGlowGlowstyle = 0;
	float VisualsGlowColor[3] = { 1.f, 1.f, 1.f };
	int VisualsGlowAlpha = 255;

	bool VisualsLocalChamsEnabled = false;
	float VisualsLocalChamsColor[3] = { 1.f, 1.f, 1.f };
	int VisualsLocalChamsMaterial = 2;
	int VisualsLocalChamsAlpha = 255;

	bool VisualsLocalChamsEnabledFakelag = false;
	float VisualsLocalChamsColorFakelag[3] = { 1.f, 1.f, 1.f };
	int VisualsLocalChamsMaterialFakelag = 2;
	int VisualsLocalChamsAlphaFakelag = 255;

	bool VisualsLocalChamsEnabledDesync = false;
	float VisualsLocalChamsColorDesync[3] = { 1.f, 1.f, 1.f };
	int VisualsLocalChamsMaterialDesync = 2;
	int VisualsLocalChamsAlphaDesync = 255;

	bool VisualsLocalGlowEnabled = false;
	int VisualsLocalGlowGlowstyle = 0;
	float VisualsLocalGlowColor[3] = { 1.f, 1.f, 1.f };
	int VisualsLocalGlowAlpha = 255;

	bool VisualsColorModulateWalls = false;
	float VisualsColorModulateWallsColor[3] = { 1.f, 1.f, 1.f };
	bool VisualsColorModulateSky = false;
	float VisualsColorModulateSkyColor[3] = { 1.f, 1.f, 1.f };
	bool VisualsAsusProps = false;
	int VisualsAsusPropsAlpha = 100;
	
	//misc
	int MiscFakelagChoke = 0;
	bool MiscBunnyhop = false;
	bool MiscAutostrafe = false;
	bool MiscLocalplayerFlags = false;
	int MiscClantag = 0;
	int MiscFakeDuckKey = 0;
	int PlayerModel = 0;
	//skins

	//menu
	int ConfigFile = 0;
	float MenuColor[3] = { 0.517f, 0.368f, 0.760f };

	bool SkinChangerEnabled = false;

	int DealgeSkin = 0;
	int BeretasSkin = 0;
	int FiveSevenSkin = 0;
	int GlockSkin = 0;
	int Ak47Skin = 0;
	int AugSkin = 0;
	int AWPSkin = 0;
	int FamasSkin = 0;
	int G3SG1Skin = 0;
	int GalilSkin = 0;
	int M249Skin = 0;
	int M4A4Skin = 0;
	int Mac10Skin = 0;
	int P90Skin = 0;
	int MP5Skin = 0;
	int UMPSkin = 0;
	int XM1014Skin = 0;
	int BizonSkin = 0;
	int MAG7Skin = 0;
	int NegevSkin = 0;
	int SawedOffSkin = 0;
	int Tec9Skin = 0;
	int P2000Skin = 0;
	int MP7Skin = 0;
	int MP9Skin = 0;
	int NovaSkin = 0;
	int P250Skin = 0;
	int Scar20Skin = 0;
	int SG556Skin = 0;
	int SSG08Skin = 0;
	int M4A1Skin = 0;
	int USPSkin = 0;
	int CZ75ASkin = 0;
	int RevolverSkin = 0;

	int KnifeModel = 0;
	int KnifeSkin = 0;
};
inline Options Variables;
