#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../singleton.hpp"
#include "../helpers/input.hpp"
#include "../hooks.hpp"
#include <deque>

class MovementFix : public Singleton<MovementFix>
{
	friend class Singleton<MovementFix>;
public:
	void Start(CUserCmd* cmd);
	void End(CUserCmd* cmd);
	float m_oldforward, m_oldsidemove;
	QAngle m_oldangle;
};
class RageAimbot :
	public Singleton<RageAimbot>
{
	friend class Singleton<RageAimbot>;
private:
	bool ShouldBaim(C_BasePlayer* Player);
	void GetMultipointPositions(C_BasePlayer* Player, std::vector<Vector>& Positions, int HitboxIndex, float Scale, matrix3x4_t* BoneMatrix);
	bool Hitscan(C_BasePlayer* Player, Vector& HitboxPos, matrix3x4_t* BoneMatrix, bool Backtrack, Hooks::TickInfo Record);
	bool Hitchance(C_BasePlayer* Player, C_BaseCombatWeapon* pWeapon, QAngle Angle, Vector Point, int Chance);
	void WeaponSettings(C_BaseCombatWeapon* Weapon);
	float HitchanceValue;
	float MinDmgValue;
	bool UseFreestand[65];
	float FreestandAngle[65];
	float LastFreestandAngle[65];
	bool ForceSafePoint[65];
public:
	bool m_should_update_fake = false;
	std::array< AnimationLayer, 13 > m_fake_layers;
	std::array< float, 20 > m_fake_poses;
	CBasePlayerAnimState* m_fake_states = nullptr;
	CBasePlayerAnimState* m_fake_state = nullptr;
	float m_fake_rotation = 0.f;
	bool init_fake_anim = false;
	float m_fake_spawntime = 0.f;
	float m_fake_delta = 0.f;
	matrix3x4_t m_fake_matrix[128];
	matrix3x4_t m_fake_position_matrix[128];
	//std::array< matrix3x4_t, 128 > m_fake_matrix;
	//std::array< matrix3x4_t, 128 > m_fake_position_matrix;
	bool m_got_fake_matrix = false;
	float m_real_yaw_ang = 0.f;
	bool m_should_update_entity_animstate = true;
	float m_server_abs_rotation = 0.f;
	struct infos
	{
		std::array<float, 24> m_poses;
		AnimationLayer m_overlays;

	};

	std::vector<Hooks::TickInfo> BacktrackRecords[65];
	void AntiFreestanding();
	void Resolver(C_BasePlayer* Player, CBasePlayerAnimState* Animstate);
	void AnimationFix();
	void LocalAnimationFix();
	void UpdateFakeAnimations();
	void StoreRecords();
	void Do(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket);
	void DoFakelag(bool& bSendPacket);
	void DoAntiaim(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket);
};