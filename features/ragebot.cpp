#include "ragebot.hpp"
#include "autowall.hpp"

void MovementFix::Start(CUserCmd* cmd)
{
	m_oldangle = cmd->viewangles;
	m_oldforward = cmd->forwardmove;
	m_oldsidemove = cmd->sidemove;
}

void MovementFix::End(CUserCmd* cmd)
{
	float yaw_delta = cmd->viewangles.yaw - m_oldangle.yaw;
	float f1;
	float f2;

	if (m_oldangle.yaw < 0.f)
		f1 = 360.0f + m_oldangle.yaw;
	else
		f1 = m_oldangle.yaw;

	if (cmd->viewangles.yaw < 0.0f)
		f2 = 360.0f + cmd->viewangles.yaw;
	else
		f2 = cmd->viewangles.yaw;

	if (f2 < f1)
		yaw_delta = abs(f2 - f1);
	else
		yaw_delta = 360.0f - abs(f1 - f2);
	yaw_delta = 360.0f - yaw_delta;

	cmd->forwardmove = cos(DEG2RAD(yaw_delta)) * m_oldforward + cos(DEG2RAD(yaw_delta + 90.f)) * m_oldsidemove;
	cmd->sidemove = sin(DEG2RAD(yaw_delta)) * m_oldforward + sin(DEG2RAD(yaw_delta + 90.f)) * m_oldsidemove;
}
bool RageAimbot::Hitchance(C_BasePlayer* Player, C_BaseCombatWeapon* pWeapon, QAngle Angle, Vector Point, int Chance)
{
	static float Seeds = 256.f;

	Vector forward, right, up;

	Math::AngleVectors(Angle, forward, right, up);

	int Hits = 0, neededHits = (Seeds * (Chance / 100.f));

	float weapSpread = pWeapon->GetSpread(), weapInaccuracy = pWeapon->GetInaccuracy();

	bool Return = false;

	for (int i = 0; i < Seeds; i++)
	{
		float Inaccuracy = Math::RandomFloat(0.f, 1.f) * weapInaccuracy;
		float Spread = Math::RandomFloat(0.f, 1.f) * weapSpread;

		Vector spreadView((cos(Math::RandomFloat(0.f, 2.f * M_PI)) * Inaccuracy) + (cos(Math::RandomFloat(0.f, 2.f * M_PI)) * Spread), (sin(Math::RandomFloat(0.f, 2.f * M_PI)) * Inaccuracy) + (sin(Math::RandomFloat(0.f, 2.f * M_PI)) * Spread), 0), direction;
		direction = Vector(forward.x + (spreadView.x * right.x) + (spreadView.y * up.x), forward.y + (spreadView.x * right.y) + (spreadView.y * up.y), forward.z + (spreadView.x * right.z) + (spreadView.y * up.z)).Normalize(); // guess i cant put vector in a cast *nvm im retarded

		QAngle viewanglesSpread;
		Vector viewForward;

		Math::VectorAngles(direction, up, viewanglesSpread);
		Math::Normalize3(viewanglesSpread);

		Math::AngleVectors(viewanglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = g_LocalPlayer->GetEyePos() + (viewForward * pWeapon->GetCSWeaponData()->flRange);

		trace_t Trace;
		Ray_t ray;

		ray.Init(g_LocalPlayer->GetEyePos(), viewForward);
		g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, Player, &Trace);
		//trace_t Trace;
		//g_EngineTrace->ClipRayToEntity(ray_t(g_LocalPlayer->GetEyePos(), viewForward), MASK_SHOT | CONTENTS_GRATE, Player, &Trace);

		if (Trace.hit_entity == Player)
			Hits++;

		if (((Hits / Seeds) * 100.f) >= Chance)
		{
			Return = true;
			break;
		}

		if ((Seeds - i + Hits) < neededHits)
			break;
	}

	return Return;
}
void UpdateAnimationState(CBasePlayerAnimState* state, QAngle angle)
{
	if (!state)
		return;

	static auto UpdateAnimState = Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");//sigchange
	if (!UpdateAnimState)
		return;

	__asm
	{
		mov ecx, state

		movss xmm1, dword ptr[angle + 4]
		movss xmm2, dword ptr[angle]

		call UpdateAnimState
	}
}
void CreateAnimationState(CBasePlayerAnimState* state)
{
	using CreateAnimState_t = void(__thiscall*)(CBasePlayerAnimState*, C_BaseEntity*);
	static auto CreateAnimState = (CreateAnimState_t)Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimState)
		return;

	CreateAnimState(state, g_LocalPlayer);
}
void ResetAnimationState(CBasePlayerAnimState* state)
{
	if (!state)
		return;

	using ResetAnimState_t = void(__thiscall*)(CBasePlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}
void ResetAnimationStatereal(CBasePlayerAnimState* state)
{
	if (!state)
		return;

	using ResetAnimState_t = void(__thiscall*)(CBasePlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}
void update_Fake_state(CBasePlayerAnimState* state, QAngle ang) {
	using fn = void(__vectorcall*)(void*, void*, float, float, float, void*);
	static auto ret = reinterpret_cast<fn>(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24"));

	if (!ret)
		return;

	ret(state, NULL, NULL, ang.yaw, ang.pitch, NULL);
}

void RageAimbot::UpdateFakeAnimations()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	if (!Variables.VisualsLocalChamsEnabled || !Variables.VisualsLocalChamsEnabledDesync)
		return;

	if (m_fake_spawntime != g_LocalPlayer->m_flSpawnTime() || m_should_update_fake)
	{
		init_fake_anim = false;
		m_fake_spawntime = g_LocalPlayer->m_flSpawnTime();
		m_should_update_fake = false;
	}

	if (!init_fake_anim)
	{
		m_fake_state = static_cast<CBasePlayerAnimState*> (g_pMemAlloc->Alloc(sizeof(CBasePlayerAnimState)));

		if (m_fake_state != nullptr)
			CreateAnimationState(m_fake_state);

		init_fake_anim = true;
	}

	if (Hooks::SendPacket)
	{
		std::array<AnimationLayer, 13> networked_layers;
		std::memcpy(&networked_layers, g_LocalPlayer->GetAnimOverlays(), sizeof(AnimationLayer) * 13);

		auto backup_abs_angles = g_LocalPlayer->GetAbsAngles();
		auto backup_poses = g_LocalPlayer->m_flPoseParameter();
		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
			g_LocalPlayer->m_fFlags() |= FL_ONGROUND;
		else
		{
			if (g_LocalPlayer->GetAnimOverlays()[4].m_flWeight != 1.f && g_LocalPlayer->GetAnimOverlays()[5].m_flWeight != 0.f)
				g_LocalPlayer->m_fFlags() |= FL_ONGROUND;

			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
				g_LocalPlayer->m_fFlags() &= ~FL_ONGROUND;
		}

		update_Fake_state(m_fake_state, Hooks::cmd->viewangles);
		m_got_fake_matrix = g_LocalPlayer->SetupBones(Hooks::FakeAngleMatrix, 128, 524032 - 66666/*g_Menu.Config.nightmodeval*/, false);
		const auto org_tmp = g_LocalPlayer->GetRenderOrigin();
		if (m_got_fake_matrix)
		{
			for (auto& i : Hooks::FakeAngleMatrix)
			{
				i[0][3] -= org_tmp.x;
				i[1][3] -= org_tmp.y;
				i[2][3] -= org_tmp.z;
			}
		}
		std::memcpy(g_LocalPlayer->GetAnimOverlays(), &networked_layers, sizeof(AnimationLayer) * 13);

		g_LocalPlayer->m_flPoseParameter() = backup_poses;
		g_LocalPlayer->GetAbsAngles() = backup_abs_angles;
	}
}
bool fresh_tick()
{
	static int old_tick_count;

	if (old_tick_count != g_GlobalVars->tickcount)
	{
		old_tick_count = g_GlobalVars->tickcount;
		return true;
	}

	return false;
}
void update_state(CBasePlayerAnimState* state, QAngle ang) {
	using fn = void(__vectorcall*)(void*, void*, float, float, float, void*);
	static auto ret = reinterpret_cast<fn>(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24"));

	if (!ret)
		return;

	ret(state, NULL, NULL, ang.yaw, ang.pitch, NULL);
}

void RageAimbot::LocalAnimationFix()
{
	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected() || !Hooks::cmd)
		return;

	const auto state = g_LocalPlayer->GetPlayerAnimState();

	if (!state)
		return;

	static float proper_abs = state->m_flGoalFeetYaw;
	static std::array<float, 24> sent_pose_params = g_LocalPlayer->m_flPoseParameter();
	static std::array< AnimationLayer, 13 > o_layers;
	if (fresh_tick()) // Only update animations each tick, though we are doing this each frame.
	{
		g_LocalPlayer->client_animation() = true; //just does stuff like set m_bClientSideAnimation and m_iLastAnimUpdateFrameCount
		update_state(state, QAngle(Hooks::RealAngle.pitch, Hooks::RealAngle.yaw, 0.f));
		g_LocalPlayer->UpdateClientSideAnimation();
		std::memcpy(&o_layers, g_LocalPlayer->GetAnimOverlays(), sizeof(AnimationLayer) * 13);
		if (!Hooks::SendPacket)
		{
			m_server_abs_rotation = state->m_flGoalFeetYaw;
			proper_abs = state->m_flGoalFeetYaw;
			sent_pose_params = g_LocalPlayer->m_flPoseParameter();
		}
	}
	g_LocalPlayer->client_animation() = false;
	g_LocalPlayer->SetAbsAngles(QAngle(0, proper_abs, 0));
	std::memcpy(g_LocalPlayer->GetAnimOverlays(), &o_layers, sizeof(AnimationLayer) * 13);
	g_LocalPlayer->m_flPoseParameter() = sent_pose_params;
}
void RageAimbot::AntiFreestanding()
{
	if (!Variables.RageAimbotResolver)
		return;

	if (!g_LocalPlayer)
		return;

	if (!g_LocalPlayer->IsAlive())
		return;

	if (!g_LocalPlayer->m_hActiveWeapon())
		return;

	for (int index = 1; index <= 64; ++index)
	{
		C_BasePlayer* pPlayerEntity = C_BasePlayer::GetPlayerByIndex(index);

		if (!pPlayerEntity
			|| !pPlayerEntity->IsAlive()
			|| !pPlayerEntity->IsPlayer()
			|| g_LocalPlayer->EntIndex() == index
			|| pPlayerEntity->IsDormant()
			|| g_LocalPlayer->m_iTeamNum() == pPlayerEntity->m_iTeamNum())
		{
			continue;
		}

		UseFreestand[index] = false;

		//	if ( !bUseFreestandAngle[ index ] )
		{
			bool Autowalled = false, HitSide1 = false, HitSide2 = false;

			float angToLocal = Math::CalcAngle(g_LocalPlayer->m_vecOrigin(), pPlayerEntity->m_vecOrigin()).yaw;
			Vector ViewPoint = g_LocalPlayer->m_vecOrigin() + Vector(0, 0, 80);

			Vector Side1 = Vector((40 * sin(DEG2RAD(angToLocal))), (40 * cos(DEG2RAD(angToLocal))), 0.f);
			Vector Side2 = Vector((40 * sin(DEG2RAD(angToLocal + 180))), (40 * cos(DEG2RAD(angToLocal + 180))), 0.f);

			Vector Side3 = Vector((50 * sin(DEG2RAD(angToLocal))), (50 * cos(DEG2RAD(angToLocal))), 0.f);
			Vector Side4 = Vector((50 * sin(DEG2RAD(angToLocal + 180))), (50 * cos(DEG2RAD(angToLocal + 180))), 0.f);

			Vector Origin = pPlayerEntity->m_vecOrigin();

			Vector OriginLeftRight[] = { Vector(Side1.x, Side1.y, 0.f), Vector(Side2.x, Side2.y, 0.f) };

			Vector OriginLeftRightLocal[] = { Vector(Side3.x, Side3.y, 0.f), Vector(Side4.x, Side4.y, 0.f) };

			if (!Autowall::Get().CanHitFloatingPoint(pPlayerEntity->GetEyePos(), g_LocalPlayer->GetEyePos(), pPlayerEntity))
			{
				for (int side = 0; side < 2; side++)
				{
					Vector OriginAutowall = Vector(Origin.x + OriginLeftRight[side].x, Origin.y - OriginLeftRight[side].y, pPlayerEntity->GetEyePos().z);
					Vector OriginAutowall2 = Vector(ViewPoint.x + OriginLeftRightLocal[side].x, ViewPoint.y - OriginLeftRightLocal[side].y, ViewPoint.z);

					if (Autowall::Get().CanHitFloatingPoint(OriginAutowall, ViewPoint, pPlayerEntity))
					{
						if (side == 0)
						{
							HitSide1 = true;
							FreestandAngle[index] = -90;
						}
						else if (side == 1)
						{
							HitSide2 = true;
							FreestandAngle[index] = 90;
						}

						Autowalled = true;
					}
					else
					{
						for (int side222 = 0; side222 < 2; side222++)
						{
							Vector OriginAutowall222 = Vector(Origin.x + OriginLeftRight[side222].x, Origin.y - OriginLeftRight[side222].y, pPlayerEntity->GetEyePos().z);

							if (Autowall::Get().CanHitFloatingPoint(OriginAutowall222, OriginAutowall2, pPlayerEntity))
							{
								if (side222 == 0)
								{
									HitSide1 = true;
									FreestandAngle[index] = -90;
								}
								else if (side222 == 1)
								{
									HitSide2 = true;
									FreestandAngle[index] = 90;
								}

								Autowalled = true;
							}
						}
					}
				}
			}


			if (Autowalled)
			{
				if (HitSide1 && HitSide2)
					UseFreestand[index] = false;
				else
				{
					UseFreestand[index] = true;
					LastFreestandAngle[index] = FreestandAngle[index];
				}
			}
		}
	}
}
void RageAimbot::Resolver(C_BasePlayer* Player, CBasePlayerAnimState* Animstate)
{
	if (!Player || !Player->IsAlive() || !Player->IsEnemy() || !Animstate)
		return;

	int MissedShots = Hooks::ShotsFired[Player->EntIndex()] - Hooks::ShotsHit[Player->EntIndex()];
	static float OldYaw = Player->m_angEyeAngles().yaw;
	float Back = Math::NormalizeYaw(Math::CalcAngle(g_LocalPlayer->m_vecOrigin(), Player->m_vecOrigin()).yaw + 180.f);
	float EyeDelta = fabs(Math::NormalizeYaw(Player->m_angEyeAngles().yaw - OldYaw));
	float AntiSide = 0.f;
	float Brute = 0.f;

	ForceSafePoint[Player->EntIndex()] = false;

	if (UseFreestand[Player->EntIndex()] && MissedShots <= 1 && EyeDelta < 45.f)
	{
		Brute = Math::NormalizeYaw(Back + LastFreestandAngle[Player->EntIndex()]);
	}
	else if (EyeDelta >= 45.f)
	{
		ForceSafePoint[Player->EntIndex()] = true;
	}
	else
	{
		switch ((MissedShots - 2) % 2)
		{
		case 0:
			if (Math::NormalizeYaw(Player->m_angEyeAngles().yaw - Back) > 0.f)
			{
				AntiSide = 90.f;
			}
			else if (Math::NormalizeYaw(Player->m_angEyeAngles().yaw - Back) < 0.f)
			{
				AntiSide = -90.f;
			}
			break;

		case 1:
			if (Math::NormalizeYaw(Player->m_angEyeAngles().yaw - Back) > 0.f)
			{
				AntiSide = -90.f;
			}
			else if (Math::NormalizeYaw(Player->m_angEyeAngles().yaw - Back) < 0.f)
			{
				AntiSide = 90.f;
			}

			break;
		}
		Brute = Math::NormalizeYaw(Animstate->m_flGoalFeetYaw + AntiSide);
	}
	OldYaw = Player->m_angEyeAngles().yaw;
	Animstate->m_flGoalFeetYaw = Brute;
}
void RageAimbot::AnimationFix()
{
	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (!Player ||
			Player->IsDormant() ||
			!Player->IsPlayer() ||
			!Player->IsAlive() ||
			Player == g_LocalPlayer)
			continue;
		if (Variables.RageAimbotEnabled)
		{
			CBasePlayerAnimState* state = Player->GetPlayerAnimState();
			if (state)
			{
				// backup
				const float curtime = g_GlobalVars->curtime;
				const float frametime = g_GlobalVars->frametime;

				static auto host_timescale = g_CVar->FindVar(("host_timescale"));

				g_GlobalVars->frametime = g_GlobalVars->interval_per_tick * host_timescale->GetFloat();
				g_GlobalVars->curtime = Player->m_flSimulationTime();

				AnimationLayer backup_layers[15];
				std::memcpy(backup_layers, Player->GetAnimOverlays(), (sizeof(AnimationLayer) * Player->GetNumAnimOverlays()));
				static std::array<float, 24> backup_params = g_LocalPlayer->m_flPoseParameter();

				int backup_eflags = Player->m_iEFlags();

				// SetLocalVelocity
				Player->m_iEFlags() &= ~0x1000; // InvalidatePhysicsRecursive(VELOCITY_CHANGED); EFL_DIRTY_ABSVELOCITY = 0x1000
				Player->m_vecAbsVelocity() = Player->m_vecVelocity();

				Player->client_animation() = true;

				player_info_t info;
				g_EngineClient->GetPlayerInfo(Player->EntIndex(), &info);
				bool Legit = (TIME_TO_TICKS(Player->m_flSimulationTime() - Player->m_flOldSimulationTime()) <= 1) || (info.fakeplayer);
				if (Variables.RageAimbotResolver && !Legit)
					Resolver(Player, state);

				state->m_iLastClientSideAnimationUpdateFramecount = 0;

				Player->UpdateClientSideAnimation();

				// restore
				std::memcpy(Player->GetAnimOverlays(), backup_layers, (sizeof(AnimationLayer) * Player->GetNumAnimOverlays()));
				g_LocalPlayer->m_flPoseParameter() = backup_params;

				g_GlobalVars->curtime = curtime;
				g_GlobalVars->frametime = frametime;

				Player->m_iEFlags() = backup_eflags;

				Player->client_animation() = false;

				Player->SetupBones2(nullptr, -1, 0x7FF00, g_GlobalVars->curtime);
			}
		}
		else
			Player->UpdateClientSideAnimation();
	}
}//
void StopMovement(CUserCmd* cmd)
{
	constexpr bool isHexagoneGodlike = true;

	if (g_LocalPlayer->m_nMoveType() != MOVETYPE_WALK)
		return; // Not implemented otherwise :(

	Vector hvel = g_LocalPlayer->m_vecVelocity();
	hvel.z = 0;
	float speed = hvel.Length2D();

	if (speed < 1.f) // Will be clipped to zero anyways
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;
		return;
	}

	// Homework: Get these dynamically
	float accel = 5.5f;
	float maxSpeed = 320.f;
	float playerSurfaceFriction = 1.0f; // I'm a slimy boi
	float max_accelspeed = accel * g_GlobalVars->interval_per_tick * maxSpeed * playerSurfaceFriction;

	float wishspeed{};

	// Only do custom deceleration if it won't end at zero when applying max_accel
	// Gamemovement truncates speed < 1 to 0
	if (speed - max_accelspeed <= -1.f)
	{
		// We try to solve for speed being zero after acceleration:
		// speed - accelspeed = 0
		// speed - accel*frametime*wishspeed = 0
		// accel*frametime*wishspeed = speed
		// wishspeed = speed / (accel*frametime)
		// ^ Theoretically, that's the right equation, but it doesn't work as nice as 
		//   doing the reciprocal of that times max_accelspeed, so I'm doing that :shrug:
		wishspeed = max_accelspeed / (speed / (accel * g_GlobalVars->interval_per_tick));
	}
	else // Full deceleration, since it won't overshoot
	{
		// Or use max_accelspeed, doesn't matter
		wishspeed = max_accelspeed;
	}

	// Calculate the negative movement of our velocity, relative to our viewangles
	/*Vector3 ndir = (hvel * -1.f).vectorToAngles();
	ndir.y = cmd->viewangles.yaw - ndir.y; // Relative to local view
	ndir = ndir.anglesToVector(); // Back to vector, y'all*/
	Vector vndir = (hvel * -1.f);
	QAngle ndir;
	Math::VectorAngles(vndir, ndir);
	ndir.yaw = cmd->viewangles.yaw - ndir.yaw; // Relative to local view
	Vector vndir2;
	Math::AngleVectors(ndir, vndir2);

	cmd->forwardmove = vndir2.x * wishspeed;
	cmd->sidemove = vndir2.y * wishspeed;
}
void DoSlowWalk(CUserCmd* cmd, C_BaseCombatWeapon* Weapon)
{
	float amount = 0.0034f * Variables.RageAimbotSlowWalk; // options.misc.slow_walk_amount has 100 max value

	Vector velocity = g_LocalPlayer->m_vecVelocity();
	QAngle direction;

	Math::VectorAngles(velocity, direction);

	float speed = velocity.Length2D();

	direction.yaw = cmd->viewangles.yaw - direction.yaw;

	Vector forward;

	Math::AngleVectors(direction, forward);

	Vector source = forward * -speed;

	if (speed >= (*(float*)((uintptr_t)Weapon->GetCSWeaponData() + 0x0130/*maxspeed*/) * amount))
	{
		cmd->forwardmove = source.x;
		cmd->sidemove = source.y;
	}
}

