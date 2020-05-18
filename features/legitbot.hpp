#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../singleton.hpp"
#include "../helpers/input.hpp"
#include "../hooks.hpp"

class LegitBacktrack :
	public Singleton<LegitBacktrack>
{
	friend class Singleton<LegitBacktrack>;
private:
	
public:
	void Do(CUserCmd* cmd);
	std::vector<Hooks::TickInfo> BacktrackRecords[65];
	int ClosestTick;
};
class LegitAimbot :
	public Singleton<LegitAimbot>
{
	friend class Singleton<LegitAimbot>;
private:
	int GetHitboxFromInt(int Hitbox);
	int GetClosestHitbox(C_BasePlayer* Player, matrix3x4_t* BoneMatrix, Vector EyePos);
public:
	void Do(CUserCmd* cmd, C_BaseCombatWeapon* Weapon);
	void DoLegitAntiaim(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket);
};