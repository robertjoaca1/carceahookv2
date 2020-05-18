#pragma once
#include "options.hpp"
#include "simpleini.hpp"
#include "menuarrays.hpp"
#include <iostream>
#include <fstream>

CSimpleIniA Config;

void SaveConfig()
{
	Config.SetUnicode(ConfigFiles[Variables.ConfigFile]);

	Config.SetBoolValue("Rage", "Aimbot", Variables.RageAimbotEnabled);
	Config.SetBoolValue("Rage", "Resolver", Variables.RageAimbotResolver);

	Config.SetLongValue("Rage", "HitchanceAuto", Variables.RageAimbotHitchanceAuto);
	Config.SetLongValue("Rage", "MinDmgAuto", Variables.RageAimbotMinDmgAuto);
	Config.SetLongValue("Rage", "HitchanceScout", Variables.RageAimbotHitchanceScout);
	Config.SetLongValue("Rage", "MinDmgScout", Variables.RageAimbotMinDmgScout);
	Config.SetLongValue("Rage", "HitchanceAWP", Variables.RageAimbotHitchanceAWP);
	Config.SetLongValue("Rage", "MinDmgAWP", Variables.RageAimbotMinDmgAWP);
	Config.SetLongValue("Rage", "HitchancePistol", Variables.RageAimbotHitchancePistols);
	Config.SetLongValue("Rage", "MinDmgPistol", Variables.RageAimbotMinDmgPistols);
	Config.SetLongValue("Rage", "HitchanceDeagle", Variables.RageAimbotHitchanceDeagle);
	Config.SetLongValue("Rage", "MinDmgDeagle", Variables.RageAimbotMinDmgDeagle);
	Config.SetLongValue("Rage", "HitchanceOther", Variables.RageAimbotHitchanceOther);
	Config.SetLongValue("Rage", "MinDmgOther", Variables.RageAimbotMinDmgOther);

	Config.SetLongValue("Rage", "SlowWalk", Variables.RageAimbotSlowWalk);

	Config.SetBoolValue("Rage", "Head", Variables.RageAimbotHead);
	Config.SetBoolValue("Rage", "Body", Variables.RageAimbotBody);
	Config.SetBoolValue("Rage", "Legs", Variables.RageAimbotLegs); 
	Config.SetBoolValue("Rage", "Toes", Variables.RageAimbotToes);
	Config.SetBoolValue("Rage", "Delay Shot", Variables.RageAimbotDelayShot);
	Config.SetBoolValue("Rage", "Safe Point", Variables.RageAimbotSafePoint);

	Config.SetLongValue("Rage", "Head Scale", Variables.RageAimbotHeadScale);
	Config.SetLongValue("Rage", "Body Scale", Variables.RageAimbotBodyScale);
	Config.SetLongValue("Rage", "Baim After", Variables.RageAimbotBaimAfter);

	Config.SetBoolValue("Rage", "Antiaim", Variables.RageAntiaimEnabled);

	Config.SetBoolValue("Legit", "Aimbot", Variables.LegitAimbotEnabled);
	Config.SetBoolValue("Legit", "Antiaim", Variables.LegitAntiaimEnabled);
	Config.SetLongValue("Legit", "Aimbot Type", Variables.LegitAimbotType);
	Config.SetLongValue("Legit", "Aimbot Hitbox", Variables.LegitAimbotHitbox);
	Config.SetDoubleValue("Legit", "Aimbot Fov", Variables.LegitAimbotFov);
	Config.SetLongValue("Legit", "Aimbot Smooth", Variables.LegitAimbotSmooth);
	Config.SetLongValue("Legit", "Aimbot Rcs", Variables.LegitAimbotRcs);

	Config.SetBoolValue("Legit", "Backtrack", Variables.LegitBacktrackEnabled);
	Config.SetLongValue("Legit", "Backtrack Duration", Variables.LegitBacktrackDuration);


	Config.SetBoolValue("Visuals", "Box", Variables.VisualsBox);
	Config.SetBoolValue("Visuals", "Health", Variables.VisualsHealth);
	Config.SetBoolValue("Visuals", "Name", Variables.VisualsName);
	Config.SetBoolValue("Visuals", "Weapon", Variables.VisualsWeapon);
	Config.SetBoolValue("Visuals", "Backtrack Line", Variables.VisualsBacktrackLine);
	Config.SetBoolValue("Visuals", "Backtrack Skeleton", Variables.VisualsBacktrackSkeleton);

	Config.SetBoolValue("Visuals", "No Scope Border", Variables.VisualsNoScope);
	Config.SetBoolValue("Visuals", "No Smoke", Variables.VisualsNoSmoke);
	Config.SetBoolValue("Visuals", "No Flash", Variables.VisualsNoFlash);
	Config.SetBoolValue("Visuals", "Radar", Variables.VisualsRadar);
	Config.SetBoolValue("Visuals", "Bomb Timer", Variables.VisualsBombTimer);
	Config.SetBoolValue("Visuals", "Bullet Tracers", Variables.VisualsBulletTracers);
	Config.SetDoubleValue("Visuals", "Local Tracers Red", Variables.VisualsBulletTracersLocal[0]);
	Config.SetDoubleValue("Visuals", "Local Tracers Green", Variables.VisualsBulletTracersLocal[1]);
	Config.SetDoubleValue("Visuals", "Local Tracers Blue", Variables.VisualsBulletTracersLocal[2]);
	Config.SetDoubleValue("Visuals", "Enemy Tracers Red", Variables.VisualsBulletTracersEnemy[0]);
	Config.SetDoubleValue("Visuals", "Enemy Tracers Green", Variables.VisualsBulletTracersEnemy[1]);
	Config.SetDoubleValue("Visuals", "Enemy Tracers Blue", Variables.VisualsBulletTracersEnemy[2]);
	Config.SetDoubleValue("Visuals", "Impacts Red", Variables.VisualsBulletTracersImpacts[0]);
	Config.SetDoubleValue("Visuals", "Impacts Tracers Green", Variables.VisualsBulletTracersImpacts[1]);
	Config.SetDoubleValue("Visuals", "Impacts Tracers Blue", Variables.VisualsBulletTracersImpacts[2]);


	Config.SetBoolValue("Visuals", "Colored Walls", Variables.VisualsColorModulateWalls);
	Config.SetDoubleValue("Visuals", "Walls Red", Variables.VisualsColorModulateWallsColor[0]);
	Config.SetDoubleValue("Visuals", "Walls Green", Variables.VisualsColorModulateWallsColor[1]);
	Config.SetDoubleValue("Visuals", "Walls Blue", Variables.VisualsColorModulateWallsColor[2]);

	Config.SetBoolValue("Visuals", "Colored Sky", Variables.VisualsColorModulateSky);
	Config.SetDoubleValue("Visuals", "Sky Red", Variables.VisualsColorModulateSkyColor[0]);
	Config.SetDoubleValue("Visuals", "Sky Green", Variables.VisualsColorModulateSkyColor[1]);
	Config.SetDoubleValue("Visuals", "Sky Blue", Variables.VisualsColorModulateSkyColor[2]);

	Config.SetBoolValue("Visuals", "Asus Props", Variables.VisualsAsusProps);
	Config.SetLongValue("Visuals", "Props Alpha", Variables.VisualsAsusPropsAlpha);


	Config.SetBoolValue("Visuals", "Chams Enabled", Variables.VisualsChamsEnabled);
	Config.SetDoubleValue("Visuals", "Chams Red", Variables.VisualsChamsColor[0]);
	Config.SetDoubleValue("Visuals", "Chams Green", Variables.VisualsChamsColor[1]);
	Config.SetDoubleValue("Visuals", "Chams Blue", Variables.VisualsChamsColor[2]);
	Config.SetLongValue("Visuals", "Chams Material", Variables.VisualsChamsMaterial);
	Config.SetBoolValue("Visuals", "Chams IgnoreZ", Variables.VisualsChamsIgnoreZ);
	Config.SetDoubleValue("Visuals", "Chams IgnoreZ Red", Variables.VisualsChamsColorIgnoreZ[0]);
	Config.SetDoubleValue("Visuals", "Chams IgnoreZ Green", Variables.VisualsChamsColorIgnoreZ[1]);
	Config.SetDoubleValue("Visuals", "Chams IgnoreZ Blue", Variables.VisualsChamsColorIgnoreZ[2]);
	Config.SetLongValue("Visuals", "Chams Alpha", Variables.VisualsChamsAlpha);
	Config.SetLongValue("Visuals", "Backtrack Chams", Variables.VisualsChamsBacktrack);
	Config.SetLongValue("Visuals", "Backtrack Chams Material", Variables.VisualsChamsBacktrackMaterial);
	Config.SetLongValue("Visuals", "Backtrack Chams Alpha", Variables.VisualsChamsBacktrackAlpha);
	Config.SetDoubleValue("Visuals", "Chams Backtrack Red", Variables.VisualsChamsBacktrackColor[0]);
	Config.SetDoubleValue("Visuals", "Chams Backtrack Green", Variables.VisualsChamsBacktrackColor[1]);
	Config.SetDoubleValue("Visuals", "Chams Backtrack Blue", Variables.VisualsChamsBacktrackColor[2]);

	Config.SetBoolValue("Visuals", "Local Chams Enabled", Variables.VisualsLocalChamsEnabled);
	Config.SetDoubleValue("Visuals", "Local Chams Red", Variables.VisualsLocalChamsColor[0]);
	Config.SetDoubleValue("Visuals", "Local Chams Green", Variables.VisualsLocalChamsColor[1]);
	Config.SetDoubleValue("Visuals", "Local Chams Blue", Variables.VisualsLocalChamsColor[2]);
	Config.SetLongValue("Visuals", "Local Chams Material", Variables.VisualsLocalChamsMaterial);
	Config.SetLongValue("Visuals", "Local Chams Alpha", Variables.VisualsLocalChamsAlpha);

	Config.SetBoolValue("Visuals", "Fakelag Chams Enabled", Variables.VisualsLocalChamsEnabledFakelag);
	Config.SetDoubleValue("Visuals", "Fakelag Chams Red", Variables.VisualsLocalChamsColorFakelag[0]);
	Config.SetDoubleValue("Visuals", "Fakelag Chams Green", Variables.VisualsLocalChamsColorFakelag[1]);
	Config.SetDoubleValue("Visuals", "Fakelag Chams Blue", Variables.VisualsLocalChamsColorFakelag[2]);
	Config.SetLongValue("Visuals", "Fakelag Chams Material", Variables.VisualsLocalChamsMaterialFakelag);
	Config.SetLongValue("Visuals", "Fakelag Chams Alpha", Variables.VisualsLocalChamsAlphaFakelag);

	Config.SetBoolValue("Visuals", "Desync Chams Enabled", Variables.VisualsLocalChamsEnabledDesync);
	Config.SetDoubleValue("Visuals", "Desync Chams Red", Variables.VisualsLocalChamsColorDesync[0]);
	Config.SetDoubleValue("Visuals", "Desync Chams Green", Variables.VisualsLocalChamsColorDesync[1]);
	Config.SetDoubleValue("Visuals", "Desync Chams Blue", Variables.VisualsLocalChamsColorDesync[2]);
	Config.SetLongValue("Visuals", "Desync Chams Material", Variables.VisualsLocalChamsMaterialDesync);
	Config.SetLongValue("Visuals", "Desync Chams Alpha", Variables.VisualsLocalChamsAlphaDesync);

	Config.SetBoolValue("Visuals", "Local Glow Enabled", Variables.VisualsLocalGlowEnabled);
	Config.SetLongValue("Visuals", "Local Glow Style", Variables.VisualsLocalGlowGlowstyle);
	Config.SetDoubleValue("Visuals", "Local Glow Red", Variables.VisualsLocalGlowColor[0]);
	Config.SetDoubleValue("Visuals", "Local Glow Green", Variables.VisualsLocalGlowColor[1]);
	Config.SetDoubleValue("Visuals", "Local Glow Blue", Variables.VisualsLocalGlowColor[2]);
	Config.SetDoubleValue("Visuals", "Local Glow Alpha", Variables.VisualsLocalGlowAlpha);

	Config.SetBoolValue("Visuals", "Glow Enabled", Variables.VisualsGlowEnabled);
	Config.SetLongValue("Visuals", "Glow Style", Variables.VisualsGlowGlowstyle);
	Config.SetDoubleValue("Visuals", "Glow Red", Variables.VisualsGlowColor[0]);
	Config.SetDoubleValue("Visuals", "Glow Green", Variables.VisualsGlowColor[1]);
	Config.SetDoubleValue("Visuals", "Glow Blue", Variables.VisualsGlowColor[2]);
	Config.SetDoubleValue("Visuals", "Glow Alpha", Variables.VisualsGlowAlpha);

	Config.SetLongValue("Misc", "Fakelag Choke", Variables.MiscFakelagChoke);
	Config.SetBoolValue("Misc", "Bunnyhop", Variables.MiscBunnyhop);
	Config.SetBoolValue("Misc", "Autostrafe", Variables.MiscAutostrafe);
	Config.SetBoolValue("Misc", "Localplayer Flags", Variables.MiscLocalplayerFlags);
	Config.SetLongValue("Misc", "Clantag", Variables.MiscClantag);
	Config.SetLongValue("Misc", "FakeDuck", Variables.MiscFakeDuckKey);

	Config.SetBoolValue("Skinchanger", "Enabled", Variables.SkinChangerEnabled);
	Config.SetLongValue("Skins", "MODEL_WEAPON_KNIFE", Variables.KnifeModel);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_KNIFE", Variables.KnifeSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_DEAGLE", Variables.DealgeSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_ELITE", Variables.BeretasSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_FIVESEVEN", Variables.FiveSevenSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_GLOCK", Variables.GlockSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_AK47", Variables.Ak47Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_AUG", Variables.AugSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_AWP", Variables.AWPSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_FAMAS", Variables.FamasSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_G3SG1", Variables.G3SG1Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_GALILAR", Variables.GalilSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_M249", Variables.M249Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_M4A4", Variables.M4A4Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_MAC10", Variables.Mac10Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_P90", Variables.P90Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_MP5SD", Variables.MP5Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_UMP45", Variables.UMPSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_XM1014", Variables.XM1014Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_BIZON", Variables.BizonSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_MAG7", Variables.MAG7Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_NEGEV", Variables.NegevSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_SAWEDOFF", Variables.SawedOffSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_TEC9", Variables.Tec9Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_HKP2000", Variables.P2000Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_MP7", Variables.MP7Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_MP9", Variables.MP9Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_NOVA", Variables.NovaSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_P250", Variables.P250Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_SCAR20", Variables.Scar20Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_SG553", Variables.SG556Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_SSG08", Variables.SSG08Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_M4A1_S", Variables.M4A1Skin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_USP_S", Variables.USPSkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_CZ75A", Variables.CZ75ASkin);
	Config.SetLongValue("Skins", "CUSTOMID_WEAPON_REVOLVER", Variables.RevolverSkin);

	Config.SaveFile(ConfigFiles[Variables.ConfigFile]);
}
void LoadConfig()
{
	Config.SetUnicode(ConfigFiles[Variables.ConfigFile]);
	Config.LoadFile(ConfigFiles[Variables.ConfigFile]);

	Variables.RageAimbotEnabled = Config.GetBoolValue("Rage", "Aimbot", Variables.RageAimbotEnabled);
	Variables.RageAimbotResolver = Config.GetBoolValue("Rage", "Resolver", Variables.RageAimbotResolver);
	
	Variables.RageAimbotHitchanceAuto = Config.GetLongValue("Rage", "HitchanceAuto", Variables.RageAimbotHitchanceAuto);
	Variables.RageAimbotMinDmgAuto = Config.GetLongValue("Rage", "MinDmgAuto", Variables.RageAimbotMinDmgAuto);
	Variables.RageAimbotHitchanceScout = Config.GetLongValue("Rage", "HitchanceScout", Variables.RageAimbotHitchanceScout);
	Variables.RageAimbotMinDmgScout = Config.GetLongValue("Rage", "MinDmgScout", Variables.RageAimbotMinDmgScout);
	Variables.RageAimbotHitchanceAWP = Config.GetLongValue("Rage", "HitchanceAWP", Variables.RageAimbotHitchanceAWP);
	Variables.RageAimbotMinDmgAWP = Config.GetLongValue("Rage", "MinDmgAWP", Variables.RageAimbotMinDmgAWP);
	Variables.RageAimbotHitchancePistols = Config.GetLongValue("Rage", "HitchancePistol", Variables.RageAimbotHitchancePistols);
	Variables.RageAimbotMinDmgPistols = Config.GetLongValue("Rage", "MinDmgPistol", Variables.RageAimbotMinDmgPistols);
	Variables.RageAimbotHitchanceDeagle = Config.GetLongValue("Rage", "HitchanceDeagle", Variables.RageAimbotHitchanceDeagle);
	Variables.RageAimbotMinDmgDeagle = Config.GetLongValue("Rage", "MinDmgDeagle", Variables.RageAimbotMinDmgDeagle);
	Variables.RageAimbotHitchanceOther = Config.GetLongValue("Rage", "HitchanceOther", Variables.RageAimbotHitchanceOther);
	Variables.RageAimbotMinDmgOther = Config.GetLongValue("Rage", "MinDmgOther", Variables.RageAimbotMinDmgOther);

	Variables.RageAimbotSlowWalk = Config.GetLongValue("Rage", "SlowWalk", Variables.RageAimbotSlowWalk);

	Variables.RageAimbotHead = Config.GetBoolValue("Rage", "Head", Variables.RageAimbotHead);
	Variables.RageAimbotBody = Config.GetBoolValue("Rage", "Body", Variables.RageAimbotBody);
	Variables.RageAimbotLegs = Config.GetBoolValue("Rage", "Legs", Variables.RageAimbotLegs);
	Variables.RageAimbotToes = Config.GetBoolValue("Rage", "Toes", Variables.RageAimbotToes);
	Variables.RageAimbotDelayShot = Config.GetBoolValue("Rage", "Delay Shot", Variables.RageAimbotDelayShot);
	Variables.RageAimbotSafePoint = Config.GetBoolValue("Rage", "Safe Point", Variables.RageAimbotSafePoint);

	Variables.RageAimbotHeadScale = Config.GetLongValue("Rage", "Head Scale", Variables.RageAimbotHeadScale);
	Variables.RageAimbotBodyScale = Config.GetLongValue("Rage", "Body Scale", Variables.RageAimbotBodyScale);
	Variables.RageAimbotBaimAfter = Config.GetLongValue("Rage", "Baim After", Variables.RageAimbotBaimAfter);

	Variables.RageAntiaimEnabled = Config.GetBoolValue("Rage", "Antiaim", Variables.RageAntiaimEnabled);

	Variables.LegitAimbotEnabled = Config.GetBoolValue("Legit", "Aimbot", Variables.LegitAimbotEnabled);
	Variables.LegitAntiaimEnabled = Config.GetBoolValue("Legit", "Antiaim", Variables.LegitAntiaimEnabled);
	Variables.LegitAimbotType = Config.GetLongValue("Legit", "Aimbot Type", Variables.LegitAimbotType);
	Variables.LegitAimbotHitbox = Config.GetLongValue("Legit", "Aimbot Hitbox", Variables.LegitAimbotHitbox);
	Variables.LegitAimbotFov = Config.GetDoubleValue("Legit", "Aimbot Fov", Variables.LegitAimbotFov);
	Variables.LegitAimbotSmooth = Config.GetLongValue("Legit", "Aimbot Smooth", Variables.LegitAimbotSmooth);
	Variables.LegitAimbotRcs = Config.GetLongValue("Legit", "Aimbot Rcs", Variables.LegitAimbotRcs);

	Variables.LegitBacktrackEnabled = Config.GetBoolValue("Legit", "Backtrack", Variables.LegitBacktrackEnabled);
	Variables.LegitBacktrackDuration = Config.GetLongValue("Legit", "Backtrack Duration", Variables.LegitBacktrackDuration);

	Variables.VisualsBox = Config.GetBoolValue("Visuals", "Box", Variables.VisualsBox);
	Variables.VisualsHealth = Config.GetBoolValue("Visuals", "Health", Variables.VisualsHealth);
	Variables.VisualsName = Config.GetBoolValue("Visuals", "Name", Variables.VisualsName);
	Variables.VisualsWeapon = Config.GetBoolValue("Visuals", "Weapon", Variables.VisualsWeapon);
	Variables.VisualsBacktrackLine = Config.GetBoolValue("Visuals", "Backtrack Line", Variables.VisualsBacktrackLine);
	Variables.VisualsBacktrackSkeleton = Config.GetBoolValue("Visuals", "Backtrack Skeleton", Variables.VisualsBacktrackSkeleton);

	Variables.VisualsNoScope = Config.GetBoolValue("Visuals", "No Scope Border", Variables.VisualsNoScope);
	Variables.VisualsNoSmoke = Config.GetBoolValue("Visuals", "No Smoke", Variables.VisualsNoSmoke);
	Variables.VisualsNoFlash = Config.GetBoolValue("Visuals", "No Flash", Variables.VisualsNoFlash);
	Variables.VisualsRadar = Config.GetBoolValue("Visuals", "Radar", Variables.VisualsRadar);
	Variables.VisualsBombTimer = Config.GetBoolValue("Visuals", "Bomb Timer", Variables.VisualsBombTimer);
	Variables.VisualsBulletTracers = Config.GetBoolValue("Visuals", "Bullet Tracers", Variables.VisualsBulletTracers);
	Variables.VisualsBulletTracersLocal[0] = Config.GetDoubleValue("Visuals", "Local Tracers Red", Variables.VisualsBulletTracersLocal[0]);
	Variables.VisualsBulletTracersLocal[1] = Config.GetDoubleValue("Visuals", "Local Tracers Green", Variables.VisualsBulletTracersLocal[1]);
	Variables.VisualsBulletTracersLocal[2] = Config.GetDoubleValue("Visuals", "Local Tracers Blue", Variables.VisualsBulletTracersLocal[2]);
	Variables.VisualsBulletTracersEnemy[0] = Config.GetDoubleValue("Visuals", "Enemy Tracers Red", Variables.VisualsBulletTracersEnemy[0]);
	Variables.VisualsBulletTracersEnemy[1] = Config.GetDoubleValue("Visuals", "Enemy Tracers Green", Variables.VisualsBulletTracersEnemy[1]);
	Variables.VisualsBulletTracersEnemy[2] = Config.GetDoubleValue("Visuals", "Enemy Tracers Blue", Variables.VisualsBulletTracersEnemy[2]);
	Variables.VisualsBulletTracersImpacts[0] = Config.GetDoubleValue("Visuals", "Impacts Red", Variables.VisualsBulletTracersImpacts[0]);
	Variables.VisualsBulletTracersImpacts[1] = Config.GetDoubleValue("Visuals", "Impacts Tracers Green", Variables.VisualsBulletTracersImpacts[1]);
	Variables.VisualsBulletTracersImpacts[2] = Config.GetDoubleValue("Visuals", "Impacts Tracers Blue", Variables.VisualsBulletTracersImpacts[2]);

	Variables.VisualsColorModulateWalls = Config.GetBoolValue("Visuals", "Colored Walls", Variables.VisualsColorModulateWalls);
	Variables.VisualsColorModulateWallsColor[0] = Config.GetDoubleValue("Visuals", "Walls Red", Variables.VisualsColorModulateWallsColor[0]);
	Variables.VisualsColorModulateWallsColor[1] = Config.GetDoubleValue("Visuals", "Walls Green", Variables.VisualsColorModulateWallsColor[1]);
	Variables.VisualsColorModulateWallsColor[2] = Config.GetDoubleValue("Visuals", "Walls Blue", Variables.VisualsColorModulateWallsColor[2]);

	Variables.VisualsColorModulateSky = Config.GetBoolValue("Visuals", "Colored Sky", Variables.VisualsColorModulateSky);
	Variables.VisualsColorModulateSkyColor[0] = Config.GetDoubleValue("Visuals", "Sky Red", Variables.VisualsColorModulateSkyColor[0]);
	Variables.VisualsColorModulateSkyColor[1] = Config.GetDoubleValue("Visuals", "Sky Green", Variables.VisualsColorModulateSkyColor[1]);
	Variables.VisualsColorModulateSkyColor[2] = Config.GetDoubleValue("Visuals", "Sky Blue", Variables.VisualsColorModulateSkyColor[2]);

	Variables.VisualsAsusProps = Config.GetBoolValue("Visuals", "Asus Props", Variables.VisualsAsusProps);
	Variables.VisualsAsusPropsAlpha = Config.GetLongValue("Visuals", "Props Alpha", Variables.VisualsAsusPropsAlpha);

	Variables.VisualsChamsEnabled = Config.GetBoolValue("Visuals", "Chams Enabled", Variables.VisualsChamsEnabled);
	Variables.VisualsChamsColor[0] = Config.GetDoubleValue("Visuals", "Chams Red", Variables.VisualsChamsColor[0]);
	Variables.VisualsChamsColor[1] = Config.GetDoubleValue("Visuals", "Chams Green", Variables.VisualsChamsColor[1]);
	Variables.VisualsChamsColor[2] = Config.GetDoubleValue("Visuals", "Chams Blue", Variables.VisualsChamsColor[2]);
	Variables.VisualsChamsMaterial = Config.GetLongValue("Visuals", "Chams Material", Variables.VisualsChamsMaterial);
	Variables.VisualsChamsIgnoreZ = Config.GetBoolValue("Visuals", "Chams IgnoreZ", Variables.VisualsChamsIgnoreZ);
	Variables.VisualsChamsColorIgnoreZ[0] = Config.GetDoubleValue("Visuals", "Chams IgnoreZ Red", Variables.VisualsChamsColorIgnoreZ[0]);
	Variables.VisualsChamsColorIgnoreZ[1] = Config.GetDoubleValue("Visuals", "Chams IgnoreZ Green", Variables.VisualsChamsColorIgnoreZ[1]);
	Variables.VisualsChamsColorIgnoreZ[2] = Config.GetDoubleValue("Visuals", "Chams IgnoreZ Blue", Variables.VisualsChamsColorIgnoreZ[2]);
	Variables.VisualsChamsAlpha = Config.GetLongValue("Visuals", "Chams Alpha", Variables.VisualsChamsAlpha);
	Variables.VisualsChamsBacktrack = Config.GetLongValue("Visuals", "Backtrack Chams", Variables.VisualsChamsBacktrack);
	Variables.VisualsChamsBacktrackMaterial = Config.GetLongValue("Visuals", "Backtrack Chams Material", Variables.VisualsChamsBacktrackMaterial);
	Variables.VisualsChamsBacktrackAlpha = Config.GetLongValue("Visuals", "Backtrack Chams Alpha", Variables.VisualsChamsBacktrackAlpha);
	Variables.VisualsChamsBacktrackColor[0] = Config.GetDoubleValue("Visuals", "Chams Backtrack Red", Variables.VisualsChamsBacktrackColor[0]);
	Variables.VisualsChamsBacktrackColor[1] = Config.GetDoubleValue("Visuals", "Chams Backtrack Green", Variables.VisualsChamsBacktrackColor[1]);
	Variables.VisualsChamsBacktrackColor[2] = Config.GetDoubleValue("Visuals", "Chams Backtrack Blue", Variables.VisualsChamsBacktrackColor[2]);

	Variables.VisualsGlowEnabled = Config.GetBoolValue("Visuals", "Glow Enabled", Variables.VisualsGlowEnabled);
	Variables.VisualsGlowGlowstyle = Config.GetLongValue("Visuals", "Glow Style", Variables.VisualsGlowGlowstyle);
	Variables.VisualsGlowColor[0] = Config.GetDoubleValue("Visuals", "Glow Red", Variables.VisualsGlowColor[0]);
	Variables.VisualsGlowColor[1] = Config.GetDoubleValue("Visuals", "Glow Green", Variables.VisualsGlowColor[1]);
	Variables.VisualsGlowColor[2] = Config.GetDoubleValue("Visuals", "Glow Blue", Variables.VisualsGlowColor[2]);
	Variables.VisualsGlowAlpha = Config.GetDoubleValue("Visuals", "Glow Alpha", Variables.VisualsGlowAlpha);

	Variables.VisualsLocalChamsEnabled = Config.GetBoolValue("Visuals", "Local Chams Enabled", Variables.VisualsLocalChamsEnabled);
	Variables.VisualsLocalChamsColor[0] = Config.GetDoubleValue("Visuals", "Local Chams Red", Variables.VisualsLocalChamsColor[0]);
	Variables.VisualsLocalChamsColor[1] = Config.GetDoubleValue("Visuals", "Local Chams Green", Variables.VisualsLocalChamsColor[1]);
	Variables.VisualsLocalChamsColor[2] = Config.GetDoubleValue("Visuals", "Local Chams Blue", Variables.VisualsLocalChamsColor[2]);
	Variables.VisualsLocalChamsMaterial = Config.GetLongValue("Visuals", "Local Chams Material", Variables.VisualsLocalChamsMaterial);
	Variables.VisualsLocalChamsAlpha = Config.GetLongValue("Visuals", "Local Chams Alpha", Variables.VisualsLocalChamsAlpha);

	Variables.VisualsLocalChamsEnabledFakelag = Config.GetBoolValue("Visuals", "Fakelag Chams Enabled", Variables.VisualsLocalChamsEnabledFakelag);
	Variables.VisualsLocalChamsColorFakelag[0] = Config.GetDoubleValue("Visuals", "Fakelag Chams Red", Variables.VisualsLocalChamsColorFakelag[0]);
	Variables.VisualsLocalChamsColorFakelag[1] = Config.GetDoubleValue("Visuals", "Fakelag Chams Green", Variables.VisualsLocalChamsColorFakelag[1]);
	Variables.VisualsLocalChamsColorFakelag[2] = Config.GetDoubleValue("Visuals", "Fakelag Chams Blue", Variables.VisualsLocalChamsColorFakelag[2]);
	Variables.VisualsLocalChamsMaterialFakelag = Config.GetLongValue("Visuals", "Fakelag Chams Material", Variables.VisualsLocalChamsMaterialFakelag);
	Variables.VisualsLocalChamsAlphaFakelag = Config.GetLongValue("Visuals", "Fakelag Chams Alpha", Variables.VisualsLocalChamsAlphaFakelag);


	Variables.VisualsLocalChamsEnabledDesync = Config.GetBoolValue("Visuals", "Desync Chams Enabled", Variables.VisualsLocalChamsEnabledDesync);
	Variables.VisualsLocalChamsColorDesync[0] = Config.GetDoubleValue("Visuals", "Desync Chams Red", Variables.VisualsLocalChamsColorDesync[0]);
	Variables.VisualsLocalChamsColorDesync[1] = Config.GetDoubleValue("Visuals", "Desync Chams Green", Variables.VisualsLocalChamsColorDesync[1]);
	Variables.VisualsLocalChamsColorDesync[2] = Config.GetDoubleValue("Visuals", "Desync Chams Blue", Variables.VisualsLocalChamsColorDesync[2]);
	Variables.VisualsLocalChamsMaterialDesync = Config.GetLongValue("Visuals", "Desync Chams Material", Variables.VisualsLocalChamsMaterialDesync);
	Variables.VisualsLocalChamsAlphaDesync = Config.GetLongValue("Visuals", "Desync Chams Alpha", Variables.VisualsLocalChamsAlphaDesync);

	Variables.VisualsLocalGlowEnabled = Config.GetBoolValue("Visuals", "Local Glow Enabled", Variables.VisualsLocalGlowEnabled);
	Variables.VisualsLocalGlowGlowstyle = Config.GetLongValue("Visuals", "Local Glow Style", Variables.VisualsLocalGlowGlowstyle);
	Variables.VisualsLocalGlowColor[0] = Config.GetDoubleValue("Visuals", "Local Glow Red", Variables.VisualsLocalGlowColor[0]);
	Variables.VisualsLocalGlowColor[1] = Config.GetDoubleValue("Visuals", "Local Glow Green", Variables.VisualsLocalGlowColor[1]);
	Variables.VisualsLocalGlowColor[2] = Config.GetDoubleValue("Visuals", "Local Glow Blue", Variables.VisualsLocalGlowColor[2]);
	Variables.VisualsLocalGlowAlpha = Config.GetDoubleValue("Visuals", "Local Glow Alpha", Variables.VisualsLocalGlowAlpha);

	Variables.MiscFakelagChoke = Config.GetLongValue("Misc", "Fakelag Choke", Variables.MiscFakelagChoke);
	Variables.MiscBunnyhop = Config.GetBoolValue("Misc", "Bunnyhop", Variables.MiscBunnyhop);
	Variables.MiscAutostrafe = Config.GetBoolValue("Misc", "Autostrafe", Variables.MiscAutostrafe);
	Variables.MiscLocalplayerFlags = Config.GetBoolValue("Misc", "Localplayer Flags", Variables.MiscLocalplayerFlags);
	Variables.MiscClantag = Config.GetLongValue("Misc", "Clantag", Variables.MiscClantag);
	Variables.MiscFakeDuckKey = Config.GetLongValue("Misc", "FakeDuck", Variables.MiscFakeDuckKey);

	Variables.SkinChangerEnabled = Config.GetBoolValue("Skinchanger", "Enabled", Variables.SkinChangerEnabled);
	Variables.KnifeModel = Config.GetLongValue("Skins", "MODEL_WEAPON_KNIFE", Variables.KnifeModel);
	Variables.KnifeSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_KNIFE", Variables.KnifeSkin);
	Variables.DealgeSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_DEAGLE", Variables.DealgeSkin);
	Variables.BeretasSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_ELITE", Variables.BeretasSkin);
	Variables.FiveSevenSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_FIVESEVEN", Variables.FiveSevenSkin);
	Variables.GlockSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_GLOCK", Variables.GlockSkin);
	Variables.Ak47Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_AK47", Variables.Ak47Skin);
	Variables.AugSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_AUG", Variables.AugSkin);
	Variables.AWPSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_AWP", Variables.AWPSkin);
	Variables.FamasSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_FAMAS", Variables.FamasSkin);
	Variables.G3SG1Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_G3SG1", Variables.G3SG1Skin);
	Variables.GalilSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_GALILAR", Variables.GalilSkin);
	Variables.M249Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_M249", Variables.M249Skin);
	Variables.M4A4Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_M4A4", Variables.M4A4Skin);
	Variables.Mac10Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_MAC10", Variables.Mac10Skin);
	Variables.P90Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_P90", Variables.P90Skin);
	Variables.MP5Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_MP5SD", Variables.MP5Skin);
	Variables.UMPSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_UMP45", Variables.UMPSkin);
	Variables.XM1014Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_XM1014", Variables.XM1014Skin);
	Variables.BizonSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_BIZON", Variables.BizonSkin);
	Variables.MAG7Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_MAG7", Variables.MAG7Skin);
	Variables.NegevSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_NEGEV", Variables.NegevSkin);
	Variables.SawedOffSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_SAWEDOFF", Variables.SawedOffSkin);
	Variables.Tec9Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_TEC9", Variables.Tec9Skin);
	Variables.P2000Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_HKP2000", Variables.P2000Skin);
	Variables.MP7Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_MP7", Variables.MP7Skin);
	Variables.MP9Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_MP9", Variables.MP9Skin);
	Variables.NovaSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_NOVA", Variables.NovaSkin);
	Variables.P250Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_P250", Variables.P250Skin);
	Variables.Scar20Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_SCAR20", Variables.Scar20Skin);
	Variables.SG556Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_SG553", Variables.SG556Skin);
	Variables.SSG08Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_SSG08", Variables.SSG08Skin);
	Variables.M4A1Skin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_M4A1_S", Variables.M4A1Skin);
	Variables.USPSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_USP_S", Variables.USPSkin);
	Variables.CZ75ASkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_CZ75A", Variables.CZ75ASkin);
	Variables.RevolverSkin = Config.GetLongValue("Skins", "CUSTOMID_WEAPON_REVOLVER", Variables.RevolverSkin);
}