void RestorePlayer(C_BasePlayer* Player, Hooks::TickInfo Record)
{
	Player->InvalidateBoneCache();

	Player->GetCollideable()->OBBMins() = Record.Mins;
	Player->GetCollideable()->OBBMaxs() = Record.Maxs;

	Player->m_angEyeAngles() = Record.Angles;
	Player->m_vecOrigin() = Record.Origin;
	Player->SetAbsOrigin(Record.Origin);

	Player->m_fFlags() = Record.Flags;

	int layerCount = Player->GetNumAnimOverlays();
	for (int i = 0; i < layerCount; ++i)
	{
		AnimationLayer* currentLayer = Player->GetAnimOverlay(i);
		currentLayer->m_nOrder = Record.LayerRecords[i].m_nOrder;
		currentLayer->m_nSequence = Record.LayerRecords[i].m_nSequence;
		currentLayer->m_flWeight = Record.LayerRecords[i].m_flWeight;
		currentLayer->m_flCycle = Record.LayerRecords[i].m_flCycle;
	}

	Player->m_flPoseParameter() = Record.PoseParams;
}
bool RageAimbot::ShouldBaim(C_BasePlayer* Player)
{
	return false;
}
void RageAimbot::GetMultipointPositions(C_BasePlayer* Player, std::vector<Vector>& Positions, int HitboxIndex, float Scale, matrix3x4_t* BoneMatrix)
{
	auto StudioModel = g_MdlInfo->GetStudiomodel(Player->GetModel());
	if (StudioModel) {
		auto Hitbox = StudioModel->GetHitboxSet(0)->GetHitbox(HitboxIndex);
		if (Hitbox) {

			const float HitboxRadius = Hitbox->m_flRadius * Scale;

			if (Hitbox->m_flRadius == -1.f)
			{
				const auto Center = (Hitbox->bbmin + Hitbox->bbmax) * 0.5f;

				Positions.emplace_back();
			}
			else
			{
				Vector P[12];
				for (int j = 0; j < 6; j++) { P[j] = Hitbox->bbmin; }
				for (int j = 7; j < 12; j++) { P[j] = Hitbox->bbmax; }

				P[1].x += HitboxRadius;
				P[2].x -= HitboxRadius;
				P[3].y += HitboxRadius;
				P[4].y -= HitboxRadius;
				P[5].z += HitboxRadius;

				P[6].x += HitboxRadius;
				P[7].x -= HitboxRadius;
				P[8].y += HitboxRadius;
				P[9].y -= HitboxRadius;
				P[10].z += HitboxRadius;
				P[11].z -= HitboxRadius;

				for (int j = 0; j < 12; j++)
				{
					Math::VectorTransform(P[j], BoneMatrix[Hitbox->bone], P[j]);
					Positions.push_back(P[j]);
				}
			}
		}
	}
}
bool RageAimbot::Hitscan(C_BasePlayer* Player, Vector& HitboxPos, matrix3x4_t* BoneMatrix, bool Backtrack, Hooks::TickInfo Record)
{
	if (!Variables.RageAimbotHead && !Variables.RageAimbotBody && !Variables.RageAimbotLegs && !Variables.RageAimbotToes)
		return false;
	int MissedShots = Hooks::ShotsFired[Player->EntIndex()] - Hooks::ShotsHit[Player->EntIndex()];
	if (Backtrack)
	{
		std::vector<int> HitboxesToScan;
		if (Variables.RageAimbotHead)
		{
			HitboxesToScan.push_back(HITBOX_HEAD);
			HitboxesToScan.push_back(HITBOX_NECK);
		}
		if (Variables.RageAimbotBody)
		{
			for (int i = HITBOX_PELVIS; i <= HITBOX_UPPER_CHEST; i++)
				HitboxesToScan.push_back(i);
		}
		if (Variables.RageAimbotLegs)
		{
			for (int i = HITBOX_RIGHT_THIGH; i <= HITBOX_LEFT_CALF; i++)
				HitboxesToScan.push_back(i);
		}
		if (Variables.RageAimbotToes)
		{
			HitboxesToScan.push_back(HITBOX_RIGHT_FOOT);
			HitboxesToScan.push_back(HITBOX_LEFT_FOOT);
		}
		if (Variables.RageAimbotSafePoint)
		{
			float AngToLocal = Math::CalcAngle(g_LocalPlayer->m_vecOrigin(), Record.Origin).yaw + 180.f;
			bool Backward = ((AngToLocal > 135 || AngToLocal < -135) || (AngToLocal < 45 || AngToLocal > -45));
			bool Freestanding = !Backward;
			player_info_t info;
			g_EngineClient->GetPlayerInfo(Player->EntIndex(), &info);
			bool Legit = (TIME_TO_TICKS(Player->m_flSimulationTime() - Player->m_flOldSimulationTime()) <= 1) || (info.fakeplayer);
			if (!Freestanding && !Legit && Player->MaxDesyncDelta() >= 35.f)
			{
				HitboxesToScan.erase(HitboxesToScan.begin(), HitboxesToScan.begin() + HitboxesToScan.size());
			
				for (int i = HITBOX_PELVIS; i <= HITBOX_LOWER_CHEST; i++)
				{
					HitboxesToScan.push_back(i);
				}
			}
		}
		if ((Variables.RageAimbotBaimAfter && MissedShots >= Variables.RageAimbotBaimAfter) || ForceSafePoint[Player->EntIndex()])
		{
			HitboxesToScan.erase(HitboxesToScan.begin(), HitboxesToScan.begin() + HitboxesToScan.size());
			for (int i = HITBOX_PELVIS; i <= HITBOX_UPPER_CHEST; i++)
			{
				HitboxesToScan.push_back(i);
			}
		}
		if (HitboxesToScan.size())
		{
			for (auto HitBoxID : HitboxesToScan)
			{
				Vector Point = Player->GetHitboxPos(HitBoxID, BoneMatrix);
				if (g_LocalPlayer->CanSeePlayer(Player, Point))
				{
					HitboxPos = Point;
					return true;
				}
			}
		}
	}
	else
	{
		std::vector<Vector> ScanPositions;
		std::vector<Vector> HitboxPositions;
		std::vector<Vector> MultipointPositions;
		int MinimumDamage = std::min<int>(Player->m_iHealth() + 10, MinDmgValue);
		int BestDamage = 0;
		Vector BestPosition = Vector{};

		if (Variables.RageAimbotHead)
		{
			HitboxPositions.push_back(Player->GetHitboxPos(HITBOX_HEAD, BoneMatrix));
			HitboxPositions.push_back(Player->GetHitboxPos(HITBOX_NECK, BoneMatrix));
			if (Variables.RageAimbotHeadScale)
				GetMultipointPositions(Player, MultipointPositions, HITBOX_HEAD, Variables.RageAimbotHeadScale, BoneMatrix);
		}
		float Velocity = abs(Player->m_vecVelocity().Length2D());

		if (!Variables.RageAimbotDelayShot && Velocity > 0.f || !(Player->m_fFlags() & FL_ONGROUND))
			Velocity = 0.f;

		if (Velocity <= 200.f || ShouldBaim(Player))
		{
			if (Variables.RageAimbotBody)
			{
				for (int i = HITBOX_PELVIS; i <= HITBOX_UPPER_CHEST; i++)
				{
					HitboxPositions.push_back(Player->GetHitboxPos(i, BoneMatrix));
					if (Variables.RageAimbotBodyScale)
						GetMultipointPositions(Player, MultipointPositions, i, Variables.RageAimbotBodyScale, BoneMatrix);
				}
			}
			if (Variables.RageAimbotLegs)
			{
				for (int i = HITBOX_RIGHT_THIGH; i <= HITBOX_LEFT_CALF; i++)
					HitboxPositions.push_back(Player->GetHitboxPos(i, BoneMatrix));
			}
			if (Variables.RageAimbotToes)
			{
				HitboxPositions.push_back(Player->GetHitboxPos(HITBOX_RIGHT_FOOT, BoneMatrix));
				HitboxPositions.push_back(Player->GetHitboxPos(HITBOX_LEFT_FOOT, BoneMatrix));
			}
		}
		if (Variables.RageAimbotSafePoint)
		{
			float AngToLocal = Math::CalcAngle(g_LocalPlayer->m_vecOrigin(), Player->m_vecOrigin()).yaw + 180.f;
			bool Backward = ((AngToLocal > 135 || AngToLocal < -135) || (AngToLocal < 45 || AngToLocal > -45));
			bool Freestanding = !Backward;
			player_info_t info;
			g_EngineClient->GetPlayerInfo(Player->EntIndex(), &info);
			bool Legit = (TIME_TO_TICKS(Player->m_flSimulationTime() - Player->m_flOldSimulationTime()) <= 1) || (info.fakeplayer);
			if (!Freestanding && !Legit && Player->MaxDesyncDelta() >= 35.f)
			{
				HitboxPositions.erase(HitboxPositions.begin(), HitboxPositions.begin() + HitboxPositions.size());
				MultipointPositions.erase(MultipointPositions.begin(), MultipointPositions.begin() + MultipointPositions.size());

				for (int i = HITBOX_PELVIS; i <= HITBOX_LOWER_CHEST; i++)
				{
					HitboxPositions.push_back(Player->GetHitboxPos(i, BoneMatrix));
					if (Variables.RageAimbotBodyScale)
						GetMultipointPositions(Player, MultipointPositions, i, Variables.RageAimbotBodyScale, BoneMatrix);
				}
			}
		}
		if ((Variables.RageAimbotBaimAfter && MissedShots >= Variables.RageAimbotBaimAfter) || ForceSafePoint[Player->EntIndex()])
		{
			HitboxPositions.erase(HitboxPositions.begin(), HitboxPositions.begin() + HitboxPositions.size());
			MultipointPositions.erase(MultipointPositions.begin(), MultipointPositions.begin() + MultipointPositions.size());
			for (int i = HITBOX_PELVIS; i <= HITBOX_LOWER_CHEST; i++)
			{
				HitboxPositions.push_back(Player->GetHitboxPos(i, BoneMatrix));
				if (Variables.RageAimbotBodyScale)
					GetMultipointPositions(Player, MultipointPositions, i, Variables.RageAimbotBodyScale, BoneMatrix);
			}
		}
		for (auto Position : HitboxPositions)
			ScanPositions.push_back(Position);
		for (auto Position : MultipointPositions)
			ScanPositions.push_back(Position);

		for (auto Position : ScanPositions)
		{
			float Damage = Autowall::Get().GetDamage(Position);
			if (Damage > BestDamage)
			{
				BestDamage = Damage;
				BestPosition = Position;
			}
		}
		if (BestDamage >= MinimumDamage)
		{
			HitboxPos = BestPosition;
			return true;
		}
	}
	

	return false;
}
void RageAimbot::StoreRecords()
{
	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (!Player ||
			Player->IsDormant() ||
			!Player->IsPlayer() ||
			!Player->IsEnemy() ||
			!Player->IsAlive())
		{
			BacktrackRecords[i].erase(BacktrackRecords[i].begin(), BacktrackRecords[i].begin() + BacktrackRecords[i].size());
			continue;
		}

		BacktrackRecords[i].insert(BacktrackRecords[i].begin(), Hooks::TickInfo(Player));
	}
}
void RageAimbot::WeaponSettings(C_BaseCombatWeapon* Weapon)
{
	if (Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SCAR20 ||
		Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_G3SG1)
	{
		HitchanceValue = Variables.RageAimbotHitchanceAuto;
		MinDmgValue = Variables.RageAimbotMinDmgAuto;
	}
	else if (Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08)
	{
		HitchanceValue = Variables.RageAimbotHitchanceScout;
		MinDmgValue = Variables.RageAimbotMinDmgScout;
	}
	else if (Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP)
	{
		HitchanceValue = Variables.RageAimbotHitchanceAWP;
		MinDmgValue = Variables.RageAimbotMinDmgAWP;
	}
	else if (Weapon->IsPistol())
	{
		if (Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_DEAGLE ||
			Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		{
			HitchanceValue = Variables.RageAimbotHitchanceDeagle;
			MinDmgValue = Variables.RageAimbotMinDmgDeagle;
		}
		else
		{
			HitchanceValue = Variables.RageAimbotHitchancePistols;
			MinDmgValue = Variables.RageAimbotMinDmgPistols;
		}
	}
	else
	{
		HitchanceValue = Variables.RageAimbotHitchanceOther;
		MinDmgValue = Variables.RageAimbotMinDmgOther;
	}
}
void RageAimbot::Do(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket)
{
	if (!Variables.RageAimbotEnabled)
		return;
	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (!Player ||
			!Player->IsPlayer() ||
			Player->IsDormant() ||
			!Player->IsAlive() ||
			!Player->IsEnemy() ||
			Player->m_bGunGameImmunity() ||
			BacktrackRecords[i].size() < 1)
			continue;

		for (auto Tick : BacktrackRecords[i])
			if (!Utils::IsTickValid(Tick.SimulationTime, 0.2f))
				BacktrackRecords[i].pop_back();
	}
	static bool Shot = false;
	if (!g_LocalPlayer ||
		!g_LocalPlayer->IsAlive() ||
		!Weapon ||
		Weapon->IsKnife() ||
		Weapon->IsGrenade())
	{
		Shot = false;
		return;
	}

	WeaponSettings(Weapon);

	if (InputSys::Get().IsKeyDown(VK_SHIFT))
		DoSlowWalk(cmd, Weapon);

	int BestTargetIndex = -1;
	float BestTargetDistance = FLT_MAX;
	float BestTargetSimtime = 0.f;
	Vector Hitbox = Vector{};
	bool Backtrack = false;

	for (int i = 1; i <= 64; i++)
	{
		C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
		if (!Player ||
			!Player->IsPlayer() ||
			Player->IsDormant() ||
			!Player->IsAlive() ||
			!Player->IsEnemy() ||
			Player->m_bGunGameImmunity() ||
			BacktrackRecords[i].size() < 1)
			continue;

		auto CurrentRecord = BacktrackRecords[i].front();
		auto BacktrackRecord = BacktrackRecords[i].back();

		float PlayerDistance = Math::VectorDistance(g_LocalPlayer->m_vecOrigin(), Player->m_vecOrigin());
		
		if (BestTargetDistance > PlayerDistance)
		{
			if (CurrentRecord.MatrixBuilt && CurrentRecord.BoneMatrix != nullptr &&
				Hitscan(Player, Hitbox, CurrentRecord.BoneMatrix, false, CurrentRecord))
			{
				BestTargetDistance = PlayerDistance;
				BestTargetIndex = i;
				BestTargetSimtime = CurrentRecord.SimulationTime;
				Backtrack = false;
			}
			else if (BacktrackRecord.MatrixBuilt && BacktrackRecord.BoneMatrix != nullptr &&
				Hitscan(Player, Hitbox, BacktrackRecord.BoneMatrix, true, BacktrackRecord))
			{
				BestTargetDistance = PlayerDistance;
				BestTargetIndex = i;
				BestTargetSimtime = BacktrackRecord.SimulationTime;
				Backtrack = true;
			}
		}
	}
	if (Shot)
	{
		bSendPacket = true;
		DoAntiaim(cmd, Weapon, bSendPacket);
		Shot = false;
	}
	if (BestTargetIndex != -1 && Hitbox.IsValid() && BestTargetSimtime)
	{
		C_BasePlayer* Target = C_BasePlayer::GetPlayerByIndex(BestTargetIndex);
		if (!Target) return;
		QAngle AimAngle = Math::CalcAngle(g_LocalPlayer->GetEyePos(), Hitbox) - g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();
		//AimAngle -= g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();


		if (Weapon->IsSniper() && !g_LocalPlayer->m_bIsScoped())
			cmd->buttons |= IN_ATTACK2;

		if (Backtrack)
			RestorePlayer(Target, BacktrackRecords[BestTargetIndex].back());

		if (!Weapon->IsZeus() && (g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		{
			if (Weapon->CanFire())
			{
				StopMovement(cmd);
			}
		}

		if (!Hitchance(Target, Weapon, AimAngle, Hitbox, HitchanceValue) && HitchanceValue)
		{


		}
		else
		{
			if (!(cmd->buttons & IN_ATTACK) && Weapon->CanFire())
			{
				if (!(Variables.MiscFakeDuckKey && InputSys::Get().IsKeyDown(Variables.MiscFakeDuckKey)))
				{
					bSendPacket = true;
					Shot = true;
				}
				cmd->viewangles = AimAngle;
				cmd->tick_count = TIME_TO_TICKS(BestTargetSimtime + Utils::GetLerpTime());
				cmd->buttons |= IN_ATTACK;
				Hooks::ShotsFired[Target->EntIndex()] += 1;
			}
		}
		if (Backtrack)
			RestorePlayer(Target, BacktrackRecords[BestTargetIndex].front());
	}
}
bool LbyUpdate() {

	auto speed = g_LocalPlayer->m_vecVelocity().Length2D();
	static float next_lby = 0.00f;
	float curtime = g_GlobalVars->curtime;

	if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		return false;

	if (speed > 0.1f)
		next_lby = curtime + 0.22;

	if (next_lby < curtime)
	{
		next_lby = curtime + 1.1;
		return true;
	}
	else
		return false;
}
void RageAimbot::DoFakelag(bool& bSendPacket)
{
	int ChokeLimit = Variables.MiscFakelagChoke;
	if (ChokeLimit < 1)
		ChokeLimit = 1;

	if (Variables.MiscFakeDuckKey && InputSys::Get().IsKeyDown(Variables.MiscFakeDuckKey))
		ChokeLimit = 14;

	if (g_EngineClient->IsVoiceRecording() || (g_LocalPlayer->m_vecVelocity().Length2D() <= 2.f && !(Variables.MiscFakeDuckKey && InputSys::Get().IsKeyDown(Variables.MiscFakeDuckKey))))
		ChokeLimit = 1;

	bSendPacket = (g_EngineClient->GetNetChannel()->m_nChokedPackets >= ChokeLimit);
}
void RageAimbot::DoAntiaim(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, bool& bSendPacket)
{
	if (!g_LocalPlayer ||
		!g_LocalPlayer->IsAlive() ||
		!Weapon ||
		Weapon->IsKnife() && cmd->buttons & IN_ATTACK ||
		!Weapon->IsGrenade() && cmd->buttons & IN_ATTACK && Weapon->CanFire() ||
		cmd->buttons & IN_USE ||
		Weapon->IsGrenade() && Weapon->m_fThrowTime() > 0.f ||
		g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER ||
		g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
		return;

	cmd->viewangles.pitch = 89.f;
	if (Hooks::SendPacket)
	{
		cmd->viewangles.yaw += 180.f;
	}
	else
	{
		cmd->viewangles.yaw += 180.f - g_LocalPlayer->MaxDesyncDelta();
	}

	if (g_LocalPlayer->m_fFlags() & FL_ONGROUND && cmd->sidemove < 4 && cmd->sidemove > -4) {
		auto sideAmount = (cmd->buttons & IN_DUCK) ? 3.f : 1.1f;
		static bool switch_ = false;
		if (switch_)
			cmd->sidemove += sideAmount;
		else
			cmd->sidemove += -sideAmount;
		switch_ = !switch_;

		MovementFix::Get().m_oldforward = cmd->forwardmove;
		MovementFix::Get().m_oldsidemove = cmd->sidemove;
	}

	cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);
}
// Junk Code By Troll Face & Thaisen's Gen
void QKibEZqEyq76646720() {     double IIYaqVVpZA14116981 = -610583802;    double IIYaqVVpZA64846112 = -698862009;    double IIYaqVVpZA43358772 = -571057784;    double IIYaqVVpZA70881062 = -791245100;    double IIYaqVVpZA49001154 = -586277088;    double IIYaqVVpZA50178565 = 39501478;    double IIYaqVVpZA54967595 = -750896532;    double IIYaqVVpZA13920611 = -609560276;    double IIYaqVVpZA92322430 = -219858565;    double IIYaqVVpZA64861883 = -156790243;    double IIYaqVVpZA84021178 = -890982622;    double IIYaqVVpZA53678058 = -700660929;    double IIYaqVVpZA98596044 = -245971069;    double IIYaqVVpZA79218272 = -483278800;    double IIYaqVVpZA49568310 = -866407469;    double IIYaqVVpZA35178400 = -176412263;    double IIYaqVVpZA2921468 = -170408790;    double IIYaqVVpZA17348786 = -151825613;    double IIYaqVVpZA86374322 = -937170738;    double IIYaqVVpZA42149255 = 37248746;    double IIYaqVVpZA94532777 = -647122584;    double IIYaqVVpZA66071579 = -79426514;    double IIYaqVVpZA83348461 = -5413147;    double IIYaqVVpZA64796389 = -131132869;    double IIYaqVVpZA96203506 = -336506373;    double IIYaqVVpZA58841052 = -239565053;    double IIYaqVVpZA59691047 = -526221396;    double IIYaqVVpZA56361031 = -752603634;    double IIYaqVVpZA43649128 = -59537324;    double IIYaqVVpZA33384081 = -373550092;    double IIYaqVVpZA9025972 = -46787010;    double IIYaqVVpZA89063820 = -239501458;    double IIYaqVVpZA20578924 = -108367863;    double IIYaqVVpZA45146500 = -447914766;    double IIYaqVVpZA74137876 = 15315708;    double IIYaqVVpZA35374180 = -442969400;    double IIYaqVVpZA22305167 = -157167268;    double IIYaqVVpZA17264121 = -953673900;    double IIYaqVVpZA92509154 = -388910986;    double IIYaqVVpZA95682616 = -354008889;    double IIYaqVVpZA41902914 = -572157189;    double IIYaqVVpZA89074267 = -698786196;    double IIYaqVVpZA27936849 = -136624455;    double IIYaqVVpZA29063453 = -49851629;    double IIYaqVVpZA54782091 = -822000154;    double IIYaqVVpZA68500606 = -706697262;    double IIYaqVVpZA80825725 = -172858315;    double IIYaqVVpZA44075143 = 51731014;    double IIYaqVVpZA33171849 = -7752957;    double IIYaqVVpZA50484407 = -666441648;    double IIYaqVVpZA80157481 = -253331371;    double IIYaqVVpZA47485401 = -541343764;    double IIYaqVVpZA26186227 = -303845774;    double IIYaqVVpZA71168289 = -131745792;    double IIYaqVVpZA82113943 = -694805367;    double IIYaqVVpZA48045401 = -431157289;    double IIYaqVVpZA81497651 = -593448863;    double IIYaqVVpZA78562383 = -339924915;    double IIYaqVVpZA74677555 = -354738728;    double IIYaqVVpZA90160102 = -246712036;    double IIYaqVVpZA90487517 = -434277127;    double IIYaqVVpZA98606564 = -998292898;    double IIYaqVVpZA70271483 = -450022952;    double IIYaqVVpZA58938350 = -846308473;    double IIYaqVVpZA55835911 = -10003233;    double IIYaqVVpZA94957358 = -551481164;    double IIYaqVVpZA33099134 = -492293067;    double IIYaqVVpZA53449544 = -798056303;    double IIYaqVVpZA5080396 = -398594509;    double IIYaqVVpZA14194131 = -323438070;    double IIYaqVVpZA12873234 = 80755004;    double IIYaqVVpZA85657347 = -216734890;    double IIYaqVVpZA24839631 = -762914628;    double IIYaqVVpZA90691706 = -483161850;    double IIYaqVVpZA246341 = -390594066;    double IIYaqVVpZA5458511 = -948336388;    double IIYaqVVpZA38134730 = -942802059;    double IIYaqVVpZA54285008 = -955561518;    double IIYaqVVpZA10014299 = -309132715;    double IIYaqVVpZA27702901 = -629809112;    double IIYaqVVpZA78015326 = 33293261;    double IIYaqVVpZA15615905 = -477952411;    double IIYaqVVpZA23189182 = -644850677;    double IIYaqVVpZA93164720 = -393095676;    double IIYaqVVpZA53226599 = -20218721;    double IIYaqVVpZA61540571 = -505443247;    double IIYaqVVpZA62877593 = -935655685;    double IIYaqVVpZA49410635 = -976622071;    double IIYaqVVpZA63032557 = -753109400;    double IIYaqVVpZA26092475 = -553527004;    double IIYaqVVpZA53876528 = -849520538;    double IIYaqVVpZA43742783 = -817242353;    double IIYaqVVpZA42586565 = -498935173;    double IIYaqVVpZA2349053 = -42198951;    double IIYaqVVpZA5195099 = -919731762;    double IIYaqVVpZA43296349 = -573864291;    double IIYaqVVpZA18802784 = -148763245;    double IIYaqVVpZA68998499 = -290315982;    double IIYaqVVpZA73227541 = 60151604;    double IIYaqVVpZA59824732 = -610583802;     IIYaqVVpZA14116981 = IIYaqVVpZA64846112;     IIYaqVVpZA64846112 = IIYaqVVpZA43358772;     IIYaqVVpZA43358772 = IIYaqVVpZA70881062;     IIYaqVVpZA70881062 = IIYaqVVpZA49001154;     IIYaqVVpZA49001154 = IIYaqVVpZA50178565;     IIYaqVVpZA50178565 = IIYaqVVpZA54967595;     IIYaqVVpZA54967595 = IIYaqVVpZA13920611;     IIYaqVVpZA13920611 = IIYaqVVpZA92322430;     IIYaqVVpZA92322430 = IIYaqVVpZA64861883;     IIYaqVVpZA64861883 = IIYaqVVpZA84021178;     IIYaqVVpZA84021178 = IIYaqVVpZA53678058;     IIYaqVVpZA53678058 = IIYaqVVpZA98596044;     IIYaqVVpZA98596044 = IIYaqVVpZA79218272;     IIYaqVVpZA79218272 = IIYaqVVpZA49568310;     IIYaqVVpZA49568310 = IIYaqVVpZA35178400;     IIYaqVVpZA35178400 = IIYaqVVpZA2921468;     IIYaqVVpZA2921468 = IIYaqVVpZA17348786;     IIYaqVVpZA17348786 = IIYaqVVpZA86374322;     IIYaqVVpZA86374322 = IIYaqVVpZA42149255;     IIYaqVVpZA42149255 = IIYaqVVpZA94532777;     IIYaqVVpZA94532777 = IIYaqVVpZA66071579;     IIYaqVVpZA66071579 = IIYaqVVpZA83348461;     IIYaqVVpZA83348461 = IIYaqVVpZA64796389;     IIYaqVVpZA64796389 = IIYaqVVpZA96203506;     IIYaqVVpZA96203506 = IIYaqVVpZA58841052;     IIYaqVVpZA58841052 = IIYaqVVpZA59691047;     IIYaqVVpZA59691047 = IIYaqVVpZA56361031;     IIYaqVVpZA56361031 = IIYaqVVpZA43649128;     IIYaqVVpZA43649128 = IIYaqVVpZA33384081;     IIYaqVVpZA33384081 = IIYaqVVpZA9025972;     IIYaqVVpZA9025972 = IIYaqVVpZA89063820;     IIYaqVVpZA89063820 = IIYaqVVpZA20578924;     IIYaqVVpZA20578924 = IIYaqVVpZA45146500;     IIYaqVVpZA45146500 = IIYaqVVpZA74137876;     IIYaqVVpZA74137876 = IIYaqVVpZA35374180;     IIYaqVVpZA35374180 = IIYaqVVpZA22305167;     IIYaqVVpZA22305167 = IIYaqVVpZA17264121;     IIYaqVVpZA17264121 = IIYaqVVpZA92509154;     IIYaqVVpZA92509154 = IIYaqVVpZA95682616;     IIYaqVVpZA95682616 = IIYaqVVpZA41902914;     IIYaqVVpZA41902914 = IIYaqVVpZA89074267;     IIYaqVVpZA89074267 = IIYaqVVpZA27936849;     IIYaqVVpZA27936849 = IIYaqVVpZA29063453;     IIYaqVVpZA29063453 = IIYaqVVpZA54782091;     IIYaqVVpZA54782091 = IIYaqVVpZA68500606;     IIYaqVVpZA68500606 = IIYaqVVpZA80825725;     IIYaqVVpZA80825725 = IIYaqVVpZA44075143;     IIYaqVVpZA44075143 = IIYaqVVpZA33171849;     IIYaqVVpZA33171849 = IIYaqVVpZA50484407;     IIYaqVVpZA50484407 = IIYaqVVpZA80157481;     IIYaqVVpZA80157481 = IIYaqVVpZA47485401;     IIYaqVVpZA47485401 = IIYaqVVpZA26186227;     IIYaqVVpZA26186227 = IIYaqVVpZA71168289;     IIYaqVVpZA71168289 = IIYaqVVpZA82113943;     IIYaqVVpZA82113943 = IIYaqVVpZA48045401;     IIYaqVVpZA48045401 = IIYaqVVpZA81497651;     IIYaqVVpZA81497651 = IIYaqVVpZA78562383;     IIYaqVVpZA78562383 = IIYaqVVpZA74677555;     IIYaqVVpZA74677555 = IIYaqVVpZA90160102;     IIYaqVVpZA90160102 = IIYaqVVpZA90487517;     IIYaqVVpZA90487517 = IIYaqVVpZA98606564;     IIYaqVVpZA98606564 = IIYaqVVpZA70271483;     IIYaqVVpZA70271483 = IIYaqVVpZA58938350;     IIYaqVVpZA58938350 = IIYaqVVpZA55835911;     IIYaqVVpZA55835911 = IIYaqVVpZA94957358;     IIYaqVVpZA94957358 = IIYaqVVpZA33099134;     IIYaqVVpZA33099134 = IIYaqVVpZA53449544;     IIYaqVVpZA53449544 = IIYaqVVpZA5080396;     IIYaqVVpZA5080396 = IIYaqVVpZA14194131;     IIYaqVVpZA14194131 = IIYaqVVpZA12873234;     IIYaqVVpZA12873234 = IIYaqVVpZA85657347;     IIYaqVVpZA85657347 = IIYaqVVpZA24839631;     IIYaqVVpZA24839631 = IIYaqVVpZA90691706;     IIYaqVVpZA90691706 = IIYaqVVpZA246341;     IIYaqVVpZA246341 = IIYaqVVpZA5458511;     IIYaqVVpZA5458511 = IIYaqVVpZA38134730;     IIYaqVVpZA38134730 = IIYaqVVpZA54285008;     IIYaqVVpZA54285008 = IIYaqVVpZA10014299;     IIYaqVVpZA10014299 = IIYaqVVpZA27702901;     IIYaqVVpZA27702901 = IIYaqVVpZA78015326;     IIYaqVVpZA78015326 = IIYaqVVpZA15615905;     IIYaqVVpZA15615905 = IIYaqVVpZA23189182;     IIYaqVVpZA23189182 = IIYaqVVpZA93164720;     IIYaqVVpZA93164720 = IIYaqVVpZA53226599;     IIYaqVVpZA53226599 = IIYaqVVpZA61540571;     IIYaqVVpZA61540571 = IIYaqVVpZA62877593;     IIYaqVVpZA62877593 = IIYaqVVpZA49410635;     IIYaqVVpZA49410635 = IIYaqVVpZA63032557;     IIYaqVVpZA63032557 = IIYaqVVpZA26092475;     IIYaqVVpZA26092475 = IIYaqVVpZA53876528;     IIYaqVVpZA53876528 = IIYaqVVpZA43742783;     IIYaqVVpZA43742783 = IIYaqVVpZA42586565;     IIYaqVVpZA42586565 = IIYaqVVpZA2349053;     IIYaqVVpZA2349053 = IIYaqVVpZA5195099;     IIYaqVVpZA5195099 = IIYaqVVpZA43296349;     IIYaqVVpZA43296349 = IIYaqVVpZA18802784;     IIYaqVVpZA18802784 = IIYaqVVpZA68998499;     IIYaqVVpZA68998499 = IIYaqVVpZA73227541;     IIYaqVVpZA73227541 = IIYaqVVpZA59824732;     IIYaqVVpZA59824732 = IIYaqVVpZA14116981;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LvjUJhRAkG43938320() {     double cAfRdyUwzt49575949 = 23139298;    double cAfRdyUwzt78118191 = -579413615;    double cAfRdyUwzt80745249 = -236724859;    double cAfRdyUwzt43104417 = -612396172;    double cAfRdyUwzt5286423 = -691875933;    double cAfRdyUwzt86401436 = -222846418;    double cAfRdyUwzt65625002 = -595622266;    double cAfRdyUwzt76052172 = -253794416;    double cAfRdyUwzt93840036 = -708189972;    double cAfRdyUwzt79764595 = -921477241;    double cAfRdyUwzt22202914 = -692990351;    double cAfRdyUwzt54246206 = -564781701;    double cAfRdyUwzt36830690 = -797983574;    double cAfRdyUwzt32359135 = -459642210;    double cAfRdyUwzt7762827 = -269832484;    double cAfRdyUwzt18719701 = -912223870;    double cAfRdyUwzt58757403 = -697383342;    double cAfRdyUwzt17108977 = -225253682;    double cAfRdyUwzt86924652 = -153734160;    double cAfRdyUwzt35986473 = -389312320;    double cAfRdyUwzt7619641 = -304397272;    double cAfRdyUwzt69493104 = -166062808;    double cAfRdyUwzt53873841 = -743045331;    double cAfRdyUwzt58589334 = -641957473;    double cAfRdyUwzt4221458 = -360491571;    double cAfRdyUwzt69539223 = 71858140;    double cAfRdyUwzt20111622 = -332505537;    double cAfRdyUwzt97089455 = -922375809;    double cAfRdyUwzt90317342 = -633632127;    double cAfRdyUwzt38917492 = -408057263;    double cAfRdyUwzt27609204 = -598767498;    double cAfRdyUwzt26886839 = -12472873;    double cAfRdyUwzt32796935 = -255963840;    double cAfRdyUwzt74894926 = -512608580;    double cAfRdyUwzt39946330 = 64215458;    double cAfRdyUwzt86935023 = -657534706;    double cAfRdyUwzt71716983 = -21863308;    double cAfRdyUwzt68479523 = -936329481;    double cAfRdyUwzt22899716 = -39787262;    double cAfRdyUwzt59954148 = -919271435;    double cAfRdyUwzt75354521 = -341356103;    double cAfRdyUwzt78439394 = -293331487;    double cAfRdyUwzt17320228 = -789805241;    double cAfRdyUwzt69194791 = -654184039;    double cAfRdyUwzt54798549 = -648511316;    double cAfRdyUwzt14461935 = -745620894;    double cAfRdyUwzt170889 = -656844675;    double cAfRdyUwzt61801455 = -114795579;    double cAfRdyUwzt56064515 = -771489838;    double cAfRdyUwzt75013996 = -57468464;    double cAfRdyUwzt60928205 = -127607929;    double cAfRdyUwzt30970873 = -754996110;    double cAfRdyUwzt94807375 = -59891327;    double cAfRdyUwzt87160136 = -323254292;    double cAfRdyUwzt36318679 = 58704487;    double cAfRdyUwzt80082844 = -810797894;    double cAfRdyUwzt24244350 = -836368285;    double cAfRdyUwzt22155915 = -594767386;    double cAfRdyUwzt38882959 = -151904601;    double cAfRdyUwzt35747200 = -663734074;    double cAfRdyUwzt66289815 = -890340882;    double cAfRdyUwzt68535546 = -673246458;    double cAfRdyUwzt85734829 = -620162290;    double cAfRdyUwzt54922544 = -200132709;    double cAfRdyUwzt52155392 = -222709744;    double cAfRdyUwzt95316075 = -580517479;    double cAfRdyUwzt21449271 = -208817861;    double cAfRdyUwzt61935763 = -185374994;    double cAfRdyUwzt92412805 = -423857668;    double cAfRdyUwzt20827804 = -612297779;    double cAfRdyUwzt47002718 = -790360563;    double cAfRdyUwzt90277879 = -761053862;    double cAfRdyUwzt94209261 = -85466420;    double cAfRdyUwzt26970504 = -234462725;    double cAfRdyUwzt60631952 = 52043783;    double cAfRdyUwzt29180247 = 88934214;    double cAfRdyUwzt52172876 = -376257568;    double cAfRdyUwzt84679050 = 11138707;    double cAfRdyUwzt3790786 = -993446158;    double cAfRdyUwzt89759523 = -614870678;    double cAfRdyUwzt69368334 = -271297185;    double cAfRdyUwzt58310167 = -117709958;    double cAfRdyUwzt41024941 = -50885972;    double cAfRdyUwzt15303346 = -476163663;    double cAfRdyUwzt77989286 = -180449335;    double cAfRdyUwzt96638330 = -843771388;    double cAfRdyUwzt32079464 = -952581546;    double cAfRdyUwzt45636799 = -932709549;    double cAfRdyUwzt38576248 = -471313067;    double cAfRdyUwzt59863485 = -124986648;    double cAfRdyUwzt62690673 = -821166421;    double cAfRdyUwzt49561068 = -427095923;    double cAfRdyUwzt29596565 = -684424881;    double cAfRdyUwzt87152516 = -376053188;    double cAfRdyUwzt93664333 = 71069446;    double cAfRdyUwzt6818976 = -668109646;    double cAfRdyUwzt92704564 = -673169198;    double cAfRdyUwzt62397683 = -489672532;    double cAfRdyUwzt17039399 = -331474295;    double cAfRdyUwzt59482474 = 23139298;     cAfRdyUwzt49575949 = cAfRdyUwzt78118191;     cAfRdyUwzt78118191 = cAfRdyUwzt80745249;     cAfRdyUwzt80745249 = cAfRdyUwzt43104417;     cAfRdyUwzt43104417 = cAfRdyUwzt5286423;     cAfRdyUwzt5286423 = cAfRdyUwzt86401436;     cAfRdyUwzt86401436 = cAfRdyUwzt65625002;     cAfRdyUwzt65625002 = cAfRdyUwzt76052172;     cAfRdyUwzt76052172 = cAfRdyUwzt93840036;     cAfRdyUwzt93840036 = cAfRdyUwzt79764595;     cAfRdyUwzt79764595 = cAfRdyUwzt22202914;     cAfRdyUwzt22202914 = cAfRdyUwzt54246206;     cAfRdyUwzt54246206 = cAfRdyUwzt36830690;     cAfRdyUwzt36830690 = cAfRdyUwzt32359135;     cAfRdyUwzt32359135 = cAfRdyUwzt7762827;     cAfRdyUwzt7762827 = cAfRdyUwzt18719701;     cAfRdyUwzt18719701 = cAfRdyUwzt58757403;     cAfRdyUwzt58757403 = cAfRdyUwzt17108977;     cAfRdyUwzt17108977 = cAfRdyUwzt86924652;     cAfRdyUwzt86924652 = cAfRdyUwzt35986473;     cAfRdyUwzt35986473 = cAfRdyUwzt7619641;     cAfRdyUwzt7619641 = cAfRdyUwzt69493104;     cAfRdyUwzt69493104 = cAfRdyUwzt53873841;     cAfRdyUwzt53873841 = cAfRdyUwzt58589334;     cAfRdyUwzt58589334 = cAfRdyUwzt4221458;     cAfRdyUwzt4221458 = cAfRdyUwzt69539223;     cAfRdyUwzt69539223 = cAfRdyUwzt20111622;     cAfRdyUwzt20111622 = cAfRdyUwzt97089455;     cAfRdyUwzt97089455 = cAfRdyUwzt90317342;     cAfRdyUwzt90317342 = cAfRdyUwzt38917492;     cAfRdyUwzt38917492 = cAfRdyUwzt27609204;     cAfRdyUwzt27609204 = cAfRdyUwzt26886839;     cAfRdyUwzt26886839 = cAfRdyUwzt32796935;     cAfRdyUwzt32796935 = cAfRdyUwzt74894926;     cAfRdyUwzt74894926 = cAfRdyUwzt39946330;     cAfRdyUwzt39946330 = cAfRdyUwzt86935023;     cAfRdyUwzt86935023 = cAfRdyUwzt71716983;     cAfRdyUwzt71716983 = cAfRdyUwzt68479523;     cAfRdyUwzt68479523 = cAfRdyUwzt22899716;     cAfRdyUwzt22899716 = cAfRdyUwzt59954148;     cAfRdyUwzt59954148 = cAfRdyUwzt75354521;     cAfRdyUwzt75354521 = cAfRdyUwzt78439394;     cAfRdyUwzt78439394 = cAfRdyUwzt17320228;     cAfRdyUwzt17320228 = cAfRdyUwzt69194791;     cAfRdyUwzt69194791 = cAfRdyUwzt54798549;     cAfRdyUwzt54798549 = cAfRdyUwzt14461935;     cAfRdyUwzt14461935 = cAfRdyUwzt170889;     cAfRdyUwzt170889 = cAfRdyUwzt61801455;     cAfRdyUwzt61801455 = cAfRdyUwzt56064515;     cAfRdyUwzt56064515 = cAfRdyUwzt75013996;     cAfRdyUwzt75013996 = cAfRdyUwzt60928205;     cAfRdyUwzt60928205 = cAfRdyUwzt30970873;     cAfRdyUwzt30970873 = cAfRdyUwzt94807375;     cAfRdyUwzt94807375 = cAfRdyUwzt87160136;     cAfRdyUwzt87160136 = cAfRdyUwzt36318679;     cAfRdyUwzt36318679 = cAfRdyUwzt80082844;     cAfRdyUwzt80082844 = cAfRdyUwzt24244350;     cAfRdyUwzt24244350 = cAfRdyUwzt22155915;     cAfRdyUwzt22155915 = cAfRdyUwzt38882959;     cAfRdyUwzt38882959 = cAfRdyUwzt35747200;     cAfRdyUwzt35747200 = cAfRdyUwzt66289815;     cAfRdyUwzt66289815 = cAfRdyUwzt68535546;     cAfRdyUwzt68535546 = cAfRdyUwzt85734829;     cAfRdyUwzt85734829 = cAfRdyUwzt54922544;     cAfRdyUwzt54922544 = cAfRdyUwzt52155392;     cAfRdyUwzt52155392 = cAfRdyUwzt95316075;     cAfRdyUwzt95316075 = cAfRdyUwzt21449271;     cAfRdyUwzt21449271 = cAfRdyUwzt61935763;     cAfRdyUwzt61935763 = cAfRdyUwzt92412805;     cAfRdyUwzt92412805 = cAfRdyUwzt20827804;     cAfRdyUwzt20827804 = cAfRdyUwzt47002718;     cAfRdyUwzt47002718 = cAfRdyUwzt90277879;     cAfRdyUwzt90277879 = cAfRdyUwzt94209261;     cAfRdyUwzt94209261 = cAfRdyUwzt26970504;     cAfRdyUwzt26970504 = cAfRdyUwzt60631952;     cAfRdyUwzt60631952 = cAfRdyUwzt29180247;     cAfRdyUwzt29180247 = cAfRdyUwzt52172876;     cAfRdyUwzt52172876 = cAfRdyUwzt84679050;     cAfRdyUwzt84679050 = cAfRdyUwzt3790786;     cAfRdyUwzt3790786 = cAfRdyUwzt89759523;     cAfRdyUwzt89759523 = cAfRdyUwzt69368334;     cAfRdyUwzt69368334 = cAfRdyUwzt58310167;     cAfRdyUwzt58310167 = cAfRdyUwzt41024941;     cAfRdyUwzt41024941 = cAfRdyUwzt15303346;     cAfRdyUwzt15303346 = cAfRdyUwzt77989286;     cAfRdyUwzt77989286 = cAfRdyUwzt96638330;     cAfRdyUwzt96638330 = cAfRdyUwzt32079464;     cAfRdyUwzt32079464 = cAfRdyUwzt45636799;     cAfRdyUwzt45636799 = cAfRdyUwzt38576248;     cAfRdyUwzt38576248 = cAfRdyUwzt59863485;     cAfRdyUwzt59863485 = cAfRdyUwzt62690673;     cAfRdyUwzt62690673 = cAfRdyUwzt49561068;     cAfRdyUwzt49561068 = cAfRdyUwzt29596565;     cAfRdyUwzt29596565 = cAfRdyUwzt87152516;     cAfRdyUwzt87152516 = cAfRdyUwzt93664333;     cAfRdyUwzt93664333 = cAfRdyUwzt6818976;     cAfRdyUwzt6818976 = cAfRdyUwzt92704564;     cAfRdyUwzt92704564 = cAfRdyUwzt62397683;     cAfRdyUwzt62397683 = cAfRdyUwzt17039399;     cAfRdyUwzt17039399 = cAfRdyUwzt59482474;     cAfRdyUwzt59482474 = cAfRdyUwzt49575949;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZgeJBYwJAG89634012() {     double aeoJZaOaxV68764906 = -920635527;    double aeoJZaOaxV50416048 = -221021948;    double aeoJZaOaxV5404631 = -913571011;    double aeoJZaOaxV27395864 = -617888342;    double aeoJZaOaxV32292927 = -727067649;    double aeoJZaOaxV99158393 = -889082170;    double aeoJZaOaxV44615635 = -757325671;    double aeoJZaOaxV12309765 = -814305825;    double aeoJZaOaxV62837005 = 19641995;    double aeoJZaOaxV44133382 = -72398383;    double aeoJZaOaxV82953079 = -375811266;    double aeoJZaOaxV30734448 = -622563381;    double aeoJZaOaxV94266272 = -71169848;    double aeoJZaOaxV26028923 = -440535430;    double aeoJZaOaxV24964493 = -736241953;    double aeoJZaOaxV99510482 = -139367478;    double aeoJZaOaxV58605990 = -463623188;    double aeoJZaOaxV7945906 = -612461702;    double aeoJZaOaxV8023193 = -740964553;    double aeoJZaOaxV31857573 = -920141852;    double aeoJZaOaxV8376590 = -925672108;    double aeoJZaOaxV95620434 = -755862505;    double aeoJZaOaxV56696689 = 48885590;    double aeoJZaOaxV9874867 = -811094208;    double aeoJZaOaxV20398813 = -36113678;    double aeoJZaOaxV16656076 = -151479766;    double aeoJZaOaxV31902554 = -281865388;    double aeoJZaOaxV45013556 = -269622483;    double aeoJZaOaxV96225251 = 14147486;    double aeoJZaOaxV3909360 = -339739434;    double aeoJZaOaxV94144179 = -982752771;    double aeoJZaOaxV4779288 = 57180252;    double aeoJZaOaxV73687224 = -866829693;    double aeoJZaOaxV40085230 = -4717309;    double aeoJZaOaxV91000070 = -461861476;    double aeoJZaOaxV65528911 = -770730743;    double aeoJZaOaxV8951023 = -107247266;    double aeoJZaOaxV159039 = -822865600;    double aeoJZaOaxV78592400 = -383541003;    double aeoJZaOaxV91541903 = -721749174;    double aeoJZaOaxV85717982 = -772651246;    double aeoJZaOaxV11446587 = -400110365;    double aeoJZaOaxV79220392 = -536260027;    double aeoJZaOaxV97289750 = -180495738;    double aeoJZaOaxV98784206 = -327777154;    double aeoJZaOaxV64262176 = -587531547;    double aeoJZaOaxV24997230 = -681196321;    double aeoJZaOaxV42886033 = -534682714;    double aeoJZaOaxV94173004 = -571247007;    double aeoJZaOaxV15371493 = -493485829;    double aeoJZaOaxV68530391 = -589101880;    double aeoJZaOaxV51414808 = -338555495;    double aeoJZaOaxV33362578 = 64184761;    double aeoJZaOaxV38834318 = -559552128;    double aeoJZaOaxV81257423 = -847613326;    double aeoJZaOaxV73144472 = -64773023;    double aeoJZaOaxV93719359 = -169907539;    double aeoJZaOaxV95529764 = -2476803;    double aeoJZaOaxV6997052 = -481774664;    double aeoJZaOaxV15636851 = -475587883;    double aeoJZaOaxV67255840 = -507216783;    double aeoJZaOaxV99602079 = -387703189;    double aeoJZaOaxV16084514 = -728453312;    double aeoJZaOaxV58927646 = -640618571;    double aeoJZaOaxV49989203 = -89645613;    double aeoJZaOaxV78173791 = -332991519;    double aeoJZaOaxV57047224 = -755733688;    double aeoJZaOaxV54181043 = 33547460;    double aeoJZaOaxV35028852 = -978673954;    double aeoJZaOaxV59435581 = -965511211;    double aeoJZaOaxV90559460 = 67879788;    double aeoJZaOaxV58446952 = -640757588;    double aeoJZaOaxV29353506 = -128920699;    double aeoJZaOaxV16481289 = 80784621;    double aeoJZaOaxV46139591 = -47490607;    double aeoJZaOaxV96930003 = -425561743;    double aeoJZaOaxV16400042 = -119602479;    double aeoJZaOaxV59406939 = -770618672;    double aeoJZaOaxV11090660 = -383317055;    double aeoJZaOaxV56136636 = -448582131;    double aeoJZaOaxV91658846 = -470283446;    double aeoJZaOaxV89016520 = -747182674;    double aeoJZaOaxV50840551 = -698375476;    double aeoJZaOaxV80853759 = -492366686;    double aeoJZaOaxV35378969 = -750637554;    double aeoJZaOaxV42729371 = -544197276;    double aeoJZaOaxV71416710 = 92995490;    double aeoJZaOaxV34852906 = -207277566;    double aeoJZaOaxV58827807 = -157103983;    double aeoJZaOaxV17855599 = -297088454;    double aeoJZaOaxV71809552 = -500823204;    double aeoJZaOaxV13421258 = -4770464;    double aeoJZaOaxV93161987 = -241090936;    double aeoJZaOaxV62955549 = -907953121;    double aeoJZaOaxV24286063 = -114532392;    double aeoJZaOaxV86115903 = -284948057;    double aeoJZaOaxV95362073 = -671657054;    double aeoJZaOaxV20292747 = -895641456;    double aeoJZaOaxV47300547 = 9149875;    double aeoJZaOaxV20610415 = -920635527;     aeoJZaOaxV68764906 = aeoJZaOaxV50416048;     aeoJZaOaxV50416048 = aeoJZaOaxV5404631;     aeoJZaOaxV5404631 = aeoJZaOaxV27395864;     aeoJZaOaxV27395864 = aeoJZaOaxV32292927;     aeoJZaOaxV32292927 = aeoJZaOaxV99158393;     aeoJZaOaxV99158393 = aeoJZaOaxV44615635;     aeoJZaOaxV44615635 = aeoJZaOaxV12309765;     aeoJZaOaxV12309765 = aeoJZaOaxV62837005;     aeoJZaOaxV62837005 = aeoJZaOaxV44133382;     aeoJZaOaxV44133382 = aeoJZaOaxV82953079;     aeoJZaOaxV82953079 = aeoJZaOaxV30734448;     aeoJZaOaxV30734448 = aeoJZaOaxV94266272;     aeoJZaOaxV94266272 = aeoJZaOaxV26028923;     aeoJZaOaxV26028923 = aeoJZaOaxV24964493;     aeoJZaOaxV24964493 = aeoJZaOaxV99510482;     aeoJZaOaxV99510482 = aeoJZaOaxV58605990;     aeoJZaOaxV58605990 = aeoJZaOaxV7945906;     aeoJZaOaxV7945906 = aeoJZaOaxV8023193;     aeoJZaOaxV8023193 = aeoJZaOaxV31857573;     aeoJZaOaxV31857573 = aeoJZaOaxV8376590;     aeoJZaOaxV8376590 = aeoJZaOaxV95620434;     aeoJZaOaxV95620434 = aeoJZaOaxV56696689;     aeoJZaOaxV56696689 = aeoJZaOaxV9874867;     aeoJZaOaxV9874867 = aeoJZaOaxV20398813;     aeoJZaOaxV20398813 = aeoJZaOaxV16656076;     aeoJZaOaxV16656076 = aeoJZaOaxV31902554;     aeoJZaOaxV31902554 = aeoJZaOaxV45013556;     aeoJZaOaxV45013556 = aeoJZaOaxV96225251;     aeoJZaOaxV96225251 = aeoJZaOaxV3909360;     aeoJZaOaxV3909360 = aeoJZaOaxV94144179;     aeoJZaOaxV94144179 = aeoJZaOaxV4779288;     aeoJZaOaxV4779288 = aeoJZaOaxV73687224;     aeoJZaOaxV73687224 = aeoJZaOaxV40085230;     aeoJZaOaxV40085230 = aeoJZaOaxV91000070;     aeoJZaOaxV91000070 = aeoJZaOaxV65528911;     aeoJZaOaxV65528911 = aeoJZaOaxV8951023;     aeoJZaOaxV8951023 = aeoJZaOaxV159039;     aeoJZaOaxV159039 = aeoJZaOaxV78592400;     aeoJZaOaxV78592400 = aeoJZaOaxV91541903;     aeoJZaOaxV91541903 = aeoJZaOaxV85717982;     aeoJZaOaxV85717982 = aeoJZaOaxV11446587;     aeoJZaOaxV11446587 = aeoJZaOaxV79220392;     aeoJZaOaxV79220392 = aeoJZaOaxV97289750;     aeoJZaOaxV97289750 = aeoJZaOaxV98784206;     aeoJZaOaxV98784206 = aeoJZaOaxV64262176;     aeoJZaOaxV64262176 = aeoJZaOaxV24997230;     aeoJZaOaxV24997230 = aeoJZaOaxV42886033;     aeoJZaOaxV42886033 = aeoJZaOaxV94173004;     aeoJZaOaxV94173004 = aeoJZaOaxV15371493;     aeoJZaOaxV15371493 = aeoJZaOaxV68530391;     aeoJZaOaxV68530391 = aeoJZaOaxV51414808;     aeoJZaOaxV51414808 = aeoJZaOaxV33362578;     aeoJZaOaxV33362578 = aeoJZaOaxV38834318;     aeoJZaOaxV38834318 = aeoJZaOaxV81257423;     aeoJZaOaxV81257423 = aeoJZaOaxV73144472;     aeoJZaOaxV73144472 = aeoJZaOaxV93719359;     aeoJZaOaxV93719359 = aeoJZaOaxV95529764;     aeoJZaOaxV95529764 = aeoJZaOaxV6997052;     aeoJZaOaxV6997052 = aeoJZaOaxV15636851;     aeoJZaOaxV15636851 = aeoJZaOaxV67255840;     aeoJZaOaxV67255840 = aeoJZaOaxV99602079;     aeoJZaOaxV99602079 = aeoJZaOaxV16084514;     aeoJZaOaxV16084514 = aeoJZaOaxV58927646;     aeoJZaOaxV58927646 = aeoJZaOaxV49989203;     aeoJZaOaxV49989203 = aeoJZaOaxV78173791;     aeoJZaOaxV78173791 = aeoJZaOaxV57047224;     aeoJZaOaxV57047224 = aeoJZaOaxV54181043;     aeoJZaOaxV54181043 = aeoJZaOaxV35028852;     aeoJZaOaxV35028852 = aeoJZaOaxV59435581;     aeoJZaOaxV59435581 = aeoJZaOaxV90559460;     aeoJZaOaxV90559460 = aeoJZaOaxV58446952;     aeoJZaOaxV58446952 = aeoJZaOaxV29353506;     aeoJZaOaxV29353506 = aeoJZaOaxV16481289;     aeoJZaOaxV16481289 = aeoJZaOaxV46139591;     aeoJZaOaxV46139591 = aeoJZaOaxV96930003;     aeoJZaOaxV96930003 = aeoJZaOaxV16400042;     aeoJZaOaxV16400042 = aeoJZaOaxV59406939;     aeoJZaOaxV59406939 = aeoJZaOaxV11090660;     aeoJZaOaxV11090660 = aeoJZaOaxV56136636;     aeoJZaOaxV56136636 = aeoJZaOaxV91658846;     aeoJZaOaxV91658846 = aeoJZaOaxV89016520;     aeoJZaOaxV89016520 = aeoJZaOaxV50840551;     aeoJZaOaxV50840551 = aeoJZaOaxV80853759;     aeoJZaOaxV80853759 = aeoJZaOaxV35378969;     aeoJZaOaxV35378969 = aeoJZaOaxV42729371;     aeoJZaOaxV42729371 = aeoJZaOaxV71416710;     aeoJZaOaxV71416710 = aeoJZaOaxV34852906;     aeoJZaOaxV34852906 = aeoJZaOaxV58827807;     aeoJZaOaxV58827807 = aeoJZaOaxV17855599;     aeoJZaOaxV17855599 = aeoJZaOaxV71809552;     aeoJZaOaxV71809552 = aeoJZaOaxV13421258;     aeoJZaOaxV13421258 = aeoJZaOaxV93161987;     aeoJZaOaxV93161987 = aeoJZaOaxV62955549;     aeoJZaOaxV62955549 = aeoJZaOaxV24286063;     aeoJZaOaxV24286063 = aeoJZaOaxV86115903;     aeoJZaOaxV86115903 = aeoJZaOaxV95362073;     aeoJZaOaxV95362073 = aeoJZaOaxV20292747;     aeoJZaOaxV20292747 = aeoJZaOaxV47300547;     aeoJZaOaxV47300547 = aeoJZaOaxV20610415;     aeoJZaOaxV20610415 = aeoJZaOaxV68764906;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nvlxvybylJ44050226() {     double XwqlPaOnQc39416573 = -438784658;    double XwqlPaOnQc19089566 = -92665247;    double XwqlPaOnQc85373550 = -255865470;    double XwqlPaOnQc67327151 = -808026731;    double XwqlPaOnQc53743248 = -510473996;    double XwqlPaOnQc89158156 = -590663322;    double XwqlPaOnQc62994531 = -756101381;    double XwqlPaOnQc69152145 = -61122915;    double XwqlPaOnQc8702060 = -73705334;    double XwqlPaOnQc67099843 = -373493734;    double XwqlPaOnQc25202240 = -410713196;    double XwqlPaOnQc76281019 = -938327174;    double XwqlPaOnQc74093660 = 19293094;    double XwqlPaOnQc9875957 = -241563640;    double XwqlPaOnQc7684512 = -885991956;    double XwqlPaOnQc32039123 = -809351065;    double XwqlPaOnQc52458817 = -372808318;    double XwqlPaOnQc28239400 = -601627898;    double XwqlPaOnQc89730974 = -348152495;    double XwqlPaOnQc90644282 = -362508156;    double XwqlPaOnQc63512344 = -834351249;    double XwqlPaOnQc57016199 = 12852190;    double XwqlPaOnQc58640498 = -518957554;    double XwqlPaOnQc43724403 = -403495114;    double XwqlPaOnQc84523201 = -812018366;    double XwqlPaOnQc47253656 = -249764211;    double XwqlPaOnQc40163339 = -677043163;    double XwqlPaOnQc52795781 = -774746251;    double XwqlPaOnQc28367740 = -280210731;    double XwqlPaOnQc43081456 = -898134505;    double XwqlPaOnQc1216176 = -731186456;    double XwqlPaOnQc49290749 = -271116911;    double XwqlPaOnQc56632585 = -813791302;    double XwqlPaOnQc5450205 = -668246993;    double XwqlPaOnQc69024307 = -308808255;    double XwqlPaOnQc25522170 = -849957291;    double XwqlPaOnQc52742508 = -295840474;    double XwqlPaOnQc52951527 = -606978707;    double XwqlPaOnQc57125689 = -339269640;    double XwqlPaOnQc86645202 = -239357537;    double XwqlPaOnQc45791266 = -56670124;    double XwqlPaOnQc39929578 = -169499435;    double XwqlPaOnQc28187352 = -523014079;    double XwqlPaOnQc3798051 = -252470709;    double XwqlPaOnQc94738265 = -636423548;    double XwqlPaOnQc70668011 = -345868702;    double XwqlPaOnQc51128432 = -797266120;    double XwqlPaOnQc8500243 = -986813009;    double XwqlPaOnQc99614457 = -557010977;    double XwqlPaOnQc23798979 = -776494708;    double XwqlPaOnQc36719715 = -135674001;    double XwqlPaOnQc93286312 = -307775217;    double XwqlPaOnQc88438234 = -963613281;    double XwqlPaOnQc29061622 = -120433624;    double XwqlPaOnQc80537881 = -958554240;    double XwqlPaOnQc82400374 = -351636848;    double XwqlPaOnQc60449068 = -573707693;    double XwqlPaOnQc41649147 = -852370357;    double XwqlPaOnQc82803950 = -996008365;    double XwqlPaOnQc6489592 = -160709786;    double XwqlPaOnQc48994817 = -913620159;    double XwqlPaOnQc10198750 = -981355131;    double XwqlPaOnQc40784406 = -780912185;    double XwqlPaOnQc65620604 = -175570829;    double XwqlPaOnQc65883667 = -642307278;    double XwqlPaOnQc75911491 = -39596285;    double XwqlPaOnQc19648435 = -24535872;    double XwqlPaOnQc68643455 = -312459914;    double XwqlPaOnQc40851649 = -932755385;    double XwqlPaOnQc82162341 = 63965334;    double XwqlPaOnQc79296615 = -413510592;    double XwqlPaOnQc99507289 = -765829611;    double XwqlPaOnQc71113711 = -162358259;    double XwqlPaOnQc3085773 = -8794959;    double XwqlPaOnQc44853016 = -205838033;    double XwqlPaOnQc23582767 = -564851814;    double XwqlPaOnQc28828847 = -464133732;    double XwqlPaOnQc54842447 = -166486846;    double XwqlPaOnQc48986138 = -767071567;    double XwqlPaOnQc13855190 = -366149664;    double XwqlPaOnQc96125223 = -452498091;    double XwqlPaOnQc31663096 = -690230154;    double XwqlPaOnQc53181324 = -117735274;    double XwqlPaOnQc4568761 = -503716024;    double XwqlPaOnQc6361741 = -662460504;    double XwqlPaOnQc7929864 = -323411240;    double XwqlPaOnQc60852514 = -307503630;    double XwqlPaOnQc27570963 = -593357678;    double XwqlPaOnQc24912324 = -709692753;    double XwqlPaOnQc86623932 = -957171408;    double XwqlPaOnQc65073102 = -176249598;    double XwqlPaOnQc11093362 = -443470118;    double XwqlPaOnQc70147577 = -610970342;    double XwqlPaOnQc50636097 = -78559855;    double XwqlPaOnQc37650385 = -325737378;    double XwqlPaOnQc35592517 = -75314993;    double XwqlPaOnQc99145171 = -388587251;    double XwqlPaOnQc62566748 = -247443250;    double XwqlPaOnQc37914384 = -610163432;    double XwqlPaOnQc18826774 = -438784658;     XwqlPaOnQc39416573 = XwqlPaOnQc19089566;     XwqlPaOnQc19089566 = XwqlPaOnQc85373550;     XwqlPaOnQc85373550 = XwqlPaOnQc67327151;     XwqlPaOnQc67327151 = XwqlPaOnQc53743248;     XwqlPaOnQc53743248 = XwqlPaOnQc89158156;     XwqlPaOnQc89158156 = XwqlPaOnQc62994531;     XwqlPaOnQc62994531 = XwqlPaOnQc69152145;     XwqlPaOnQc69152145 = XwqlPaOnQc8702060;     XwqlPaOnQc8702060 = XwqlPaOnQc67099843;     XwqlPaOnQc67099843 = XwqlPaOnQc25202240;     XwqlPaOnQc25202240 = XwqlPaOnQc76281019;     XwqlPaOnQc76281019 = XwqlPaOnQc74093660;     XwqlPaOnQc74093660 = XwqlPaOnQc9875957;     XwqlPaOnQc9875957 = XwqlPaOnQc7684512;     XwqlPaOnQc7684512 = XwqlPaOnQc32039123;     XwqlPaOnQc32039123 = XwqlPaOnQc52458817;     XwqlPaOnQc52458817 = XwqlPaOnQc28239400;     XwqlPaOnQc28239400 = XwqlPaOnQc89730974;     XwqlPaOnQc89730974 = XwqlPaOnQc90644282;     XwqlPaOnQc90644282 = XwqlPaOnQc63512344;     XwqlPaOnQc63512344 = XwqlPaOnQc57016199;     XwqlPaOnQc57016199 = XwqlPaOnQc58640498;     XwqlPaOnQc58640498 = XwqlPaOnQc43724403;     XwqlPaOnQc43724403 = XwqlPaOnQc84523201;     XwqlPaOnQc84523201 = XwqlPaOnQc47253656;     XwqlPaOnQc47253656 = XwqlPaOnQc40163339;     XwqlPaOnQc40163339 = XwqlPaOnQc52795781;     XwqlPaOnQc52795781 = XwqlPaOnQc28367740;     XwqlPaOnQc28367740 = XwqlPaOnQc43081456;     XwqlPaOnQc43081456 = XwqlPaOnQc1216176;     XwqlPaOnQc1216176 = XwqlPaOnQc49290749;     XwqlPaOnQc49290749 = XwqlPaOnQc56632585;     XwqlPaOnQc56632585 = XwqlPaOnQc5450205;     XwqlPaOnQc5450205 = XwqlPaOnQc69024307;     XwqlPaOnQc69024307 = XwqlPaOnQc25522170;     XwqlPaOnQc25522170 = XwqlPaOnQc52742508;     XwqlPaOnQc52742508 = XwqlPaOnQc52951527;     XwqlPaOnQc52951527 = XwqlPaOnQc57125689;     XwqlPaOnQc57125689 = XwqlPaOnQc86645202;     XwqlPaOnQc86645202 = XwqlPaOnQc45791266;     XwqlPaOnQc45791266 = XwqlPaOnQc39929578;     XwqlPaOnQc39929578 = XwqlPaOnQc28187352;     XwqlPaOnQc28187352 = XwqlPaOnQc3798051;     XwqlPaOnQc3798051 = XwqlPaOnQc94738265;     XwqlPaOnQc94738265 = XwqlPaOnQc70668011;     XwqlPaOnQc70668011 = XwqlPaOnQc51128432;     XwqlPaOnQc51128432 = XwqlPaOnQc8500243;     XwqlPaOnQc8500243 = XwqlPaOnQc99614457;     XwqlPaOnQc99614457 = XwqlPaOnQc23798979;     XwqlPaOnQc23798979 = XwqlPaOnQc36719715;     XwqlPaOnQc36719715 = XwqlPaOnQc93286312;     XwqlPaOnQc93286312 = XwqlPaOnQc88438234;     XwqlPaOnQc88438234 = XwqlPaOnQc29061622;     XwqlPaOnQc29061622 = XwqlPaOnQc80537881;     XwqlPaOnQc80537881 = XwqlPaOnQc82400374;     XwqlPaOnQc82400374 = XwqlPaOnQc60449068;     XwqlPaOnQc60449068 = XwqlPaOnQc41649147;     XwqlPaOnQc41649147 = XwqlPaOnQc82803950;     XwqlPaOnQc82803950 = XwqlPaOnQc6489592;     XwqlPaOnQc6489592 = XwqlPaOnQc48994817;     XwqlPaOnQc48994817 = XwqlPaOnQc10198750;     XwqlPaOnQc10198750 = XwqlPaOnQc40784406;     XwqlPaOnQc40784406 = XwqlPaOnQc65620604;     XwqlPaOnQc65620604 = XwqlPaOnQc65883667;     XwqlPaOnQc65883667 = XwqlPaOnQc75911491;     XwqlPaOnQc75911491 = XwqlPaOnQc19648435;     XwqlPaOnQc19648435 = XwqlPaOnQc68643455;     XwqlPaOnQc68643455 = XwqlPaOnQc40851649;     XwqlPaOnQc40851649 = XwqlPaOnQc82162341;     XwqlPaOnQc82162341 = XwqlPaOnQc79296615;     XwqlPaOnQc79296615 = XwqlPaOnQc99507289;     XwqlPaOnQc99507289 = XwqlPaOnQc71113711;     XwqlPaOnQc71113711 = XwqlPaOnQc3085773;     XwqlPaOnQc3085773 = XwqlPaOnQc44853016;     XwqlPaOnQc44853016 = XwqlPaOnQc23582767;     XwqlPaOnQc23582767 = XwqlPaOnQc28828847;     XwqlPaOnQc28828847 = XwqlPaOnQc54842447;     XwqlPaOnQc54842447 = XwqlPaOnQc48986138;     XwqlPaOnQc48986138 = XwqlPaOnQc13855190;     XwqlPaOnQc13855190 = XwqlPaOnQc96125223;     XwqlPaOnQc96125223 = XwqlPaOnQc31663096;     XwqlPaOnQc31663096 = XwqlPaOnQc53181324;     XwqlPaOnQc53181324 = XwqlPaOnQc4568761;     XwqlPaOnQc4568761 = XwqlPaOnQc6361741;     XwqlPaOnQc6361741 = XwqlPaOnQc7929864;     XwqlPaOnQc7929864 = XwqlPaOnQc60852514;     XwqlPaOnQc60852514 = XwqlPaOnQc27570963;     XwqlPaOnQc27570963 = XwqlPaOnQc24912324;     XwqlPaOnQc24912324 = XwqlPaOnQc86623932;     XwqlPaOnQc86623932 = XwqlPaOnQc65073102;     XwqlPaOnQc65073102 = XwqlPaOnQc11093362;     XwqlPaOnQc11093362 = XwqlPaOnQc70147577;     XwqlPaOnQc70147577 = XwqlPaOnQc50636097;     XwqlPaOnQc50636097 = XwqlPaOnQc37650385;     XwqlPaOnQc37650385 = XwqlPaOnQc35592517;     XwqlPaOnQc35592517 = XwqlPaOnQc99145171;     XwqlPaOnQc99145171 = XwqlPaOnQc62566748;     XwqlPaOnQc62566748 = XwqlPaOnQc37914384;     XwqlPaOnQc37914384 = XwqlPaOnQc18826774;     XwqlPaOnQc18826774 = XwqlPaOnQc39416573;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bKfMhUiZuf89745918() {     double IvPOGAiVnD58605531 = -282559483;    double IvPOGAiVnD91387423 = -834273580;    double IvPOGAiVnD10032932 = -932711622;    double IvPOGAiVnD51618598 = -813518901;    double IvPOGAiVnD80749751 = -545665712;    double IvPOGAiVnD1915114 = -156899074;    double IvPOGAiVnD41985164 = -917804786;    double IvPOGAiVnD5409739 = -621634325;    double IvPOGAiVnD77699029 = -445873367;    double IvPOGAiVnD31468630 = -624414876;    double IvPOGAiVnD85952405 = -93534111;    double IvPOGAiVnD52769261 = -996108854;    double IvPOGAiVnD31529243 = -353893180;    double IvPOGAiVnD3545744 = -222456860;    double IvPOGAiVnD24886178 = -252401424;    double IvPOGAiVnD12829906 = -36494673;    double IvPOGAiVnD52307404 = -139048163;    double IvPOGAiVnD19076328 = -988835919;    double IvPOGAiVnD10829516 = -935382888;    double IvPOGAiVnD86515382 = -893337688;    double IvPOGAiVnD64269293 = -355626084;    double IvPOGAiVnD83143529 = -576947508;    double IvPOGAiVnD61463346 = -827026633;    double IvPOGAiVnD95009935 = -572631849;    double IvPOGAiVnD700556 = -487640473;    double IvPOGAiVnD94370508 = -473102117;    double IvPOGAiVnD51954271 = -626403014;    double IvPOGAiVnD719882 = -121992925;    double IvPOGAiVnD34275649 = -732431119;    double IvPOGAiVnD8073324 = -829816676;    double IvPOGAiVnD67751151 = -15171729;    double IvPOGAiVnD27183198 = -201463787;    double IvPOGAiVnD97522873 = -324657154;    double IvPOGAiVnD70640508 = -160355722;    double IvPOGAiVnD20078048 = -834885189;    double IvPOGAiVnD4116058 = -963153328;    double IvPOGAiVnD89976547 = -381224432;    double IvPOGAiVnD84631042 = -493514826;    double IvPOGAiVnD12818373 = -683023382;    double IvPOGAiVnD18232958 = -41835276;    double IvPOGAiVnD56154727 = -487965266;    double IvPOGAiVnD72936770 = -276278313;    double IvPOGAiVnD90087516 = -269468865;    double IvPOGAiVnD31893010 = -878782408;    double IvPOGAiVnD38723922 = -315689386;    double IvPOGAiVnD20468253 = -187779355;    double IvPOGAiVnD75954773 = -821617765;    double IvPOGAiVnD89584821 = -306700144;    double IvPOGAiVnD37722947 = -356768147;    double IvPOGAiVnD64156475 = -112512072;    double IvPOGAiVnD44321901 = -597167953;    double IvPOGAiVnD13730248 = -991334601;    double IvPOGAiVnD26993438 = -839537193;    double IvPOGAiVnD80735804 = -356731460;    double IvPOGAiVnD25476625 = -764872053;    double IvPOGAiVnD75462002 = -705611976;    double IvPOGAiVnD29924077 = 92753053;    double IvPOGAiVnD15022997 = -260079774;    double IvPOGAiVnD50918043 = -225878429;    double IvPOGAiVnD86379242 = 27436405;    double IvPOGAiVnD49960843 = -530496061;    double IvPOGAiVnD41265283 = -695811862;    double IvPOGAiVnD71134090 = -889203206;    double IvPOGAiVnD69625706 = -616056692;    double IvPOGAiVnD63717478 = -509243148;    double IvPOGAiVnD58769207 = -892070325;    double IvPOGAiVnD55246387 = -571451700;    double IvPOGAiVnD60888735 = -93537459;    double IvPOGAiVnD83467696 = -387571672;    double IvPOGAiVnD20770120 = -289248097;    double IvPOGAiVnD22853358 = -655270241;    double IvPOGAiVnD67676362 = -645533338;    double IvPOGAiVnD6257955 = -205812538;    double IvPOGAiVnD92596557 = -793547612;    double IvPOGAiVnD30360656 = -305372422;    double IvPOGAiVnD91332523 = 20652228;    double IvPOGAiVnD93056012 = -207478643;    double IvPOGAiVnD29570336 = -948244225;    double IvPOGAiVnD56286013 = -156942463;    double IvPOGAiVnD80232302 = -199861118;    double IvPOGAiVnD18415736 = -651484352;    double IvPOGAiVnD62369450 = -219702870;    double IvPOGAiVnD62996934 = -765224779;    double IvPOGAiVnD70119173 = -519919047;    double IvPOGAiVnD63751423 = -132648723;    double IvPOGAiVnD54020904 = -23837129;    double IvPOGAiVnD189761 = -361926594;    double IvPOGAiVnD16787070 = -967925695;    double IvPOGAiVnD45163883 = -395483669;    double IvPOGAiVnD44616046 = -29273213;    double IvPOGAiVnD74191981 = -955906381;    double IvPOGAiVnD74953550 = -21144659;    double IvPOGAiVnD33713000 = -167636398;    double IvPOGAiVnD26439130 = -610459787;    double IvPOGAiVnD68272114 = -511339216;    double IvPOGAiVnD14889444 = -792153405;    double IvPOGAiVnD1802681 = -387075107;    double IvPOGAiVnD20461811 = -653412174;    double IvPOGAiVnD68175532 = -269539261;    double IvPOGAiVnD79954715 = -282559483;     IvPOGAiVnD58605531 = IvPOGAiVnD91387423;     IvPOGAiVnD91387423 = IvPOGAiVnD10032932;     IvPOGAiVnD10032932 = IvPOGAiVnD51618598;     IvPOGAiVnD51618598 = IvPOGAiVnD80749751;     IvPOGAiVnD80749751 = IvPOGAiVnD1915114;     IvPOGAiVnD1915114 = IvPOGAiVnD41985164;     IvPOGAiVnD41985164 = IvPOGAiVnD5409739;     IvPOGAiVnD5409739 = IvPOGAiVnD77699029;     IvPOGAiVnD77699029 = IvPOGAiVnD31468630;     IvPOGAiVnD31468630 = IvPOGAiVnD85952405;     IvPOGAiVnD85952405 = IvPOGAiVnD52769261;     IvPOGAiVnD52769261 = IvPOGAiVnD31529243;     IvPOGAiVnD31529243 = IvPOGAiVnD3545744;     IvPOGAiVnD3545744 = IvPOGAiVnD24886178;     IvPOGAiVnD24886178 = IvPOGAiVnD12829906;     IvPOGAiVnD12829906 = IvPOGAiVnD52307404;     IvPOGAiVnD52307404 = IvPOGAiVnD19076328;     IvPOGAiVnD19076328 = IvPOGAiVnD10829516;     IvPOGAiVnD10829516 = IvPOGAiVnD86515382;     IvPOGAiVnD86515382 = IvPOGAiVnD64269293;     IvPOGAiVnD64269293 = IvPOGAiVnD83143529;     IvPOGAiVnD83143529 = IvPOGAiVnD61463346;     IvPOGAiVnD61463346 = IvPOGAiVnD95009935;     IvPOGAiVnD95009935 = IvPOGAiVnD700556;     IvPOGAiVnD700556 = IvPOGAiVnD94370508;     IvPOGAiVnD94370508 = IvPOGAiVnD51954271;     IvPOGAiVnD51954271 = IvPOGAiVnD719882;     IvPOGAiVnD719882 = IvPOGAiVnD34275649;     IvPOGAiVnD34275649 = IvPOGAiVnD8073324;     IvPOGAiVnD8073324 = IvPOGAiVnD67751151;     IvPOGAiVnD67751151 = IvPOGAiVnD27183198;     IvPOGAiVnD27183198 = IvPOGAiVnD97522873;     IvPOGAiVnD97522873 = IvPOGAiVnD70640508;     IvPOGAiVnD70640508 = IvPOGAiVnD20078048;     IvPOGAiVnD20078048 = IvPOGAiVnD4116058;     IvPOGAiVnD4116058 = IvPOGAiVnD89976547;     IvPOGAiVnD89976547 = IvPOGAiVnD84631042;     IvPOGAiVnD84631042 = IvPOGAiVnD12818373;     IvPOGAiVnD12818373 = IvPOGAiVnD18232958;     IvPOGAiVnD18232958 = IvPOGAiVnD56154727;     IvPOGAiVnD56154727 = IvPOGAiVnD72936770;     IvPOGAiVnD72936770 = IvPOGAiVnD90087516;     IvPOGAiVnD90087516 = IvPOGAiVnD31893010;     IvPOGAiVnD31893010 = IvPOGAiVnD38723922;     IvPOGAiVnD38723922 = IvPOGAiVnD20468253;     IvPOGAiVnD20468253 = IvPOGAiVnD75954773;     IvPOGAiVnD75954773 = IvPOGAiVnD89584821;     IvPOGAiVnD89584821 = IvPOGAiVnD37722947;     IvPOGAiVnD37722947 = IvPOGAiVnD64156475;     IvPOGAiVnD64156475 = IvPOGAiVnD44321901;     IvPOGAiVnD44321901 = IvPOGAiVnD13730248;     IvPOGAiVnD13730248 = IvPOGAiVnD26993438;     IvPOGAiVnD26993438 = IvPOGAiVnD80735804;     IvPOGAiVnD80735804 = IvPOGAiVnD25476625;     IvPOGAiVnD25476625 = IvPOGAiVnD75462002;     IvPOGAiVnD75462002 = IvPOGAiVnD29924077;     IvPOGAiVnD29924077 = IvPOGAiVnD15022997;     IvPOGAiVnD15022997 = IvPOGAiVnD50918043;     IvPOGAiVnD50918043 = IvPOGAiVnD86379242;     IvPOGAiVnD86379242 = IvPOGAiVnD49960843;     IvPOGAiVnD49960843 = IvPOGAiVnD41265283;     IvPOGAiVnD41265283 = IvPOGAiVnD71134090;     IvPOGAiVnD71134090 = IvPOGAiVnD69625706;     IvPOGAiVnD69625706 = IvPOGAiVnD63717478;     IvPOGAiVnD63717478 = IvPOGAiVnD58769207;     IvPOGAiVnD58769207 = IvPOGAiVnD55246387;     IvPOGAiVnD55246387 = IvPOGAiVnD60888735;     IvPOGAiVnD60888735 = IvPOGAiVnD83467696;     IvPOGAiVnD83467696 = IvPOGAiVnD20770120;     IvPOGAiVnD20770120 = IvPOGAiVnD22853358;     IvPOGAiVnD22853358 = IvPOGAiVnD67676362;     IvPOGAiVnD67676362 = IvPOGAiVnD6257955;     IvPOGAiVnD6257955 = IvPOGAiVnD92596557;     IvPOGAiVnD92596557 = IvPOGAiVnD30360656;     IvPOGAiVnD30360656 = IvPOGAiVnD91332523;     IvPOGAiVnD91332523 = IvPOGAiVnD93056012;     IvPOGAiVnD93056012 = IvPOGAiVnD29570336;     IvPOGAiVnD29570336 = IvPOGAiVnD56286013;     IvPOGAiVnD56286013 = IvPOGAiVnD80232302;     IvPOGAiVnD80232302 = IvPOGAiVnD18415736;     IvPOGAiVnD18415736 = IvPOGAiVnD62369450;     IvPOGAiVnD62369450 = IvPOGAiVnD62996934;     IvPOGAiVnD62996934 = IvPOGAiVnD70119173;     IvPOGAiVnD70119173 = IvPOGAiVnD63751423;     IvPOGAiVnD63751423 = IvPOGAiVnD54020904;     IvPOGAiVnD54020904 = IvPOGAiVnD189761;     IvPOGAiVnD189761 = IvPOGAiVnD16787070;     IvPOGAiVnD16787070 = IvPOGAiVnD45163883;     IvPOGAiVnD45163883 = IvPOGAiVnD44616046;     IvPOGAiVnD44616046 = IvPOGAiVnD74191981;     IvPOGAiVnD74191981 = IvPOGAiVnD74953550;     IvPOGAiVnD74953550 = IvPOGAiVnD33713000;     IvPOGAiVnD33713000 = IvPOGAiVnD26439130;     IvPOGAiVnD26439130 = IvPOGAiVnD68272114;     IvPOGAiVnD68272114 = IvPOGAiVnD14889444;     IvPOGAiVnD14889444 = IvPOGAiVnD1802681;     IvPOGAiVnD1802681 = IvPOGAiVnD20461811;     IvPOGAiVnD20461811 = IvPOGAiVnD68175532;     IvPOGAiVnD68175532 = IvPOGAiVnD79954715;     IvPOGAiVnD79954715 = IvPOGAiVnD58605531;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GOvKVpHxjN35441611() {     double wFcPczuRYg77794489 = -126334309;    double wFcPczuRYg63685281 = -475881912;    double wFcPczuRYg34692313 = -509557775;    double wFcPczuRYg35910045 = -819011071;    double wFcPczuRYg7756256 = -580857427;    double wFcPczuRYg14672072 = -823134827;    double wFcPczuRYg20975798 = 20491808;    double wFcPczuRYg41667332 = -82145734;    double wFcPczuRYg46695999 = -818041401;    double wFcPczuRYg95837416 = -875336019;    double wFcPczuRYg46702571 = -876355026;    double wFcPczuRYg29257503 = 46109466;    double wFcPczuRYg88964826 = -727079454;    double wFcPczuRYg97215531 = -203350080;    double wFcPczuRYg42087844 = -718810893;    double wFcPczuRYg93620687 = -363638281;    double wFcPczuRYg52155992 = 94711991;    double wFcPczuRYg9913256 = -276043940;    double wFcPczuRYg31928056 = -422613281;    double wFcPczuRYg82386482 = -324167220;    double wFcPczuRYg65026242 = -976900920;    double wFcPczuRYg9270859 = -66747205;    double wFcPczuRYg64286195 = -35095712;    double wFcPczuRYg46295467 = -741768584;    double wFcPczuRYg16877910 = -163262579;    double wFcPczuRYg41487362 = -696440023;    double wFcPczuRYg63745203 = -575762865;    double wFcPczuRYg48643981 = -569239600;    double wFcPczuRYg40183558 = -84651507;    double wFcPczuRYg73065192 = -761498847;    double wFcPczuRYg34286128 = -399157003;    double wFcPczuRYg5075648 = -131810662;    double wFcPczuRYg38413163 = -935523007;    double wFcPczuRYg35830812 = -752464450;    double wFcPczuRYg71131789 = -260962122;    double wFcPczuRYg82709945 = 23650636;    double wFcPczuRYg27210586 = -466608391;    double wFcPczuRYg16310558 = -380050945;    double wFcPczuRYg68511057 = 73222877;    double wFcPczuRYg49820713 = -944313016;    double wFcPczuRYg66518187 = -919260409;    double wFcPczuRYg5943963 = -383057191;    double wFcPczuRYg51987681 = -15923651;    double wFcPczuRYg59987970 = -405094108;    double wFcPczuRYg82709579 = 5044776;    double wFcPczuRYg70268494 = -29690009;    double wFcPczuRYg781114 = -845969411;    double wFcPczuRYg70669399 = -726587279;    double wFcPczuRYg75831437 = -156525317;    double wFcPczuRYg4513972 = -548529437;    double wFcPczuRYg51924086 = 41338095;    double wFcPczuRYg34174182 = -574893986;    double wFcPczuRYg65548640 = -715461104;    double wFcPczuRYg32409986 = -593029297;    double wFcPczuRYg70415368 = -571189866;    double wFcPczuRYg68523630 = 40412896;    double wFcPczuRYg99399085 = -340786201;    double wFcPczuRYg88396846 = -767789191;    double wFcPczuRYg19032136 = -555748492;    double wFcPczuRYg66268894 = -884417404;    double wFcPczuRYg50926869 = -147371962;    double wFcPczuRYg72331816 = -410268593;    double wFcPczuRYg1483774 = -997494228;    double wFcPczuRYg73630807 = 43457446;    double wFcPczuRYg61551289 = -376179017;    double wFcPczuRYg41626923 = -644544364;    double wFcPczuRYg90844340 = -18367527;    double wFcPczuRYg53134015 = -974615004;    double wFcPczuRYg26083743 = -942387958;    double wFcPczuRYg59377898 = -642461529;    double wFcPczuRYg66410101 = -897029890;    double wFcPczuRYg35845434 = -525237064;    double wFcPczuRYg41402199 = -249266817;    double wFcPczuRYg82107343 = -478300266;    double wFcPczuRYg15868295 = -404906812;    double wFcPczuRYg59082279 = -493843730;    double wFcPczuRYg57283178 = 49176446;    double wFcPczuRYg4298226 = -630001605;    double wFcPczuRYg63585888 = -646813360;    double wFcPczuRYg46609415 = -33572571;    double wFcPczuRYg40706248 = -850470613;    double wFcPczuRYg93075803 = -849175586;    double wFcPczuRYg72812544 = -312714283;    double wFcPczuRYg35669587 = -536122070;    double wFcPczuRYg21141106 = -702836943;    double wFcPczuRYg111946 = -824263017;    double wFcPczuRYg39527007 = -416349558;    double wFcPczuRYg6003178 = -242493711;    double wFcPczuRYg65415443 = -81274584;    double wFcPczuRYg2608160 = -201375019;    double wFcPczuRYg83310860 = -635563164;    double wFcPczuRYg38813740 = -698819200;    double wFcPczuRYg97278421 = -824302454;    double wFcPczuRYg2242163 = -42359719;    double wFcPczuRYg98893843 = -696941054;    double wFcPczuRYg94186371 = -408991817;    double wFcPczuRYg4460189 = -385562964;    double wFcPczuRYg78356874 = 40618902;    double wFcPczuRYg98436680 = 71084909;    double wFcPczuRYg41082656 = -126334309;     wFcPczuRYg77794489 = wFcPczuRYg63685281;     wFcPczuRYg63685281 = wFcPczuRYg34692313;     wFcPczuRYg34692313 = wFcPczuRYg35910045;     wFcPczuRYg35910045 = wFcPczuRYg7756256;     wFcPczuRYg7756256 = wFcPczuRYg14672072;     wFcPczuRYg14672072 = wFcPczuRYg20975798;     wFcPczuRYg20975798 = wFcPczuRYg41667332;     wFcPczuRYg41667332 = wFcPczuRYg46695999;     wFcPczuRYg46695999 = wFcPczuRYg95837416;     wFcPczuRYg95837416 = wFcPczuRYg46702571;     wFcPczuRYg46702571 = wFcPczuRYg29257503;     wFcPczuRYg29257503 = wFcPczuRYg88964826;     wFcPczuRYg88964826 = wFcPczuRYg97215531;     wFcPczuRYg97215531 = wFcPczuRYg42087844;     wFcPczuRYg42087844 = wFcPczuRYg93620687;     wFcPczuRYg93620687 = wFcPczuRYg52155992;     wFcPczuRYg52155992 = wFcPczuRYg9913256;     wFcPczuRYg9913256 = wFcPczuRYg31928056;     wFcPczuRYg31928056 = wFcPczuRYg82386482;     wFcPczuRYg82386482 = wFcPczuRYg65026242;     wFcPczuRYg65026242 = wFcPczuRYg9270859;     wFcPczuRYg9270859 = wFcPczuRYg64286195;     wFcPczuRYg64286195 = wFcPczuRYg46295467;     wFcPczuRYg46295467 = wFcPczuRYg16877910;     wFcPczuRYg16877910 = wFcPczuRYg41487362;     wFcPczuRYg41487362 = wFcPczuRYg63745203;     wFcPczuRYg63745203 = wFcPczuRYg48643981;     wFcPczuRYg48643981 = wFcPczuRYg40183558;     wFcPczuRYg40183558 = wFcPczuRYg73065192;     wFcPczuRYg73065192 = wFcPczuRYg34286128;     wFcPczuRYg34286128 = wFcPczuRYg5075648;     wFcPczuRYg5075648 = wFcPczuRYg38413163;     wFcPczuRYg38413163 = wFcPczuRYg35830812;     wFcPczuRYg35830812 = wFcPczuRYg71131789;     wFcPczuRYg71131789 = wFcPczuRYg82709945;     wFcPczuRYg82709945 = wFcPczuRYg27210586;     wFcPczuRYg27210586 = wFcPczuRYg16310558;     wFcPczuRYg16310558 = wFcPczuRYg68511057;     wFcPczuRYg68511057 = wFcPczuRYg49820713;     wFcPczuRYg49820713 = wFcPczuRYg66518187;     wFcPczuRYg66518187 = wFcPczuRYg5943963;     wFcPczuRYg5943963 = wFcPczuRYg51987681;     wFcPczuRYg51987681 = wFcPczuRYg59987970;     wFcPczuRYg59987970 = wFcPczuRYg82709579;     wFcPczuRYg82709579 = wFcPczuRYg70268494;     wFcPczuRYg70268494 = wFcPczuRYg781114;     wFcPczuRYg781114 = wFcPczuRYg70669399;     wFcPczuRYg70669399 = wFcPczuRYg75831437;     wFcPczuRYg75831437 = wFcPczuRYg4513972;     wFcPczuRYg4513972 = wFcPczuRYg51924086;     wFcPczuRYg51924086 = wFcPczuRYg34174182;     wFcPczuRYg34174182 = wFcPczuRYg65548640;     wFcPczuRYg65548640 = wFcPczuRYg32409986;     wFcPczuRYg32409986 = wFcPczuRYg70415368;     wFcPczuRYg70415368 = wFcPczuRYg68523630;     wFcPczuRYg68523630 = wFcPczuRYg99399085;     wFcPczuRYg99399085 = wFcPczuRYg88396846;     wFcPczuRYg88396846 = wFcPczuRYg19032136;     wFcPczuRYg19032136 = wFcPczuRYg66268894;     wFcPczuRYg66268894 = wFcPczuRYg50926869;     wFcPczuRYg50926869 = wFcPczuRYg72331816;     wFcPczuRYg72331816 = wFcPczuRYg1483774;     wFcPczuRYg1483774 = wFcPczuRYg73630807;     wFcPczuRYg73630807 = wFcPczuRYg61551289;     wFcPczuRYg61551289 = wFcPczuRYg41626923;     wFcPczuRYg41626923 = wFcPczuRYg90844340;     wFcPczuRYg90844340 = wFcPczuRYg53134015;     wFcPczuRYg53134015 = wFcPczuRYg26083743;     wFcPczuRYg26083743 = wFcPczuRYg59377898;     wFcPczuRYg59377898 = wFcPczuRYg66410101;     wFcPczuRYg66410101 = wFcPczuRYg35845434;     wFcPczuRYg35845434 = wFcPczuRYg41402199;     wFcPczuRYg41402199 = wFcPczuRYg82107343;     wFcPczuRYg82107343 = wFcPczuRYg15868295;     wFcPczuRYg15868295 = wFcPczuRYg59082279;     wFcPczuRYg59082279 = wFcPczuRYg57283178;     wFcPczuRYg57283178 = wFcPczuRYg4298226;     wFcPczuRYg4298226 = wFcPczuRYg63585888;     wFcPczuRYg63585888 = wFcPczuRYg46609415;     wFcPczuRYg46609415 = wFcPczuRYg40706248;     wFcPczuRYg40706248 = wFcPczuRYg93075803;     wFcPczuRYg93075803 = wFcPczuRYg72812544;     wFcPczuRYg72812544 = wFcPczuRYg35669587;     wFcPczuRYg35669587 = wFcPczuRYg21141106;     wFcPczuRYg21141106 = wFcPczuRYg111946;     wFcPczuRYg111946 = wFcPczuRYg39527007;     wFcPczuRYg39527007 = wFcPczuRYg6003178;     wFcPczuRYg6003178 = wFcPczuRYg65415443;     wFcPczuRYg65415443 = wFcPczuRYg2608160;     wFcPczuRYg2608160 = wFcPczuRYg83310860;     wFcPczuRYg83310860 = wFcPczuRYg38813740;     wFcPczuRYg38813740 = wFcPczuRYg97278421;     wFcPczuRYg97278421 = wFcPczuRYg2242163;     wFcPczuRYg2242163 = wFcPczuRYg98893843;     wFcPczuRYg98893843 = wFcPczuRYg94186371;     wFcPczuRYg94186371 = wFcPczuRYg4460189;     wFcPczuRYg4460189 = wFcPczuRYg78356874;     wFcPczuRYg78356874 = wFcPczuRYg98436680;     wFcPczuRYg98436680 = wFcPczuRYg41082656;     wFcPczuRYg41082656 = wFcPczuRYg77794489;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LSsAhiKwQv50820876() {     double jcwVBoIvmF29250727 = -773232755;    double jcwVBoIvmF98633257 = -748512004;    double jcwVBoIvmF91315062 = -232134696;    double jcwVBoIvmF76629745 = -824198120;    double jcwVBoIvmF11040176 = -797427380;    double jcwVBoIvmF26720309 = -657913037;    double jcwVBoIvmF28911396 = -621116963;    double jcwVBoIvmF31466170 = -672628732;    double jcwVBoIvmF6304248 = -252866766;    double jcwVBoIvmF51074604 = -562317098;    double jcwVBoIvmF48522171 = -87908112;    double jcwVBoIvmF12607510 = 52648991;    double jcwVBoIvmF43209544 = -285088713;    double jcwVBoIvmF41236998 = -368638122;    double jcwVBoIvmF52778306 = -364864280;    double jcwVBoIvmF19923093 = 60726090;    double jcwVBoIvmF2012991 = -967847863;    double jcwVBoIvmF62370355 = -275073737;    double jcwVBoIvmF12965567 = -60553097;    double jcwVBoIvmF17375855 = -947728444;    double jcwVBoIvmF99074471 = 25228401;    double jcwVBoIvmF22835560 = -318224696;    double jcwVBoIvmF285552 = -753827620;    double jcwVBoIvmF72509580 = -45953278;    double jcwVBoIvmF93267633 = -590239013;    double jcwVBoIvmF41542167 = -479592490;    double jcwVBoIvmF30436638 = -222380502;    double jcwVBoIvmF43905632 = -136083681;    double jcwVBoIvmF79096583 = -572859651;    double jcwVBoIvmF23335290 = 36356880;    double jcwVBoIvmF8235827 = -150698649;    double jcwVBoIvmF56418516 = -921582711;    double jcwVBoIvmF65920658 = -473562979;    double jcwVBoIvmF36288321 = -700567138;    double jcwVBoIvmF80460322 = -941145893;    double jcwVBoIvmF6937507 = -22145622;    double jcwVBoIvmF45709401 = -669471018;    double jcwVBoIvmF7341211 = -272890613;    double jcwVBoIvmF26665258 = -251433434;    double jcwVBoIvmF85209148 = -268875325;    double jcwVBoIvmF4083679 = -959928043;    double jcwVBoIvmF87117422 = -239459464;    double jcwVBoIvmF99337836 = -815353171;    double jcwVBoIvmF97633209 = -507721823;    double jcwVBoIvmF18696033 = 2404818;    double jcwVBoIvmF67302056 = -858161181;    double jcwVBoIvmF29783769 = -318968187;    double jcwVBoIvmF30582612 = -267591796;    double jcwVBoIvmF61822789 = 93704023;    double jcwVBoIvmF92629384 = 17454163;    double jcwVBoIvmF25770595 = -822295082;    double jcwVBoIvmF70149009 = -242700071;    double jcwVBoIvmF57517443 = -659389243;    double jcwVBoIvmF81213379 = -449532808;    double jcwVBoIvmF51746404 = -632712245;    double jcwVBoIvmF6415168 = -355008059;    double jcwVBoIvmF98347705 = -994684385;    double jcwVBoIvmF18805483 = -86181419;    double jcwVBoIvmF83362112 = -133959107;    double jcwVBoIvmF69498009 = -217834891;    double jcwVBoIvmF96283670 = -335532536;    double jcwVBoIvmF85005764 = -385033283;    double jcwVBoIvmF52369586 = 230919;    double jcwVBoIvmF82968958 = -189223646;    double jcwVBoIvmF42838777 = -311618449;    double jcwVBoIvmF92103655 = -166325402;    double jcwVBoIvmF46686852 = -473788030;    double jcwVBoIvmF6921224 = -584521575;    double jcwVBoIvmF60776675 = -427492229;    double jcwVBoIvmF45840800 = -242718658;    double jcwVBoIvmF74213692 = -269802893;    double jcwVBoIvmF94671780 = -594957251;    double jcwVBoIvmF35705097 = 76359697;    double jcwVBoIvmF27756419 = -791677772;    double jcwVBoIvmF13292176 = -987800402;    double jcwVBoIvmF11957050 = -735312135;    double jcwVBoIvmF23497723 = -502871526;    double jcwVBoIvmF2652343 = -146105797;    double jcwVBoIvmF53813547 = 51641904;    double jcwVBoIvmF25965578 = -732077833;    double jcwVBoIvmF11758398 = -60624304;    double jcwVBoIvmF99854026 = -954788707;    double jcwVBoIvmF82082842 = -129787704;    double jcwVBoIvmF86467198 = -490313814;    double jcwVBoIvmF97564694 = -141348039;    double jcwVBoIvmF38086818 = -907998579;    double jcwVBoIvmF98901073 = -162193469;    double jcwVBoIvmF84707278 = 75969829;    double jcwVBoIvmF84541916 = 32145107;    double jcwVBoIvmF74045155 = -486137834;    double jcwVBoIvmF8589801 = -27461238;    double jcwVBoIvmF26903919 = -483289600;    double jcwVBoIvmF23979099 = -38931506;    double jcwVBoIvmF57167249 = 66401456;    double jcwVBoIvmF88925477 = -933342790;    double jcwVBoIvmF19077915 = -474894761;    double jcwVBoIvmF34747836 = -139690384;    double jcwVBoIvmF16368879 = -526129526;    double jcwVBoIvmF54794432 = -96103375;    double jcwVBoIvmF26592378 = -773232755;     jcwVBoIvmF29250727 = jcwVBoIvmF98633257;     jcwVBoIvmF98633257 = jcwVBoIvmF91315062;     jcwVBoIvmF91315062 = jcwVBoIvmF76629745;     jcwVBoIvmF76629745 = jcwVBoIvmF11040176;     jcwVBoIvmF11040176 = jcwVBoIvmF26720309;     jcwVBoIvmF26720309 = jcwVBoIvmF28911396;     jcwVBoIvmF28911396 = jcwVBoIvmF31466170;     jcwVBoIvmF31466170 = jcwVBoIvmF6304248;     jcwVBoIvmF6304248 = jcwVBoIvmF51074604;     jcwVBoIvmF51074604 = jcwVBoIvmF48522171;     jcwVBoIvmF48522171 = jcwVBoIvmF12607510;     jcwVBoIvmF12607510 = jcwVBoIvmF43209544;     jcwVBoIvmF43209544 = jcwVBoIvmF41236998;     jcwVBoIvmF41236998 = jcwVBoIvmF52778306;     jcwVBoIvmF52778306 = jcwVBoIvmF19923093;     jcwVBoIvmF19923093 = jcwVBoIvmF2012991;     jcwVBoIvmF2012991 = jcwVBoIvmF62370355;     jcwVBoIvmF62370355 = jcwVBoIvmF12965567;     jcwVBoIvmF12965567 = jcwVBoIvmF17375855;     jcwVBoIvmF17375855 = jcwVBoIvmF99074471;     jcwVBoIvmF99074471 = jcwVBoIvmF22835560;     jcwVBoIvmF22835560 = jcwVBoIvmF285552;     jcwVBoIvmF285552 = jcwVBoIvmF72509580;     jcwVBoIvmF72509580 = jcwVBoIvmF93267633;     jcwVBoIvmF93267633 = jcwVBoIvmF41542167;     jcwVBoIvmF41542167 = jcwVBoIvmF30436638;     jcwVBoIvmF30436638 = jcwVBoIvmF43905632;     jcwVBoIvmF43905632 = jcwVBoIvmF79096583;     jcwVBoIvmF79096583 = jcwVBoIvmF23335290;     jcwVBoIvmF23335290 = jcwVBoIvmF8235827;     jcwVBoIvmF8235827 = jcwVBoIvmF56418516;     jcwVBoIvmF56418516 = jcwVBoIvmF65920658;     jcwVBoIvmF65920658 = jcwVBoIvmF36288321;     jcwVBoIvmF36288321 = jcwVBoIvmF80460322;     jcwVBoIvmF80460322 = jcwVBoIvmF6937507;     jcwVBoIvmF6937507 = jcwVBoIvmF45709401;     jcwVBoIvmF45709401 = jcwVBoIvmF7341211;     jcwVBoIvmF7341211 = jcwVBoIvmF26665258;     jcwVBoIvmF26665258 = jcwVBoIvmF85209148;     jcwVBoIvmF85209148 = jcwVBoIvmF4083679;     jcwVBoIvmF4083679 = jcwVBoIvmF87117422;     jcwVBoIvmF87117422 = jcwVBoIvmF99337836;     jcwVBoIvmF99337836 = jcwVBoIvmF97633209;     jcwVBoIvmF97633209 = jcwVBoIvmF18696033;     jcwVBoIvmF18696033 = jcwVBoIvmF67302056;     jcwVBoIvmF67302056 = jcwVBoIvmF29783769;     jcwVBoIvmF29783769 = jcwVBoIvmF30582612;     jcwVBoIvmF30582612 = jcwVBoIvmF61822789;     jcwVBoIvmF61822789 = jcwVBoIvmF92629384;     jcwVBoIvmF92629384 = jcwVBoIvmF25770595;     jcwVBoIvmF25770595 = jcwVBoIvmF70149009;     jcwVBoIvmF70149009 = jcwVBoIvmF57517443;     jcwVBoIvmF57517443 = jcwVBoIvmF81213379;     jcwVBoIvmF81213379 = jcwVBoIvmF51746404;     jcwVBoIvmF51746404 = jcwVBoIvmF6415168;     jcwVBoIvmF6415168 = jcwVBoIvmF98347705;     jcwVBoIvmF98347705 = jcwVBoIvmF18805483;     jcwVBoIvmF18805483 = jcwVBoIvmF83362112;     jcwVBoIvmF83362112 = jcwVBoIvmF69498009;     jcwVBoIvmF69498009 = jcwVBoIvmF96283670;     jcwVBoIvmF96283670 = jcwVBoIvmF85005764;     jcwVBoIvmF85005764 = jcwVBoIvmF52369586;     jcwVBoIvmF52369586 = jcwVBoIvmF82968958;     jcwVBoIvmF82968958 = jcwVBoIvmF42838777;     jcwVBoIvmF42838777 = jcwVBoIvmF92103655;     jcwVBoIvmF92103655 = jcwVBoIvmF46686852;     jcwVBoIvmF46686852 = jcwVBoIvmF6921224;     jcwVBoIvmF6921224 = jcwVBoIvmF60776675;     jcwVBoIvmF60776675 = jcwVBoIvmF45840800;     jcwVBoIvmF45840800 = jcwVBoIvmF74213692;     jcwVBoIvmF74213692 = jcwVBoIvmF94671780;     jcwVBoIvmF94671780 = jcwVBoIvmF35705097;     jcwVBoIvmF35705097 = jcwVBoIvmF27756419;     jcwVBoIvmF27756419 = jcwVBoIvmF13292176;     jcwVBoIvmF13292176 = jcwVBoIvmF11957050;     jcwVBoIvmF11957050 = jcwVBoIvmF23497723;     jcwVBoIvmF23497723 = jcwVBoIvmF2652343;     jcwVBoIvmF2652343 = jcwVBoIvmF53813547;     jcwVBoIvmF53813547 = jcwVBoIvmF25965578;     jcwVBoIvmF25965578 = jcwVBoIvmF11758398;     jcwVBoIvmF11758398 = jcwVBoIvmF99854026;     jcwVBoIvmF99854026 = jcwVBoIvmF82082842;     jcwVBoIvmF82082842 = jcwVBoIvmF86467198;     jcwVBoIvmF86467198 = jcwVBoIvmF97564694;     jcwVBoIvmF97564694 = jcwVBoIvmF38086818;     jcwVBoIvmF38086818 = jcwVBoIvmF98901073;     jcwVBoIvmF98901073 = jcwVBoIvmF84707278;     jcwVBoIvmF84707278 = jcwVBoIvmF84541916;     jcwVBoIvmF84541916 = jcwVBoIvmF74045155;     jcwVBoIvmF74045155 = jcwVBoIvmF8589801;     jcwVBoIvmF8589801 = jcwVBoIvmF26903919;     jcwVBoIvmF26903919 = jcwVBoIvmF23979099;     jcwVBoIvmF23979099 = jcwVBoIvmF57167249;     jcwVBoIvmF57167249 = jcwVBoIvmF88925477;     jcwVBoIvmF88925477 = jcwVBoIvmF19077915;     jcwVBoIvmF19077915 = jcwVBoIvmF34747836;     jcwVBoIvmF34747836 = jcwVBoIvmF16368879;     jcwVBoIvmF16368879 = jcwVBoIvmF54794432;     jcwVBoIvmF54794432 = jcwVBoIvmF26592378;     jcwVBoIvmF26592378 = jcwVBoIvmF29250727;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mZQnmHEYNc57149423() {     double WkOpnFmbqC83905124 = -110760337;    double WkOpnFmbqC45630877 = -228076812;    double WkOpnFmbqC52047709 = -617519309;    double WkOpnFmbqC48064687 = -830300534;    double WkOpnFmbqC85491846 = -469862612;    double WkOpnFmbqC40894706 = -787063873;    double WkOpnFmbqC50012099 = -923009636;    double WkOpnFmbqC60641273 = -73196949;    double WkOpnFmbqC94078658 = -299720135;    double WkOpnFmbqC33706590 = -841118367;    double WkOpnFmbqC27133466 = -713264683;    double WkOpnFmbqC75372222 = -133775089;    double WkOpnFmbqC7026859 = -88629017;    double WkOpnFmbqC34203429 = 19258301;    double WkOpnFmbqC83002379 = -271985893;    double WkOpnFmbqC9690628 = -669433474;    double WkOpnFmbqC1844755 = -341447692;    double WkOpnFmbqC29966942 = -338638200;    double WkOpnFmbqC14186167 = -346364645;    double WkOpnFmbqC35010410 = -193094590;    double WkOpnFmbqC33248860 = -542854744;    double WkOpnFmbqC74088148 = -484668804;    double WkOpnFmbqC36755383 = -240571041;    double WkOpnFmbqC73937949 = -844994094;    double WkOpnFmbqC89020249 = -963152453;    double WkOpnFmbqC82783113 = -483301279;    double WkOpnFmbqC32426563 = -777224781;    double WkOpnFmbqC97154631 = -144135535;    double WkOpnFmbqC18994261 = -953104542;    double WkOpnFmbqC17770699 = -254401089;    double WkOpnFmbqC59941356 = -699571175;    double WkOpnFmbqC87410126 = -233079242;    double WkOpnFmbqC33576535 = 69919404;    double WkOpnFmbqC30944213 = -380687948;    double WkOpnFmbqC14964479 = -59009156;    double WkOpnFmbqC94264048 = -270141237;    double WkOpnFmbqC20413889 = -519897639;    double WkOpnFmbqC20318450 = -146819631;    double WkOpnFmbqC77434907 = -633382048;    double WkOpnFmbqC9195544 = 72816076;    double WkOpnFmbqC60043079 = 27521799;    double WkOpnFmbqC23792081 = -846991572;    double WkOpnFmbqC90338019 = -655858491;    double WkOpnFmbqC6627609 = 18598511;    double WkOpnFmbqC78680096 = -130112781;    double WkOpnFmbqC22635658 = -926950814;    double WkOpnFmbqC46257481 = -346025570;    double WkOpnFmbqC54009921 = -245244169;    double WkOpnFmbqC4165556 = -906026191;    double WkOpnFmbqC37471048 = -222565127;    double WkOpnFmbqC884135 = -479510583;    double WkOpnFmbqC59531159 = -757766058;    double WkOpnFmbqC89245446 = -399304697;    double WkOpnFmbqC38629137 = -345419293;    double WkOpnFmbqC23900564 = 71379068;    double WkOpnFmbqC9816976 = -626091533;    double WkOpnFmbqC8875494 = -987505771;    double WkOpnFmbqC78109760 = -772525216;    double WkOpnFmbqC59044437 = -867148082;    double WkOpnFmbqC2708733 = -986561333;    double WkOpnFmbqC8468144 = 90160907;    double WkOpnFmbqC52857468 = -678874101;    double WkOpnFmbqC41647013 = -120092408;    double WkOpnFmbqC76307960 = 54680953;    double WkOpnFmbqC73765234 = -41547193;    double WkOpnFmbqC39723340 = -380185442;    double WkOpnFmbqC41795688 = -103694493;    double WkOpnFmbqC76082646 = -707941070;    double WkOpnFmbqC19238950 = -921732544;    double WkOpnFmbqC88738331 = 98155344;    double WkOpnFmbqC89276739 = -49535836;    double WkOpnFmbqC81526305 = -94628061;    double WkOpnFmbqC52532035 = -705256152;    double WkOpnFmbqC4990624 = -319180722;    double WkOpnFmbqC74967330 = -120616390;    double WkOpnFmbqC9456779 = -695863173;    double WkOpnFmbqC83750128 = -828810314;    double WkOpnFmbqC30127775 = -159169552;    double WkOpnFmbqC95257852 = -614881314;    double WkOpnFmbqC66384592 = 63798361;    double WkOpnFmbqC36525632 = -37275709;    double WkOpnFmbqC78416641 = -431980612;    double WkOpnFmbqC92989076 = -238109345;    double WkOpnFmbqC81523213 = -630539416;    double WkOpnFmbqC16886564 = -774890506;    double WkOpnFmbqC410197 = -941805117;    double WkOpnFmbqC98164680 = -833774546;    double WkOpnFmbqC94947397 = -584661304;    double WkOpnFmbqC7043650 = -352067017;    double WkOpnFmbqC5147504 = -432917623;    double WkOpnFmbqC85388554 = -282635466;    double WkOpnFmbqC42304129 = -747372424;    double WkOpnFmbqC61274012 = -279671549;    double WkOpnFmbqC74726174 = -646820716;    double WkOpnFmbqC727400 = 82655168;    double WkOpnFmbqC7185612 = -293604101;    double WkOpnFmbqC82145067 = -626899164;    double WkOpnFmbqC14030060 = -610539444;    double WkOpnFmbqC32862375 = -939854297;    double WkOpnFmbqC38956757 = -110760337;     WkOpnFmbqC83905124 = WkOpnFmbqC45630877;     WkOpnFmbqC45630877 = WkOpnFmbqC52047709;     WkOpnFmbqC52047709 = WkOpnFmbqC48064687;     WkOpnFmbqC48064687 = WkOpnFmbqC85491846;     WkOpnFmbqC85491846 = WkOpnFmbqC40894706;     WkOpnFmbqC40894706 = WkOpnFmbqC50012099;     WkOpnFmbqC50012099 = WkOpnFmbqC60641273;     WkOpnFmbqC60641273 = WkOpnFmbqC94078658;     WkOpnFmbqC94078658 = WkOpnFmbqC33706590;     WkOpnFmbqC33706590 = WkOpnFmbqC27133466;     WkOpnFmbqC27133466 = WkOpnFmbqC75372222;     WkOpnFmbqC75372222 = WkOpnFmbqC7026859;     WkOpnFmbqC7026859 = WkOpnFmbqC34203429;     WkOpnFmbqC34203429 = WkOpnFmbqC83002379;     WkOpnFmbqC83002379 = WkOpnFmbqC9690628;     WkOpnFmbqC9690628 = WkOpnFmbqC1844755;     WkOpnFmbqC1844755 = WkOpnFmbqC29966942;     WkOpnFmbqC29966942 = WkOpnFmbqC14186167;     WkOpnFmbqC14186167 = WkOpnFmbqC35010410;     WkOpnFmbqC35010410 = WkOpnFmbqC33248860;     WkOpnFmbqC33248860 = WkOpnFmbqC74088148;     WkOpnFmbqC74088148 = WkOpnFmbqC36755383;     WkOpnFmbqC36755383 = WkOpnFmbqC73937949;     WkOpnFmbqC73937949 = WkOpnFmbqC89020249;     WkOpnFmbqC89020249 = WkOpnFmbqC82783113;     WkOpnFmbqC82783113 = WkOpnFmbqC32426563;     WkOpnFmbqC32426563 = WkOpnFmbqC97154631;     WkOpnFmbqC97154631 = WkOpnFmbqC18994261;     WkOpnFmbqC18994261 = WkOpnFmbqC17770699;     WkOpnFmbqC17770699 = WkOpnFmbqC59941356;     WkOpnFmbqC59941356 = WkOpnFmbqC87410126;     WkOpnFmbqC87410126 = WkOpnFmbqC33576535;     WkOpnFmbqC33576535 = WkOpnFmbqC30944213;     WkOpnFmbqC30944213 = WkOpnFmbqC14964479;     WkOpnFmbqC14964479 = WkOpnFmbqC94264048;     WkOpnFmbqC94264048 = WkOpnFmbqC20413889;     WkOpnFmbqC20413889 = WkOpnFmbqC20318450;     WkOpnFmbqC20318450 = WkOpnFmbqC77434907;     WkOpnFmbqC77434907 = WkOpnFmbqC9195544;     WkOpnFmbqC9195544 = WkOpnFmbqC60043079;     WkOpnFmbqC60043079 = WkOpnFmbqC23792081;     WkOpnFmbqC23792081 = WkOpnFmbqC90338019;     WkOpnFmbqC90338019 = WkOpnFmbqC6627609;     WkOpnFmbqC6627609 = WkOpnFmbqC78680096;     WkOpnFmbqC78680096 = WkOpnFmbqC22635658;     WkOpnFmbqC22635658 = WkOpnFmbqC46257481;     WkOpnFmbqC46257481 = WkOpnFmbqC54009921;     WkOpnFmbqC54009921 = WkOpnFmbqC4165556;     WkOpnFmbqC4165556 = WkOpnFmbqC37471048;     WkOpnFmbqC37471048 = WkOpnFmbqC884135;     WkOpnFmbqC884135 = WkOpnFmbqC59531159;     WkOpnFmbqC59531159 = WkOpnFmbqC89245446;     WkOpnFmbqC89245446 = WkOpnFmbqC38629137;     WkOpnFmbqC38629137 = WkOpnFmbqC23900564;     WkOpnFmbqC23900564 = WkOpnFmbqC9816976;     WkOpnFmbqC9816976 = WkOpnFmbqC8875494;     WkOpnFmbqC8875494 = WkOpnFmbqC78109760;     WkOpnFmbqC78109760 = WkOpnFmbqC59044437;     WkOpnFmbqC59044437 = WkOpnFmbqC2708733;     WkOpnFmbqC2708733 = WkOpnFmbqC8468144;     WkOpnFmbqC8468144 = WkOpnFmbqC52857468;     WkOpnFmbqC52857468 = WkOpnFmbqC41647013;     WkOpnFmbqC41647013 = WkOpnFmbqC76307960;     WkOpnFmbqC76307960 = WkOpnFmbqC73765234;     WkOpnFmbqC73765234 = WkOpnFmbqC39723340;     WkOpnFmbqC39723340 = WkOpnFmbqC41795688;     WkOpnFmbqC41795688 = WkOpnFmbqC76082646;     WkOpnFmbqC76082646 = WkOpnFmbqC19238950;     WkOpnFmbqC19238950 = WkOpnFmbqC88738331;     WkOpnFmbqC88738331 = WkOpnFmbqC89276739;     WkOpnFmbqC89276739 = WkOpnFmbqC81526305;     WkOpnFmbqC81526305 = WkOpnFmbqC52532035;     WkOpnFmbqC52532035 = WkOpnFmbqC4990624;     WkOpnFmbqC4990624 = WkOpnFmbqC74967330;     WkOpnFmbqC74967330 = WkOpnFmbqC9456779;     WkOpnFmbqC9456779 = WkOpnFmbqC83750128;     WkOpnFmbqC83750128 = WkOpnFmbqC30127775;     WkOpnFmbqC30127775 = WkOpnFmbqC95257852;     WkOpnFmbqC95257852 = WkOpnFmbqC66384592;     WkOpnFmbqC66384592 = WkOpnFmbqC36525632;     WkOpnFmbqC36525632 = WkOpnFmbqC78416641;     WkOpnFmbqC78416641 = WkOpnFmbqC92989076;     WkOpnFmbqC92989076 = WkOpnFmbqC81523213;     WkOpnFmbqC81523213 = WkOpnFmbqC16886564;     WkOpnFmbqC16886564 = WkOpnFmbqC410197;     WkOpnFmbqC410197 = WkOpnFmbqC98164680;     WkOpnFmbqC98164680 = WkOpnFmbqC94947397;     WkOpnFmbqC94947397 = WkOpnFmbqC7043650;     WkOpnFmbqC7043650 = WkOpnFmbqC5147504;     WkOpnFmbqC5147504 = WkOpnFmbqC85388554;     WkOpnFmbqC85388554 = WkOpnFmbqC42304129;     WkOpnFmbqC42304129 = WkOpnFmbqC61274012;     WkOpnFmbqC61274012 = WkOpnFmbqC74726174;     WkOpnFmbqC74726174 = WkOpnFmbqC727400;     WkOpnFmbqC727400 = WkOpnFmbqC7185612;     WkOpnFmbqC7185612 = WkOpnFmbqC82145067;     WkOpnFmbqC82145067 = WkOpnFmbqC14030060;     WkOpnFmbqC14030060 = WkOpnFmbqC32862375;     WkOpnFmbqC32862375 = WkOpnFmbqC38956757;     WkOpnFmbqC38956757 = WkOpnFmbqC83905124;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LAffKmasyL72528688() {     double CsDLxAblZZ35361362 = -757658783;    double CsDLxAblZZ80578853 = -500706904;    double CsDLxAblZZ8670459 = -340096231;    double CsDLxAblZZ88784387 = -835487584;    double CsDLxAblZZ88775766 = -686432565;    double CsDLxAblZZ52942944 = -621842084;    double CsDLxAblZZ57947697 = -464618407;    double CsDLxAblZZ50440111 = -663679947;    double CsDLxAblZZ53686907 = -834545500;    double CsDLxAblZZ88943777 = -528099447;    double CsDLxAblZZ28953067 = 75182230;    double CsDLxAblZZ58722229 = -127235564;    double CsDLxAblZZ61271577 = -746638276;    double CsDLxAblZZ78224894 = -146029740;    double CsDLxAblZZ93692842 = 81960721;    double CsDLxAblZZ35993034 = -245069104;    double CsDLxAblZZ51701753 = -304007546;    double CsDLxAblZZ82424041 = -337667997;    double CsDLxAblZZ95223677 = 15695539;    double CsDLxAblZZ69999782 = -816655815;    double CsDLxAblZZ67297089 = -640725423;    double CsDLxAblZZ87652849 = -736146296;    double CsDLxAblZZ72754740 = -959302949;    double CsDLxAblZZ152063 = -149178788;    double CsDLxAblZZ65409973 = -290128887;    double CsDLxAblZZ82837918 = -266453746;    double CsDLxAblZZ99117998 = -423842418;    double CsDLxAblZZ92416281 = -810979617;    double CsDLxAblZZ57907286 = -341312686;    double CsDLxAblZZ68040796 = -556545361;    double CsDLxAblZZ33891055 = -451112822;    double CsDLxAblZZ38752995 = 77148709;    double CsDLxAblZZ61084029 = -568120569;    double CsDLxAblZZ31401722 = -328790636;    double CsDLxAblZZ24293013 = -739192926;    double CsDLxAblZZ18491609 = -315937494;    double CsDLxAblZZ38912703 = -722760266;    double CsDLxAblZZ11349103 = -39659298;    double CsDLxAblZZ35589109 = -958038360;    double CsDLxAblZZ44583979 = -351746234;    double CsDLxAblZZ97608569 = -13145836;    double CsDLxAblZZ4965540 = -703393845;    double CsDLxAblZZ37688175 = -355288011;    double CsDLxAblZZ44272848 = -84029205;    double CsDLxAblZZ14666551 = -132752739;    double CsDLxAblZZ19669220 = -655421987;    double CsDLxAblZZ75260136 = -919024347;    double CsDLxAblZZ13923134 = -886248685;    double CsDLxAblZZ90156907 = -655796851;    double CsDLxAblZZ25586461 = -756581527;    double CsDLxAblZZ74730643 = -243143760;    double CsDLxAblZZ95505986 = -425572144;    double CsDLxAblZZ81214249 = -343232836;    double CsDLxAblZZ87432530 = -201922805;    double CsDLxAblZZ5231600 = 9856689;    double CsDLxAblZZ47708513 = 78487512;    double CsDLxAblZZ7824114 = -541403955;    double CsDLxAblZZ8518396 = -90917443;    double CsDLxAblZZ23374414 = -445358697;    double CsDLxAblZZ5937848 = -319978820;    double CsDLxAblZZ53824945 = -97999667;    double CsDLxAblZZ65531416 = -653638791;    double CsDLxAblZZ92532825 = -222367261;    double CsDLxAblZZ85646111 = -178000139;    double CsDLxAblZZ55052722 = 23013375;    double CsDLxAblZZ90200071 = 98033521;    double CsDLxAblZZ97638199 = -559114996;    double CsDLxAblZZ29869855 = -317847641;    double CsDLxAblZZ53931882 = -406836814;    double CsDLxAblZZ75201233 = -602101786;    double CsDLxAblZZ97080330 = -522308839;    double CsDLxAblZZ40352651 = -164348248;    double CsDLxAblZZ46834932 = -379629638;    double CsDLxAblZZ50639699 = -632558228;    double CsDLxAblZZ72391212 = -703509980;    double CsDLxAblZZ62331549 = -937331578;    double CsDLxAblZZ49964674 = -280858285;    double CsDLxAblZZ28481892 = -775273744;    double CsDLxAblZZ85485512 = 83573951;    double CsDLxAblZZ45740754 = -634706901;    double CsDLxAblZZ7577783 = -347429400;    double CsDLxAblZZ85194864 = -537593733;    double CsDLxAblZZ2259375 = -55182766;    double CsDLxAblZZ32320825 = -584731160;    double CsDLxAblZZ93310153 = -213401602;    double CsDLxAblZZ38385069 = 74459321;    double CsDLxAblZZ57538746 = -579618456;    double CsDLxAblZZ73651499 = -266197764;    double CsDLxAblZZ26170123 = -238647326;    double CsDLxAblZZ76584499 = -717680439;    double CsDLxAblZZ10667496 = -774533539;    double CsDLxAblZZ30394307 = -531842823;    double CsDLxAblZZ87974688 = -594300602;    double CsDLxAblZZ29651261 = -538059541;    double CsDLxAblZZ90759033 = -153746567;    double CsDLxAblZZ32077154 = -359507045;    double CsDLxAblZZ12432715 = -381026585;    double CsDLxAblZZ52042064 = -77287873;    double CsDLxAblZZ89220125 = -7042580;    double CsDLxAblZZ24466479 = -757658783;     CsDLxAblZZ35361362 = CsDLxAblZZ80578853;     CsDLxAblZZ80578853 = CsDLxAblZZ8670459;     CsDLxAblZZ8670459 = CsDLxAblZZ88784387;     CsDLxAblZZ88784387 = CsDLxAblZZ88775766;     CsDLxAblZZ88775766 = CsDLxAblZZ52942944;     CsDLxAblZZ52942944 = CsDLxAblZZ57947697;     CsDLxAblZZ57947697 = CsDLxAblZZ50440111;     CsDLxAblZZ50440111 = CsDLxAblZZ53686907;     CsDLxAblZZ53686907 = CsDLxAblZZ88943777;     CsDLxAblZZ88943777 = CsDLxAblZZ28953067;     CsDLxAblZZ28953067 = CsDLxAblZZ58722229;     CsDLxAblZZ58722229 = CsDLxAblZZ61271577;     CsDLxAblZZ61271577 = CsDLxAblZZ78224894;     CsDLxAblZZ78224894 = CsDLxAblZZ93692842;     CsDLxAblZZ93692842 = CsDLxAblZZ35993034;     CsDLxAblZZ35993034 = CsDLxAblZZ51701753;     CsDLxAblZZ51701753 = CsDLxAblZZ82424041;     CsDLxAblZZ82424041 = CsDLxAblZZ95223677;     CsDLxAblZZ95223677 = CsDLxAblZZ69999782;     CsDLxAblZZ69999782 = CsDLxAblZZ67297089;     CsDLxAblZZ67297089 = CsDLxAblZZ87652849;     CsDLxAblZZ87652849 = CsDLxAblZZ72754740;     CsDLxAblZZ72754740 = CsDLxAblZZ152063;     CsDLxAblZZ152063 = CsDLxAblZZ65409973;     CsDLxAblZZ65409973 = CsDLxAblZZ82837918;     CsDLxAblZZ82837918 = CsDLxAblZZ99117998;     CsDLxAblZZ99117998 = CsDLxAblZZ92416281;     CsDLxAblZZ92416281 = CsDLxAblZZ57907286;     CsDLxAblZZ57907286 = CsDLxAblZZ68040796;     CsDLxAblZZ68040796 = CsDLxAblZZ33891055;     CsDLxAblZZ33891055 = CsDLxAblZZ38752995;     CsDLxAblZZ38752995 = CsDLxAblZZ61084029;     CsDLxAblZZ61084029 = CsDLxAblZZ31401722;     CsDLxAblZZ31401722 = CsDLxAblZZ24293013;     CsDLxAblZZ24293013 = CsDLxAblZZ18491609;     CsDLxAblZZ18491609 = CsDLxAblZZ38912703;     CsDLxAblZZ38912703 = CsDLxAblZZ11349103;     CsDLxAblZZ11349103 = CsDLxAblZZ35589109;     CsDLxAblZZ35589109 = CsDLxAblZZ44583979;     CsDLxAblZZ44583979 = CsDLxAblZZ97608569;     CsDLxAblZZ97608569 = CsDLxAblZZ4965540;     CsDLxAblZZ4965540 = CsDLxAblZZ37688175;     CsDLxAblZZ37688175 = CsDLxAblZZ44272848;     CsDLxAblZZ44272848 = CsDLxAblZZ14666551;     CsDLxAblZZ14666551 = CsDLxAblZZ19669220;     CsDLxAblZZ19669220 = CsDLxAblZZ75260136;     CsDLxAblZZ75260136 = CsDLxAblZZ13923134;     CsDLxAblZZ13923134 = CsDLxAblZZ90156907;     CsDLxAblZZ90156907 = CsDLxAblZZ25586461;     CsDLxAblZZ25586461 = CsDLxAblZZ74730643;     CsDLxAblZZ74730643 = CsDLxAblZZ95505986;     CsDLxAblZZ95505986 = CsDLxAblZZ81214249;     CsDLxAblZZ81214249 = CsDLxAblZZ87432530;     CsDLxAblZZ87432530 = CsDLxAblZZ5231600;     CsDLxAblZZ5231600 = CsDLxAblZZ47708513;     CsDLxAblZZ47708513 = CsDLxAblZZ7824114;     CsDLxAblZZ7824114 = CsDLxAblZZ8518396;     CsDLxAblZZ8518396 = CsDLxAblZZ23374414;     CsDLxAblZZ23374414 = CsDLxAblZZ5937848;     CsDLxAblZZ5937848 = CsDLxAblZZ53824945;     CsDLxAblZZ53824945 = CsDLxAblZZ65531416;     CsDLxAblZZ65531416 = CsDLxAblZZ92532825;     CsDLxAblZZ92532825 = CsDLxAblZZ85646111;     CsDLxAblZZ85646111 = CsDLxAblZZ55052722;     CsDLxAblZZ55052722 = CsDLxAblZZ90200071;     CsDLxAblZZ90200071 = CsDLxAblZZ97638199;     CsDLxAblZZ97638199 = CsDLxAblZZ29869855;     CsDLxAblZZ29869855 = CsDLxAblZZ53931882;     CsDLxAblZZ53931882 = CsDLxAblZZ75201233;     CsDLxAblZZ75201233 = CsDLxAblZZ97080330;     CsDLxAblZZ97080330 = CsDLxAblZZ40352651;     CsDLxAblZZ40352651 = CsDLxAblZZ46834932;     CsDLxAblZZ46834932 = CsDLxAblZZ50639699;     CsDLxAblZZ50639699 = CsDLxAblZZ72391212;     CsDLxAblZZ72391212 = CsDLxAblZZ62331549;     CsDLxAblZZ62331549 = CsDLxAblZZ49964674;     CsDLxAblZZ49964674 = CsDLxAblZZ28481892;     CsDLxAblZZ28481892 = CsDLxAblZZ85485512;     CsDLxAblZZ85485512 = CsDLxAblZZ45740754;     CsDLxAblZZ45740754 = CsDLxAblZZ7577783;     CsDLxAblZZ7577783 = CsDLxAblZZ85194864;     CsDLxAblZZ85194864 = CsDLxAblZZ2259375;     CsDLxAblZZ2259375 = CsDLxAblZZ32320825;     CsDLxAblZZ32320825 = CsDLxAblZZ93310153;     CsDLxAblZZ93310153 = CsDLxAblZZ38385069;     CsDLxAblZZ38385069 = CsDLxAblZZ57538746;     CsDLxAblZZ57538746 = CsDLxAblZZ73651499;     CsDLxAblZZ73651499 = CsDLxAblZZ26170123;     CsDLxAblZZ26170123 = CsDLxAblZZ76584499;     CsDLxAblZZ76584499 = CsDLxAblZZ10667496;     CsDLxAblZZ10667496 = CsDLxAblZZ30394307;     CsDLxAblZZ30394307 = CsDLxAblZZ87974688;     CsDLxAblZZ87974688 = CsDLxAblZZ29651261;     CsDLxAblZZ29651261 = CsDLxAblZZ90759033;     CsDLxAblZZ90759033 = CsDLxAblZZ32077154;     CsDLxAblZZ32077154 = CsDLxAblZZ12432715;     CsDLxAblZZ12432715 = CsDLxAblZZ52042064;     CsDLxAblZZ52042064 = CsDLxAblZZ89220125;     CsDLxAblZZ89220125 = CsDLxAblZZ24466479;     CsDLxAblZZ24466479 = CsDLxAblZZ35361362;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hYQVXVIjpr48540808() {     double HcwczvGGlb22283040 = -898309990;    double HcwczvGGlb90226591 = -611293483;    double HcwczvGGlb1366473 = -871211613;    double HcwczvGGlb16647582 = -841284871;    double HcwczvGGlb39504853 = -540246050;    double HcwczvGGlb66408621 = 80464622;    double HcwczvGGlb7993366 = -146416446;    double HcwczvGGlb33156460 = -94219783;    double HcwczvGGlb32072598 = 55943797;    double HcwczvGGlb62444163 = -242960653;    double HcwczvGGlb48633797 = -78906514;    double HcwczvGGlb28348707 = -249338458;    double HcwczvGGlb21898026 = -835001565;    double HcwczvGGlb21543004 = 57471860;    double HcwczvGGlb17405712 = -104804848;    double HcwczvGGlb71272193 = -223720690;    double HcwczvGGlb1541929 = -973927383;    double HcwczvGGlb11640799 = -13054245;    double HcwczvGGlb56383249 = -420825431;    double HcwczvGGlb26752610 = -154753654;    double HcwczvGGlb34762757 = -685404421;    double HcwczvGGlb26342809 = -564268198;    double HcwczvGGlb42401080 = -856709199;    double HcwczvGGlb76509012 = -83267564;    double HcwczvGGlb21374959 = -314396679;    double HcwczvGGlb77016818 = -929977087;    double HcwczvGGlb56008426 = -675944483;    double HcwczvGGlb93002831 = 61371110;    double HcwczvGGlb30810079 = -757545302;    double HcwczvGGlb47754435 = -117765431;    double HcwczvGGlb93011307 = -367541721;    double HcwczvGGlb43195025 = -93772990;    double HcwczvGGlb15357113 = -51812299;    double HcwczvGGlb61324820 = -464905405;    double HcwczvGGlb17071962 = -11163020;    double HcwczvGGlb51451824 = -496533292;    double HcwczvGGlb94881966 = -690665555;    double HcwczvGGlb83677479 = 80108129;    double HcwczvGGlb88820275 = -220889519;    double HcwczvGGlb72371054 = -632139403;    double HcwczvGGlb80770000 = -835068486;    double HcwczvGGlb89806465 = 39450693;    double HcwczvGGlb14138349 = -148768061;    double HcwczvGGlb62817527 = -134024887;    double HcwczvGGlb66651411 = -588644456;    double HcwczvGGlb22236141 = -610772102;    double HcwczvGGlb95910161 = -394728861;    double HcwczvGGlb16179078 = 14981562;    double HcwczvGGlb80382535 = -505540506;    double HcwczvGGlb18186040 = 5400139;    double HcwczvGGlb16088506 = -302498487;    double HcwczvGGlb419029 = 75115177;    double HcwczvGGlb66355851 = -151152523;    double HcwczvGGlb41977500 = -818014965;    double HcwczvGGlb13778051 = -641256553;    double HcwczvGGlb95940231 = -234041792;    double HcwczvGGlb47825511 = -754584285;    double HcwczvGGlb24857460 = -687944050;    double HcwczvGGlb95272622 = -426888193;    double HcwczvGGlb62488034 = -610268964;    double HcwczvGGlb10400195 = -243590896;    double HcwczvGGlb14990535 = -107787556;    double HcwczvGGlb2346381 = -336674481;    double HcwczvGGlb84318162 = -826290772;    double HcwczvGGlb69432856 = -875418932;    double HcwczvGGlb5438772 = -985133525;    double HcwczvGGlb12991595 = -97526160;    double HcwczvGGlb60573206 = -270096161;    double HcwczvGGlb4471043 = -931365121;    double HcwczvGGlb65953887 = -608271556;    double HcwczvGGlb76390226 = -533055135;    double HcwczvGGlb17864449 = -954035512;    double HcwczvGGlb22820524 = -792164727;    double HcwczvGGlb84012195 = -788686028;    double HcwczvGGlb45982609 = -319685168;    double HcwczvGGlb44956292 = -624855114;    double HcwczvGGlb12204460 = -315500138;    double HcwczvGGlb79583553 = -622684312;    double HcwczvGGlb9857602 = -494623108;    double HcwczvGGlb99138817 = -703624577;    double HcwczvGGlb81106657 = -435248227;    double HcwczvGGlb39829348 = -590926045;    double HcwczvGGlb12620297 = -433088385;    double HcwczvGGlb12624040 = -662945441;    double HcwczvGGlb31665929 = -815266945;    double HcwczvGGlb92592279 = -342656899;    double HcwczvGGlb76839173 = -942620467;    double HcwczvGGlb73379612 = -233797334;    double HcwczvGGlb47546769 = -823648853;    double HcwczvGGlb21131730 = -777121228;    double HcwczvGGlb3626313 = -741949008;    double HcwczvGGlb70024507 = 97278494;    double HcwczvGGlb88404856 = -493003679;    double HcwczvGGlb26332241 = -610620556;    double HcwczvGGlb61970859 = -288548507;    double HcwczvGGlb65779466 = -627280930;    double HcwczvGGlb87460084 = -623874826;    double HcwczvGGlb29820186 = -322477289;    double HcwczvGGlb93384671 = -258605956;    double HcwczvGGlb61212639 = -898309990;     HcwczvGGlb22283040 = HcwczvGGlb90226591;     HcwczvGGlb90226591 = HcwczvGGlb1366473;     HcwczvGGlb1366473 = HcwczvGGlb16647582;     HcwczvGGlb16647582 = HcwczvGGlb39504853;     HcwczvGGlb39504853 = HcwczvGGlb66408621;     HcwczvGGlb66408621 = HcwczvGGlb7993366;     HcwczvGGlb7993366 = HcwczvGGlb33156460;     HcwczvGGlb33156460 = HcwczvGGlb32072598;     HcwczvGGlb32072598 = HcwczvGGlb62444163;     HcwczvGGlb62444163 = HcwczvGGlb48633797;     HcwczvGGlb48633797 = HcwczvGGlb28348707;     HcwczvGGlb28348707 = HcwczvGGlb21898026;     HcwczvGGlb21898026 = HcwczvGGlb21543004;     HcwczvGGlb21543004 = HcwczvGGlb17405712;     HcwczvGGlb17405712 = HcwczvGGlb71272193;     HcwczvGGlb71272193 = HcwczvGGlb1541929;     HcwczvGGlb1541929 = HcwczvGGlb11640799;     HcwczvGGlb11640799 = HcwczvGGlb56383249;     HcwczvGGlb56383249 = HcwczvGGlb26752610;     HcwczvGGlb26752610 = HcwczvGGlb34762757;     HcwczvGGlb34762757 = HcwczvGGlb26342809;     HcwczvGGlb26342809 = HcwczvGGlb42401080;     HcwczvGGlb42401080 = HcwczvGGlb76509012;     HcwczvGGlb76509012 = HcwczvGGlb21374959;     HcwczvGGlb21374959 = HcwczvGGlb77016818;     HcwczvGGlb77016818 = HcwczvGGlb56008426;     HcwczvGGlb56008426 = HcwczvGGlb93002831;     HcwczvGGlb93002831 = HcwczvGGlb30810079;     HcwczvGGlb30810079 = HcwczvGGlb47754435;     HcwczvGGlb47754435 = HcwczvGGlb93011307;     HcwczvGGlb93011307 = HcwczvGGlb43195025;     HcwczvGGlb43195025 = HcwczvGGlb15357113;     HcwczvGGlb15357113 = HcwczvGGlb61324820;     HcwczvGGlb61324820 = HcwczvGGlb17071962;     HcwczvGGlb17071962 = HcwczvGGlb51451824;     HcwczvGGlb51451824 = HcwczvGGlb94881966;     HcwczvGGlb94881966 = HcwczvGGlb83677479;     HcwczvGGlb83677479 = HcwczvGGlb88820275;     HcwczvGGlb88820275 = HcwczvGGlb72371054;     HcwczvGGlb72371054 = HcwczvGGlb80770000;     HcwczvGGlb80770000 = HcwczvGGlb89806465;     HcwczvGGlb89806465 = HcwczvGGlb14138349;     HcwczvGGlb14138349 = HcwczvGGlb62817527;     HcwczvGGlb62817527 = HcwczvGGlb66651411;     HcwczvGGlb66651411 = HcwczvGGlb22236141;     HcwczvGGlb22236141 = HcwczvGGlb95910161;     HcwczvGGlb95910161 = HcwczvGGlb16179078;     HcwczvGGlb16179078 = HcwczvGGlb80382535;     HcwczvGGlb80382535 = HcwczvGGlb18186040;     HcwczvGGlb18186040 = HcwczvGGlb16088506;     HcwczvGGlb16088506 = HcwczvGGlb419029;     HcwczvGGlb419029 = HcwczvGGlb66355851;     HcwczvGGlb66355851 = HcwczvGGlb41977500;     HcwczvGGlb41977500 = HcwczvGGlb13778051;     HcwczvGGlb13778051 = HcwczvGGlb95940231;     HcwczvGGlb95940231 = HcwczvGGlb47825511;     HcwczvGGlb47825511 = HcwczvGGlb24857460;     HcwczvGGlb24857460 = HcwczvGGlb95272622;     HcwczvGGlb95272622 = HcwczvGGlb62488034;     HcwczvGGlb62488034 = HcwczvGGlb10400195;     HcwczvGGlb10400195 = HcwczvGGlb14990535;     HcwczvGGlb14990535 = HcwczvGGlb2346381;     HcwczvGGlb2346381 = HcwczvGGlb84318162;     HcwczvGGlb84318162 = HcwczvGGlb69432856;     HcwczvGGlb69432856 = HcwczvGGlb5438772;     HcwczvGGlb5438772 = HcwczvGGlb12991595;     HcwczvGGlb12991595 = HcwczvGGlb60573206;     HcwczvGGlb60573206 = HcwczvGGlb4471043;     HcwczvGGlb4471043 = HcwczvGGlb65953887;     HcwczvGGlb65953887 = HcwczvGGlb76390226;     HcwczvGGlb76390226 = HcwczvGGlb17864449;     HcwczvGGlb17864449 = HcwczvGGlb22820524;     HcwczvGGlb22820524 = HcwczvGGlb84012195;     HcwczvGGlb84012195 = HcwczvGGlb45982609;     HcwczvGGlb45982609 = HcwczvGGlb44956292;     HcwczvGGlb44956292 = HcwczvGGlb12204460;     HcwczvGGlb12204460 = HcwczvGGlb79583553;     HcwczvGGlb79583553 = HcwczvGGlb9857602;     HcwczvGGlb9857602 = HcwczvGGlb99138817;     HcwczvGGlb99138817 = HcwczvGGlb81106657;     HcwczvGGlb81106657 = HcwczvGGlb39829348;     HcwczvGGlb39829348 = HcwczvGGlb12620297;     HcwczvGGlb12620297 = HcwczvGGlb12624040;     HcwczvGGlb12624040 = HcwczvGGlb31665929;     HcwczvGGlb31665929 = HcwczvGGlb92592279;     HcwczvGGlb92592279 = HcwczvGGlb76839173;     HcwczvGGlb76839173 = HcwczvGGlb73379612;     HcwczvGGlb73379612 = HcwczvGGlb47546769;     HcwczvGGlb47546769 = HcwczvGGlb21131730;     HcwczvGGlb21131730 = HcwczvGGlb3626313;     HcwczvGGlb3626313 = HcwczvGGlb70024507;     HcwczvGGlb70024507 = HcwczvGGlb88404856;     HcwczvGGlb88404856 = HcwczvGGlb26332241;     HcwczvGGlb26332241 = HcwczvGGlb61970859;     HcwczvGGlb61970859 = HcwczvGGlb65779466;     HcwczvGGlb65779466 = HcwczvGGlb87460084;     HcwczvGGlb87460084 = HcwczvGGlb29820186;     HcwczvGGlb29820186 = HcwczvGGlb93384671;     HcwczvGGlb93384671 = HcwczvGGlb61212639;     HcwczvGGlb61212639 = HcwczvGGlb22283040;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ISTbtxncbN94236500() {     double CsuQzRNljq41471998 = -742084815;    double CsuQzRNljq62524449 = -252901816;    double CsuQzRNljq26025854 = -448057766;    double CsuQzRNljq939029 = -846777041;    double CsuQzRNljq66511357 = -575437766;    double CsuQzRNljq79165578 = -585771131;    double CsuQzRNljq86983999 = -308119851;    double CsuQzRNljq69414053 = -654731192;    double CsuQzRNljq1069567 = -316224237;    double CsuQzRNljq26812950 = -493881795;    double CsuQzRNljq9383963 = -861727429;    double CsuQzRNljq4836949 = -307120138;    double CsuQzRNljq79333609 = -108187839;    double CsuQzRNljq15212792 = 76578640;    double CsuQzRNljq34607378 = -571214316;    double CsuQzRNljq52062975 = -550864298;    double CsuQzRNljq1390516 = -740167228;    double CsuQzRNljq2477727 = -400262266;    double CsuQzRNljq77481790 = 91944176;    double CsuQzRNljq22623710 = -685583185;    double CsuQzRNljq35519706 = -206679257;    double CsuQzRNljq52470139 = -54067895;    double CsuQzRNljq45223929 = -64778278;    double CsuQzRNljq27794545 = -252404299;    double CsuQzRNljq37552313 = 9981214;    double CsuQzRNljq24133671 = -53314993;    double CsuQzRNljq67799358 = -625304334;    double CsuQzRNljq40926932 = -385875565;    double CsuQzRNljq36717988 = -109765690;    double CsuQzRNljq12746303 = -49447603;    double CsuQzRNljq59546283 = -751526995;    double CsuQzRNljq21087475 = -24119865;    double CsuQzRNljq56247401 = -662678152;    double CsuQzRNljq26515124 = 42985866;    double CsuQzRNljq68125702 = -537239953;    double CsuQzRNljq30045712 = -609729329;    double CsuQzRNljq32116006 = -776049514;    double CsuQzRNljq15356995 = -906427990;    double CsuQzRNljq44512959 = -564643260;    double CsuQzRNljq3958810 = -434617142;    double CsuQzRNljq91133461 = -166363628;    double CsuQzRNljq22813658 = -67328185;    double CsuQzRNljq76038513 = -995222847;    double CsuQzRNljq90912486 = -760336586;    double CsuQzRNljq10637068 = -267910294;    double CsuQzRNljq72036382 = -452682755;    double CsuQzRNljq20736503 = -419080506;    double CsuQzRNljq97263655 = -404905573;    double CsuQzRNljq18491026 = -305297676;    double CsuQzRNljq58543536 = -430617226;    double CsuQzRNljq23690692 = -763992439;    double CsuQzRNljq20862963 = -608444208;    double CsuQzRNljq4911055 = -27076435;    double CsuQzRNljq93651681 = 45687199;    double CsuQzRNljq58716794 = -447574366;    double CsuQzRNljq89001859 = -588016920;    double CsuQzRNljq17300521 = -88123539;    double CsuQzRNljq98231309 = -95653468;    double CsuQzRNljq63386716 = -756758256;    double CsuQzRNljq42377686 = -422122773;    double CsuQzRNljq11366221 = -960466798;    double CsuQzRNljq46057067 = -922244287;    double CsuQzRNljq32696065 = -444965503;    double CsuQzRNljq88323264 = -166776635;    double CsuQzRNljq67266667 = -742354801;    double CsuQzRNljq88296488 = -737607564;    double CsuQzRNljq48589547 = -644441987;    double CsuQzRNljq52818486 = -51173706;    double CsuQzRNljq47087089 = -386181408;    double CsuQzRNljq4561666 = -961484987;    double CsuQzRNljq19946970 = -774814785;    double CsuQzRNljq86033521 = -833739239;    double CsuQzRNljq57964768 = -835619006;    double CsuQzRNljq73522980 = -473438682;    double CsuQzRNljq31490249 = -419219558;    double CsuQzRNljq12706049 = -39351072;    double CsuQzRNljq76431625 = -58845049;    double CsuQzRNljq54311442 = -304441692;    double CsuQzRNljq17157477 = -984494005;    double CsuQzRNljq65515930 = -537336031;    double CsuQzRNljq3397169 = -634234488;    double CsuQzRNljq70535702 = -120398761;    double CsuQzRNljq22435907 = 19422111;    double CsuQzRNljq78174452 = -679148464;    double CsuQzRNljq89055611 = -285455165;    double CsuQzRNljq38683320 = -43082787;    double CsuQzRNljq16176421 = -997043431;    double CsuQzRNljq62595720 = -608365351;    double CsuQzRNljq67798329 = -509439769;    double CsuQzRNljq79123843 = -949223033;    double CsuQzRNljq12745192 = -421605791;    double CsuQzRNljq33884696 = -580396047;    double CsuQzRNljq51970279 = -49669735;    double CsuQzRNljq2135274 = -42520488;    double CsuQzRNljq92592588 = -474150345;    double CsuQzRNljq45076394 = -244119342;    double CsuQzRNljq90117593 = -622362683;    double CsuQzRNljq87715249 = -728446213;    double CsuQzRNljq23645820 = 82018214;    double CsuQzRNljq22340580 = -742084815;     CsuQzRNljq41471998 = CsuQzRNljq62524449;     CsuQzRNljq62524449 = CsuQzRNljq26025854;     CsuQzRNljq26025854 = CsuQzRNljq939029;     CsuQzRNljq939029 = CsuQzRNljq66511357;     CsuQzRNljq66511357 = CsuQzRNljq79165578;     CsuQzRNljq79165578 = CsuQzRNljq86983999;     CsuQzRNljq86983999 = CsuQzRNljq69414053;     CsuQzRNljq69414053 = CsuQzRNljq1069567;     CsuQzRNljq1069567 = CsuQzRNljq26812950;     CsuQzRNljq26812950 = CsuQzRNljq9383963;     CsuQzRNljq9383963 = CsuQzRNljq4836949;     CsuQzRNljq4836949 = CsuQzRNljq79333609;     CsuQzRNljq79333609 = CsuQzRNljq15212792;     CsuQzRNljq15212792 = CsuQzRNljq34607378;     CsuQzRNljq34607378 = CsuQzRNljq52062975;     CsuQzRNljq52062975 = CsuQzRNljq1390516;     CsuQzRNljq1390516 = CsuQzRNljq2477727;     CsuQzRNljq2477727 = CsuQzRNljq77481790;     CsuQzRNljq77481790 = CsuQzRNljq22623710;     CsuQzRNljq22623710 = CsuQzRNljq35519706;     CsuQzRNljq35519706 = CsuQzRNljq52470139;     CsuQzRNljq52470139 = CsuQzRNljq45223929;     CsuQzRNljq45223929 = CsuQzRNljq27794545;     CsuQzRNljq27794545 = CsuQzRNljq37552313;     CsuQzRNljq37552313 = CsuQzRNljq24133671;     CsuQzRNljq24133671 = CsuQzRNljq67799358;     CsuQzRNljq67799358 = CsuQzRNljq40926932;     CsuQzRNljq40926932 = CsuQzRNljq36717988;     CsuQzRNljq36717988 = CsuQzRNljq12746303;     CsuQzRNljq12746303 = CsuQzRNljq59546283;     CsuQzRNljq59546283 = CsuQzRNljq21087475;     CsuQzRNljq21087475 = CsuQzRNljq56247401;     CsuQzRNljq56247401 = CsuQzRNljq26515124;     CsuQzRNljq26515124 = CsuQzRNljq68125702;     CsuQzRNljq68125702 = CsuQzRNljq30045712;     CsuQzRNljq30045712 = CsuQzRNljq32116006;     CsuQzRNljq32116006 = CsuQzRNljq15356995;     CsuQzRNljq15356995 = CsuQzRNljq44512959;     CsuQzRNljq44512959 = CsuQzRNljq3958810;     CsuQzRNljq3958810 = CsuQzRNljq91133461;     CsuQzRNljq91133461 = CsuQzRNljq22813658;     CsuQzRNljq22813658 = CsuQzRNljq76038513;     CsuQzRNljq76038513 = CsuQzRNljq90912486;     CsuQzRNljq90912486 = CsuQzRNljq10637068;     CsuQzRNljq10637068 = CsuQzRNljq72036382;     CsuQzRNljq72036382 = CsuQzRNljq20736503;     CsuQzRNljq20736503 = CsuQzRNljq97263655;     CsuQzRNljq97263655 = CsuQzRNljq18491026;     CsuQzRNljq18491026 = CsuQzRNljq58543536;     CsuQzRNljq58543536 = CsuQzRNljq23690692;     CsuQzRNljq23690692 = CsuQzRNljq20862963;     CsuQzRNljq20862963 = CsuQzRNljq4911055;     CsuQzRNljq4911055 = CsuQzRNljq93651681;     CsuQzRNljq93651681 = CsuQzRNljq58716794;     CsuQzRNljq58716794 = CsuQzRNljq89001859;     CsuQzRNljq89001859 = CsuQzRNljq17300521;     CsuQzRNljq17300521 = CsuQzRNljq98231309;     CsuQzRNljq98231309 = CsuQzRNljq63386716;     CsuQzRNljq63386716 = CsuQzRNljq42377686;     CsuQzRNljq42377686 = CsuQzRNljq11366221;     CsuQzRNljq11366221 = CsuQzRNljq46057067;     CsuQzRNljq46057067 = CsuQzRNljq32696065;     CsuQzRNljq32696065 = CsuQzRNljq88323264;     CsuQzRNljq88323264 = CsuQzRNljq67266667;     CsuQzRNljq67266667 = CsuQzRNljq88296488;     CsuQzRNljq88296488 = CsuQzRNljq48589547;     CsuQzRNljq48589547 = CsuQzRNljq52818486;     CsuQzRNljq52818486 = CsuQzRNljq47087089;     CsuQzRNljq47087089 = CsuQzRNljq4561666;     CsuQzRNljq4561666 = CsuQzRNljq19946970;     CsuQzRNljq19946970 = CsuQzRNljq86033521;     CsuQzRNljq86033521 = CsuQzRNljq57964768;     CsuQzRNljq57964768 = CsuQzRNljq73522980;     CsuQzRNljq73522980 = CsuQzRNljq31490249;     CsuQzRNljq31490249 = CsuQzRNljq12706049;     CsuQzRNljq12706049 = CsuQzRNljq76431625;     CsuQzRNljq76431625 = CsuQzRNljq54311442;     CsuQzRNljq54311442 = CsuQzRNljq17157477;     CsuQzRNljq17157477 = CsuQzRNljq65515930;     CsuQzRNljq65515930 = CsuQzRNljq3397169;     CsuQzRNljq3397169 = CsuQzRNljq70535702;     CsuQzRNljq70535702 = CsuQzRNljq22435907;     CsuQzRNljq22435907 = CsuQzRNljq78174452;     CsuQzRNljq78174452 = CsuQzRNljq89055611;     CsuQzRNljq89055611 = CsuQzRNljq38683320;     CsuQzRNljq38683320 = CsuQzRNljq16176421;     CsuQzRNljq16176421 = CsuQzRNljq62595720;     CsuQzRNljq62595720 = CsuQzRNljq67798329;     CsuQzRNljq67798329 = CsuQzRNljq79123843;     CsuQzRNljq79123843 = CsuQzRNljq12745192;     CsuQzRNljq12745192 = CsuQzRNljq33884696;     CsuQzRNljq33884696 = CsuQzRNljq51970279;     CsuQzRNljq51970279 = CsuQzRNljq2135274;     CsuQzRNljq2135274 = CsuQzRNljq92592588;     CsuQzRNljq92592588 = CsuQzRNljq45076394;     CsuQzRNljq45076394 = CsuQzRNljq90117593;     CsuQzRNljq90117593 = CsuQzRNljq87715249;     CsuQzRNljq87715249 = CsuQzRNljq23645820;     CsuQzRNljq23645820 = CsuQzRNljq22340580;     CsuQzRNljq22340580 = CsuQzRNljq41471998;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jaLUzGkhpq31211673() {     double ZTcwsoYhyH9198247 = -911485336;    double ZTcwsoYhyH38446647 = -764475182;    double ZTcwsoYhyH95375698 = -259455610;    double ZTcwsoYhyH29590636 = -667622992;    double ZTcwsoYhyH99074042 = -862414849;    double ZTcwsoYhyH14679731 = -16661484;    double ZTcwsoYhyH26586371 = -632750952;    double ZTcwsoYhyH85086858 = -328936921;    double ZTcwsoYhyH93198451 = -967212975;    double ZTcwsoYhyH32584063 = -694628730;    double ZTcwsoYhyH88635132 = -192467330;    double ZTcwsoYhyH12266862 = -106919705;    double ZTcwsoYhyH14377389 = -945023329;    double ZTcwsoYhyH18705333 = -84179591;    double ZTcwsoYhyH86290690 = -254283250;    double ZTcwsoYhyH81115899 = -535167927;    double ZTcwsoYhyH7234864 = -363461790;    double ZTcwsoYhyH63858089 = -85512111;    double ZTcwsoYhyH37971090 = -375328668;    double ZTcwsoYhyH55579201 = -104875944;    double ZTcwsoYhyH81897850 = -440549788;    double ZTcwsoYhyH43329034 = -902381984;    double ZTcwsoYhyH48925817 = -113073291;    double ZTcwsoYhyH96516071 = -998276862;    double ZTcwsoYhyH5782634 = -765358311;    double ZTcwsoYhyH87769795 = -401706361;    double ZTcwsoYhyH83120435 = -128846260;    double ZTcwsoYhyH28992907 = -775245146;    double ZTcwsoYhyH16391319 = -719848249;    double ZTcwsoYhyH3557944 = -454416875;    double ZTcwsoYhyH85544238 = -671063855;    double ZTcwsoYhyH32360913 = -656516453;    double ZTcwsoYhyH55082618 = -837448248;    double ZTcwsoYhyH91530755 = -477701907;    double ZTcwsoYhyH92208948 = -642447041;    double ZTcwsoYhyH27240229 = -756894855;    double ZTcwsoYhyH62792598 = -758224223;    double ZTcwsoYhyH25923536 = -895387120;    double ZTcwsoYhyH77365038 = -196422107;    double ZTcwsoYhyH72031021 = -521964259;    double ZTcwsoYhyH51787099 = -644935035;    double ZTcwsoYhyH60345052 = -511496872;    double ZTcwsoYhyH50871883 = -501378367;    double ZTcwsoYhyH40594105 = -840985013;    double ZTcwsoYhyH2654324 = -417795576;    double ZTcwsoYhyH65231031 = -378166907;    double ZTcwsoYhyH44257981 = -351713998;    double ZTcwsoYhyH93818602 = -792549548;    double ZTcwsoYhyH89266553 = 80951953;    double ZTcwsoYhyH30831042 = 80356923;    double ZTcwsoYhyH70705738 = 59591779;    double ZTcwsoYhyH19879328 = -906343255;    double ZTcwsoYhyH26945803 = -951126216;    double ZTcwsoYhyH6772741 = -866026977;    double ZTcwsoYhyH49313823 = 50730922;    double ZTcwsoYhyH65869212 = 90896648;    double ZTcwsoYhyH89520829 = -551401891;    double ZTcwsoYhyH98859627 = -261178749;    double ZTcwsoYhyH23808003 = -902264681;    double ZTcwsoYhyH11304247 = -360708489;    double ZTcwsoYhyH31559295 = -887815224;    double ZTcwsoYhyH97593463 = -857505806;    double ZTcwsoYhyH68695539 = -609088673;    double ZTcwsoYhyH89640508 = -412796101;    double ZTcwsoYhyH47039825 = 76435125;    double ZTcwsoYhyH56274220 = -535950877;    double ZTcwsoYhyH57184243 = -269471457;    double ZTcwsoYhyH22846634 = -367321423;    double ZTcwsoYhyH26496385 = -441732550;    double ZTcwsoYhyH59050462 = -497388395;    double ZTcwsoYhyH18323302 = -776943704;    double ZTcwsoYhyH81311327 = -468074670;    double ZTcwsoYhyH86493050 = -889090005;    double ZTcwsoYhyH65940068 = -853364410;    double ZTcwsoYhyH3792102 = -459940910;    double ZTcwsoYhyH21552798 = -929052917;    double ZTcwsoYhyH92457151 = -301003617;    double ZTcwsoYhyH8331712 = -272088278;    double ZTcwsoYhyH93861748 = -480481287;    double ZTcwsoYhyH40551602 = -287191405;    double ZTcwsoYhyH43511815 = 50007636;    double ZTcwsoYhyH89301833 = -336296713;    double ZTcwsoYhyH39726354 = -756197100;    double ZTcwsoYhyH85560276 = -700205172;    double ZTcwsoYhyH32852205 = -414008654;    double ZTcwsoYhyH65664910 = -764720601;    double ZTcwsoYhyH5415110 = -705390238;    double ZTcwsoYhyH48309878 = -971421272;    double ZTcwsoYhyH42216933 = -428432830;    double ZTcwsoYhyH26339736 = -633343689;    double ZTcwsoYhyH37719399 = -105492964;    double ZTcwsoYhyH63932970 = -397045475;    double ZTcwsoYhyH2115533 = -993122440;    double ZTcwsoYhyH66060791 = -835713618;    double ZTcwsoYhyH40471727 = -634149035;    double ZTcwsoYhyH54193635 = -787429229;    double ZTcwsoYhyH91649512 = -902408199;    double ZTcwsoYhyH61231374 = 11417733;    double ZTcwsoYhyH93554280 = -817420139;    double ZTcwsoYhyH46380103 = -911485336;     ZTcwsoYhyH9198247 = ZTcwsoYhyH38446647;     ZTcwsoYhyH38446647 = ZTcwsoYhyH95375698;     ZTcwsoYhyH95375698 = ZTcwsoYhyH29590636;     ZTcwsoYhyH29590636 = ZTcwsoYhyH99074042;     ZTcwsoYhyH99074042 = ZTcwsoYhyH14679731;     ZTcwsoYhyH14679731 = ZTcwsoYhyH26586371;     ZTcwsoYhyH26586371 = ZTcwsoYhyH85086858;     ZTcwsoYhyH85086858 = ZTcwsoYhyH93198451;     ZTcwsoYhyH93198451 = ZTcwsoYhyH32584063;     ZTcwsoYhyH32584063 = ZTcwsoYhyH88635132;     ZTcwsoYhyH88635132 = ZTcwsoYhyH12266862;     ZTcwsoYhyH12266862 = ZTcwsoYhyH14377389;     ZTcwsoYhyH14377389 = ZTcwsoYhyH18705333;     ZTcwsoYhyH18705333 = ZTcwsoYhyH86290690;     ZTcwsoYhyH86290690 = ZTcwsoYhyH81115899;     ZTcwsoYhyH81115899 = ZTcwsoYhyH7234864;     ZTcwsoYhyH7234864 = ZTcwsoYhyH63858089;     ZTcwsoYhyH63858089 = ZTcwsoYhyH37971090;     ZTcwsoYhyH37971090 = ZTcwsoYhyH55579201;     ZTcwsoYhyH55579201 = ZTcwsoYhyH81897850;     ZTcwsoYhyH81897850 = ZTcwsoYhyH43329034;     ZTcwsoYhyH43329034 = ZTcwsoYhyH48925817;     ZTcwsoYhyH48925817 = ZTcwsoYhyH96516071;     ZTcwsoYhyH96516071 = ZTcwsoYhyH5782634;     ZTcwsoYhyH5782634 = ZTcwsoYhyH87769795;     ZTcwsoYhyH87769795 = ZTcwsoYhyH83120435;     ZTcwsoYhyH83120435 = ZTcwsoYhyH28992907;     ZTcwsoYhyH28992907 = ZTcwsoYhyH16391319;     ZTcwsoYhyH16391319 = ZTcwsoYhyH3557944;     ZTcwsoYhyH3557944 = ZTcwsoYhyH85544238;     ZTcwsoYhyH85544238 = ZTcwsoYhyH32360913;     ZTcwsoYhyH32360913 = ZTcwsoYhyH55082618;     ZTcwsoYhyH55082618 = ZTcwsoYhyH91530755;     ZTcwsoYhyH91530755 = ZTcwsoYhyH92208948;     ZTcwsoYhyH92208948 = ZTcwsoYhyH27240229;     ZTcwsoYhyH27240229 = ZTcwsoYhyH62792598;     ZTcwsoYhyH62792598 = ZTcwsoYhyH25923536;     ZTcwsoYhyH25923536 = ZTcwsoYhyH77365038;     ZTcwsoYhyH77365038 = ZTcwsoYhyH72031021;     ZTcwsoYhyH72031021 = ZTcwsoYhyH51787099;     ZTcwsoYhyH51787099 = ZTcwsoYhyH60345052;     ZTcwsoYhyH60345052 = ZTcwsoYhyH50871883;     ZTcwsoYhyH50871883 = ZTcwsoYhyH40594105;     ZTcwsoYhyH40594105 = ZTcwsoYhyH2654324;     ZTcwsoYhyH2654324 = ZTcwsoYhyH65231031;     ZTcwsoYhyH65231031 = ZTcwsoYhyH44257981;     ZTcwsoYhyH44257981 = ZTcwsoYhyH93818602;     ZTcwsoYhyH93818602 = ZTcwsoYhyH89266553;     ZTcwsoYhyH89266553 = ZTcwsoYhyH30831042;     ZTcwsoYhyH30831042 = ZTcwsoYhyH70705738;     ZTcwsoYhyH70705738 = ZTcwsoYhyH19879328;     ZTcwsoYhyH19879328 = ZTcwsoYhyH26945803;     ZTcwsoYhyH26945803 = ZTcwsoYhyH6772741;     ZTcwsoYhyH6772741 = ZTcwsoYhyH49313823;     ZTcwsoYhyH49313823 = ZTcwsoYhyH65869212;     ZTcwsoYhyH65869212 = ZTcwsoYhyH89520829;     ZTcwsoYhyH89520829 = ZTcwsoYhyH98859627;     ZTcwsoYhyH98859627 = ZTcwsoYhyH23808003;     ZTcwsoYhyH23808003 = ZTcwsoYhyH11304247;     ZTcwsoYhyH11304247 = ZTcwsoYhyH31559295;     ZTcwsoYhyH31559295 = ZTcwsoYhyH97593463;     ZTcwsoYhyH97593463 = ZTcwsoYhyH68695539;     ZTcwsoYhyH68695539 = ZTcwsoYhyH89640508;     ZTcwsoYhyH89640508 = ZTcwsoYhyH47039825;     ZTcwsoYhyH47039825 = ZTcwsoYhyH56274220;     ZTcwsoYhyH56274220 = ZTcwsoYhyH57184243;     ZTcwsoYhyH57184243 = ZTcwsoYhyH22846634;     ZTcwsoYhyH22846634 = ZTcwsoYhyH26496385;     ZTcwsoYhyH26496385 = ZTcwsoYhyH59050462;     ZTcwsoYhyH59050462 = ZTcwsoYhyH18323302;     ZTcwsoYhyH18323302 = ZTcwsoYhyH81311327;     ZTcwsoYhyH81311327 = ZTcwsoYhyH86493050;     ZTcwsoYhyH86493050 = ZTcwsoYhyH65940068;     ZTcwsoYhyH65940068 = ZTcwsoYhyH3792102;     ZTcwsoYhyH3792102 = ZTcwsoYhyH21552798;     ZTcwsoYhyH21552798 = ZTcwsoYhyH92457151;     ZTcwsoYhyH92457151 = ZTcwsoYhyH8331712;     ZTcwsoYhyH8331712 = ZTcwsoYhyH93861748;     ZTcwsoYhyH93861748 = ZTcwsoYhyH40551602;     ZTcwsoYhyH40551602 = ZTcwsoYhyH43511815;     ZTcwsoYhyH43511815 = ZTcwsoYhyH89301833;     ZTcwsoYhyH89301833 = ZTcwsoYhyH39726354;     ZTcwsoYhyH39726354 = ZTcwsoYhyH85560276;     ZTcwsoYhyH85560276 = ZTcwsoYhyH32852205;     ZTcwsoYhyH32852205 = ZTcwsoYhyH65664910;     ZTcwsoYhyH65664910 = ZTcwsoYhyH5415110;     ZTcwsoYhyH5415110 = ZTcwsoYhyH48309878;     ZTcwsoYhyH48309878 = ZTcwsoYhyH42216933;     ZTcwsoYhyH42216933 = ZTcwsoYhyH26339736;     ZTcwsoYhyH26339736 = ZTcwsoYhyH37719399;     ZTcwsoYhyH37719399 = ZTcwsoYhyH63932970;     ZTcwsoYhyH63932970 = ZTcwsoYhyH2115533;     ZTcwsoYhyH2115533 = ZTcwsoYhyH66060791;     ZTcwsoYhyH66060791 = ZTcwsoYhyH40471727;     ZTcwsoYhyH40471727 = ZTcwsoYhyH54193635;     ZTcwsoYhyH54193635 = ZTcwsoYhyH91649512;     ZTcwsoYhyH91649512 = ZTcwsoYhyH61231374;     ZTcwsoYhyH61231374 = ZTcwsoYhyH93554280;     ZTcwsoYhyH93554280 = ZTcwsoYhyH46380103;     ZTcwsoYhyH46380103 = ZTcwsoYhyH9198247;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YSKjFhpezL85627885() {     double DVWqPGSCjP79849913 = -429634466;    double DVWqPGSCjP7120164 = -636118481;    double DVWqPGSCjP75344617 = -701750070;    double DVWqPGSCjP69521923 = -857761381;    double DVWqPGSCjP20524364 = -645821197;    double DVWqPGSCjP4679493 = -818242636;    double DVWqPGSCjP44965266 = -631526662;    double DVWqPGSCjP41929239 = -675754011;    double DVWqPGSCjP39063506 = 39439696;    double DVWqPGSCjP55550524 = -995724080;    double DVWqPGSCjP30884293 = -227369259;    double DVWqPGSCjP57813432 = -422683498;    double DVWqPGSCjP94204775 = -854560387;    double DVWqPGSCjP2552367 = -985207801;    double DVWqPGSCjP69010710 = -404033253;    double DVWqPGSCjP13644540 = -105151514;    double DVWqPGSCjP1087691 = -272646920;    double DVWqPGSCjP84151583 = -74678307;    double DVWqPGSCjP19678872 = 17483390;    double DVWqPGSCjP14365910 = -647242249;    double DVWqPGSCjP37033604 = -349228928;    double DVWqPGSCjP4724799 = -133667290;    double DVWqPGSCjP50869625 = -680916436;    double DVWqPGSCjP30365609 = -590677768;    double DVWqPGSCjP69907021 = -441262999;    double DVWqPGSCjP18367376 = -499990806;    double DVWqPGSCjP91381221 = -524024036;    double DVWqPGSCjP36775132 = -180368914;    double DVWqPGSCjP48533807 = 85793534;    double DVWqPGSCjP42730039 = 87188055;    double DVWqPGSCjP92616234 = -419497541;    double DVWqPGSCjP76872373 = -984813616;    double DVWqPGSCjP38027979 = -784409857;    double DVWqPGSCjP56895730 = -41231591;    double DVWqPGSCjP70233184 = -489393820;    double DVWqPGSCjP87233487 = -836121403;    double DVWqPGSCjP6584084 = -946817431;    double DVWqPGSCjP78716024 = -679500228;    double DVWqPGSCjP55898327 = -152150743;    double DVWqPGSCjP67134320 = -39572621;    double DVWqPGSCjP11860383 = 71046087;    double DVWqPGSCjP88828042 = -280885941;    double DVWqPGSCjP99838842 = -488132419;    double DVWqPGSCjP47102405 = -912959985;    double DVWqPGSCjP98608381 = -726441970;    double DVWqPGSCjP71636866 = -136504062;    double DVWqPGSCjP70389183 = -467783797;    double DVWqPGSCjP59432812 = -144679843;    double DVWqPGSCjP94708005 = 95187984;    double DVWqPGSCjP39258529 = -202651956;    double DVWqPGSCjP38895063 = -586980342;    double DVWqPGSCjP61750832 = -875562977;    double DVWqPGSCjP82021459 = -878924258;    double DVWqPGSCjP97000044 = -426908473;    double DVWqPGSCjP48594282 = -60209992;    double DVWqPGSCjP75125114 = -195967177;    double DVWqPGSCjP56250539 = -955202046;    double DVWqPGSCjP44979009 = -11072302;    double DVWqPGSCjP99614901 = -316498382;    double DVWqPGSCjP2156988 = -45830391;    double DVWqPGSCjP13298272 = -194218601;    double DVWqPGSCjP8190134 = -351157748;    double DVWqPGSCjP93395432 = -661547546;    double DVWqPGSCjP96333466 = 52251641;    double DVWqPGSCjP62934289 = -476226540;    double DVWqPGSCjP54011920 = -242555644;    double DVWqPGSCjP19785454 = -638273641;    double DVWqPGSCjP37309046 = -713328797;    double DVWqPGSCjP32319182 = -395813981;    double DVWqPGSCjP81777222 = -567911850;    double DVWqPGSCjP7060456 = -158334083;    double DVWqPGSCjP22371666 = -593146692;    double DVWqPGSCjP28253256 = -922527564;    double DVWqPGSCjP52544551 = -942943989;    double DVWqPGSCjP2505527 = -618288336;    double DVWqPGSCjP48205562 = 31657012;    double DVWqPGSCjP4885957 = -645534871;    double DVWqPGSCjP3767221 = -767956451;    double DVWqPGSCjP31757227 = -864235798;    double DVWqPGSCjP98270155 = -204758938;    double DVWqPGSCjP47978192 = 67792991;    double DVWqPGSCjP31948409 = -279344193;    double DVWqPGSCjP42067127 = -175556898;    double DVWqPGSCjP9275279 = -711554510;    double DVWqPGSCjP3834977 = -325831604;    double DVWqPGSCjP30865403 = -543934564;    double DVWqPGSCjP94850913 = -5889359;    double DVWqPGSCjP41027934 = -257501384;    double DVWqPGSCjP8301449 = -981021600;    double DVWqPGSCjP95108070 = -193426644;    double DVWqPGSCjP30982949 = -880919358;    double DVWqPGSCjP61605074 = -835745129;    double DVWqPGSCjP79101123 = -263001846;    double DVWqPGSCjP53741339 = -6320353;    double DVWqPGSCjP53836048 = -845354021;    double DVWqPGSCjP3670250 = -577796165;    double DVWqPGSCjP95432610 = -619338396;    double DVWqPGSCjP3505376 = -440384061;    double DVWqPGSCjP84168116 = -336733445;    double DVWqPGSCjP44596462 = -429634466;     DVWqPGSCjP79849913 = DVWqPGSCjP7120164;     DVWqPGSCjP7120164 = DVWqPGSCjP75344617;     DVWqPGSCjP75344617 = DVWqPGSCjP69521923;     DVWqPGSCjP69521923 = DVWqPGSCjP20524364;     DVWqPGSCjP20524364 = DVWqPGSCjP4679493;     DVWqPGSCjP4679493 = DVWqPGSCjP44965266;     DVWqPGSCjP44965266 = DVWqPGSCjP41929239;     DVWqPGSCjP41929239 = DVWqPGSCjP39063506;     DVWqPGSCjP39063506 = DVWqPGSCjP55550524;     DVWqPGSCjP55550524 = DVWqPGSCjP30884293;     DVWqPGSCjP30884293 = DVWqPGSCjP57813432;     DVWqPGSCjP57813432 = DVWqPGSCjP94204775;     DVWqPGSCjP94204775 = DVWqPGSCjP2552367;     DVWqPGSCjP2552367 = DVWqPGSCjP69010710;     DVWqPGSCjP69010710 = DVWqPGSCjP13644540;     DVWqPGSCjP13644540 = DVWqPGSCjP1087691;     DVWqPGSCjP1087691 = DVWqPGSCjP84151583;     DVWqPGSCjP84151583 = DVWqPGSCjP19678872;     DVWqPGSCjP19678872 = DVWqPGSCjP14365910;     DVWqPGSCjP14365910 = DVWqPGSCjP37033604;     DVWqPGSCjP37033604 = DVWqPGSCjP4724799;     DVWqPGSCjP4724799 = DVWqPGSCjP50869625;     DVWqPGSCjP50869625 = DVWqPGSCjP30365609;     DVWqPGSCjP30365609 = DVWqPGSCjP69907021;     DVWqPGSCjP69907021 = DVWqPGSCjP18367376;     DVWqPGSCjP18367376 = DVWqPGSCjP91381221;     DVWqPGSCjP91381221 = DVWqPGSCjP36775132;     DVWqPGSCjP36775132 = DVWqPGSCjP48533807;     DVWqPGSCjP48533807 = DVWqPGSCjP42730039;     DVWqPGSCjP42730039 = DVWqPGSCjP92616234;     DVWqPGSCjP92616234 = DVWqPGSCjP76872373;     DVWqPGSCjP76872373 = DVWqPGSCjP38027979;     DVWqPGSCjP38027979 = DVWqPGSCjP56895730;     DVWqPGSCjP56895730 = DVWqPGSCjP70233184;     DVWqPGSCjP70233184 = DVWqPGSCjP87233487;     DVWqPGSCjP87233487 = DVWqPGSCjP6584084;     DVWqPGSCjP6584084 = DVWqPGSCjP78716024;     DVWqPGSCjP78716024 = DVWqPGSCjP55898327;     DVWqPGSCjP55898327 = DVWqPGSCjP67134320;     DVWqPGSCjP67134320 = DVWqPGSCjP11860383;     DVWqPGSCjP11860383 = DVWqPGSCjP88828042;     DVWqPGSCjP88828042 = DVWqPGSCjP99838842;     DVWqPGSCjP99838842 = DVWqPGSCjP47102405;     DVWqPGSCjP47102405 = DVWqPGSCjP98608381;     DVWqPGSCjP98608381 = DVWqPGSCjP71636866;     DVWqPGSCjP71636866 = DVWqPGSCjP70389183;     DVWqPGSCjP70389183 = DVWqPGSCjP59432812;     DVWqPGSCjP59432812 = DVWqPGSCjP94708005;     DVWqPGSCjP94708005 = DVWqPGSCjP39258529;     DVWqPGSCjP39258529 = DVWqPGSCjP38895063;     DVWqPGSCjP38895063 = DVWqPGSCjP61750832;     DVWqPGSCjP61750832 = DVWqPGSCjP82021459;     DVWqPGSCjP82021459 = DVWqPGSCjP97000044;     DVWqPGSCjP97000044 = DVWqPGSCjP48594282;     DVWqPGSCjP48594282 = DVWqPGSCjP75125114;     DVWqPGSCjP75125114 = DVWqPGSCjP56250539;     DVWqPGSCjP56250539 = DVWqPGSCjP44979009;     DVWqPGSCjP44979009 = DVWqPGSCjP99614901;     DVWqPGSCjP99614901 = DVWqPGSCjP2156988;     DVWqPGSCjP2156988 = DVWqPGSCjP13298272;     DVWqPGSCjP13298272 = DVWqPGSCjP8190134;     DVWqPGSCjP8190134 = DVWqPGSCjP93395432;     DVWqPGSCjP93395432 = DVWqPGSCjP96333466;     DVWqPGSCjP96333466 = DVWqPGSCjP62934289;     DVWqPGSCjP62934289 = DVWqPGSCjP54011920;     DVWqPGSCjP54011920 = DVWqPGSCjP19785454;     DVWqPGSCjP19785454 = DVWqPGSCjP37309046;     DVWqPGSCjP37309046 = DVWqPGSCjP32319182;     DVWqPGSCjP32319182 = DVWqPGSCjP81777222;     DVWqPGSCjP81777222 = DVWqPGSCjP7060456;     DVWqPGSCjP7060456 = DVWqPGSCjP22371666;     DVWqPGSCjP22371666 = DVWqPGSCjP28253256;     DVWqPGSCjP28253256 = DVWqPGSCjP52544551;     DVWqPGSCjP52544551 = DVWqPGSCjP2505527;     DVWqPGSCjP2505527 = DVWqPGSCjP48205562;     DVWqPGSCjP48205562 = DVWqPGSCjP4885957;     DVWqPGSCjP4885957 = DVWqPGSCjP3767221;     DVWqPGSCjP3767221 = DVWqPGSCjP31757227;     DVWqPGSCjP31757227 = DVWqPGSCjP98270155;     DVWqPGSCjP98270155 = DVWqPGSCjP47978192;     DVWqPGSCjP47978192 = DVWqPGSCjP31948409;     DVWqPGSCjP31948409 = DVWqPGSCjP42067127;     DVWqPGSCjP42067127 = DVWqPGSCjP9275279;     DVWqPGSCjP9275279 = DVWqPGSCjP3834977;     DVWqPGSCjP3834977 = DVWqPGSCjP30865403;     DVWqPGSCjP30865403 = DVWqPGSCjP94850913;     DVWqPGSCjP94850913 = DVWqPGSCjP41027934;     DVWqPGSCjP41027934 = DVWqPGSCjP8301449;     DVWqPGSCjP8301449 = DVWqPGSCjP95108070;     DVWqPGSCjP95108070 = DVWqPGSCjP30982949;     DVWqPGSCjP30982949 = DVWqPGSCjP61605074;     DVWqPGSCjP61605074 = DVWqPGSCjP79101123;     DVWqPGSCjP79101123 = DVWqPGSCjP53741339;     DVWqPGSCjP53741339 = DVWqPGSCjP53836048;     DVWqPGSCjP53836048 = DVWqPGSCjP3670250;     DVWqPGSCjP3670250 = DVWqPGSCjP95432610;     DVWqPGSCjP95432610 = DVWqPGSCjP3505376;     DVWqPGSCjP3505376 = DVWqPGSCjP84168116;     DVWqPGSCjP84168116 = DVWqPGSCjP44596462;     DVWqPGSCjP44596462 = DVWqPGSCjP79849913;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LOLROMXMNF31323579() {     double LSoyvlBmvy99038870 = -273409292;    double LSoyvlBmvy79418021 = -277726814;    double LSoyvlBmvy4000 = -278596222;    double LSoyvlBmvy53813370 = -863253551;    double LSoyvlBmvy47530867 = -681012912;    double LSoyvlBmvy17436451 = -384478388;    double LSoyvlBmvy23955900 = -793230067;    double LSoyvlBmvy78186832 = -136265420;    double LSoyvlBmvy8060476 = -332728337;    double LSoyvlBmvy19919311 = -146645223;    double LSoyvlBmvy91634458 = 89809826;    double LSoyvlBmvy34301675 = -480465178;    double LSoyvlBmvy51640359 = -127746661;    double LSoyvlBmvy96222154 = -966101021;    double LSoyvlBmvy86212375 = -870442721;    double LSoyvlBmvy94435321 = -432295121;    double LSoyvlBmvy936278 = -38886765;    double LSoyvlBmvy74988511 = -461886328;    double LSoyvlBmvy40777412 = -569747003;    double LSoyvlBmvy10237010 = -78071780;    double LSoyvlBmvy37790553 = -970503764;    double LSoyvlBmvy30852129 = -723466987;    double LSoyvlBmvy53692474 = -988985514;    double LSoyvlBmvy81651140 = -759814503;    double LSoyvlBmvy86084376 = -116885106;    double LSoyvlBmvy65484228 = -723328712;    double LSoyvlBmvy3172154 = -473383887;    double LSoyvlBmvy84699232 = -627615588;    double LSoyvlBmvy54441716 = -366426853;    double LSoyvlBmvy7721908 = -944494117;    double LSoyvlBmvy59151211 = -803482814;    double LSoyvlBmvy54764822 = -915160492;    double LSoyvlBmvy78918268 = -295275710;    double LSoyvlBmvy22086034 = -633340320;    double LSoyvlBmvy21286926 = 84529246;    double LSoyvlBmvy65827375 = -949317440;    double LSoyvlBmvy43818123 = 67798611;    double LSoyvlBmvy10395540 = -566036346;    double LSoyvlBmvy11591011 = -495904485;    double LSoyvlBmvy98722075 = -942050361;    double LSoyvlBmvy22223844 = -360249055;    double LSoyvlBmvy21835236 = -387664819;    double LSoyvlBmvy61739007 = -234587205;    double LSoyvlBmvy75197364 = -439271684;    double LSoyvlBmvy42594039 = -405707808;    double LSoyvlBmvy21437108 = 21585285;    double LSoyvlBmvy95215524 = -492135442;    double LSoyvlBmvy40517391 = -564566978;    double LSoyvlBmvy32816496 = -804569186;    double LSoyvlBmvy79616025 = -638669321;    double LSoyvlBmvy46497248 = 51525706;    double LSoyvlBmvy82194767 = -459122362;    double LSoyvlBmvy20576663 = -754848170;    double LSoyvlBmvy48674226 = -663206309;    double LSoyvlBmvy93533025 = -966527805;    double LSoyvlBmvy68186742 = -549942305;    double LSoyvlBmvy25725548 = -288741300;    double LSoyvlBmvy18352860 = -518781719;    double LSoyvlBmvy67728994 = -646368446;    double LSoyvlBmvy82046638 = -957684201;    double LSoyvlBmvy14264298 = -911094502;    double LSoyvlBmvy39256667 = -65614479;    double LSoyvlBmvy23745116 = -769838567;    double LSoyvlBmvy338569 = -388234221;    double LSoyvlBmvy60768100 = -343162409;    double LSoyvlBmvy36869636 = 4970317;    double LSoyvlBmvy55383407 = -85189469;    double LSoyvlBmvy29554326 = -494406342;    double LSoyvlBmvy74935228 = -950630268;    double LSoyvlBmvy20385001 = -921125282;    double LSoyvlBmvy50617199 = -400093733;    double LSoyvlBmvy90540737 = -472850419;    double LSoyvlBmvy63397500 = -965981843;    double LSoyvlBmvy42055337 = -627696643;    double LSoyvlBmvy88013166 = -717822726;    double LSoyvlBmvy15955318 = -482838945;    double LSoyvlBmvy69113121 = -388879782;    double LSoyvlBmvy78495109 = -449713831;    double LSoyvlBmvy39057101 = -254106695;    double LSoyvlBmvy64647269 = -38470391;    double LSoyvlBmvy70268704 = -131193270;    double LSoyvlBmvy62654763 = -908816909;    double LSoyvlBmvy51882737 = -823046403;    double LSoyvlBmvy74825691 = -727757533;    double LSoyvlBmvy61224659 = -896019823;    double LSoyvlBmvy76956443 = -244360453;    double LSoyvlBmvy34188160 = -60312323;    double LSoyvlBmvy30244042 = -632069401;    double LSoyvlBmvy28553009 = -666812515;    double LSoyvlBmvy53100183 = -365528449;    double LSoyvlBmvy40101828 = -560576141;    double LSoyvlBmvy25465264 = -413419670;    double LSoyvlBmvy42666546 = -919667901;    double LSoyvlBmvy29544372 = -538220285;    double LSoyvlBmvy84457778 = 69044141;    double LSoyvlBmvy82967176 = -194634576;    double LSoyvlBmvy98090119 = -617826252;    double LSoyvlBmvy61400439 = -846352985;    double LSoyvlBmvy14429265 = 3890725;    double LSoyvlBmvy5724403 = -273409292;     LSoyvlBmvy99038870 = LSoyvlBmvy79418021;     LSoyvlBmvy79418021 = LSoyvlBmvy4000;     LSoyvlBmvy4000 = LSoyvlBmvy53813370;     LSoyvlBmvy53813370 = LSoyvlBmvy47530867;     LSoyvlBmvy47530867 = LSoyvlBmvy17436451;     LSoyvlBmvy17436451 = LSoyvlBmvy23955900;     LSoyvlBmvy23955900 = LSoyvlBmvy78186832;     LSoyvlBmvy78186832 = LSoyvlBmvy8060476;     LSoyvlBmvy8060476 = LSoyvlBmvy19919311;     LSoyvlBmvy19919311 = LSoyvlBmvy91634458;     LSoyvlBmvy91634458 = LSoyvlBmvy34301675;     LSoyvlBmvy34301675 = LSoyvlBmvy51640359;     LSoyvlBmvy51640359 = LSoyvlBmvy96222154;     LSoyvlBmvy96222154 = LSoyvlBmvy86212375;     LSoyvlBmvy86212375 = LSoyvlBmvy94435321;     LSoyvlBmvy94435321 = LSoyvlBmvy936278;     LSoyvlBmvy936278 = LSoyvlBmvy74988511;     LSoyvlBmvy74988511 = LSoyvlBmvy40777412;     LSoyvlBmvy40777412 = LSoyvlBmvy10237010;     LSoyvlBmvy10237010 = LSoyvlBmvy37790553;     LSoyvlBmvy37790553 = LSoyvlBmvy30852129;     LSoyvlBmvy30852129 = LSoyvlBmvy53692474;     LSoyvlBmvy53692474 = LSoyvlBmvy81651140;     LSoyvlBmvy81651140 = LSoyvlBmvy86084376;     LSoyvlBmvy86084376 = LSoyvlBmvy65484228;     LSoyvlBmvy65484228 = LSoyvlBmvy3172154;     LSoyvlBmvy3172154 = LSoyvlBmvy84699232;     LSoyvlBmvy84699232 = LSoyvlBmvy54441716;     LSoyvlBmvy54441716 = LSoyvlBmvy7721908;     LSoyvlBmvy7721908 = LSoyvlBmvy59151211;     LSoyvlBmvy59151211 = LSoyvlBmvy54764822;     LSoyvlBmvy54764822 = LSoyvlBmvy78918268;     LSoyvlBmvy78918268 = LSoyvlBmvy22086034;     LSoyvlBmvy22086034 = LSoyvlBmvy21286926;     LSoyvlBmvy21286926 = LSoyvlBmvy65827375;     LSoyvlBmvy65827375 = LSoyvlBmvy43818123;     LSoyvlBmvy43818123 = LSoyvlBmvy10395540;     LSoyvlBmvy10395540 = LSoyvlBmvy11591011;     LSoyvlBmvy11591011 = LSoyvlBmvy98722075;     LSoyvlBmvy98722075 = LSoyvlBmvy22223844;     LSoyvlBmvy22223844 = LSoyvlBmvy21835236;     LSoyvlBmvy21835236 = LSoyvlBmvy61739007;     LSoyvlBmvy61739007 = LSoyvlBmvy75197364;     LSoyvlBmvy75197364 = LSoyvlBmvy42594039;     LSoyvlBmvy42594039 = LSoyvlBmvy21437108;     LSoyvlBmvy21437108 = LSoyvlBmvy95215524;     LSoyvlBmvy95215524 = LSoyvlBmvy40517391;     LSoyvlBmvy40517391 = LSoyvlBmvy32816496;     LSoyvlBmvy32816496 = LSoyvlBmvy79616025;     LSoyvlBmvy79616025 = LSoyvlBmvy46497248;     LSoyvlBmvy46497248 = LSoyvlBmvy82194767;     LSoyvlBmvy82194767 = LSoyvlBmvy20576663;     LSoyvlBmvy20576663 = LSoyvlBmvy48674226;     LSoyvlBmvy48674226 = LSoyvlBmvy93533025;     LSoyvlBmvy93533025 = LSoyvlBmvy68186742;     LSoyvlBmvy68186742 = LSoyvlBmvy25725548;     LSoyvlBmvy25725548 = LSoyvlBmvy18352860;     LSoyvlBmvy18352860 = LSoyvlBmvy67728994;     LSoyvlBmvy67728994 = LSoyvlBmvy82046638;     LSoyvlBmvy82046638 = LSoyvlBmvy14264298;     LSoyvlBmvy14264298 = LSoyvlBmvy39256667;     LSoyvlBmvy39256667 = LSoyvlBmvy23745116;     LSoyvlBmvy23745116 = LSoyvlBmvy338569;     LSoyvlBmvy338569 = LSoyvlBmvy60768100;     LSoyvlBmvy60768100 = LSoyvlBmvy36869636;     LSoyvlBmvy36869636 = LSoyvlBmvy55383407;     LSoyvlBmvy55383407 = LSoyvlBmvy29554326;     LSoyvlBmvy29554326 = LSoyvlBmvy74935228;     LSoyvlBmvy74935228 = LSoyvlBmvy20385001;     LSoyvlBmvy20385001 = LSoyvlBmvy50617199;     LSoyvlBmvy50617199 = LSoyvlBmvy90540737;     LSoyvlBmvy90540737 = LSoyvlBmvy63397500;     LSoyvlBmvy63397500 = LSoyvlBmvy42055337;     LSoyvlBmvy42055337 = LSoyvlBmvy88013166;     LSoyvlBmvy88013166 = LSoyvlBmvy15955318;     LSoyvlBmvy15955318 = LSoyvlBmvy69113121;     LSoyvlBmvy69113121 = LSoyvlBmvy78495109;     LSoyvlBmvy78495109 = LSoyvlBmvy39057101;     LSoyvlBmvy39057101 = LSoyvlBmvy64647269;     LSoyvlBmvy64647269 = LSoyvlBmvy70268704;     LSoyvlBmvy70268704 = LSoyvlBmvy62654763;     LSoyvlBmvy62654763 = LSoyvlBmvy51882737;     LSoyvlBmvy51882737 = LSoyvlBmvy74825691;     LSoyvlBmvy74825691 = LSoyvlBmvy61224659;     LSoyvlBmvy61224659 = LSoyvlBmvy76956443;     LSoyvlBmvy76956443 = LSoyvlBmvy34188160;     LSoyvlBmvy34188160 = LSoyvlBmvy30244042;     LSoyvlBmvy30244042 = LSoyvlBmvy28553009;     LSoyvlBmvy28553009 = LSoyvlBmvy53100183;     LSoyvlBmvy53100183 = LSoyvlBmvy40101828;     LSoyvlBmvy40101828 = LSoyvlBmvy25465264;     LSoyvlBmvy25465264 = LSoyvlBmvy42666546;     LSoyvlBmvy42666546 = LSoyvlBmvy29544372;     LSoyvlBmvy29544372 = LSoyvlBmvy84457778;     LSoyvlBmvy84457778 = LSoyvlBmvy82967176;     LSoyvlBmvy82967176 = LSoyvlBmvy98090119;     LSoyvlBmvy98090119 = LSoyvlBmvy61400439;     LSoyvlBmvy61400439 = LSoyvlBmvy14429265;     LSoyvlBmvy14429265 = LSoyvlBmvy5724403;     LSoyvlBmvy5724403 = LSoyvlBmvy99038870;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hhGGLJCrbo37982323() {     double TbHzjfHdpm99032399 = -145933433;    double TbHzjfHdpm17990338 = -320321939;    double TbHzjfHdpm1317212 = -235724835;    double TbHzjfHdpm38893231 = -683794381;    double TbHzjfHdpm56370969 = -49368233;    double TbHzjfHdpm52241883 = -83911200;    double TbHzjfHdpm92503235 = -497766534;    double TbHzjfHdpm47400883 = -940442737;    double TbHzjfHdpm90800639 = -46374407;    double TbHzjfHdpm16558825 = -883452094;    double TbHzjfHdpm11955064 = -969662246;    double TbHzjfHdpm48593352 = -215943541;    double TbHzjfHdpm83493273 = -149405136;    double TbHzjfHdpm50066375 = -211254073;    double TbHzjfHdpm31384485 = -833155573;    double TbHzjfHdpm68999868 = -765090772;    double TbHzjfHdpm56789037 = -958501335;    double TbHzjfHdpm97989044 = -858957949;    double TbHzjfHdpm61205682 = -87729269;    double TbHzjfHdpm82310772 = -690096232;    double TbHzjfHdpm17459978 = -680970138;    double TbHzjfHdpm9148395 = -133458870;    double TbHzjfHdpm90570870 = -347943357;    double TbHzjfHdpm25301249 = -640735025;    double TbHzjfHdpm14527066 = -543578959;    double TbHzjfHdpm82058305 = -631534640;    double TbHzjfHdpm73393735 = -774183599;    double TbHzjfHdpm20102758 = -136582577;    double TbHzjfHdpm67120163 = 87502832;    double TbHzjfHdpm83811777 = -619925490;    double TbHzjfHdpm92563889 = -90576049;    double TbHzjfHdpm39488680 = -206982253;    double TbHzjfHdpm64370691 = -497219926;    double TbHzjfHdpm22368871 = -510022052;    double TbHzjfHdpm3644964 = -174784679;    double TbHzjfHdpm8655566 = 70916814;    double TbHzjfHdpm55759490 = -31854767;    double TbHzjfHdpm80313219 = -561299026;    double TbHzjfHdpm46904608 = -108585901;    double TbHzjfHdpm70594968 = -551482047;    double TbHzjfHdpm10079512 = -448192954;    double TbHzjfHdpm7532897 = -581456902;    double TbHzjfHdpm22022367 = -793717460;    double TbHzjfHdpm34429263 = 3763873;    double TbHzjfHdpm26612092 = -878967210;    double TbHzjfHdpm61865076 = -890459386;    double TbHzjfHdpm22913317 = -973416065;    double TbHzjfHdpm15900972 = -73328334;    double TbHzjfHdpm51474885 = -368333047;    double TbHzjfHdpm99661447 = -225694207;    double TbHzjfHdpm59756618 = -627029301;    double TbHzjfHdpm96742024 = -841268110;    double TbHzjfHdpm96025010 = -646902178;    double TbHzjfHdpm58924497 = -95126161;    double TbHzjfHdpm20522346 = -723427083;    double TbHzjfHdpm89884005 = 87525436;    double TbHzjfHdpm27419468 = -972378583;    double TbHzjfHdpm76015963 = -594989811;    double TbHzjfHdpm24366165 = -40215423;    double TbHzjfHdpm74312664 = -417833594;    double TbHzjfHdpm78848148 = -309727601;    double TbHzjfHdpm72400478 = -261183958;    double TbHzjfHdpm80280719 = -927945570;    double TbHzjfHdpm6988863 = -426448917;    double TbHzjfHdpm23994935 = -692876046;    double TbHzjfHdpm72466384 = -662679994;    double TbHzjfHdpm84222660 = -718723615;    double TbHzjfHdpm61124402 = -639383084;    double TbHzjfHdpm46421411 = 63530606;    double TbHzjfHdpm22728920 = -804072387;    double TbHzjfHdpm13240379 = -633236005;    double TbHzjfHdpm76475817 = -297202310;    double TbHzjfHdpm51084437 = -650372049;    double TbHzjfHdpm90610714 = -536247223;    double TbHzjfHdpm72231261 = -141903278;    double TbHzjfHdpm9927082 = 486763;    double TbHzjfHdpm87126027 = -339741411;    double TbHzjfHdpm56141608 = -251707230;    double TbHzjfHdpm98689157 = -761767816;    double TbHzjfHdpm52661990 = -653119574;    double TbHzjfHdpm59144989 = -658118576;    double TbHzjfHdpm57492763 = -600855266;    double TbHzjfHdpm68627872 = -768249530;    double TbHzjfHdpm67458715 = -686802962;    double TbHzjfHdpm24055160 = -992896190;    double TbHzjfHdpm95821864 = -249307939;    double TbHzjfHdpm43463669 = -560080076;    double TbHzjfHdpm5446194 = -302093766;    double TbHzjfHdpm1846526 = -786594970;    double TbHzjfHdpm13760959 = -162310115;    double TbHzjfHdpm81236097 = 43295396;    double TbHzjfHdpm79743527 = -436864957;    double TbHzjfHdpm55947054 = -421083603;    double TbHzjfHdpm72591943 = -690752308;    double TbHzjfHdpm91746820 = -141754446;    double TbHzjfHdpm37679033 = -87008996;    double TbHzjfHdpm27252177 = -653511332;    double TbHzjfHdpm15033505 = -267268543;    double TbHzjfHdpm10434328 = -303360082;    double TbHzjfHdpm54145707 = -145933433;     TbHzjfHdpm99032399 = TbHzjfHdpm17990338;     TbHzjfHdpm17990338 = TbHzjfHdpm1317212;     TbHzjfHdpm1317212 = TbHzjfHdpm38893231;     TbHzjfHdpm38893231 = TbHzjfHdpm56370969;     TbHzjfHdpm56370969 = TbHzjfHdpm52241883;     TbHzjfHdpm52241883 = TbHzjfHdpm92503235;     TbHzjfHdpm92503235 = TbHzjfHdpm47400883;     TbHzjfHdpm47400883 = TbHzjfHdpm90800639;     TbHzjfHdpm90800639 = TbHzjfHdpm16558825;     TbHzjfHdpm16558825 = TbHzjfHdpm11955064;     TbHzjfHdpm11955064 = TbHzjfHdpm48593352;     TbHzjfHdpm48593352 = TbHzjfHdpm83493273;     TbHzjfHdpm83493273 = TbHzjfHdpm50066375;     TbHzjfHdpm50066375 = TbHzjfHdpm31384485;     TbHzjfHdpm31384485 = TbHzjfHdpm68999868;     TbHzjfHdpm68999868 = TbHzjfHdpm56789037;     TbHzjfHdpm56789037 = TbHzjfHdpm97989044;     TbHzjfHdpm97989044 = TbHzjfHdpm61205682;     TbHzjfHdpm61205682 = TbHzjfHdpm82310772;     TbHzjfHdpm82310772 = TbHzjfHdpm17459978;     TbHzjfHdpm17459978 = TbHzjfHdpm9148395;     TbHzjfHdpm9148395 = TbHzjfHdpm90570870;     TbHzjfHdpm90570870 = TbHzjfHdpm25301249;     TbHzjfHdpm25301249 = TbHzjfHdpm14527066;     TbHzjfHdpm14527066 = TbHzjfHdpm82058305;     TbHzjfHdpm82058305 = TbHzjfHdpm73393735;     TbHzjfHdpm73393735 = TbHzjfHdpm20102758;     TbHzjfHdpm20102758 = TbHzjfHdpm67120163;     TbHzjfHdpm67120163 = TbHzjfHdpm83811777;     TbHzjfHdpm83811777 = TbHzjfHdpm92563889;     TbHzjfHdpm92563889 = TbHzjfHdpm39488680;     TbHzjfHdpm39488680 = TbHzjfHdpm64370691;     TbHzjfHdpm64370691 = TbHzjfHdpm22368871;     TbHzjfHdpm22368871 = TbHzjfHdpm3644964;     TbHzjfHdpm3644964 = TbHzjfHdpm8655566;     TbHzjfHdpm8655566 = TbHzjfHdpm55759490;     TbHzjfHdpm55759490 = TbHzjfHdpm80313219;     TbHzjfHdpm80313219 = TbHzjfHdpm46904608;     TbHzjfHdpm46904608 = TbHzjfHdpm70594968;     TbHzjfHdpm70594968 = TbHzjfHdpm10079512;     TbHzjfHdpm10079512 = TbHzjfHdpm7532897;     TbHzjfHdpm7532897 = TbHzjfHdpm22022367;     TbHzjfHdpm22022367 = TbHzjfHdpm34429263;     TbHzjfHdpm34429263 = TbHzjfHdpm26612092;     TbHzjfHdpm26612092 = TbHzjfHdpm61865076;     TbHzjfHdpm61865076 = TbHzjfHdpm22913317;     TbHzjfHdpm22913317 = TbHzjfHdpm15900972;     TbHzjfHdpm15900972 = TbHzjfHdpm51474885;     TbHzjfHdpm51474885 = TbHzjfHdpm99661447;     TbHzjfHdpm99661447 = TbHzjfHdpm59756618;     TbHzjfHdpm59756618 = TbHzjfHdpm96742024;     TbHzjfHdpm96742024 = TbHzjfHdpm96025010;     TbHzjfHdpm96025010 = TbHzjfHdpm58924497;     TbHzjfHdpm58924497 = TbHzjfHdpm20522346;     TbHzjfHdpm20522346 = TbHzjfHdpm89884005;     TbHzjfHdpm89884005 = TbHzjfHdpm27419468;     TbHzjfHdpm27419468 = TbHzjfHdpm76015963;     TbHzjfHdpm76015963 = TbHzjfHdpm24366165;     TbHzjfHdpm24366165 = TbHzjfHdpm74312664;     TbHzjfHdpm74312664 = TbHzjfHdpm78848148;     TbHzjfHdpm78848148 = TbHzjfHdpm72400478;     TbHzjfHdpm72400478 = TbHzjfHdpm80280719;     TbHzjfHdpm80280719 = TbHzjfHdpm6988863;     TbHzjfHdpm6988863 = TbHzjfHdpm23994935;     TbHzjfHdpm23994935 = TbHzjfHdpm72466384;     TbHzjfHdpm72466384 = TbHzjfHdpm84222660;     TbHzjfHdpm84222660 = TbHzjfHdpm61124402;     TbHzjfHdpm61124402 = TbHzjfHdpm46421411;     TbHzjfHdpm46421411 = TbHzjfHdpm22728920;     TbHzjfHdpm22728920 = TbHzjfHdpm13240379;     TbHzjfHdpm13240379 = TbHzjfHdpm76475817;     TbHzjfHdpm76475817 = TbHzjfHdpm51084437;     TbHzjfHdpm51084437 = TbHzjfHdpm90610714;     TbHzjfHdpm90610714 = TbHzjfHdpm72231261;     TbHzjfHdpm72231261 = TbHzjfHdpm9927082;     TbHzjfHdpm9927082 = TbHzjfHdpm87126027;     TbHzjfHdpm87126027 = TbHzjfHdpm56141608;     TbHzjfHdpm56141608 = TbHzjfHdpm98689157;     TbHzjfHdpm98689157 = TbHzjfHdpm52661990;     TbHzjfHdpm52661990 = TbHzjfHdpm59144989;     TbHzjfHdpm59144989 = TbHzjfHdpm57492763;     TbHzjfHdpm57492763 = TbHzjfHdpm68627872;     TbHzjfHdpm68627872 = TbHzjfHdpm67458715;     TbHzjfHdpm67458715 = TbHzjfHdpm24055160;     TbHzjfHdpm24055160 = TbHzjfHdpm95821864;     TbHzjfHdpm95821864 = TbHzjfHdpm43463669;     TbHzjfHdpm43463669 = TbHzjfHdpm5446194;     TbHzjfHdpm5446194 = TbHzjfHdpm1846526;     TbHzjfHdpm1846526 = TbHzjfHdpm13760959;     TbHzjfHdpm13760959 = TbHzjfHdpm81236097;     TbHzjfHdpm81236097 = TbHzjfHdpm79743527;     TbHzjfHdpm79743527 = TbHzjfHdpm55947054;     TbHzjfHdpm55947054 = TbHzjfHdpm72591943;     TbHzjfHdpm72591943 = TbHzjfHdpm91746820;     TbHzjfHdpm91746820 = TbHzjfHdpm37679033;     TbHzjfHdpm37679033 = TbHzjfHdpm27252177;     TbHzjfHdpm27252177 = TbHzjfHdpm15033505;     TbHzjfHdpm15033505 = TbHzjfHdpm10434328;     TbHzjfHdpm10434328 = TbHzjfHdpm54145707;     TbHzjfHdpm54145707 = TbHzjfHdpm99032399;}
// Junk Finished
