#pragma once
#include "options.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/math.hpp"
#include "helpers/utils.hpp"
#include "singleton.hpp"
#include "Hooks.hpp"

class BulletTracers :
	public Singleton<BulletTracers>
{
	friend class Singleton<BulletTracers>;
public:
	class BulletTracersInfo
	{
	public:
		BulletTracersInfo(Vector src, Vector dst, float time, Color color)
		{
			this->src = src;
			this->dst = dst;
			this->time = time;
			this->color = color;
		}
		Vector src, dst;
		float time;
		Color color;
	};

	std::vector<BulletTracersInfo> Logs;
};

class EventListener : public IGameEventListener2
{
public:
	void FireGameEvent(IGameEvent* Event)
	{
		if (!Event)
			return;

		static int OldShotsFired[65];

		if (strstr(Event->GetName(), "player_hurt"))
		{
			C_BasePlayer* Attacker =
				C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetPlayerForUserID(Event->GetInt("attacker")));

			C_BasePlayer* Victim =
				C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetPlayerForUserID(Event->GetInt("userid")));

			if (Attacker == g_LocalPlayer && Victim != g_LocalPlayer)
			{
				if (Hooks::ShotsFired[Victim->EntIndex()] != OldShotsFired[Victim->EntIndex()])
				{
					Hooks::ShotsHit[Victim->EntIndex()] += 1;

					OldShotsFired[Victim->EntIndex()] = Hooks::ShotsFired[Victim->EntIndex()];
				}
			}
		}
		if (strstr(Event->GetName(), "round_start"))
		{
			for (int i = 0; i <= 64; i++)
			{
				Hooks::ShotsHit[i] = 0;
				Hooks::ShotsFired[i] = 0;
				OldShotsFired[i] = 0;
			}
			BulletTracers::Get().Logs.erase(BulletTracers::Get().Logs.begin(), BulletTracers::Get().Logs.begin() + BulletTracers::Get().Logs.size());
		}
		//if we receive bullet_impact event
		if (strstr(Event->GetName(), "bullet_impact"))
		{
			//get the user who fired the bullet
			auto index = g_EngineClient->GetPlayerForUserID(Event->GetInt("userid"));
			auto entity = C_BasePlayer::GetPlayerByIndex(index);
			if (!entity->IsDormant() && entity->m_iTeamNum() != g_LocalPlayer->m_iTeamNum() || index == g_EngineClient->GetLocalPlayer())
			{
				//get the bullet impact's position
				Vector position(Event->GetFloat("x"), Event->GetFloat("y"), Event->GetFloat("z"));

				Ray_t ray;
				ray.Init(entity->GetEyePos() - Vector(0, 0, 1), position);

				//skip local player
				CTraceFilter filter;
				filter.pSkip = entity;

				//trace a ray
				trace_t tr;
				g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

				//use different color when we hit a player
				auto color = index == g_EngineClient->GetLocalPlayer() ? Color(int(Variables.VisualsBulletTracersLocal[0] * 255), int(Variables.VisualsBulletTracersLocal[1] * 255), int(Variables.VisualsBulletTracersLocal[2] * 255), 255) :
					Color(int(Variables.VisualsBulletTracersEnemy[0] * 255), int(Variables.VisualsBulletTracersEnemy[1] * 255), int(Variables.VisualsBulletTracersEnemy[2] * 255), 255);

				//push info to our vector
				BulletTracers::Get().Logs.push_back(BulletTracers::BulletTracersInfo(entity->GetEyePos() - Vector(0, 0, 1), position, g_GlobalVars->curtime, color));
			}
		}
		else if (!Variables.VisualsBulletTracers && BulletTracers::Get().Logs.size() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
			BulletTracers::Get().Logs.erase(BulletTracers::Get().Logs.begin(), BulletTracers::Get().Logs.begin() + BulletTracers::Get().Logs.size());
	}
	int GetEventDebugID()
	{
		return 42;
	}
	void Initialize()
	{
		g_GameEvents->AddListener(this, "player_hurt", false);
		g_GameEvents->AddListener(this, "round_start", false);
		g_GameEvents->AddListener(this, "bullet_impact", false);
	}
	void Shutdown()
	{
		g_GameEvents->RemoveListener(this);
	}
	static EventListener& Get()
	{
		static EventListener inst{};
		return inst;
	}
};
