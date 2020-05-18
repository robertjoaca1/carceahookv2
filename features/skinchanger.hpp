#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../singleton.hpp"

class SkinChanger
	: public Singleton<SkinChanger>
{
private:
	struct SkinInfo {
		int nFallbackPaintKit;
		float flFallbackWear;
		SkinInfo(int PaintKit = 0, float wear = 0.000000001f) {
			nFallbackPaintKit = PaintKit;
			flFallbackWear = wear;
		}
	};
	SkinInfo SkinChangerInfo[5036];
	int GetSkinId(int WeaponId, int Skin);
public:
	void SetSkins();
	void DoSkinChanger();
	void AnimationFixHook();
	void AnimationFixUnhook();
};
