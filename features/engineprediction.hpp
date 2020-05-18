#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../singleton.hpp"
#include "../helpers/input.hpp"
#include "../hooks.hpp"

class PredictionSystem : public Singleton<PredictionSystem>
{

public:
	PredictionSystem() {
		predictionRandomSeed = *(int**)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 2);
		predictionPlayer = *reinterpret_cast<C_BasePlayer * *>(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "89 35 ? ? ? ? F3 0F 10 46") + 2);
	}

	void Start(CUserCmd* userCMD, C_BasePlayer* player);
	void End(C_BasePlayer* player);

	inline float GetOldCurTime() { return m_flOldCurTime; }

private:

	float m_flOldCurTime;
	float m_flOldFrametime;

	CMoveData moveData;

	int* predictionRandomSeed;
	C_BasePlayer* predictionPlayer;
};
