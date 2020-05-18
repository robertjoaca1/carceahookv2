#include "hooks.hpp"
#include <intrin.h>  
#include "helpers/input.hpp"
#include "options.hpp"
#include "helpers/utils.hpp"
#include "features/visuals.hpp"
#include "features/legitbot.hpp"
#include "features/ragebot.hpp"
#include "features/menu.hpp"
#include "features/misc.hpp"
#include "features/engineprediction.hpp"
#include "features/skinchanger.hpp"
#include "eventlistener.hpp"

#pragma intrinsic(_ReturnAddress) 
QAngle Hooks::RealAngle;
QAngle Hooks::FakeAngle;
bool Hooks::SendPacket;
int Hooks::TickCount;
matrix3x4_t Hooks::FakeLagMatrix[128];
matrix3x4_t Hooks::FakeAngleMatrix[128];
Vector Hooks::FakeOrigin;
int Hooks::ShotsFired[65];
int Hooks::ShotsHit[65]; 
CUserCmd* Hooks::cmd;
int Hooks::TickBaseShift;
bool bInSendMove = false, bFirstSendMovePack = false;
C_BasePlayer* UTIL_PlayerByIndex(int index)
{
	typedef C_BasePlayer* (__fastcall * PlayerByIndex)(int);
	static PlayerByIndex UTIL_PlayerByIndex = reinterpret_cast<PlayerByIndex>(Utils::PatternScan(GetModuleHandleW(L"server.dll"), "85 C9 7E 2A A1"));

	if (!UTIL_PlayerByIndex)
		return false;

	return UTIL_PlayerByIndex(index);
}
void playerModel(ClientFrameStage_t stage)
{
	if (stage != FRAME_RENDER_START && stage != FRAME_RENDER_END)
		return;

	static int originalIdx = 0;

	if (!g_LocalPlayer || !Variables.PlayerModel) {
		originalIdx = 0;
		return;
	}

	constexpr auto getModel = [](int team) constexpr noexcept -> const char* {
		constexpr std::array models{
		"models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
		"models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
		"models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
		"models/player/custom_player/legacy/ctm_fbi_varianth.mdl",
		"models/player/custom_player/legacy/ctm_sas_variantf.mdl",
		"models/player/custom_player/legacy/ctm_st6_variante.mdl",
		"models/player/custom_player/legacy/ctm_st6_variantg.mdl",
		"models/player/custom_player/legacy/ctm_st6_varianti.mdl",
		"models/player/custom_player/legacy/ctm_st6_variantk.mdl",
		"models/player/custom_player/legacy/ctm_st6_variantm.mdl",
		"models/player/custom_player/legacy/tm_balkan_variantf.mdl",
		"models/player/custom_player/legacy/tm_balkan_variantg.mdl",
		"models/player/custom_player/legacy/tm_balkan_varianth.mdl",
		"models/player/custom_player/legacy/tm_balkan_varianti.mdl",
		"models/player/custom_player/legacy/tm_balkan_variantj.mdl",
		"models/player/custom_player/legacy/tm_leet_variantf.mdl",
		"models/player/custom_player/legacy/tm_leet_variantg.mdl",
		"models/player/custom_player/legacy/tm_leet_varianth.mdl",
		"models/player/custom_player/legacy/tm_leet_varianti.mdl",
		"models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
		"models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
		"models/player/custom_player/legacy/tm_phoenix_varianth.mdl"
		};


		return static_cast<std::size_t>(Variables.PlayerModel - 1) < models.size() ? models[Variables.PlayerModel - 1] : nullptr;

	};

	if (const auto model = getModel(2)) {
		if (stage == FRAME_RENDER_START)
			originalIdx = g_LocalPlayer->m_nModelIndex();

		const auto idx = stage == FRAME_RENDER_END && originalIdx ? originalIdx : g_MdlInfo->GetModelIndex(model);

		g_LocalPlayer->m_nModelIndex() = idx;
	}
}
namespace Hooks {

	void Initialize()
	{
		bsp_query_hook.setup(g_EngineClient->GetBSPTreeQuery());
		direct3d_hook.setup(g_D3DDevice9);
		hlclient_hook.setup(g_CHLClient);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		mdlrender_hook.setup(g_MdlRender);
		clientmode_hook.setup(g_ClientMode);
		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);
		studio_render_hook.setup(g_StudioRender);

		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		hlclient_hook.hook_index(index::WriteUserCmdDeltaToBuffer, hkWriteUserCmdDeltaToBuffer);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		vguisurf_hook.hook_index(index::LockCursor, hkLockCursor);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		clientmode_hook.hook_index(index::CreateMove, hkCreateMove);
		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);
		bsp_query_hook.hook_index(index::ListLeavesInBox, hkListLeavesInBox);
		studio_render_hook.hook_index(index::BeginFrame, hkBeginFrame);

		EventListener::Get().Initialize();

		Render::Get().CreateFonts();

		g_InputSystem->EnableInput(true);
	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		g_InputSystem->EnableInput(true);

		direct3d_hook.unhook_all();
		hlclient_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sv_cheats.unhook_all(); 
		studio_render_hook.unhook_all();

		Glow::Get().Shutdown();
		EventListener::Get().Shutdown();
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>(index::EndScene);

		DWORD colorwrite, srgbwrite;
		IDirect3DVertexDeclaration9* vert_dec = nullptr;
		IDirect3DVertexShader9* vert_shader = nullptr;
		DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		//removes the source engine color correction
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->GetVertexDeclaration(&vert_dec);
		pDevice->GetVertexShader(&vert_shader);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		/*ImGui_ImplDX9_NewFrame();

		//Menu::Get().RenderMenu();
		//Menu::Get().RenderSpectatorList();
		//Menu::Get().RenderLocalplayerFlags();
		//Menu::Get().RenderBombTimer();

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());*/

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		pDevice->SetVertexDeclaration(vert_dec);
		pDevice->SetVertexShader(vert_shader);

		return oEndScene(pDevice);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto oReset = direct3d_hook.get_original<decltype(&hkReset)>(index::Reset);

		//Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0)
		{
			//Menu::Get().OnDeviceReset();
			Render::Get().CreateFonts();
		}

		return hr;
	}
	//--------------------------------------------------------------------------------
	bool __fastcall hkCreateMove(IClientMode* thisptr, void* edx, float sample_frametime, CUserCmd* cmd)
	{
		// Call original createmove before we start screwing with it
		static auto oCreateMove = clientmode_hook.get_original<decltype(&hkCreateMove)>(index::CreateMove);
		oCreateMove(thisptr, edx, sample_frametime, cmd);

		if (!cmd || !cmd->command_number)
			return oCreateMove(thisptr, edx, sample_frametime, cmd);

		Hooks::cmd = cmd;
		Hooks::SendPacket = true;

		if (!g_LocalPlayer)
			return oCreateMove(thisptr, edx, sample_frametime, cmd);

		C_BaseCombatWeapon* Weapon = g_LocalPlayer->m_hActiveWeapon();

		//if (cmd->buttons & IN_ATTACK)
			//bSendPacket = true;
		if (Variables.MiscBunnyhop)
			Misc::Get().DoBhop(cmd);

		if (Variables.MiscAutostrafe)
			Misc::Get().DoAutostrafe(cmd);

		PredictionSystem::Get().Start(cmd, g_LocalPlayer);
		RageAimbot::Get().DoFakelag(Hooks::SendPacket);
		if (Variables.RageAimbotEnabled)
		{
			if (Variables.LegitAimbotEnabled)
				Variables.LegitAimbotEnabled = false;
			if (Variables.LegitBacktrackEnabled)
				Variables.LegitBacktrackEnabled = false;
			if (Variables.LegitAntiaimEnabled)
				Variables.LegitAntiaimEnabled = false;
		}
		if (Variables.RageAntiaimEnabled)
		{
			MovementFix::Get().Start(cmd);
			RageAimbot::Get().DoAntiaim(cmd, Weapon, Hooks::SendPacket);
			MovementFix::Get().End(cmd);
		}
		if (Variables.RageAimbotEnabled)
		{
			if (Variables.RageAimbotResolver)
				RageAimbot::Get().AntiFreestanding();
			RageAimbot::Get().Do(cmd, Weapon, Hooks::SendPacket);
		}

		if (Variables.LegitBacktrackEnabled)
			LegitBacktrack::Get().Do(cmd);

		if (Variables.LegitAimbotEnabled)
			LegitAimbot::Get().Do(cmd, Weapon);

		if (Variables.LegitAntiaimEnabled)
		{
			MovementFix::Get().Start(cmd);
			LegitAimbot::Get().DoLegitAntiaim(cmd, Weapon, Hooks::SendPacket);
			MovementFix::Get().End(cmd);
		}

		PredictionSystem::Get().End(g_LocalPlayer);

		//RageAimbot::Get().EndEnginePred(cmd);

		Misc::Get().DoClantag();

		Misc::Get().DoFakeDuck(cmd);

		RageAimbot::Get().UpdateFakeAnimations();

		Math::SanitizeAngle(cmd->viewangles);

		if (Hooks::SendPacket)
		{
			Hooks::FakeOrigin = g_LocalPlayer->m_vecOrigin();
			g_LocalPlayer->SetupBones(Hooks::FakeLagMatrix, 128, BONE_USED_BY_ANYTHING, g_GlobalVars->curtime);
		}

		if (Hooks::SendPacket)
		{
			Hooks::RealAngle = cmd->viewangles;
		}
		else
		{
			Hooks::FakeAngle = cmd->viewangles;
		}


		uintptr_t* framePtr;
		__asm mov framePtr, ebp;
		*(bool*)(*framePtr - 0x1C) = Hooks::SendPacket;
		return false;
	}
	/*void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		static auto oCreateMove = hlclient_hook.get_original<decltype(&hkCreateMove_Proxy)>(index::CreateMove);

		oCreateMove(g_CHLClient, 0, sequence_number, input_sample_frametime, active);

		auto cmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);

		if (!cmd || !cmd->command_number)
			return;

		C_BaseCombatWeapon* Weapon = g_LocalPlayer->m_hActiveWeapon();

		//if (cmd->buttons & IN_ATTACK)
			//bSendPacket = true;
		if (Variables.MiscBunnyhop)
			Misc::Get().DoBhop(cmd);

		if (Variables.MiscAutostrafe)
			Misc::Get().DoAutostrafe(cmd);

		PredictionSystem::Get().Start(cmd, g_LocalPlayer);
		RageAimbot::Get().DoFakelag(bSendPacket);
		if (Variables.RageAimbotEnabled)
		{
			if (Variables.LegitAimbotEnabled)
				Variables.LegitAimbotEnabled = false;
			if (Variables.LegitBacktrackEnabled)
				Variables.LegitBacktrackEnabled = false;
			if (Variables.LegitAntiaimEnabled)
				Variables.LegitAntiaimEnabled = false;
		}
		if (Variables.RageAntiaimEnabled)
		{
			MovementFix::Get().Start(cmd);
			RageAimbot::Get().DoAntiaim(cmd, Weapon, bSendPacket);
			MovementFix::Get().End(cmd);
		}
		if (Variables.RageAimbotEnabled)
		{
			if (Variables.RageAimbotResolver)
				RageAimbot::Get().AntiFreestanding();
			RageAimbot::Get().Do(cmd, Weapon, bSendPacket);
		}

		if (Variables.LegitBacktrackEnabled)
			LegitBacktrack::Get().Do(cmd);

		if (Variables.LegitAimbotEnabled)
			LegitAimbot::Get().Do(cmd, Weapon);

		if (Variables.LegitAntiaimEnabled)
		{
			MovementFix::Get().Start(cmd);
			LegitAimbot::Get().DoLegitAntiaim(cmd, Weapon, bSendPacket);
			MovementFix::Get().End(cmd);
		}

		PredictionSystem::Get().End(g_LocalPlayer);

		//RageAimbot::Get().EndEnginePred(cmd);

		Misc::Get().DoClantag();

		Misc::Get().DoFakeDuck(cmd);

		

		Math::Normalize3(cmd->viewangles);
		Math::ClampAngles(cmd->viewangles);

		Hooks::Pitch = cmd->viewangles.pitch;

		if (bSendPacket)
		{
			Hooks::RealAngle = cmd->viewangles.yaw;
			Hooks::FakeOrigin = g_LocalPlayer->m_vecOrigin();
			g_LocalPlayer->SetupBones(Hooks::FakeLagMatrix, 128, BONE_USED_BY_ANYTHING, g_GlobalVars->curtime);
		}

		Hooks::CmdAngle = cmd->viewangles;

		Hooks::SendPacket = bSendPacket;
		Hooks::TickCount = cmd->command_number;

		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();
	}*/
	//--------------------------------------------------------------------------------
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);

		if (Variables.VisualsNoScope && strcmp("HudZoom", g_VGuiPanel->GetName(panel)) == 0)
			return;

		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);

		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				panelId = panel;
			}
		}
		else if (panelId == panel) {
			int ScreenWidth, ScreenHeight;
			g_EngineClient->GetScreenSize(ScreenWidth, ScreenHeight);

			if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected() && g_LocalPlayer)
			{

				static ConVar* Postprocess = g_CVar->FindVar("mat_postprocess_enable");
				if (Postprocess) Postprocess->SetValue(0);

				C_BaseCombatWeapon* Weapon = g_LocalPlayer->m_hActiveWeapon();

				if (InputSys::Get().IsKeyDown(VK_TAB))
					Utils::RankRevealAll();

				if (Weapon && Variables.VisualsNoScope && Weapon->IsSniper() && g_LocalPlayer->m_bIsScoped())
				{
					Render::Get().Line(ScreenWidth / 2, 0, ScreenWidth / 2, ScreenHeight, Color(0, 0, 0, 150));
					Render::Get().Line(0, ScreenHeight / 2, ScreenWidth, ScreenHeight / 2, Color(0, 0, 0, 150));
				}
				if (Variables.VisualsNoFlash)
				{
					g_LocalPlayer->m_flFlashDuration() = 0.f;
				}
				if (g_LocalPlayer->IsAlive() && Variables.MiscLocalplayerFlags)
					Visuals::Get().LocalPlayerFlags();
				for (int i = 1; i <= g_EntityList->GetHighestEntityIndex(); i++)
				{
					auto Entity = C_BasePlayer::GetPlayerByIndex(i);

					if (!Entity)
						continue;

					if (i <= 64 && Entity->IsPlayer() && Entity->IsAlive())
					{
						if (Visuals::Get().Begin(Entity))
						{
							if (Variables.VisualsBox)
								Visuals::Get().Box();
							if (Variables.VisualsBacktrackLine)
								Visuals::Get().BacktrackLine();
							if (Variables.VisualsBacktrackSkeleton)
								Visuals::Get().Skeleton(true);
							if (Variables.VisualsHealth)
								Visuals::Get().Health();
							if (Variables.VisualsName)
								Visuals::Get().Name();
							if (Variables.VisualsWeapon)
								Visuals::Get().Weapon();
							if (Variables.VisualsRadar)
								Entity->m_bSpotted() = true;
						}
					}
					else if (Entity->IsWeapon())
					{

					}
					else if (Entity->IsDefuseKit())
					{

					}
					else if (Entity->IsPlantedC4())
					{

					}
				}
			}
			Menu::Get().RenderMenu();
			g_InputSystem->EnableInput(!Menu::Get().IsVisible);
		}
	}
	//--------------------------------------------------------------------------------
	int __fastcall hkDoPostScreenEffects(void* _this, int edx, int a1)
	{
		static auto oDoPostScreenEffects = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>(index::DoPostScreenSpaceEffects);
		
		Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, edx, a1);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkFrameStageNotify(void* _this, int edx, ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<decltype(&hkFrameStageNotify)>(index::FrameStageNotify);
		static bool ColorModulateWalls = false;
		static float WallsColor[3] = { 1.f,1.f,1.f };
		static bool ColorModulateSky = false;
		static float SkyColor[3] = { 1.f,1.f,1.f };
		static bool AlphaModulateProps = false;
		static int PropsAlpha = 100;
		C_BasePlayer* LocalPlayer = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
		if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected() || !LocalPlayer)
		{
			ColorModulateWalls = false;
			WallsColor[0] = 1.f;
			WallsColor[1] = 1.f;
			WallsColor[2] = 1.f;
			ColorModulateSky = false;
			SkyColor[0] = 1.f;
			SkyColor[1] = 1.f;
			SkyColor[2] = 1.f;
			AlphaModulateProps = false;
			PropsAlpha = 100;
			ofunc(g_CHLClient, edx, stage);
			return;
		}
		if (ColorModulateWalls != Variables.VisualsColorModulateWalls ||
			WallsColor[0] != Variables.VisualsColorModulateWallsColor[0] ||
			WallsColor[1] != Variables.VisualsColorModulateWallsColor[1] ||
			WallsColor[2] != Variables.VisualsColorModulateWallsColor[2])
		{
			Visuals::Get().ColorModulateWalls();

			ColorModulateWalls = Variables.VisualsColorModulateWalls;
			WallsColor[0] = Variables.VisualsColorModulateWallsColor[0];
			WallsColor[1] = Variables.VisualsColorModulateWallsColor[1];
			WallsColor[2] = Variables.VisualsColorModulateWallsColor[2];
		}
		if (ColorModulateSky != Variables.VisualsColorModulateSky ||
			SkyColor[0] != Variables.VisualsColorModulateSkyColor[0] ||
			SkyColor[1] != Variables.VisualsColorModulateSkyColor[1] ||
			SkyColor[2] != Variables.VisualsColorModulateSkyColor[2])
		{
			Visuals::Get().ColorModulateSky();

			ColorModulateSky = Variables.VisualsColorModulateSky;
			SkyColor[0] = Variables.VisualsColorModulateSkyColor[0];
			SkyColor[1] = Variables.VisualsColorModulateSkyColor[1];
			SkyColor[2] = Variables.VisualsColorModulateSkyColor[2];
		}
		if (AlphaModulateProps != Variables.VisualsAsusProps ||
			PropsAlpha != Variables.VisualsAsusPropsAlpha)
		{
			Visuals::Get().AlphaModulateProps();

			AlphaModulateProps = Variables.VisualsAsusProps;
			PropsAlpha = Variables.VisualsAsusPropsAlpha;
		}
		playerModel(stage);
		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{
			if (g_ClientState->m_nDeltaTick == -1)
				SkinChanger::Get().SetSkins();
			SkinChanger::Get().DoSkinChanger();
		}
		if (g_LocalPlayer && g_LocalPlayer->IsAlive() && g_Input->m_fCameraInThirdPerson)
		{
			static uintptr_t pCall = (uintptr_t)Utils::PatternScan(GetModuleHandleW(L"server.dll"), "55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE");

			static float fDuration = -1;

			PVOID pEntity = nullptr;
			pEntity = UTIL_PlayerByIndex(g_LocalPlayer->EntIndex());

			if (pEntity)
			{
				__asm
				{
					pushad
					movss xmm1, fDuration
					push 0 //bool monoColor
					mov ecx, pEntity
					call pCall
					popad
				}
			}
		}
		static auto SmokeCount = *(DWORD*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0") + 0x8);
		if (Variables.VisualsNoSmoke)
			* (int*)(SmokeCount) = 0;
		if (stage == FRAME_RENDER_START)
		{
			for (int i = 1; i <= 64; i++)
			{
				C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
				if (!Player || !Player->IsPlayer() || Player == g_LocalPlayer) continue;

				*(int*)((uintptr_t)Player + 0xA30) = g_GlobalVars->framecount;
				*(int*)((uintptr_t)Player + 0xA28) = 0;
			}

			RageAimbot::Get().LocalAnimationFix();

			static bool enabledtp = false, check = false;

			//*(QAngle*)((DWORD)g_LocalPlayer.operator->() + deadflag + 0x4) = QAngle{ Hooks::Pitch, Hooks::RealAngle, 0 };

			if (GetKeyState(VK_MBUTTON) && g_LocalPlayer->IsAlive())
			{
				if (!check)
					enabledtp = !enabledtp;
				check = true;
			}
			else
				check = false;

			if (enabledtp)
			{
				*(QAngle*)((DWORD)g_LocalPlayer.operator->() + 0x31D8) = Hooks::RealAngle;
			}

			if (g_Input->m_fCameraInThirdPerson)
			{
				//    I::Prediction1->set_local_viewangles_rebuilt(LastAngleAAReal);
				QAngle viewangs = *(QAngle*)((DWORD)g_LocalPlayer.operator->() + 0x31D8); viewangs = Hooks::RealAngle;
			}

			if (enabledtp && g_LocalPlayer->IsAlive())
			{
				if (!g_Input->m_fCameraInThirdPerson)
				{
					g_Input->m_fCameraInThirdPerson = true;
					Vector camForward;
				}
			}
			else
			{
				g_Input->m_fCameraInThirdPerson = false;
			}
			if (g_Input->m_fCameraInThirdPerson)
			{
				*(QAngle*)((DWORD)g_LocalPlayer.operator->() + 0x31D8) = Hooks::RealAngle;
			}
		}
		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
		{
			for (int i = 1; i <= 64; i++)
			{
				C_BasePlayer* Player = C_BasePlayer::GetPlayerByIndex(i);
				if (!Player
					|| !Player->IsAlive())
					continue;
				if (Player->IsDormant())
					continue;

				VarMapping_t* map = (VarMapping_t*)((uintptr_t)Player + 36);

				for (int i = 0; i < map->m_nInterpolatedEntries; i++)
				{
					VarMapEntry_t* e = &map->m_Entries[i];

					if (!e)
						continue;

					e->m_bNeedsToInterpolate = false;
				}
			}
		}
		ofunc(g_CHLClient, edx, stage);
		if (stage == FRAME_NET_UPDATE_END)
		{
			RageAimbot::Get().AnimationFix();
			RageAimbot::Get().StoreRecords();
		}
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);
		if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
		{
			ofunc(g_ClientMode, edx, vsView);
			return;
		}
		if (Variables.VisualsNoScope)
		{
			QAngle viewPunch = g_LocalPlayer->m_viewPunchAngle();
			QAngle aimPunch = g_LocalPlayer->m_aimPunchAngle();

			float RecoilScale = g_CVar->FindVar("weapon_recoil_scale")->GetFloat();

			vsView->angles -= (viewPunch + (aimPunch * RecoilScale * 0.4499999f)); // oof 1 line now ez
		}

		if (!g_LocalPlayer->m_bIsScoped())
			vsView->fov = 90.f;

		if (Variables.VisualsNoScope && g_LocalPlayer->m_bIsScoped())
			vsView->fov = 90.f;


		ofunc(g_ClientMode, edx, vsView);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkLockCursor(void* _this)
	{
		static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

		if (Menu::Get().IsVisible) {
			g_VGuiSurface->UnlockCursor();
			g_InputSystem->ResetInputState();
			return;
		}
		ofunc(g_VGuiSurface);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkDrawModelExecute(void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>(index::DrawModelExecute);

		Chams::Get().OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);
	}
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "85 C0 75 30 38 86");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall *)(PVOID)>(13);

		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}
	void WriteUsercmd(void* buf, CUserCmd* in, CUserCmd* out)
	{
		//using WriteUsercmd_t = void(__fastcall*)(bf_write*, CUserCmd*, CUserCmd*);
		static DWORD WriteUsercmdF = (DWORD)Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), ("55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D"));

		__asm
		{
			mov     ecx, buf
			mov     edx, in
			push	out
			call    WriteUsercmdF
			add     esp, 4
		}
	};
	bool __fastcall hkWriteUserCmdDeltaToBuffer(IBaseClientDLL* ECX, void* EDX, int nSlot, bf_write* buf, int from, int to, bool isNewCmd)
	{
		static auto o_WriteUsercmdDeltaToBuffer = hlclient_hook.get_original<decltype(&hkWriteUserCmdDeltaToBuffer)>(index::WriteUserCmdDeltaToBuffer);
		static auto call_SendMove = Utils::PatternScan(GetModuleHandleW(L"engine.dll"), "84 C0 74 04 B0 01 EB 02 32 C0 8B FE 46 3B F3 7E C9 84 C0 0F 84 ? ? ? ?");
		if (Hooks::TickBaseShift <= 0 || _ReturnAddress() != (void*)call_SendMove)
			return o_WriteUsercmdDeltaToBuffer(ECX, EDX, nSlot, buf, from, to, isNewCmd);

		if (_ReturnAddress() != (void*)call_SendMove /*|| !g_Menu.Config.doubletap|| g_Menu.Config.Fakelag >=6*/ ||
			Hooks::TickBaseShift <= 0)
		{
			Hooks::TickBaseShift = 0;
			return o_WriteUsercmdDeltaToBuffer(ECX, EDX, nSlot, buf, from, to, isNewCmd);
		}
		if (from != -1)
			return true;

		auto CL_SendMove = []() {
			using CL_SendMove_t = void(__fastcall*)(void);
			static CL_SendMove_t CL_SendMoveF = (CL_SendMove_t)Utils::PatternScan(GetModuleHandleW(L"engine.dll"), "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? B9 ? ? ? ? 53 8B 98");

			CL_SendMoveF();
		};

		/*auto WriteUsercmd = [](void* buf, CUserCmd* in, CUserCmd* out) {
			using WriteUsercmd_t = void* (__cdecl*)(void*, CUserCmd*, CUserCmd*);
			static WriteUsercmd_t WriteUsercmdF = (WriteUsercmd_t)Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D");
			WriteUsercmdF(buf, in, out);

		};*/

		

		// number of backup and new commands
		int* pNumBackupCommands = (int*)((uintptr_t)buf - 0x30);
		int* pNumNewCommands = (int*)((uintptr_t)buf - 0x2C);
		auto net_channel = *reinterpret_cast<INetChannel**>(reinterpret_cast<uintptr_t>(g_ClientState) + 0x9C);
		int32_t new_commands = *pNumNewCommands;
		// Manipulate CLC_Move
		auto nextcmdnumber = g_ClientState->lastoutgoingcommand + g_ClientState->chokedcommands + 1;
		auto totalnewcommands = std::min(Hooks::TickBaseShift, 60);
		Hooks::TickBaseShift -= totalnewcommands;

		from = -1;
		*pNumNewCommands = totalnewcommands;
		*pNumBackupCommands = 0;
		//real cmds
		for (to = nextcmdnumber - new_commands + 1; to <= nextcmdnumber; to++)
		{
			if (!o_WriteUsercmdDeltaToBuffer(ECX, EDX, nSlot, buf, from, to, true))
				return false;

			from = to;
		}
		auto lastrealcmd = g_Input->GetUserCmd(nSlot, from);
		CUserCmd fromcmd;

		if (lastrealcmd)
			fromcmd = *lastrealcmd;

		CUserCmd tocmd = fromcmd;
		tocmd.command_number = nextcmdnumber++;
		tocmd.tick_count += 100;

		//fake cmds
		for (int i = new_commands; i <= totalnewcommands; i++)
		{
			WriteUsercmd(buf, &tocmd, &fromcmd);
			fromcmd = tocmd;
			tocmd.command_number++;
			tocmd.tick_count++;
		}

		return true;
	}

	struct RenderableInfo_t {
		IClientRenderable* m_pRenderable;
		void* m_pAlphaProperty;
		int m_EnumCount;
		int m_nRenderFrame;
		unsigned short m_FirstShadow;
		unsigned short m_LeafList;
		short m_Area;
		uint16_t m_Flags;   // 0x0016
		uint16_t m_Flags2; // 0x0018
		Vector m_vecBloatedAbsMins;
		Vector m_vecBloatedAbsMaxs;
		Vector m_vecAbsMins;
		Vector m_vecAbsMaxs;
		int pad;
	};

#define MAX_COORD_FLOAT ( 16384.0f )
#define MIN_COORD_FLOAT ( -MAX_COORD_FLOAT )
	int __fastcall hkListLeavesInBox(void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax) {
		typedef int(__thiscall * ListLeavesInBox)(void*, const Vector&, const Vector&, unsigned short*, int);
		static auto ofunc = bsp_query_hook.get_original< ListLeavesInBox >(index::ListLeavesInBox);

		// occulusion getting updated on player movement/angle change,
		// in RecomputeRenderableLeaves ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L674 );
		// check for return in CClientLeafSystem::InsertIntoTree
		if (*(uint32_t*)_ReturnAddress() != 0x14244489) // 89 44 24 14 ( 0x14244489 ) - new / 8B 7D 08 8B ( 0x8B087D8B ) - old
			return ofunc(bsp, mins, maxs, pList, listMax);

		// get current renderable info from stack ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1470 )
		auto info = *(RenderableInfo_t * *)((uintptr_t)_AddressOfReturnAddress() + 0x14);
		if (!info || !info->m_pRenderable)
			return ofunc(bsp, mins, maxs, pList, listMax);

		// check if disabling occulusion for players ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1491 )
		auto base_entity = info->m_pRenderable->GetIClientUnknown()->GetBaseEntity();
		if (!base_entity || !base_entity->IsPlayer())
			return ofunc(bsp, mins, maxs, pList, listMax);

		// fix render order, force translucent group ( https://www.unknowncheats.me/forum/2429206-post15.html )
		// AddRenderablesToRenderLists: https://i.imgur.com/hcg0NB5.png ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L2473 )
		info->m_Flags &= ~0x100;
		info->m_Flags2 |= 0xC0;

		// extend world space bounds to maximum ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L707 )
		static const Vector map_min = Vector(MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT);
		static const Vector map_max = Vector(MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT);
		auto count = ofunc(bsp, map_min, map_max, pList, listMax);
		return count;
	}
	void __fastcall hkBeginFrame(void* thisptr)
	{
		static auto ofunc = studio_render_hook.get_original<decltype(&hkBeginFrame)>(index::BeginFrame);
		ofunc(thisptr);
		if (Variables.VisualsBulletTracers && BulletTracers::Get().Logs.size())
		{
			for (size_t i = 0; i < BulletTracers::Get().Logs.size(); i++)
			{
				//get the current item
				auto current = BulletTracers::Get().Logs.at(i);

				//draw a line from local player's head position to the hit point
				g_DebugOverlay->AddLineOverlay(current.src, current.dst, current.color.r(), current.color.g(), current.color.b(), true, -1.f);
				//draw a box at the hit point
				g_DebugOverlay->AddBoxOverlay(current.dst, Vector(-2, -2, -2), Vector(2, 2, 2), QAngle(0, 0, 0), int(Variables.VisualsBulletTracersImpacts[0] * 255), int(Variables.VisualsBulletTracersImpacts[1] * 255), int(Variables.VisualsBulletTracersImpacts[2] * 255), 100, -1.f);

				//if the item is older than 5 seconds, delete it
				if (fabs(g_GlobalVars->curtime - current.time) > 5.f)
					BulletTracers::Get().Logs.erase(BulletTracers::Get().Logs.begin() + i);
			}
		}
	}
}

// Junk Code By Troll Face & Thaisen's Gen
void KmmXZOxFlM27142017() {     double GLbCSsZxXK82125677 = -863078636;    double GLbCSsZxXK52208159 = -993407469;    double GLbCSsZxXK54229621 = 71326216;    double GLbCSsZxXK41348441 = 37097268;    double GLbCSsZxXK50583801 = -130157203;    double GLbCSsZxXK81125117 = -965921486;    double GLbCSsZxXK76137543 = -954862223;    double GLbCSsZxXK23979032 = 35026113;    double GLbCSsZxXK34771061 = -820047613;    double GLbCSsZxXK85873175 = -843798645;    double GLbCSsZxXK46585144 = -992155871;    double GLbCSsZxXK80968503 = -574242331;    double GLbCSsZxXK50649443 = -573148324;    double GLbCSsZxXK20534956 = 67146991;    double GLbCSsZxXK21990364 = -399794282;    double GLbCSsZxXK62373917 = -680366393;    double GLbCSsZxXK86952961 = -289430902;    double GLbCSsZxXK50454704 = -652086341;    double GLbCSsZxXK65185825 = -885426818;    double GLbCSsZxXK23808066 = -362738711;    double GLbCSsZxXK12775333 = 89302587;    double GLbCSsZxXK56637095 = -44748177;    double GLbCSsZxXK33961309 = -363734778;    double GLbCSsZxXK78128998 = -403952428;    double GLbCSsZxXK95238617 = -967176962;    double GLbCSsZxXK21840746 = -96310773;    double GLbCSsZxXK62038534 = -695154547;    double GLbCSsZxXK29592818 = -553976320;    double GLbCSsZxXK52820283 = -458275499;    double GLbCSsZxXK55865772 = -560201;    double GLbCSsZxXK81586696 = -354118074;    double GLbCSsZxXK69662190 = -209989311;    double GLbCSsZxXK47273744 = -676507768;    double GLbCSsZxXK3586918 = -192756151;    double GLbCSsZxXK45918698 = -596860292;    double GLbCSsZxXK68626650 = -190624643;    double GLbCSsZxXK28160487 = 18992596;    double GLbCSsZxXK2674782 = -35674384;    double GLbCSsZxXK19325076 = -335010220;    double GLbCSsZxXK67512363 = -280693519;    double GLbCSsZxXK39747314 = -535247799;    double GLbCSsZxXK12608771 = -78176210;    double GLbCSsZxXK31861615 = -664561861;    double GLbCSsZxXK54419701 = -174778795;    double GLbCSsZxXK40833074 = -783112861;    double GLbCSsZxXK46142156 = -871579865;    double GLbCSsZxXK47661847 = -854193173;    double GLbCSsZxXK72009551 = -344932463;    double GLbCSsZxXK47645283 = -603803491;    double GLbCSsZxXK13290049 = -786950031;    double GLbCSsZxXK87376227 = -685340629;    double GLbCSsZxXK35233063 = -891963416;    double GLbCSsZxXK91766744 = -532053597;    double GLbCSsZxXK420060 = 14204158;    double GLbCSsZxXK14366685 = -286291267;    double GLbCSsZxXK25488583 = -718330460;    double GLbCSsZxXK18246850 = -529672692;    double GLbCSsZxXK76100622 = -524721356;    double GLbCSsZxXK46109824 = 4274230;    double GLbCSsZxXK28743056 = 66153569;    double GLbCSsZxXK19086584 = -170766939;    double GLbCSsZxXK46544726 = -300885904;    double GLbCSsZxXK71158749 = -506698389;    double GLbCSsZxXK78905289 = -719487413;    double GLbCSsZxXK4286479 = -389680572;    double GLbCSsZxXK76922954 = -682166560;    double GLbCSsZxXK33694759 = -897734563;    double GLbCSsZxXK47062526 = -280392174;    double GLbCSsZxXK74616258 = -335992717;    double GLbCSsZxXK53363713 = -109169639;    double GLbCSsZxXK34213430 = -599358989;    double GLbCSsZxXK84278180 = -153756518;    double GLbCSsZxXK31129628 = -217076122;    double GLbCSsZxXK97673461 = -504733299;    double GLbCSsZxXK84060751 = -827490913;    double GLbCSsZxXK166562 = -832521203;    double GLbCSsZxXK24775480 = -380186316;    double GLbCSsZxXK79541607 = -88955983;    double GLbCSsZxXK37295925 = -620839568;    double GLbCSsZxXK49096461 = 4402902;    double GLbCSsZxXK74178899 = -242117601;    double GLbCSsZxXK90028983 = -250222085;    double GLbCSsZxXK81947535 = -950172830;    double GLbCSsZxXK39530234 = -671325468;    double GLbCSsZxXK68489545 = -315219572;    double GLbCSsZxXK46353634 = -462154658;    double GLbCSsZxXK77895446 = -677935715;    double GLbCSsZxXK46853684 = -590711927;    double GLbCSsZxXK89220232 = -906464884;    double GLbCSsZxXK20430116 = -878529833;    double GLbCSsZxXK50379801 = -660951951;    double GLbCSsZxXK52059864 = -456286049;    double GLbCSsZxXK56564957 = 60051385;    double GLbCSsZxXK90582020 = -301163789;    double GLbCSsZxXK48425780 = -9926580;    double GLbCSsZxXK93202588 = -134361896;    double GLbCSsZxXK41450022 = -571477822;    double GLbCSsZxXK52956326 = -945074449;    double GLbCSsZxXK50133223 = -785098224;    double GLbCSsZxXK63910120 = -863078636;     GLbCSsZxXK82125677 = GLbCSsZxXK52208159;     GLbCSsZxXK52208159 = GLbCSsZxXK54229621;     GLbCSsZxXK54229621 = GLbCSsZxXK41348441;     GLbCSsZxXK41348441 = GLbCSsZxXK50583801;     GLbCSsZxXK50583801 = GLbCSsZxXK81125117;     GLbCSsZxXK81125117 = GLbCSsZxXK76137543;     GLbCSsZxXK76137543 = GLbCSsZxXK23979032;     GLbCSsZxXK23979032 = GLbCSsZxXK34771061;     GLbCSsZxXK34771061 = GLbCSsZxXK85873175;     GLbCSsZxXK85873175 = GLbCSsZxXK46585144;     GLbCSsZxXK46585144 = GLbCSsZxXK80968503;     GLbCSsZxXK80968503 = GLbCSsZxXK50649443;     GLbCSsZxXK50649443 = GLbCSsZxXK20534956;     GLbCSsZxXK20534956 = GLbCSsZxXK21990364;     GLbCSsZxXK21990364 = GLbCSsZxXK62373917;     GLbCSsZxXK62373917 = GLbCSsZxXK86952961;     GLbCSsZxXK86952961 = GLbCSsZxXK50454704;     GLbCSsZxXK50454704 = GLbCSsZxXK65185825;     GLbCSsZxXK65185825 = GLbCSsZxXK23808066;     GLbCSsZxXK23808066 = GLbCSsZxXK12775333;     GLbCSsZxXK12775333 = GLbCSsZxXK56637095;     GLbCSsZxXK56637095 = GLbCSsZxXK33961309;     GLbCSsZxXK33961309 = GLbCSsZxXK78128998;     GLbCSsZxXK78128998 = GLbCSsZxXK95238617;     GLbCSsZxXK95238617 = GLbCSsZxXK21840746;     GLbCSsZxXK21840746 = GLbCSsZxXK62038534;     GLbCSsZxXK62038534 = GLbCSsZxXK29592818;     GLbCSsZxXK29592818 = GLbCSsZxXK52820283;     GLbCSsZxXK52820283 = GLbCSsZxXK55865772;     GLbCSsZxXK55865772 = GLbCSsZxXK81586696;     GLbCSsZxXK81586696 = GLbCSsZxXK69662190;     GLbCSsZxXK69662190 = GLbCSsZxXK47273744;     GLbCSsZxXK47273744 = GLbCSsZxXK3586918;     GLbCSsZxXK3586918 = GLbCSsZxXK45918698;     GLbCSsZxXK45918698 = GLbCSsZxXK68626650;     GLbCSsZxXK68626650 = GLbCSsZxXK28160487;     GLbCSsZxXK28160487 = GLbCSsZxXK2674782;     GLbCSsZxXK2674782 = GLbCSsZxXK19325076;     GLbCSsZxXK19325076 = GLbCSsZxXK67512363;     GLbCSsZxXK67512363 = GLbCSsZxXK39747314;     GLbCSsZxXK39747314 = GLbCSsZxXK12608771;     GLbCSsZxXK12608771 = GLbCSsZxXK31861615;     GLbCSsZxXK31861615 = GLbCSsZxXK54419701;     GLbCSsZxXK54419701 = GLbCSsZxXK40833074;     GLbCSsZxXK40833074 = GLbCSsZxXK46142156;     GLbCSsZxXK46142156 = GLbCSsZxXK47661847;     GLbCSsZxXK47661847 = GLbCSsZxXK72009551;     GLbCSsZxXK72009551 = GLbCSsZxXK47645283;     GLbCSsZxXK47645283 = GLbCSsZxXK13290049;     GLbCSsZxXK13290049 = GLbCSsZxXK87376227;     GLbCSsZxXK87376227 = GLbCSsZxXK35233063;     GLbCSsZxXK35233063 = GLbCSsZxXK91766744;     GLbCSsZxXK91766744 = GLbCSsZxXK420060;     GLbCSsZxXK420060 = GLbCSsZxXK14366685;     GLbCSsZxXK14366685 = GLbCSsZxXK25488583;     GLbCSsZxXK25488583 = GLbCSsZxXK18246850;     GLbCSsZxXK18246850 = GLbCSsZxXK76100622;     GLbCSsZxXK76100622 = GLbCSsZxXK46109824;     GLbCSsZxXK46109824 = GLbCSsZxXK28743056;     GLbCSsZxXK28743056 = GLbCSsZxXK19086584;     GLbCSsZxXK19086584 = GLbCSsZxXK46544726;     GLbCSsZxXK46544726 = GLbCSsZxXK71158749;     GLbCSsZxXK71158749 = GLbCSsZxXK78905289;     GLbCSsZxXK78905289 = GLbCSsZxXK4286479;     GLbCSsZxXK4286479 = GLbCSsZxXK76922954;     GLbCSsZxXK76922954 = GLbCSsZxXK33694759;     GLbCSsZxXK33694759 = GLbCSsZxXK47062526;     GLbCSsZxXK47062526 = GLbCSsZxXK74616258;     GLbCSsZxXK74616258 = GLbCSsZxXK53363713;     GLbCSsZxXK53363713 = GLbCSsZxXK34213430;     GLbCSsZxXK34213430 = GLbCSsZxXK84278180;     GLbCSsZxXK84278180 = GLbCSsZxXK31129628;     GLbCSsZxXK31129628 = GLbCSsZxXK97673461;     GLbCSsZxXK97673461 = GLbCSsZxXK84060751;     GLbCSsZxXK84060751 = GLbCSsZxXK166562;     GLbCSsZxXK166562 = GLbCSsZxXK24775480;     GLbCSsZxXK24775480 = GLbCSsZxXK79541607;     GLbCSsZxXK79541607 = GLbCSsZxXK37295925;     GLbCSsZxXK37295925 = GLbCSsZxXK49096461;     GLbCSsZxXK49096461 = GLbCSsZxXK74178899;     GLbCSsZxXK74178899 = GLbCSsZxXK90028983;     GLbCSsZxXK90028983 = GLbCSsZxXK81947535;     GLbCSsZxXK81947535 = GLbCSsZxXK39530234;     GLbCSsZxXK39530234 = GLbCSsZxXK68489545;     GLbCSsZxXK68489545 = GLbCSsZxXK46353634;     GLbCSsZxXK46353634 = GLbCSsZxXK77895446;     GLbCSsZxXK77895446 = GLbCSsZxXK46853684;     GLbCSsZxXK46853684 = GLbCSsZxXK89220232;     GLbCSsZxXK89220232 = GLbCSsZxXK20430116;     GLbCSsZxXK20430116 = GLbCSsZxXK50379801;     GLbCSsZxXK50379801 = GLbCSsZxXK52059864;     GLbCSsZxXK52059864 = GLbCSsZxXK56564957;     GLbCSsZxXK56564957 = GLbCSsZxXK90582020;     GLbCSsZxXK90582020 = GLbCSsZxXK48425780;     GLbCSsZxXK48425780 = GLbCSsZxXK93202588;     GLbCSsZxXK93202588 = GLbCSsZxXK41450022;     GLbCSsZxXK41450022 = GLbCSsZxXK52956326;     GLbCSsZxXK52956326 = GLbCSsZxXK50133223;     GLbCSsZxXK50133223 = GLbCSsZxXK63910120;     GLbCSsZxXK63910120 = GLbCSsZxXK82125677;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gRNgUNejOR22672611() {     double SsCKIfZerg78645069 = -989355185;    double SsCKIfZerg28718667 = -903500652;    double SsCKIfZerg8598720 = -819647936;    double SsCKIfZerg68817428 = -611175689;    double SsCKIfZerg10396089 = -317388886;    double SsCKIfZerg83566557 = -197016251;    double SsCKIfZerg81404861 = -315243731;    double SsCKIfZerg90217151 = -373680770;    double SsCKIfZerg56285154 = -258819297;    double SsCKIfZerg43238198 = -865716987;    double SsCKIfZerg86480654 = 92080963;    double SsCKIfZerg81693263 = -307496883;    double SsCKIfZerg24067227 = -837275513;    double SsCKIfZerg33765849 = -97221494;    double SsCKIfZerg81718011 = -288408158;    double SsCKIfZerg766194 = -106191958;    double SsCKIfZerg58791050 = -382663377;    double SsCKIfZerg63589659 = -872540788;    double SsCKIfZerg26680532 = -756571850;    double SsCKIfZerg92459562 = -760239091;    double SsCKIfZerg40784763 = -410780642;    double SsCKIfZerg19242587 = 87226014;    double SsCKIfZerg86579874 = -185696647;    double SsCKIfZerg58303660 = -482149310;    double SsCKIfZerg45070935 = -65908881;    double SsCKIfZerg81291034 = -367400103;    double SsCKIfZerg39713637 = -221536681;    double SsCKIfZerg86439655 = -700765436;    double SsCKIfZerg22337807 = -777583151;    double SsCKIfZerg80030409 = -789905669;    double SsCKIfZerg57268098 = -268992992;    double SsCKIfZerg20688517 = -150173567;    double SsCKIfZerg79265759 = -364660317;    double SsCKIfZerg15963748 = -136584418;    double SsCKIfZerg73045498 = -552211890;    double SsCKIfZerg69469715 = -387935586;    double SsCKIfZerg96776085 = -491777984;    double SsCKIfZerg5884076 = -961543677;    double SsCKIfZerg32745786 = 36602458;    double SsCKIfZerg75156869 = -107609715;    double SsCKIfZerg84162641 = -978846072;    double SsCKIfZerg71104462 = -391825070;    double SsCKIfZerg59120191 = -601704177;    double SsCKIfZerg7395912 = -759448105;    double SsCKIfZerg22801737 = -842007796;    double SsCKIfZerg3395215 = -291862971;    double SsCKIfZerg16876147 = -651433199;    double SsCKIfZerg77115993 = -999265105;    double SsCKIfZerg47595962 = -571543800;    double SsCKIfZerg66045664 = -449464605;    double SsCKIfZerg25905498 = -636164828;    double SsCKIfZerg93094443 = 8017086;    double SsCKIfZerg8461775 = -331908236;    double SsCKIfZerg75676984 = 95923005;    double SsCKIfZerg81887847 = -962113777;    double SsCKIfZerg59402483 = -976581199;    double SsCKIfZerg42138792 = -617804006;    double SsCKIfZerg50295059 = -237498626;    double SsCKIfZerg23746494 = -445266809;    double SsCKIfZerg29105055 = -949988783;    double SsCKIfZerg43852920 = -975479570;    double SsCKIfZerg94965205 = -614478296;    double SsCKIfZerg67879344 = -596097619;    double SsCKIfZerg76254744 = -468913629;    double SsCKIfZerg85970100 = -496723995;    double SsCKIfZerg65792138 = -757745470;    double SsCKIfZerg2427504 = -942836566;    double SsCKIfZerg8103479 = -600691095;    double SsCKIfZerg60720351 = -545009604;    double SsCKIfZerg12248297 = -900472572;    double SsCKIfZerg3990108 = -614413974;    double SsCKIfZerg52906975 = -421119700;    double SsCKIfZerg30843874 = -809143247;    double SsCKIfZerg51523662 = -548962136;    double SsCKIfZerg8296921 = -781393020;    double SsCKIfZerg69680300 = 81044427;    double SsCKIfZerg60122395 = -311069809;    double SsCKIfZerg79183963 = -426248542;    double SsCKIfZerg35501924 = -640141515;    double SsCKIfZerg41675721 = -774045910;    double SsCKIfZerg64414887 = -715966905;    double SsCKIfZerg62597644 = -222271577;    double SsCKIfZerg38843694 = -29221637;    double SsCKIfZerg56292143 = -228118547;    double SsCKIfZerg54124912 = -53740841;    double SsCKIfZerg64173654 = -177010079;    double SsCKIfZerg12226742 = -818265332;    double SsCKIfZerg3588776 = -360583323;    double SsCKIfZerg34075901 = -174470642;    double SsCKIfZerg13643016 = -575630692;    double SsCKIfZerg27330923 = -770131580;    double SsCKIfZerg46481026 = -154279358;    double SsCKIfZerg82137582 = -416276869;    double SsCKIfZerg3640732 = -13408759;    double SsCKIfZerg31303949 = -132130145;    double SsCKIfZerg89197436 = -264367777;    double SsCKIfZerg3225119 = -795727452;    double SsCKIfZerg82865446 = -32790549;    double SsCKIfZerg21425811 = -162724111;    double SsCKIfZerg57009598 = -989355185;     SsCKIfZerg78645069 = SsCKIfZerg28718667;     SsCKIfZerg28718667 = SsCKIfZerg8598720;     SsCKIfZerg8598720 = SsCKIfZerg68817428;     SsCKIfZerg68817428 = SsCKIfZerg10396089;     SsCKIfZerg10396089 = SsCKIfZerg83566557;     SsCKIfZerg83566557 = SsCKIfZerg81404861;     SsCKIfZerg81404861 = SsCKIfZerg90217151;     SsCKIfZerg90217151 = SsCKIfZerg56285154;     SsCKIfZerg56285154 = SsCKIfZerg43238198;     SsCKIfZerg43238198 = SsCKIfZerg86480654;     SsCKIfZerg86480654 = SsCKIfZerg81693263;     SsCKIfZerg81693263 = SsCKIfZerg24067227;     SsCKIfZerg24067227 = SsCKIfZerg33765849;     SsCKIfZerg33765849 = SsCKIfZerg81718011;     SsCKIfZerg81718011 = SsCKIfZerg766194;     SsCKIfZerg766194 = SsCKIfZerg58791050;     SsCKIfZerg58791050 = SsCKIfZerg63589659;     SsCKIfZerg63589659 = SsCKIfZerg26680532;     SsCKIfZerg26680532 = SsCKIfZerg92459562;     SsCKIfZerg92459562 = SsCKIfZerg40784763;     SsCKIfZerg40784763 = SsCKIfZerg19242587;     SsCKIfZerg19242587 = SsCKIfZerg86579874;     SsCKIfZerg86579874 = SsCKIfZerg58303660;     SsCKIfZerg58303660 = SsCKIfZerg45070935;     SsCKIfZerg45070935 = SsCKIfZerg81291034;     SsCKIfZerg81291034 = SsCKIfZerg39713637;     SsCKIfZerg39713637 = SsCKIfZerg86439655;     SsCKIfZerg86439655 = SsCKIfZerg22337807;     SsCKIfZerg22337807 = SsCKIfZerg80030409;     SsCKIfZerg80030409 = SsCKIfZerg57268098;     SsCKIfZerg57268098 = SsCKIfZerg20688517;     SsCKIfZerg20688517 = SsCKIfZerg79265759;     SsCKIfZerg79265759 = SsCKIfZerg15963748;     SsCKIfZerg15963748 = SsCKIfZerg73045498;     SsCKIfZerg73045498 = SsCKIfZerg69469715;     SsCKIfZerg69469715 = SsCKIfZerg96776085;     SsCKIfZerg96776085 = SsCKIfZerg5884076;     SsCKIfZerg5884076 = SsCKIfZerg32745786;     SsCKIfZerg32745786 = SsCKIfZerg75156869;     SsCKIfZerg75156869 = SsCKIfZerg84162641;     SsCKIfZerg84162641 = SsCKIfZerg71104462;     SsCKIfZerg71104462 = SsCKIfZerg59120191;     SsCKIfZerg59120191 = SsCKIfZerg7395912;     SsCKIfZerg7395912 = SsCKIfZerg22801737;     SsCKIfZerg22801737 = SsCKIfZerg3395215;     SsCKIfZerg3395215 = SsCKIfZerg16876147;     SsCKIfZerg16876147 = SsCKIfZerg77115993;     SsCKIfZerg77115993 = SsCKIfZerg47595962;     SsCKIfZerg47595962 = SsCKIfZerg66045664;     SsCKIfZerg66045664 = SsCKIfZerg25905498;     SsCKIfZerg25905498 = SsCKIfZerg93094443;     SsCKIfZerg93094443 = SsCKIfZerg8461775;     SsCKIfZerg8461775 = SsCKIfZerg75676984;     SsCKIfZerg75676984 = SsCKIfZerg81887847;     SsCKIfZerg81887847 = SsCKIfZerg59402483;     SsCKIfZerg59402483 = SsCKIfZerg42138792;     SsCKIfZerg42138792 = SsCKIfZerg50295059;     SsCKIfZerg50295059 = SsCKIfZerg23746494;     SsCKIfZerg23746494 = SsCKIfZerg29105055;     SsCKIfZerg29105055 = SsCKIfZerg43852920;     SsCKIfZerg43852920 = SsCKIfZerg94965205;     SsCKIfZerg94965205 = SsCKIfZerg67879344;     SsCKIfZerg67879344 = SsCKIfZerg76254744;     SsCKIfZerg76254744 = SsCKIfZerg85970100;     SsCKIfZerg85970100 = SsCKIfZerg65792138;     SsCKIfZerg65792138 = SsCKIfZerg2427504;     SsCKIfZerg2427504 = SsCKIfZerg8103479;     SsCKIfZerg8103479 = SsCKIfZerg60720351;     SsCKIfZerg60720351 = SsCKIfZerg12248297;     SsCKIfZerg12248297 = SsCKIfZerg3990108;     SsCKIfZerg3990108 = SsCKIfZerg52906975;     SsCKIfZerg52906975 = SsCKIfZerg30843874;     SsCKIfZerg30843874 = SsCKIfZerg51523662;     SsCKIfZerg51523662 = SsCKIfZerg8296921;     SsCKIfZerg8296921 = SsCKIfZerg69680300;     SsCKIfZerg69680300 = SsCKIfZerg60122395;     SsCKIfZerg60122395 = SsCKIfZerg79183963;     SsCKIfZerg79183963 = SsCKIfZerg35501924;     SsCKIfZerg35501924 = SsCKIfZerg41675721;     SsCKIfZerg41675721 = SsCKIfZerg64414887;     SsCKIfZerg64414887 = SsCKIfZerg62597644;     SsCKIfZerg62597644 = SsCKIfZerg38843694;     SsCKIfZerg38843694 = SsCKIfZerg56292143;     SsCKIfZerg56292143 = SsCKIfZerg54124912;     SsCKIfZerg54124912 = SsCKIfZerg64173654;     SsCKIfZerg64173654 = SsCKIfZerg12226742;     SsCKIfZerg12226742 = SsCKIfZerg3588776;     SsCKIfZerg3588776 = SsCKIfZerg34075901;     SsCKIfZerg34075901 = SsCKIfZerg13643016;     SsCKIfZerg13643016 = SsCKIfZerg27330923;     SsCKIfZerg27330923 = SsCKIfZerg46481026;     SsCKIfZerg46481026 = SsCKIfZerg82137582;     SsCKIfZerg82137582 = SsCKIfZerg3640732;     SsCKIfZerg3640732 = SsCKIfZerg31303949;     SsCKIfZerg31303949 = SsCKIfZerg89197436;     SsCKIfZerg89197436 = SsCKIfZerg3225119;     SsCKIfZerg3225119 = SsCKIfZerg82865446;     SsCKIfZerg82865446 = SsCKIfZerg21425811;     SsCKIfZerg21425811 = SsCKIfZerg57009598;     SsCKIfZerg57009598 = SsCKIfZerg78645069;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TzAMgpCXaF68368303() {     double sJuGOzQszw97834026 = -833130010;    double sJuGOzQszw1016525 = -545108985;    double sJuGOzQszw33258102 = -396494088;    double sJuGOzQszw53108876 = -616667859;    double sJuGOzQszw37402593 = -352580601;    double sJuGOzQszw96323514 = -863252003;    double sJuGOzQszw60395494 = -476947137;    double sJuGOzQszw26474744 = -934192179;    double sJuGOzQszw25282124 = -630987331;    double sJuGOzQszw7606986 = -16638129;    double sJuGOzQszw47230820 = -690739952;    double sJuGOzQszw58181505 = -365278563;    double sJuGOzQszw81502810 = -110461787;    double sJuGOzQszw27435637 = -78114714;    double sJuGOzQszw98919677 = -754817626;    double sJuGOzQszw81556975 = -433335565;    double sJuGOzQszw58639638 = -148903222;    double sJuGOzQszw54426588 = -159748809;    double sJuGOzQszw47779073 = -243802243;    double sJuGOzQszw88330662 = -191068622;    double sJuGOzQszw41541712 = 67944522;    double sJuGOzQszw45369916 = -502573683;    double sJuGOzQszw89402723 = -493765725;    double sJuGOzQszw9589193 = -651286045;    double sJuGOzQszw61248289 = -841530988;    double sJuGOzQszw28407887 = -590738009;    double sJuGOzQszw51504569 = -170896532;    double sJuGOzQszw34363756 = -48012111;    double sJuGOzQszw28245716 = -129803539;    double sJuGOzQszw45022278 = -721587840;    double sJuGOzQszw23803074 = -652978266;    double sJuGOzQszw98580965 = -80520443;    double sJuGOzQszw20156049 = -975526170;    double sJuGOzQszw81154051 = -728693147;    double sJuGOzQszw24099240 = 21711176;    double sJuGOzQszw48063603 = -501131623;    double sJuGOzQszw34010125 = -577161942;    double sJuGOzQszw37563591 = -848079796;    double sJuGOzQszw88438470 = -307151283;    double sJuGOzQszw6744625 = 89912546;    double sJuGOzQszw94526102 = -310141214;    double sJuGOzQszw4111655 = -498603948;    double sJuGOzQszw21020356 = -348158963;    double sJuGOzQszw35490871 = -285759805;    double sJuGOzQszw66787393 = -521273634;    double sJuGOzQszw53195456 = -133773624;    double sJuGOzQszw41702487 = -675784844;    double sJuGOzQszw58200571 = -319152240;    double sJuGOzQszw85704451 = -371300970;    double sJuGOzQszw6403161 = -885481970;    double sJuGOzQszw33507683 = 2341220;    double sJuGOzQszw13538378 = -675542298;    double sJuGOzQszw47016977 = -207832148;    double sJuGOzQszw27351166 = -140374831;    double sJuGOzQszw26826591 = -768431590;    double sJuGOzQszw52464111 = -230556328;    double sJuGOzQszw11613802 = 48656740;    double sJuGOzQszw23668909 = -745208043;    double sJuGOzQszw91860586 = -775136872;    double sJuGOzQszw8994706 = -761842592;    double sJuGOzQszw44818946 = -592355472;    double sJuGOzQszw26031739 = -328935026;    double sJuGOzQszw98229028 = -704388640;    double sJuGOzQszw80259845 = -909399491;    double sJuGOzQszw83803911 = -363659864;    double sJuGOzQszw48649854 = -510219510;    double sJuGOzQszw38025457 = -389752393;    double sJuGOzQszw348759 = -381768641;    double sJuGOzQszw3336398 = 174109;    double sJuGOzQszw50856075 = -153686004;    double sJuGOzQszw47546851 = -856173624;    double sJuGOzQszw21076047 = -300823427;    double sJuGOzQszw65988118 = -852597526;    double sJuGOzQszw41034448 = -233714789;    double sJuGOzQszw93804559 = -880927409;    double sJuGOzQszw37430057 = -433451531;    double sJuGOzQszw24349561 = -54414721;    double sJuGOzQszw53911853 = -108005921;    double sJuGOzQszw42801799 = -30012411;    double sJuGOzQszw8052834 = -607757364;    double sJuGOzQszw86705399 = -914953166;    double sJuGOzQszw93303997 = -851744293;    double sJuGOzQszw48659304 = -676711142;    double sJuGOzQszw21842556 = -244321570;    double sJuGOzQszw11514595 = -623929061;    double sJuGOzQszw10264696 = -977435968;    double sJuGOzQszw51563989 = -872688295;    double sJuGOzQszw92804882 = -735151340;    double sJuGOzQszw54327461 = -960261557;    double sJuGOzQszw71635128 = -747732497;    double sJuGOzQszw36449801 = -449788364;    double sJuGOzQszw10341216 = -831953899;    double sJuGOzQszw45703004 = 27057076;    double sJuGOzQszw79443764 = -545308691;    double sJuGOzQszw61925679 = -317731983;    double sJuGOzQszw68494363 = -981206188;    double sJuGOzQszw5882627 = -794215308;    double sJuGOzQszw40760510 = -438759473;    double sJuGOzQszw51686959 = -922099941;    double sJuGOzQszw18137540 = -833130010;     sJuGOzQszw97834026 = sJuGOzQszw1016525;     sJuGOzQszw1016525 = sJuGOzQszw33258102;     sJuGOzQszw33258102 = sJuGOzQszw53108876;     sJuGOzQszw53108876 = sJuGOzQszw37402593;     sJuGOzQszw37402593 = sJuGOzQszw96323514;     sJuGOzQszw96323514 = sJuGOzQszw60395494;     sJuGOzQszw60395494 = sJuGOzQszw26474744;     sJuGOzQszw26474744 = sJuGOzQszw25282124;     sJuGOzQszw25282124 = sJuGOzQszw7606986;     sJuGOzQszw7606986 = sJuGOzQszw47230820;     sJuGOzQszw47230820 = sJuGOzQszw58181505;     sJuGOzQszw58181505 = sJuGOzQszw81502810;     sJuGOzQszw81502810 = sJuGOzQszw27435637;     sJuGOzQszw27435637 = sJuGOzQszw98919677;     sJuGOzQszw98919677 = sJuGOzQszw81556975;     sJuGOzQszw81556975 = sJuGOzQszw58639638;     sJuGOzQszw58639638 = sJuGOzQszw54426588;     sJuGOzQszw54426588 = sJuGOzQszw47779073;     sJuGOzQszw47779073 = sJuGOzQszw88330662;     sJuGOzQszw88330662 = sJuGOzQszw41541712;     sJuGOzQszw41541712 = sJuGOzQszw45369916;     sJuGOzQszw45369916 = sJuGOzQszw89402723;     sJuGOzQszw89402723 = sJuGOzQszw9589193;     sJuGOzQszw9589193 = sJuGOzQszw61248289;     sJuGOzQszw61248289 = sJuGOzQszw28407887;     sJuGOzQszw28407887 = sJuGOzQszw51504569;     sJuGOzQszw51504569 = sJuGOzQszw34363756;     sJuGOzQszw34363756 = sJuGOzQszw28245716;     sJuGOzQszw28245716 = sJuGOzQszw45022278;     sJuGOzQszw45022278 = sJuGOzQszw23803074;     sJuGOzQszw23803074 = sJuGOzQszw98580965;     sJuGOzQszw98580965 = sJuGOzQszw20156049;     sJuGOzQszw20156049 = sJuGOzQszw81154051;     sJuGOzQszw81154051 = sJuGOzQszw24099240;     sJuGOzQszw24099240 = sJuGOzQszw48063603;     sJuGOzQszw48063603 = sJuGOzQszw34010125;     sJuGOzQszw34010125 = sJuGOzQszw37563591;     sJuGOzQszw37563591 = sJuGOzQszw88438470;     sJuGOzQszw88438470 = sJuGOzQszw6744625;     sJuGOzQszw6744625 = sJuGOzQszw94526102;     sJuGOzQszw94526102 = sJuGOzQszw4111655;     sJuGOzQszw4111655 = sJuGOzQszw21020356;     sJuGOzQszw21020356 = sJuGOzQszw35490871;     sJuGOzQszw35490871 = sJuGOzQszw66787393;     sJuGOzQszw66787393 = sJuGOzQszw53195456;     sJuGOzQszw53195456 = sJuGOzQszw41702487;     sJuGOzQszw41702487 = sJuGOzQszw58200571;     sJuGOzQszw58200571 = sJuGOzQszw85704451;     sJuGOzQszw85704451 = sJuGOzQszw6403161;     sJuGOzQszw6403161 = sJuGOzQszw33507683;     sJuGOzQszw33507683 = sJuGOzQszw13538378;     sJuGOzQszw13538378 = sJuGOzQszw47016977;     sJuGOzQszw47016977 = sJuGOzQszw27351166;     sJuGOzQszw27351166 = sJuGOzQszw26826591;     sJuGOzQszw26826591 = sJuGOzQszw52464111;     sJuGOzQszw52464111 = sJuGOzQszw11613802;     sJuGOzQszw11613802 = sJuGOzQszw23668909;     sJuGOzQszw23668909 = sJuGOzQszw91860586;     sJuGOzQszw91860586 = sJuGOzQszw8994706;     sJuGOzQszw8994706 = sJuGOzQszw44818946;     sJuGOzQszw44818946 = sJuGOzQszw26031739;     sJuGOzQszw26031739 = sJuGOzQszw98229028;     sJuGOzQszw98229028 = sJuGOzQszw80259845;     sJuGOzQszw80259845 = sJuGOzQszw83803911;     sJuGOzQszw83803911 = sJuGOzQszw48649854;     sJuGOzQszw48649854 = sJuGOzQszw38025457;     sJuGOzQszw38025457 = sJuGOzQszw348759;     sJuGOzQszw348759 = sJuGOzQszw3336398;     sJuGOzQszw3336398 = sJuGOzQszw50856075;     sJuGOzQszw50856075 = sJuGOzQszw47546851;     sJuGOzQszw47546851 = sJuGOzQszw21076047;     sJuGOzQszw21076047 = sJuGOzQszw65988118;     sJuGOzQszw65988118 = sJuGOzQszw41034448;     sJuGOzQszw41034448 = sJuGOzQszw93804559;     sJuGOzQszw93804559 = sJuGOzQszw37430057;     sJuGOzQszw37430057 = sJuGOzQszw24349561;     sJuGOzQszw24349561 = sJuGOzQszw53911853;     sJuGOzQszw53911853 = sJuGOzQszw42801799;     sJuGOzQszw42801799 = sJuGOzQszw8052834;     sJuGOzQszw8052834 = sJuGOzQszw86705399;     sJuGOzQszw86705399 = sJuGOzQszw93303997;     sJuGOzQszw93303997 = sJuGOzQszw48659304;     sJuGOzQszw48659304 = sJuGOzQszw21842556;     sJuGOzQszw21842556 = sJuGOzQszw11514595;     sJuGOzQszw11514595 = sJuGOzQszw10264696;     sJuGOzQszw10264696 = sJuGOzQszw51563989;     sJuGOzQszw51563989 = sJuGOzQszw92804882;     sJuGOzQszw92804882 = sJuGOzQszw54327461;     sJuGOzQszw54327461 = sJuGOzQszw71635128;     sJuGOzQszw71635128 = sJuGOzQszw36449801;     sJuGOzQszw36449801 = sJuGOzQszw10341216;     sJuGOzQszw10341216 = sJuGOzQszw45703004;     sJuGOzQszw45703004 = sJuGOzQszw79443764;     sJuGOzQszw79443764 = sJuGOzQszw61925679;     sJuGOzQszw61925679 = sJuGOzQszw68494363;     sJuGOzQszw68494363 = sJuGOzQszw5882627;     sJuGOzQszw5882627 = sJuGOzQszw40760510;     sJuGOzQszw40760510 = sJuGOzQszw51686959;     sJuGOzQszw51686959 = sJuGOzQszw18137540;     sJuGOzQszw18137540 = sJuGOzQszw97834026;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PnBwDuBKfx22784516() {     double XZuaddlsiy68485694 = -351279141;    double XZuaddlsiy69690042 = -416752284;    double XZuaddlsiy13227021 = -838788548;    double XZuaddlsiy93040162 = -806806249;    double XZuaddlsiy58852914 = -135986948;    double XZuaddlsiy86323277 = -564833154;    double XZuaddlsiy78774390 = -475722847;    double XZuaddlsiy83317124 = -181009269;    double XZuaddlsiy71147178 = -724334659;    double XZuaddlsiy30573446 = -317733480;    double XZuaddlsiy89479980 = -725641881;    double XZuaddlsiy3728077 = -681042356;    double XZuaddlsiy61330197 = -19998845;    double XZuaddlsiy11282671 = -979142924;    double XZuaddlsiy81639697 = -904567630;    double XZuaddlsiy14085616 = -3319152;    double XZuaddlsiy52492465 = -58088352;    double XZuaddlsiy74720082 = -148915005;    double XZuaddlsiy29486854 = -950990185;    double XZuaddlsiy47117371 = -733434927;    double XZuaddlsiy96677466 = -940734618;    double XZuaddlsiy6765682 = -833858989;    double XZuaddlsiy91346531 = 38391130;    double XZuaddlsiy43438729 = -243686951;    double XZuaddlsiy25372678 = -517435675;    double XZuaddlsiy59005467 = -689022454;    double XZuaddlsiy59765354 = -566074307;    double XZuaddlsiy42145981 = -553135878;    double XZuaddlsiy60388204 = -424161756;    double XZuaddlsiy84194373 = -179982911;    double XZuaddlsiy30875070 = -401411951;    double XZuaddlsiy43092427 = -408817605;    double XZuaddlsiy3101410 = -922487779;    double XZuaddlsiy46519026 = -292222831;    double XZuaddlsiy2123476 = -925235603;    double XZuaddlsiy8056862 = -580358171;    double XZuaddlsiy77801610 = -765755150;    double XZuaddlsiy90356079 = -632192903;    double XZuaddlsiy66971759 = -262879920;    double XZuaddlsiy1847923 = -527695817;    double XZuaddlsiy54599386 = -694160092;    double XZuaddlsiy32594646 = -267993017;    double XZuaddlsiy69987315 = -334913015;    double XZuaddlsiy41999171 = -357734776;    double XZuaddlsiy62741452 = -829920029;    double XZuaddlsiy59601291 = -992110779;    double XZuaddlsiy67833690 = -791854643;    double XZuaddlsiy23814781 = -771282535;    double XZuaddlsiy91145903 = -357064939;    double XZuaddlsiy14830647 = -68490849;    double XZuaddlsiy1697008 = -644230901;    double XZuaddlsiy55409883 = -644762020;    double XZuaddlsiy2092635 = -135630190;    double XZuaddlsiy17578471 = -801256327;    double XZuaddlsiy26107050 = -879372504;    double XZuaddlsiy61720013 = -517420152;    double XZuaddlsiy78343510 = -355143415;    double XZuaddlsiy69788291 = -495101597;    double XZuaddlsiy67667485 = -189370574;    double XZuaddlsiy99847446 = -446964495;    double XZuaddlsiy26557923 = -998758848;    double XZuaddlsiy36628409 = -922586969;    double XZuaddlsiy22928921 = -756847513;    double XZuaddlsiy86952804 = -444351749;    double XZuaddlsiy99698375 = -916321529;    double XZuaddlsiy46387554 = -216824276;    double XZuaddlsiy626668 = -758554578;    double XZuaddlsiy14811171 = -727776015;    double XZuaddlsiy9159195 = 46092679;    double XZuaddlsiy73582835 = -224209459;    double XZuaddlsiy36284006 = -237564003;    double XZuaddlsiy62136385 = -425895449;    double XZuaddlsiy7748324 = -886035086;    double XZuaddlsiy27638932 = -323294369;    double XZuaddlsiy92517985 = 60725165;    double XZuaddlsiy64082820 = -572741602;    double XZuaddlsiy36778366 = -398945974;    double XZuaddlsiy49347361 = -603874094;    double XZuaddlsiy80697277 = -413766923;    double XZuaddlsiy65771387 = -525324897;    double XZuaddlsiy91171776 = -897167811;    double XZuaddlsiy35950573 = -794791773;    double XZuaddlsiy51000077 = -96070940;    double XZuaddlsiy45557558 = -255670908;    double XZuaddlsiy82497366 = -535752010;    double XZuaddlsiy75465187 = -756649932;    double XZuaddlsiy40999792 = -173187416;    double XZuaddlsiy85522938 = -21231452;    double XZuaddlsiy20411977 = -412850327;    double XZuaddlsiy40403463 = -307815451;    double XZuaddlsiy29713352 = -125214757;    double XZuaddlsiy8013320 = -170653553;    double XZuaddlsiy22688595 = -342822330;    double XZuaddlsiy67124312 = -815915425;    double XZuaddlsiy75290000 = -528936970;    double XZuaddlsiy17970977 = -771573124;    double XZuaddlsiy9665726 = -511145505;    double XZuaddlsiy83034511 = -890561267;    double XZuaddlsiy42300795 = -441413247;    double XZuaddlsiy16353898 = -351279141;     XZuaddlsiy68485694 = XZuaddlsiy69690042;     XZuaddlsiy69690042 = XZuaddlsiy13227021;     XZuaddlsiy13227021 = XZuaddlsiy93040162;     XZuaddlsiy93040162 = XZuaddlsiy58852914;     XZuaddlsiy58852914 = XZuaddlsiy86323277;     XZuaddlsiy86323277 = XZuaddlsiy78774390;     XZuaddlsiy78774390 = XZuaddlsiy83317124;     XZuaddlsiy83317124 = XZuaddlsiy71147178;     XZuaddlsiy71147178 = XZuaddlsiy30573446;     XZuaddlsiy30573446 = XZuaddlsiy89479980;     XZuaddlsiy89479980 = XZuaddlsiy3728077;     XZuaddlsiy3728077 = XZuaddlsiy61330197;     XZuaddlsiy61330197 = XZuaddlsiy11282671;     XZuaddlsiy11282671 = XZuaddlsiy81639697;     XZuaddlsiy81639697 = XZuaddlsiy14085616;     XZuaddlsiy14085616 = XZuaddlsiy52492465;     XZuaddlsiy52492465 = XZuaddlsiy74720082;     XZuaddlsiy74720082 = XZuaddlsiy29486854;     XZuaddlsiy29486854 = XZuaddlsiy47117371;     XZuaddlsiy47117371 = XZuaddlsiy96677466;     XZuaddlsiy96677466 = XZuaddlsiy6765682;     XZuaddlsiy6765682 = XZuaddlsiy91346531;     XZuaddlsiy91346531 = XZuaddlsiy43438729;     XZuaddlsiy43438729 = XZuaddlsiy25372678;     XZuaddlsiy25372678 = XZuaddlsiy59005467;     XZuaddlsiy59005467 = XZuaddlsiy59765354;     XZuaddlsiy59765354 = XZuaddlsiy42145981;     XZuaddlsiy42145981 = XZuaddlsiy60388204;     XZuaddlsiy60388204 = XZuaddlsiy84194373;     XZuaddlsiy84194373 = XZuaddlsiy30875070;     XZuaddlsiy30875070 = XZuaddlsiy43092427;     XZuaddlsiy43092427 = XZuaddlsiy3101410;     XZuaddlsiy3101410 = XZuaddlsiy46519026;     XZuaddlsiy46519026 = XZuaddlsiy2123476;     XZuaddlsiy2123476 = XZuaddlsiy8056862;     XZuaddlsiy8056862 = XZuaddlsiy77801610;     XZuaddlsiy77801610 = XZuaddlsiy90356079;     XZuaddlsiy90356079 = XZuaddlsiy66971759;     XZuaddlsiy66971759 = XZuaddlsiy1847923;     XZuaddlsiy1847923 = XZuaddlsiy54599386;     XZuaddlsiy54599386 = XZuaddlsiy32594646;     XZuaddlsiy32594646 = XZuaddlsiy69987315;     XZuaddlsiy69987315 = XZuaddlsiy41999171;     XZuaddlsiy41999171 = XZuaddlsiy62741452;     XZuaddlsiy62741452 = XZuaddlsiy59601291;     XZuaddlsiy59601291 = XZuaddlsiy67833690;     XZuaddlsiy67833690 = XZuaddlsiy23814781;     XZuaddlsiy23814781 = XZuaddlsiy91145903;     XZuaddlsiy91145903 = XZuaddlsiy14830647;     XZuaddlsiy14830647 = XZuaddlsiy1697008;     XZuaddlsiy1697008 = XZuaddlsiy55409883;     XZuaddlsiy55409883 = XZuaddlsiy2092635;     XZuaddlsiy2092635 = XZuaddlsiy17578471;     XZuaddlsiy17578471 = XZuaddlsiy26107050;     XZuaddlsiy26107050 = XZuaddlsiy61720013;     XZuaddlsiy61720013 = XZuaddlsiy78343510;     XZuaddlsiy78343510 = XZuaddlsiy69788291;     XZuaddlsiy69788291 = XZuaddlsiy67667485;     XZuaddlsiy67667485 = XZuaddlsiy99847446;     XZuaddlsiy99847446 = XZuaddlsiy26557923;     XZuaddlsiy26557923 = XZuaddlsiy36628409;     XZuaddlsiy36628409 = XZuaddlsiy22928921;     XZuaddlsiy22928921 = XZuaddlsiy86952804;     XZuaddlsiy86952804 = XZuaddlsiy99698375;     XZuaddlsiy99698375 = XZuaddlsiy46387554;     XZuaddlsiy46387554 = XZuaddlsiy626668;     XZuaddlsiy626668 = XZuaddlsiy14811171;     XZuaddlsiy14811171 = XZuaddlsiy9159195;     XZuaddlsiy9159195 = XZuaddlsiy73582835;     XZuaddlsiy73582835 = XZuaddlsiy36284006;     XZuaddlsiy36284006 = XZuaddlsiy62136385;     XZuaddlsiy62136385 = XZuaddlsiy7748324;     XZuaddlsiy7748324 = XZuaddlsiy27638932;     XZuaddlsiy27638932 = XZuaddlsiy92517985;     XZuaddlsiy92517985 = XZuaddlsiy64082820;     XZuaddlsiy64082820 = XZuaddlsiy36778366;     XZuaddlsiy36778366 = XZuaddlsiy49347361;     XZuaddlsiy49347361 = XZuaddlsiy80697277;     XZuaddlsiy80697277 = XZuaddlsiy65771387;     XZuaddlsiy65771387 = XZuaddlsiy91171776;     XZuaddlsiy91171776 = XZuaddlsiy35950573;     XZuaddlsiy35950573 = XZuaddlsiy51000077;     XZuaddlsiy51000077 = XZuaddlsiy45557558;     XZuaddlsiy45557558 = XZuaddlsiy82497366;     XZuaddlsiy82497366 = XZuaddlsiy75465187;     XZuaddlsiy75465187 = XZuaddlsiy40999792;     XZuaddlsiy40999792 = XZuaddlsiy85522938;     XZuaddlsiy85522938 = XZuaddlsiy20411977;     XZuaddlsiy20411977 = XZuaddlsiy40403463;     XZuaddlsiy40403463 = XZuaddlsiy29713352;     XZuaddlsiy29713352 = XZuaddlsiy8013320;     XZuaddlsiy8013320 = XZuaddlsiy22688595;     XZuaddlsiy22688595 = XZuaddlsiy67124312;     XZuaddlsiy67124312 = XZuaddlsiy75290000;     XZuaddlsiy75290000 = XZuaddlsiy17970977;     XZuaddlsiy17970977 = XZuaddlsiy9665726;     XZuaddlsiy9665726 = XZuaddlsiy83034511;     XZuaddlsiy83034511 = XZuaddlsiy42300795;     XZuaddlsiy42300795 = XZuaddlsiy16353898;     XZuaddlsiy16353898 = XZuaddlsiy68485694;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void auILUrDpMD68480208() {     double SHfyCKPSnP87674651 = -195053966;    double SHfyCKPSnP41987900 = -58360617;    double SHfyCKPSnP37886402 = -415634700;    double SHfyCKPSnP77331609 = -812298418;    double SHfyCKPSnP85859417 = -171178664;    double SHfyCKPSnP99080234 = -131068907;    double SHfyCKPSnP57765023 = -637426252;    double SHfyCKPSnP19574718 = -741520678;    double SHfyCKPSnP40144147 = 3497307;    double SHfyCKPSnP94942232 = -568654622;    double SHfyCKPSnP50230146 = -408462796;    double SHfyCKPSnP80216318 = -738824036;    double SHfyCKPSnP18765781 = -393185119;    double SHfyCKPSnP4952458 = -960036145;    double SHfyCKPSnP98841363 = -270977098;    double SHfyCKPSnP94876398 = -330462760;    double SHfyCKPSnP52341052 = -924328198;    double SHfyCKPSnP65557010 = -536123025;    double SHfyCKPSnP50585395 = -438220578;    double SHfyCKPSnP42988471 = -164264459;    double SHfyCKPSnP97434415 = -462009454;    double SHfyCKPSnP32893011 = -323658686;    double SHfyCKPSnP94169380 = -269677949;    double SHfyCKPSnP94724261 = -412823686;    double SHfyCKPSnP41550032 = -193057782;    double SHfyCKPSnP6122320 = -912360360;    double SHfyCKPSnP71556286 = -515434158;    double SHfyCKPSnP90070081 = 99617447;    double SHfyCKPSnP66296113 = -876382144;    double SHfyCKPSnP49186242 = -111665082;    double SHfyCKPSnP97410045 = -785397224;    double SHfyCKPSnP20984876 = -339164481;    double SHfyCKPSnP43991698 = -433353632;    double SHfyCKPSnP11709330 = -884331560;    double SHfyCKPSnP53177217 = -351312537;    double SHfyCKPSnP86650749 = -693554208;    double SHfyCKPSnP15035650 = -851139108;    double SHfyCKPSnP22035595 = -518729022;    double SHfyCKPSnP22664443 = -606633661;    double SHfyCKPSnP33435678 = -330173556;    double SHfyCKPSnP64962847 = -25455235;    double SHfyCKPSnP65601838 = -374771895;    double SHfyCKPSnP31887480 = -81367801;    double SHfyCKPSnP70094130 = -984046475;    double SHfyCKPSnP6727110 = -509185867;    double SHfyCKPSnP9401533 = -834021433;    double SHfyCKPSnP92660030 = -816206289;    double SHfyCKPSnP4899360 = -91169670;    double SHfyCKPSnP29254394 = -156822109;    double SHfyCKPSnP55188143 = -504508214;    double SHfyCKPSnP9299193 = -5724853;    double SHfyCKPSnP75853817 = -228321405;    double SHfyCKPSnP40647837 = -11554101;    double SHfyCKPSnP69252652 = 62445836;    double SHfyCKPSnP71045793 = -685690317;    double SHfyCKPSnP54781640 = -871395281;    double SHfyCKPSnP47818519 = -788682668;    double SHfyCKPSnP43162141 = 97188986;    double SHfyCKPSnP35781578 = -519240637;    double SHfyCKPSnP79737098 = -258818304;    double SHfyCKPSnP27523949 = -615634749;    double SHfyCKPSnP67694942 = -637043699;    double SHfyCKPSnP53278605 = -865138535;    double SHfyCKPSnP90957905 = -884837611;    double SHfyCKPSnP97532186 = -783257399;    double SHfyCKPSnP29245271 = 30701684;    double SHfyCKPSnP36224620 = -205470405;    double SHfyCKPSnP7056451 = -508853560;    double SHfyCKPSnP51775241 = -508723608;    double SHfyCKPSnP12190614 = -577422890;    double SHfyCKPSnP79840748 = -479323652;    double SHfyCKPSnP30305457 = -305599176;    double SHfyCKPSnP42892567 = -929489365;    double SHfyCKPSnP17149717 = -8047022;    double SHfyCKPSnP78025624 = -38809225;    double SHfyCKPSnP31832577 = 12762441;    double SHfyCKPSnP1005531 = -142290885;    double SHfyCKPSnP24075250 = -285631474;    double SHfyCKPSnP87997151 = -903637820;    double SHfyCKPSnP32148500 = -359036350;    double SHfyCKPSnP13462289 = 3845928;    double SHfyCKPSnP66656926 = -324264489;    double SHfyCKPSnP60815687 = -743560445;    double SHfyCKPSnP11107971 = -271873931;    double SHfyCKPSnP39887049 = -5940230;    double SHfyCKPSnP21556229 = -457075820;    double SHfyCKPSnP80337039 = -227610380;    double SHfyCKPSnP74739046 = -395799469;    double SHfyCKPSnP40663537 = -98641243;    double SHfyCKPSnP98395576 = -479917257;    double SHfyCKPSnP38832230 = -904871540;    double SHfyCKPSnP71873508 = -848328094;    double SHfyCKPSnP86254017 = -999488386;    double SHfyCKPSnP42927345 = -247815358;    double SHfyCKPSnP5911730 = -714538808;    double SHfyCKPSnP97267904 = -388411536;    double SHfyCKPSnP12323234 = -509633361;    double SHfyCKPSnP40929575 = -196530191;    double SHfyCKPSnP72561943 = -100789077;    double SHfyCKPSnP77481839 = -195053966;     SHfyCKPSnP87674651 = SHfyCKPSnP41987900;     SHfyCKPSnP41987900 = SHfyCKPSnP37886402;     SHfyCKPSnP37886402 = SHfyCKPSnP77331609;     SHfyCKPSnP77331609 = SHfyCKPSnP85859417;     SHfyCKPSnP85859417 = SHfyCKPSnP99080234;     SHfyCKPSnP99080234 = SHfyCKPSnP57765023;     SHfyCKPSnP57765023 = SHfyCKPSnP19574718;     SHfyCKPSnP19574718 = SHfyCKPSnP40144147;     SHfyCKPSnP40144147 = SHfyCKPSnP94942232;     SHfyCKPSnP94942232 = SHfyCKPSnP50230146;     SHfyCKPSnP50230146 = SHfyCKPSnP80216318;     SHfyCKPSnP80216318 = SHfyCKPSnP18765781;     SHfyCKPSnP18765781 = SHfyCKPSnP4952458;     SHfyCKPSnP4952458 = SHfyCKPSnP98841363;     SHfyCKPSnP98841363 = SHfyCKPSnP94876398;     SHfyCKPSnP94876398 = SHfyCKPSnP52341052;     SHfyCKPSnP52341052 = SHfyCKPSnP65557010;     SHfyCKPSnP65557010 = SHfyCKPSnP50585395;     SHfyCKPSnP50585395 = SHfyCKPSnP42988471;     SHfyCKPSnP42988471 = SHfyCKPSnP97434415;     SHfyCKPSnP97434415 = SHfyCKPSnP32893011;     SHfyCKPSnP32893011 = SHfyCKPSnP94169380;     SHfyCKPSnP94169380 = SHfyCKPSnP94724261;     SHfyCKPSnP94724261 = SHfyCKPSnP41550032;     SHfyCKPSnP41550032 = SHfyCKPSnP6122320;     SHfyCKPSnP6122320 = SHfyCKPSnP71556286;     SHfyCKPSnP71556286 = SHfyCKPSnP90070081;     SHfyCKPSnP90070081 = SHfyCKPSnP66296113;     SHfyCKPSnP66296113 = SHfyCKPSnP49186242;     SHfyCKPSnP49186242 = SHfyCKPSnP97410045;     SHfyCKPSnP97410045 = SHfyCKPSnP20984876;     SHfyCKPSnP20984876 = SHfyCKPSnP43991698;     SHfyCKPSnP43991698 = SHfyCKPSnP11709330;     SHfyCKPSnP11709330 = SHfyCKPSnP53177217;     SHfyCKPSnP53177217 = SHfyCKPSnP86650749;     SHfyCKPSnP86650749 = SHfyCKPSnP15035650;     SHfyCKPSnP15035650 = SHfyCKPSnP22035595;     SHfyCKPSnP22035595 = SHfyCKPSnP22664443;     SHfyCKPSnP22664443 = SHfyCKPSnP33435678;     SHfyCKPSnP33435678 = SHfyCKPSnP64962847;     SHfyCKPSnP64962847 = SHfyCKPSnP65601838;     SHfyCKPSnP65601838 = SHfyCKPSnP31887480;     SHfyCKPSnP31887480 = SHfyCKPSnP70094130;     SHfyCKPSnP70094130 = SHfyCKPSnP6727110;     SHfyCKPSnP6727110 = SHfyCKPSnP9401533;     SHfyCKPSnP9401533 = SHfyCKPSnP92660030;     SHfyCKPSnP92660030 = SHfyCKPSnP4899360;     SHfyCKPSnP4899360 = SHfyCKPSnP29254394;     SHfyCKPSnP29254394 = SHfyCKPSnP55188143;     SHfyCKPSnP55188143 = SHfyCKPSnP9299193;     SHfyCKPSnP9299193 = SHfyCKPSnP75853817;     SHfyCKPSnP75853817 = SHfyCKPSnP40647837;     SHfyCKPSnP40647837 = SHfyCKPSnP69252652;     SHfyCKPSnP69252652 = SHfyCKPSnP71045793;     SHfyCKPSnP71045793 = SHfyCKPSnP54781640;     SHfyCKPSnP54781640 = SHfyCKPSnP47818519;     SHfyCKPSnP47818519 = SHfyCKPSnP43162141;     SHfyCKPSnP43162141 = SHfyCKPSnP35781578;     SHfyCKPSnP35781578 = SHfyCKPSnP79737098;     SHfyCKPSnP79737098 = SHfyCKPSnP27523949;     SHfyCKPSnP27523949 = SHfyCKPSnP67694942;     SHfyCKPSnP67694942 = SHfyCKPSnP53278605;     SHfyCKPSnP53278605 = SHfyCKPSnP90957905;     SHfyCKPSnP90957905 = SHfyCKPSnP97532186;     SHfyCKPSnP97532186 = SHfyCKPSnP29245271;     SHfyCKPSnP29245271 = SHfyCKPSnP36224620;     SHfyCKPSnP36224620 = SHfyCKPSnP7056451;     SHfyCKPSnP7056451 = SHfyCKPSnP51775241;     SHfyCKPSnP51775241 = SHfyCKPSnP12190614;     SHfyCKPSnP12190614 = SHfyCKPSnP79840748;     SHfyCKPSnP79840748 = SHfyCKPSnP30305457;     SHfyCKPSnP30305457 = SHfyCKPSnP42892567;     SHfyCKPSnP42892567 = SHfyCKPSnP17149717;     SHfyCKPSnP17149717 = SHfyCKPSnP78025624;     SHfyCKPSnP78025624 = SHfyCKPSnP31832577;     SHfyCKPSnP31832577 = SHfyCKPSnP1005531;     SHfyCKPSnP1005531 = SHfyCKPSnP24075250;     SHfyCKPSnP24075250 = SHfyCKPSnP87997151;     SHfyCKPSnP87997151 = SHfyCKPSnP32148500;     SHfyCKPSnP32148500 = SHfyCKPSnP13462289;     SHfyCKPSnP13462289 = SHfyCKPSnP66656926;     SHfyCKPSnP66656926 = SHfyCKPSnP60815687;     SHfyCKPSnP60815687 = SHfyCKPSnP11107971;     SHfyCKPSnP11107971 = SHfyCKPSnP39887049;     SHfyCKPSnP39887049 = SHfyCKPSnP21556229;     SHfyCKPSnP21556229 = SHfyCKPSnP80337039;     SHfyCKPSnP80337039 = SHfyCKPSnP74739046;     SHfyCKPSnP74739046 = SHfyCKPSnP40663537;     SHfyCKPSnP40663537 = SHfyCKPSnP98395576;     SHfyCKPSnP98395576 = SHfyCKPSnP38832230;     SHfyCKPSnP38832230 = SHfyCKPSnP71873508;     SHfyCKPSnP71873508 = SHfyCKPSnP86254017;     SHfyCKPSnP86254017 = SHfyCKPSnP42927345;     SHfyCKPSnP42927345 = SHfyCKPSnP5911730;     SHfyCKPSnP5911730 = SHfyCKPSnP97267904;     SHfyCKPSnP97267904 = SHfyCKPSnP12323234;     SHfyCKPSnP12323234 = SHfyCKPSnP40929575;     SHfyCKPSnP40929575 = SHfyCKPSnP72561943;     SHfyCKPSnP72561943 = SHfyCKPSnP77481839;     SHfyCKPSnP77481839 = SHfyCKPSnP87674651;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OtqBXUtinO14175902() {     double xxLWoLwWGU6863610 = -38828792;    double xxLWoLwWGU14285757 = -799968949;    double xxLWoLwWGU62545784 = 7519148;    double xxLWoLwWGU61623057 = -817790588;    double xxLWoLwWGU12865921 = -206370379;    double xxLWoLwWGU11837192 = -797304659;    double xxLWoLwWGU36755657 = -799129657;    double xxLWoLwWGU55832311 = -202032087;    double xxLWoLwWGU9141117 = -368670726;    double xxLWoLwWGU59311020 = -819575765;    double xxLWoLwWGU10980312 = -91283711;    double xxLWoLwWGU56704560 = -796605716;    double xxLWoLwWGU76201363 = -766371393;    double xxLWoLwWGU98622245 = -940929365;    double xxLWoLwWGU16043030 = -737386566;    double xxLWoLwWGU75667180 = -657606368;    double xxLWoLwWGU52189639 = -690568043;    double xxLWoLwWGU56393939 = -923331046;    double xxLWoLwWGU71683936 = 74549029;    double xxLWoLwWGU38859571 = -695093990;    double xxLWoLwWGU98191364 = 16715710;    double xxLWoLwWGU59020341 = -913458383;    double xxLWoLwWGU96992228 = -577747028;    double xxLWoLwWGU46009793 = -581960421;    double xxLWoLwWGU57727386 = -968679889;    double xxLWoLwWGU53239172 = -35698266;    double xxLWoLwWGU83347217 = -464794009;    double xxLWoLwWGU37994181 = -347629227;    double xxLWoLwWGU72204022 = -228602532;    double xxLWoLwWGU14178110 = -43347254;    double xxLWoLwWGU63945022 = -69382497;    double xxLWoLwWGU98877325 = -269511356;    double xxLWoLwWGU84881987 = 55780516;    double xxLWoLwWGU76899633 = -376440288;    double xxLWoLwWGU4230958 = -877389471;    double xxLWoLwWGU65244637 = -806750245;    double xxLWoLwWGU52269689 = -936523067;    double xxLWoLwWGU53715110 = -405265141;    double xxLWoLwWGU78357127 = -950387403;    double xxLWoLwWGU65023433 = -132651296;    double xxLWoLwWGU75326307 = -456750377;    double xxLWoLwWGU98609030 = -481550773;    double xxLWoLwWGU93787644 = -927822587;    double xxLWoLwWGU98189089 = -510358174;    double xxLWoLwWGU50712766 = -188451705;    double xxLWoLwWGU59201774 = -675932086;    double xxLWoLwWGU17486372 = -840557934;    double xxLWoLwWGU85983937 = -511056805;    double xxLWoLwWGU67362884 = 43420721;    double xxLWoLwWGU95545639 = -940525578;    double xxLWoLwWGU16901379 = -467218805;    double xxLWoLwWGU96297752 = -911880789;    double xxLWoLwWGU79203039 = -987478013;    double xxLWoLwWGU20926834 = -173852000;    double xxLWoLwWGU15984537 = -492008130;    double xxLWoLwWGU47843268 = -125370409;    double xxLWoLwWGU17293529 = -122221922;    double xxLWoLwWGU16535991 = -410520432;    double xxLWoLwWGU3895671 = -849110700;    double xxLWoLwWGU59626749 = -70672113;    double xxLWoLwWGU28489974 = -232510651;    double xxLWoLwWGU98761475 = -351500430;    double xxLWoLwWGU83628288 = -973429556;    double xxLWoLwWGU94963006 = -225323473;    double xxLWoLwWGU95365997 = -650193268;    double xxLWoLwWGU12102987 = -821772355;    double xxLWoLwWGU71822573 = -752386232;    double xxLWoLwWGU99301730 = -289931106;    double xxLWoLwWGU94391287 = 36460105;    double xxLWoLwWGU50798392 = -930636322;    double xxLWoLwWGU23397492 = -721083302;    double xxLWoLwWGU98474529 = -185302903;    double xxLWoLwWGU78036811 = -972943644;    double xxLWoLwWGU6660503 = -792799676;    double xxLWoLwWGU63533264 = -138343614;    double xxLWoLwWGU99582333 = -501733517;    double xxLWoLwWGU65232696 = -985635796;    double xxLWoLwWGU98803138 = 32611146;    double xxLWoLwWGU95297026 = -293508716;    double xxLWoLwWGU98525612 = -192747804;    double xxLWoLwWGU35752801 = -195140333;    double xxLWoLwWGU97363280 = -953737205;    double xxLWoLwWGU70631297 = -291049949;    double xxLWoLwWGU76658383 = -288076954;    double xxLWoLwWGU97276731 = -576128450;    double xxLWoLwWGU67647270 = -157501709;    double xxLWoLwWGU19674286 = -282033344;    double xxLWoLwWGU63955153 = -770367485;    double xxLWoLwWGU60915096 = -884432159;    double xxLWoLwWGU56387689 = -652019062;    double xxLWoLwWGU47951109 = -584528324;    double xxLWoLwWGU35733698 = -426002635;    double xxLWoLwWGU49819439 = -556154441;    double xxLWoLwWGU18730378 = -779715290;    double xxLWoLwWGU36533460 = -900140645;    double xxLWoLwWGU76564832 = -5249947;    double xxLWoLwWGU14980743 = -508121218;    double xxLWoLwWGU98824637 = -602499115;    double xxLWoLwWGU2823092 = -860164907;    double xxLWoLwWGU38609780 = -38828792;     xxLWoLwWGU6863610 = xxLWoLwWGU14285757;     xxLWoLwWGU14285757 = xxLWoLwWGU62545784;     xxLWoLwWGU62545784 = xxLWoLwWGU61623057;     xxLWoLwWGU61623057 = xxLWoLwWGU12865921;     xxLWoLwWGU12865921 = xxLWoLwWGU11837192;     xxLWoLwWGU11837192 = xxLWoLwWGU36755657;     xxLWoLwWGU36755657 = xxLWoLwWGU55832311;     xxLWoLwWGU55832311 = xxLWoLwWGU9141117;     xxLWoLwWGU9141117 = xxLWoLwWGU59311020;     xxLWoLwWGU59311020 = xxLWoLwWGU10980312;     xxLWoLwWGU10980312 = xxLWoLwWGU56704560;     xxLWoLwWGU56704560 = xxLWoLwWGU76201363;     xxLWoLwWGU76201363 = xxLWoLwWGU98622245;     xxLWoLwWGU98622245 = xxLWoLwWGU16043030;     xxLWoLwWGU16043030 = xxLWoLwWGU75667180;     xxLWoLwWGU75667180 = xxLWoLwWGU52189639;     xxLWoLwWGU52189639 = xxLWoLwWGU56393939;     xxLWoLwWGU56393939 = xxLWoLwWGU71683936;     xxLWoLwWGU71683936 = xxLWoLwWGU38859571;     xxLWoLwWGU38859571 = xxLWoLwWGU98191364;     xxLWoLwWGU98191364 = xxLWoLwWGU59020341;     xxLWoLwWGU59020341 = xxLWoLwWGU96992228;     xxLWoLwWGU96992228 = xxLWoLwWGU46009793;     xxLWoLwWGU46009793 = xxLWoLwWGU57727386;     xxLWoLwWGU57727386 = xxLWoLwWGU53239172;     xxLWoLwWGU53239172 = xxLWoLwWGU83347217;     xxLWoLwWGU83347217 = xxLWoLwWGU37994181;     xxLWoLwWGU37994181 = xxLWoLwWGU72204022;     xxLWoLwWGU72204022 = xxLWoLwWGU14178110;     xxLWoLwWGU14178110 = xxLWoLwWGU63945022;     xxLWoLwWGU63945022 = xxLWoLwWGU98877325;     xxLWoLwWGU98877325 = xxLWoLwWGU84881987;     xxLWoLwWGU84881987 = xxLWoLwWGU76899633;     xxLWoLwWGU76899633 = xxLWoLwWGU4230958;     xxLWoLwWGU4230958 = xxLWoLwWGU65244637;     xxLWoLwWGU65244637 = xxLWoLwWGU52269689;     xxLWoLwWGU52269689 = xxLWoLwWGU53715110;     xxLWoLwWGU53715110 = xxLWoLwWGU78357127;     xxLWoLwWGU78357127 = xxLWoLwWGU65023433;     xxLWoLwWGU65023433 = xxLWoLwWGU75326307;     xxLWoLwWGU75326307 = xxLWoLwWGU98609030;     xxLWoLwWGU98609030 = xxLWoLwWGU93787644;     xxLWoLwWGU93787644 = xxLWoLwWGU98189089;     xxLWoLwWGU98189089 = xxLWoLwWGU50712766;     xxLWoLwWGU50712766 = xxLWoLwWGU59201774;     xxLWoLwWGU59201774 = xxLWoLwWGU17486372;     xxLWoLwWGU17486372 = xxLWoLwWGU85983937;     xxLWoLwWGU85983937 = xxLWoLwWGU67362884;     xxLWoLwWGU67362884 = xxLWoLwWGU95545639;     xxLWoLwWGU95545639 = xxLWoLwWGU16901379;     xxLWoLwWGU16901379 = xxLWoLwWGU96297752;     xxLWoLwWGU96297752 = xxLWoLwWGU79203039;     xxLWoLwWGU79203039 = xxLWoLwWGU20926834;     xxLWoLwWGU20926834 = xxLWoLwWGU15984537;     xxLWoLwWGU15984537 = xxLWoLwWGU47843268;     xxLWoLwWGU47843268 = xxLWoLwWGU17293529;     xxLWoLwWGU17293529 = xxLWoLwWGU16535991;     xxLWoLwWGU16535991 = xxLWoLwWGU3895671;     xxLWoLwWGU3895671 = xxLWoLwWGU59626749;     xxLWoLwWGU59626749 = xxLWoLwWGU28489974;     xxLWoLwWGU28489974 = xxLWoLwWGU98761475;     xxLWoLwWGU98761475 = xxLWoLwWGU83628288;     xxLWoLwWGU83628288 = xxLWoLwWGU94963006;     xxLWoLwWGU94963006 = xxLWoLwWGU95365997;     xxLWoLwWGU95365997 = xxLWoLwWGU12102987;     xxLWoLwWGU12102987 = xxLWoLwWGU71822573;     xxLWoLwWGU71822573 = xxLWoLwWGU99301730;     xxLWoLwWGU99301730 = xxLWoLwWGU94391287;     xxLWoLwWGU94391287 = xxLWoLwWGU50798392;     xxLWoLwWGU50798392 = xxLWoLwWGU23397492;     xxLWoLwWGU23397492 = xxLWoLwWGU98474529;     xxLWoLwWGU98474529 = xxLWoLwWGU78036811;     xxLWoLwWGU78036811 = xxLWoLwWGU6660503;     xxLWoLwWGU6660503 = xxLWoLwWGU63533264;     xxLWoLwWGU63533264 = xxLWoLwWGU99582333;     xxLWoLwWGU99582333 = xxLWoLwWGU65232696;     xxLWoLwWGU65232696 = xxLWoLwWGU98803138;     xxLWoLwWGU98803138 = xxLWoLwWGU95297026;     xxLWoLwWGU95297026 = xxLWoLwWGU98525612;     xxLWoLwWGU98525612 = xxLWoLwWGU35752801;     xxLWoLwWGU35752801 = xxLWoLwWGU97363280;     xxLWoLwWGU97363280 = xxLWoLwWGU70631297;     xxLWoLwWGU70631297 = xxLWoLwWGU76658383;     xxLWoLwWGU76658383 = xxLWoLwWGU97276731;     xxLWoLwWGU97276731 = xxLWoLwWGU67647270;     xxLWoLwWGU67647270 = xxLWoLwWGU19674286;     xxLWoLwWGU19674286 = xxLWoLwWGU63955153;     xxLWoLwWGU63955153 = xxLWoLwWGU60915096;     xxLWoLwWGU60915096 = xxLWoLwWGU56387689;     xxLWoLwWGU56387689 = xxLWoLwWGU47951109;     xxLWoLwWGU47951109 = xxLWoLwWGU35733698;     xxLWoLwWGU35733698 = xxLWoLwWGU49819439;     xxLWoLwWGU49819439 = xxLWoLwWGU18730378;     xxLWoLwWGU18730378 = xxLWoLwWGU36533460;     xxLWoLwWGU36533460 = xxLWoLwWGU76564832;     xxLWoLwWGU76564832 = xxLWoLwWGU14980743;     xxLWoLwWGU14980743 = xxLWoLwWGU98824637;     xxLWoLwWGU98824637 = xxLWoLwWGU2823092;     xxLWoLwWGU2823092 = xxLWoLwWGU38609780;     xxLWoLwWGU38609780 = xxLWoLwWGU6863610;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WZiyCyEZQb29555167() {     double IVitYXPSEw58319847 = -685727238;    double IVitYXPSEw49233733 = 27400959;    double IVitYXPSEw19168534 = -815057773;    double IVitYXPSEw2342758 = -822977638;    double IVitYXPSEw16149842 = -422940333;    double IVitYXPSEw23885430 = -632082870;    double IVitYXPSEw44691255 = -340738429;    double IVitYXPSEw45631149 = -792515085;    double IVitYXPSEw68749366 = -903496091;    double IVitYXPSEw14548208 = -506556844;    double IVitYXPSEw12799913 = -402836798;    double IVitYXPSEw40054567 = -790066191;    double IVitYXPSEw30446081 = -324380652;    double IVitYXPSEw42643712 = -6217406;    double IVitYXPSEw26733492 = -383439953;    double IVitYXPSEw1969586 = -233241997;    double IVitYXPSEw2046638 = -653127897;    double IVitYXPSEw8851038 = -922360843;    double IVitYXPSEw52721446 = -663390787;    double IVitYXPSEw73848943 = -218655215;    double IVitYXPSEw32239594 = -81154968;    double IVitYXPSEw72585042 = -64935875;    double IVitYXPSEw32991586 = -196478936;    double IVitYXPSEw72223906 = -986145115;    double IVitYXPSEw34117110 = -295656323;    double IVitYXPSEw53293977 = -918850733;    double IVitYXPSEw50038653 = -111411646;    double IVitYXPSEw33255832 = 85526691;    double IVitYXPSEw11117048 = -716810676;    double IVitYXPSEw64448208 = -345491527;    double IVitYXPSEw37894721 = -920924144;    double IVitYXPSEw50220194 = 40716595;    double IVitYXPSEw12389483 = -582259456;    double IVitYXPSEw77357142 = -324542976;    double IVitYXPSEw13559491 = -457573241;    double IVitYXPSEw89472198 = -852546502;    double IVitYXPSEw70768503 = -39385694;    double IVitYXPSEw44745763 = -298104809;    double IVitYXPSEw36511329 = -175043714;    double IVitYXPSEw411870 = -557213605;    double IVitYXPSEw12891799 = -497418011;    double IVitYXPSEw79782490 = -337953047;    double IVitYXPSEw41137800 = -627252108;    double IVitYXPSEw35834329 = -612985890;    double IVitYXPSEw86699220 = -191091663;    double IVitYXPSEw56235335 = -404403258;    double IVitYXPSEw46489026 = -313556710;    double IVitYXPSEw45897150 = -52061321;    double IVitYXPSEw53354236 = -806349939;    double IVitYXPSEw83661052 = -374541979;    double IVitYXPSEw90747887 = -230851981;    double IVitYXPSEw32272579 = -579686875;    double IVitYXPSEw71171842 = -931406151;    double IVitYXPSEw69730228 = -30355511;    double IVitYXPSEw97315572 = -553530509;    double IVitYXPSEw85734805 = -520791364;    double IVitYXPSEw16242148 = -776120106;    double IVitYXPSEw46944627 = -828912659;    double IVitYXPSEw68225647 = -427321315;    double IVitYXPSEw62855864 = -504089600;    double IVitYXPSEw73846776 = -420671224;    double IVitYXPSEw11435424 = -326265120;    double IVitYXPSEw34514101 = 24295590;    double IVitYXPSEw4301159 = -458004565;    double IVitYXPSEw76653486 = -585632700;    double IVitYXPSEw62579718 = -343553393;    double IVitYXPSEw27665085 = -107806736;    double IVitYXPSEw53088939 = -999837676;    double IVitYXPSEw29084221 = -548644165;    double IVitYXPSEw37261294 = -530893451;    double IVitYXPSEw31201083 = -93856304;    double IVitYXPSEw57300875 = -255023089;    double IVitYXPSEw72339709 = -647317130;    double IVitYXPSEw52309577 = -6177182;    double IVitYXPSEw60957145 = -721237204;    double IVitYXPSEw52457104 = -743201922;    double IVitYXPSEw31447242 = -437683768;    double IVitYXPSEw97157256 = -583493046;    double IVitYXPSEw85524686 = -695053452;    double IVitYXPSEw77881774 = -891253065;    double IVitYXPSEw6804951 = -505294024;    double IVitYXPSEw4141504 = 40649675;    double IVitYXPSEw79901596 = -108123370;    double IVitYXPSEw27455996 = -242268698;    double IVitYXPSEw73700320 = -14639546;    double IVitYXPSEw5622142 = -241237270;    double IVitYXPSEw79048351 = -27877254;    double IVitYXPSEw42659255 = -451903945;    double IVitYXPSEw80041569 = -771012468;    double IVitYXPSEw27824686 = -936781878;    double IVitYXPSEw73230050 = 23573603;    double IVitYXPSEw23823877 = -210473035;    double IVitYXPSEw76520116 = -870783494;    double IVitYXPSEw73655464 = -670954115;    double IVitYXPSEw26565093 = -36542381;    double IVitYXPSEw1456375 = -71152892;    double IVitYXPSEw45268389 = -262248638;    double IVitYXPSEw36836642 = -69247543;    double IVitYXPSEw59180843 = 72646810;    double IVitYXPSEw24119502 = -685727238;     IVitYXPSEw58319847 = IVitYXPSEw49233733;     IVitYXPSEw49233733 = IVitYXPSEw19168534;     IVitYXPSEw19168534 = IVitYXPSEw2342758;     IVitYXPSEw2342758 = IVitYXPSEw16149842;     IVitYXPSEw16149842 = IVitYXPSEw23885430;     IVitYXPSEw23885430 = IVitYXPSEw44691255;     IVitYXPSEw44691255 = IVitYXPSEw45631149;     IVitYXPSEw45631149 = IVitYXPSEw68749366;     IVitYXPSEw68749366 = IVitYXPSEw14548208;     IVitYXPSEw14548208 = IVitYXPSEw12799913;     IVitYXPSEw12799913 = IVitYXPSEw40054567;     IVitYXPSEw40054567 = IVitYXPSEw30446081;     IVitYXPSEw30446081 = IVitYXPSEw42643712;     IVitYXPSEw42643712 = IVitYXPSEw26733492;     IVitYXPSEw26733492 = IVitYXPSEw1969586;     IVitYXPSEw1969586 = IVitYXPSEw2046638;     IVitYXPSEw2046638 = IVitYXPSEw8851038;     IVitYXPSEw8851038 = IVitYXPSEw52721446;     IVitYXPSEw52721446 = IVitYXPSEw73848943;     IVitYXPSEw73848943 = IVitYXPSEw32239594;     IVitYXPSEw32239594 = IVitYXPSEw72585042;     IVitYXPSEw72585042 = IVitYXPSEw32991586;     IVitYXPSEw32991586 = IVitYXPSEw72223906;     IVitYXPSEw72223906 = IVitYXPSEw34117110;     IVitYXPSEw34117110 = IVitYXPSEw53293977;     IVitYXPSEw53293977 = IVitYXPSEw50038653;     IVitYXPSEw50038653 = IVitYXPSEw33255832;     IVitYXPSEw33255832 = IVitYXPSEw11117048;     IVitYXPSEw11117048 = IVitYXPSEw64448208;     IVitYXPSEw64448208 = IVitYXPSEw37894721;     IVitYXPSEw37894721 = IVitYXPSEw50220194;     IVitYXPSEw50220194 = IVitYXPSEw12389483;     IVitYXPSEw12389483 = IVitYXPSEw77357142;     IVitYXPSEw77357142 = IVitYXPSEw13559491;     IVitYXPSEw13559491 = IVitYXPSEw89472198;     IVitYXPSEw89472198 = IVitYXPSEw70768503;     IVitYXPSEw70768503 = IVitYXPSEw44745763;     IVitYXPSEw44745763 = IVitYXPSEw36511329;     IVitYXPSEw36511329 = IVitYXPSEw411870;     IVitYXPSEw411870 = IVitYXPSEw12891799;     IVitYXPSEw12891799 = IVitYXPSEw79782490;     IVitYXPSEw79782490 = IVitYXPSEw41137800;     IVitYXPSEw41137800 = IVitYXPSEw35834329;     IVitYXPSEw35834329 = IVitYXPSEw86699220;     IVitYXPSEw86699220 = IVitYXPSEw56235335;     IVitYXPSEw56235335 = IVitYXPSEw46489026;     IVitYXPSEw46489026 = IVitYXPSEw45897150;     IVitYXPSEw45897150 = IVitYXPSEw53354236;     IVitYXPSEw53354236 = IVitYXPSEw83661052;     IVitYXPSEw83661052 = IVitYXPSEw90747887;     IVitYXPSEw90747887 = IVitYXPSEw32272579;     IVitYXPSEw32272579 = IVitYXPSEw71171842;     IVitYXPSEw71171842 = IVitYXPSEw69730228;     IVitYXPSEw69730228 = IVitYXPSEw97315572;     IVitYXPSEw97315572 = IVitYXPSEw85734805;     IVitYXPSEw85734805 = IVitYXPSEw16242148;     IVitYXPSEw16242148 = IVitYXPSEw46944627;     IVitYXPSEw46944627 = IVitYXPSEw68225647;     IVitYXPSEw68225647 = IVitYXPSEw62855864;     IVitYXPSEw62855864 = IVitYXPSEw73846776;     IVitYXPSEw73846776 = IVitYXPSEw11435424;     IVitYXPSEw11435424 = IVitYXPSEw34514101;     IVitYXPSEw34514101 = IVitYXPSEw4301159;     IVitYXPSEw4301159 = IVitYXPSEw76653486;     IVitYXPSEw76653486 = IVitYXPSEw62579718;     IVitYXPSEw62579718 = IVitYXPSEw27665085;     IVitYXPSEw27665085 = IVitYXPSEw53088939;     IVitYXPSEw53088939 = IVitYXPSEw29084221;     IVitYXPSEw29084221 = IVitYXPSEw37261294;     IVitYXPSEw37261294 = IVitYXPSEw31201083;     IVitYXPSEw31201083 = IVitYXPSEw57300875;     IVitYXPSEw57300875 = IVitYXPSEw72339709;     IVitYXPSEw72339709 = IVitYXPSEw52309577;     IVitYXPSEw52309577 = IVitYXPSEw60957145;     IVitYXPSEw60957145 = IVitYXPSEw52457104;     IVitYXPSEw52457104 = IVitYXPSEw31447242;     IVitYXPSEw31447242 = IVitYXPSEw97157256;     IVitYXPSEw97157256 = IVitYXPSEw85524686;     IVitYXPSEw85524686 = IVitYXPSEw77881774;     IVitYXPSEw77881774 = IVitYXPSEw6804951;     IVitYXPSEw6804951 = IVitYXPSEw4141504;     IVitYXPSEw4141504 = IVitYXPSEw79901596;     IVitYXPSEw79901596 = IVitYXPSEw27455996;     IVitYXPSEw27455996 = IVitYXPSEw73700320;     IVitYXPSEw73700320 = IVitYXPSEw5622142;     IVitYXPSEw5622142 = IVitYXPSEw79048351;     IVitYXPSEw79048351 = IVitYXPSEw42659255;     IVitYXPSEw42659255 = IVitYXPSEw80041569;     IVitYXPSEw80041569 = IVitYXPSEw27824686;     IVitYXPSEw27824686 = IVitYXPSEw73230050;     IVitYXPSEw73230050 = IVitYXPSEw23823877;     IVitYXPSEw23823877 = IVitYXPSEw76520116;     IVitYXPSEw76520116 = IVitYXPSEw73655464;     IVitYXPSEw73655464 = IVitYXPSEw26565093;     IVitYXPSEw26565093 = IVitYXPSEw1456375;     IVitYXPSEw1456375 = IVitYXPSEw45268389;     IVitYXPSEw45268389 = IVitYXPSEw36836642;     IVitYXPSEw36836642 = IVitYXPSEw59180843;     IVitYXPSEw59180843 = IVitYXPSEw24119502;     IVitYXPSEw24119502 = IVitYXPSEw58319847;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void savZPIKxqM66530338() {     double WOAkgRaNNZ26046096 = -855127757;    double WOAkgRaNNZ25155931 = -484172401;    double WOAkgRaNNZ88518377 = -626455618;    double WOAkgRaNNZ30994365 = -643823592;    double WOAkgRaNNZ48712527 = -709917408;    double WOAkgRaNNZ59399582 = -62973224;    double WOAkgRaNNZ84293626 = -665369529;    double WOAkgRaNNZ61303955 = -466720799;    double WOAkgRaNNZ60878250 = -454484829;    double WOAkgRaNNZ20319320 = -707303779;    double WOAkgRaNNZ92051082 = -833576697;    double WOAkgRaNNZ47484479 = -589865749;    double WOAkgRaNNZ65489861 = -61216142;    double WOAkgRaNNZ46136253 = -166975635;    double WOAkgRaNNZ78416804 = -66508868;    double WOAkgRaNNZ31022509 = -217545626;    double WOAkgRaNNZ7890986 = -276422459;    double WOAkgRaNNZ70231400 = -607610684;    double WOAkgRaNNZ13210746 = -30663632;    double WOAkgRaNNZ6804435 = -737947973;    double WOAkgRaNNZ78617737 = -315025494;    double WOAkgRaNNZ63443937 = -913249963;    double WOAkgRaNNZ36693473 = -244773949;    double WOAkgRaNNZ40945433 = -632017678;    double WOAkgRaNNZ2347431 = 29004163;    double WOAkgRaNNZ16930101 = -167242105;    double WOAkgRaNNZ65359731 = -714953573;    double WOAkgRaNNZ21321807 = -303842884;    double WOAkgRaNNZ90790378 = -226893250;    double WOAkgRaNNZ55259848 = -750460799;    double WOAkgRaNNZ63892676 = -840461005;    double WOAkgRaNNZ61493632 = -591679996;    double WOAkgRaNNZ11224700 = -757029556;    double WOAkgRaNNZ42372774 = -845230749;    double WOAkgRaNNZ37642737 = -562780332;    double WOAkgRaNNZ86666714 = -999712047;    double WOAkgRaNNZ1445096 = -21560403;    double WOAkgRaNNZ55312304 = -287063936;    double WOAkgRaNNZ69363407 = -906822573;    double WOAkgRaNNZ68484082 = -644560721;    double WOAkgRaNNZ73545436 = -975989418;    double WOAkgRaNNZ17313885 = -782121754;    double WOAkgRaNNZ15971170 = -133407630;    double WOAkgRaNNZ85515947 = -693634317;    double WOAkgRaNNZ78716476 = -340976945;    double WOAkgRaNNZ49429984 = -329887428;    double WOAkgRaNNZ70010505 = -246190201;    double WOAkgRaNNZ42452097 = -439705297;    double WOAkgRaNNZ24129764 = -420100335;    double WOAkgRaNNZ55948559 = -963567824;    double WOAkgRaNNZ37762934 = -507267764;    double WOAkgRaNNZ31288944 = -877585926;    double WOAkgRaNNZ93206591 = -755455929;    double WOAkgRaNNZ82851286 = -942069687;    double WOAkgRaNNZ87912600 = -55225226;    double WOAkgRaNNZ62602159 = -941877794;    double WOAkgRaNNZ88462457 = -139398453;    double WOAkgRaNNZ47572945 = -994437940;    double WOAkgRaNNZ28646935 = -572827756;    double WOAkgRaNNZ31782426 = -442675303;    double WOAkgRaNNZ94039850 = -348019651;    double WOAkgRaNNZ62971819 = -261526646;    double WOAkgRaNNZ70513576 = -139827549;    double WOAkgRaNNZ5618403 = -704024031;    double WOAkgRaNNZ56426644 = -866842774;    double WOAkgRaNNZ30557451 = -141896701;    double WOAkgRaNNZ36259780 = -832836194;    double WOAkgRaNNZ23117087 = -215985393;    double WOAkgRaNNZ8493517 = -604195304;    double WOAkgRaNNZ91750089 = -66796822;    double WOAkgRaNNZ29577414 = -95985223;    double WOAkgRaNNZ52578681 = -989358524;    double WOAkgRaNNZ867993 = -700788112;    double WOAkgRaNNZ44726664 = -386102911;    double WOAkgRaNNZ33258998 = -761958556;    double WOAkgRaNNZ61303853 = -532903741;    double WOAkgRaNNZ47472768 = -679842334;    double WOAkgRaNNZ51177526 = -551139632;    double WOAkgRaNNZ62228957 = -191040733;    double WOAkgRaNNZ52917447 = -641108409;    double WOAkgRaNNZ46919596 = -921051904;    double WOAkgRaNNZ22907635 = -175248276;    double WOAkgRaNNZ97192043 = -883742550;    double WOAkgRaNNZ34841820 = -263325426;    double WOAkgRaNNZ17496915 = -143193036;    double WOAkgRaNNZ32603732 = -962875079;    double WOAkgRaNNZ68287041 = -836224067;    double WOAkgRaNNZ28373413 = -814959870;    double WOAkgRaNNZ54460173 = -690005524;    double WOAkgRaNNZ75040578 = -620902539;    double WOAkgRaNNZ98204256 = -760313595;    double WOAkgRaNNZ53872151 = -27122464;    double WOAkgRaNNZ26665370 = -714236180;    double WOAkgRaNNZ37580982 = -364147270;    double WOAkgRaNNZ74444232 = -196541071;    double WOAkgRaNNZ10573617 = -614462773;    double WOAkgRaNNZ46800308 = -542294206;    double WOAkgRaNNZ10352768 = -429383600;    double WOAkgRaNNZ29089304 = -826791543;    double WOAkgRaNNZ48159026 = -855127757;     WOAkgRaNNZ26046096 = WOAkgRaNNZ25155931;     WOAkgRaNNZ25155931 = WOAkgRaNNZ88518377;     WOAkgRaNNZ88518377 = WOAkgRaNNZ30994365;     WOAkgRaNNZ30994365 = WOAkgRaNNZ48712527;     WOAkgRaNNZ48712527 = WOAkgRaNNZ59399582;     WOAkgRaNNZ59399582 = WOAkgRaNNZ84293626;     WOAkgRaNNZ84293626 = WOAkgRaNNZ61303955;     WOAkgRaNNZ61303955 = WOAkgRaNNZ60878250;     WOAkgRaNNZ60878250 = WOAkgRaNNZ20319320;     WOAkgRaNNZ20319320 = WOAkgRaNNZ92051082;     WOAkgRaNNZ92051082 = WOAkgRaNNZ47484479;     WOAkgRaNNZ47484479 = WOAkgRaNNZ65489861;     WOAkgRaNNZ65489861 = WOAkgRaNNZ46136253;     WOAkgRaNNZ46136253 = WOAkgRaNNZ78416804;     WOAkgRaNNZ78416804 = WOAkgRaNNZ31022509;     WOAkgRaNNZ31022509 = WOAkgRaNNZ7890986;     WOAkgRaNNZ7890986 = WOAkgRaNNZ70231400;     WOAkgRaNNZ70231400 = WOAkgRaNNZ13210746;     WOAkgRaNNZ13210746 = WOAkgRaNNZ6804435;     WOAkgRaNNZ6804435 = WOAkgRaNNZ78617737;     WOAkgRaNNZ78617737 = WOAkgRaNNZ63443937;     WOAkgRaNNZ63443937 = WOAkgRaNNZ36693473;     WOAkgRaNNZ36693473 = WOAkgRaNNZ40945433;     WOAkgRaNNZ40945433 = WOAkgRaNNZ2347431;     WOAkgRaNNZ2347431 = WOAkgRaNNZ16930101;     WOAkgRaNNZ16930101 = WOAkgRaNNZ65359731;     WOAkgRaNNZ65359731 = WOAkgRaNNZ21321807;     WOAkgRaNNZ21321807 = WOAkgRaNNZ90790378;     WOAkgRaNNZ90790378 = WOAkgRaNNZ55259848;     WOAkgRaNNZ55259848 = WOAkgRaNNZ63892676;     WOAkgRaNNZ63892676 = WOAkgRaNNZ61493632;     WOAkgRaNNZ61493632 = WOAkgRaNNZ11224700;     WOAkgRaNNZ11224700 = WOAkgRaNNZ42372774;     WOAkgRaNNZ42372774 = WOAkgRaNNZ37642737;     WOAkgRaNNZ37642737 = WOAkgRaNNZ86666714;     WOAkgRaNNZ86666714 = WOAkgRaNNZ1445096;     WOAkgRaNNZ1445096 = WOAkgRaNNZ55312304;     WOAkgRaNNZ55312304 = WOAkgRaNNZ69363407;     WOAkgRaNNZ69363407 = WOAkgRaNNZ68484082;     WOAkgRaNNZ68484082 = WOAkgRaNNZ73545436;     WOAkgRaNNZ73545436 = WOAkgRaNNZ17313885;     WOAkgRaNNZ17313885 = WOAkgRaNNZ15971170;     WOAkgRaNNZ15971170 = WOAkgRaNNZ85515947;     WOAkgRaNNZ85515947 = WOAkgRaNNZ78716476;     WOAkgRaNNZ78716476 = WOAkgRaNNZ49429984;     WOAkgRaNNZ49429984 = WOAkgRaNNZ70010505;     WOAkgRaNNZ70010505 = WOAkgRaNNZ42452097;     WOAkgRaNNZ42452097 = WOAkgRaNNZ24129764;     WOAkgRaNNZ24129764 = WOAkgRaNNZ55948559;     WOAkgRaNNZ55948559 = WOAkgRaNNZ37762934;     WOAkgRaNNZ37762934 = WOAkgRaNNZ31288944;     WOAkgRaNNZ31288944 = WOAkgRaNNZ93206591;     WOAkgRaNNZ93206591 = WOAkgRaNNZ82851286;     WOAkgRaNNZ82851286 = WOAkgRaNNZ87912600;     WOAkgRaNNZ87912600 = WOAkgRaNNZ62602159;     WOAkgRaNNZ62602159 = WOAkgRaNNZ88462457;     WOAkgRaNNZ88462457 = WOAkgRaNNZ47572945;     WOAkgRaNNZ47572945 = WOAkgRaNNZ28646935;     WOAkgRaNNZ28646935 = WOAkgRaNNZ31782426;     WOAkgRaNNZ31782426 = WOAkgRaNNZ94039850;     WOAkgRaNNZ94039850 = WOAkgRaNNZ62971819;     WOAkgRaNNZ62971819 = WOAkgRaNNZ70513576;     WOAkgRaNNZ70513576 = WOAkgRaNNZ5618403;     WOAkgRaNNZ5618403 = WOAkgRaNNZ56426644;     WOAkgRaNNZ56426644 = WOAkgRaNNZ30557451;     WOAkgRaNNZ30557451 = WOAkgRaNNZ36259780;     WOAkgRaNNZ36259780 = WOAkgRaNNZ23117087;     WOAkgRaNNZ23117087 = WOAkgRaNNZ8493517;     WOAkgRaNNZ8493517 = WOAkgRaNNZ91750089;     WOAkgRaNNZ91750089 = WOAkgRaNNZ29577414;     WOAkgRaNNZ29577414 = WOAkgRaNNZ52578681;     WOAkgRaNNZ52578681 = WOAkgRaNNZ867993;     WOAkgRaNNZ867993 = WOAkgRaNNZ44726664;     WOAkgRaNNZ44726664 = WOAkgRaNNZ33258998;     WOAkgRaNNZ33258998 = WOAkgRaNNZ61303853;     WOAkgRaNNZ61303853 = WOAkgRaNNZ47472768;     WOAkgRaNNZ47472768 = WOAkgRaNNZ51177526;     WOAkgRaNNZ51177526 = WOAkgRaNNZ62228957;     WOAkgRaNNZ62228957 = WOAkgRaNNZ52917447;     WOAkgRaNNZ52917447 = WOAkgRaNNZ46919596;     WOAkgRaNNZ46919596 = WOAkgRaNNZ22907635;     WOAkgRaNNZ22907635 = WOAkgRaNNZ97192043;     WOAkgRaNNZ97192043 = WOAkgRaNNZ34841820;     WOAkgRaNNZ34841820 = WOAkgRaNNZ17496915;     WOAkgRaNNZ17496915 = WOAkgRaNNZ32603732;     WOAkgRaNNZ32603732 = WOAkgRaNNZ68287041;     WOAkgRaNNZ68287041 = WOAkgRaNNZ28373413;     WOAkgRaNNZ28373413 = WOAkgRaNNZ54460173;     WOAkgRaNNZ54460173 = WOAkgRaNNZ75040578;     WOAkgRaNNZ75040578 = WOAkgRaNNZ98204256;     WOAkgRaNNZ98204256 = WOAkgRaNNZ53872151;     WOAkgRaNNZ53872151 = WOAkgRaNNZ26665370;     WOAkgRaNNZ26665370 = WOAkgRaNNZ37580982;     WOAkgRaNNZ37580982 = WOAkgRaNNZ74444232;     WOAkgRaNNZ74444232 = WOAkgRaNNZ10573617;     WOAkgRaNNZ10573617 = WOAkgRaNNZ46800308;     WOAkgRaNNZ46800308 = WOAkgRaNNZ10352768;     WOAkgRaNNZ10352768 = WOAkgRaNNZ29089304;     WOAkgRaNNZ29089304 = WOAkgRaNNZ48159026;     WOAkgRaNNZ48159026 = WOAkgRaNNZ26046096;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MdHoiaLBje51262979() {     double bUJrgYndrj64430483 = -670153268;    double bUJrgYndrj31179330 = -824793947;    double bUJrgYndrj36523929 = -923019308;    double bUJrgYndrj14497399 = -834267098;    double bUJrgYndrj93885431 = -311945525;    double bUJrgYndrj50108064 = -596011917;    double bUJrgYndrj73727557 = -184239873;    double bUJrgYndrj64605090 = -783566315;    double bUJrgYndrj16132026 = -385174827;    double bUJrgYndrj52417380 = -472339193;    double bUJrgYndrj93230807 = -239746456;    double bUJrgYndrj86169286 = -969950756;    double bUJrgYndrj48508114 = -785930215;    double bUJrgYndrj79631608 = -883609025;    double bUJrgYndrj67648027 = 63385028;    double bUJrgYndrj18039527 = -539037191;    double bUJrgYndrj51735401 = 10712420;    double bUJrgYndrj28904724 = -984955108;    double bUJrgYndrj34979558 = -587142151;    double bUJrgYndrj26472871 = -87582585;    double bUJrgYndrj462212 = -747108798;    double bUJrgYndrj37402331 = -482857474;    double bUJrgYndrj5460774 = -401954265;    double bUJrgYndrj99866388 = 10629375;    double bUJrgYndrj6259450 = 4453791;    double bUJrgYndrj94589729 = -705711985;    double bUJrgYndrj18720014 = -312873562;    double bUJrgYndrj81766481 = -589369251;    double bUJrgYndrj89927750 = -485263695;    double bUJrgYndrj9153715 = -938393768;    double bUJrgYndrj63549949 = -121338317;    double bUJrgYndrj32554673 = -60551983;    double bUJrgYndrj7552855 = -676817043;    double bUJrgYndrj72470543 = 47233526;    double bUJrgYndrj57392181 = -255620271;    double bUJrgYndrj1026301 = -46338356;    double bUJrgYndrj63971805 = -92674942;    double bUJrgYndrj48753655 = -64873497;    double bUJrgYndrj45435179 = -881648627;    double bUJrgYndrj59786700 = -640084514;    double bUJrgYndrj6416690 = -650635804;    double bUJrgYndrj97630608 = -801887408;    double bUJrgYndrj79488138 = -167186946;    double bUJrgYndrj82473968 = -189293272;    double bUJrgYndrj82669737 = -326249219;    double bUJrgYndrj8602499 = -201664045;    double bUJrgYndrj91965393 = -913612870;    double bUJrgYndrj29237672 = -670718210;    double bUJrgYndrj81688354 = -455850789;    double bUJrgYndrj16618128 = -48577673;    double bUJrgYndrj39707935 = -751700660;    double bUJrgYndrj57629556 = -762558943;    double bUJrgYndrj94868647 = -615249747;    double bUJrgYndrj75949379 = -882745508;    double bUJrgYndrj50800767 = 89038431;    double bUJrgYndrj27028152 = -87295794;    double bUJrgYndrj25718556 = -322839683;    double bUJrgYndrj36657541 = -833648684;    double bUJrgYndrj8237949 = -738720890;    double bUJrgYndrj99295702 = -606233541;    double bUJrgYndrj31388051 = -183138355;    double bUJrgYndrj91961075 = -594870622;    double bUJrgYndrj74677340 = -198302621;    double bUJrgYndrj6978311 = -446781060;    double bUJrgYndrj88867430 = -251000876;    double bUJrgYndrj60676135 = -79194474;    double bUJrgYndrj78616432 = -193133714;    double bUJrgYndrj76037570 = -733163742;    double bUJrgYndrj22239428 = -527988755;    double bUJrgYndrj66621726 = -890276616;    double bUJrgYndrj54067721 = -346362250;    double bUJrgYndrj2981746 = -924414083;    double bUJrgYndrj83469544 = -3306481;    double bUJrgYndrj75192858 = -947057637;    double bUJrgYndrj20056182 = -436946782;    double bUJrgYndrj2831604 = -945221390;    double bUJrgYndrj57914193 = -215670529;    double bUJrgYndrj22986806 = -112660993;    double bUJrgYndrj17196651 = -663121407;    double bUJrgYndrj97656951 = -793882164;    double bUJrgYndrj2624337 = -792099116;    double bUJrgYndrj89482341 = -642155353;    double bUJrgYndrj78128 = -33518462;    double bUJrgYndrj73309622 = -336686023;    double bUJrgYndrj69445779 = -86693109;    double bUJrgYndrj5920394 = -358779374;    double bUJrgYndrj37686025 = -445302236;    double bUJrgYndrj31603475 = -794071535;    double bUJrgYndrj21669776 = 58195095;    double bUJrgYndrj30364030 = -68324477;    double bUJrgYndrj75307745 = -723498674;    double bUJrgYndrj27314265 = -259026259;    double bUJrgYndrj40515706 = -326152608;    double bUJrgYndrj46139476 = -175415087;    double bUJrgYndrj28398649 = -356946159;    double bUJrgYndrj14455615 = 44234818;    double bUJrgYndrj22953268 = -503584787;    double bUJrgYndrj72509827 = -720405886;    double bUJrgYndrj93606537 = -938292396;    double bUJrgYndrj21993603 = -670153268;     bUJrgYndrj64430483 = bUJrgYndrj31179330;     bUJrgYndrj31179330 = bUJrgYndrj36523929;     bUJrgYndrj36523929 = bUJrgYndrj14497399;     bUJrgYndrj14497399 = bUJrgYndrj93885431;     bUJrgYndrj93885431 = bUJrgYndrj50108064;     bUJrgYndrj50108064 = bUJrgYndrj73727557;     bUJrgYndrj73727557 = bUJrgYndrj64605090;     bUJrgYndrj64605090 = bUJrgYndrj16132026;     bUJrgYndrj16132026 = bUJrgYndrj52417380;     bUJrgYndrj52417380 = bUJrgYndrj93230807;     bUJrgYndrj93230807 = bUJrgYndrj86169286;     bUJrgYndrj86169286 = bUJrgYndrj48508114;     bUJrgYndrj48508114 = bUJrgYndrj79631608;     bUJrgYndrj79631608 = bUJrgYndrj67648027;     bUJrgYndrj67648027 = bUJrgYndrj18039527;     bUJrgYndrj18039527 = bUJrgYndrj51735401;     bUJrgYndrj51735401 = bUJrgYndrj28904724;     bUJrgYndrj28904724 = bUJrgYndrj34979558;     bUJrgYndrj34979558 = bUJrgYndrj26472871;     bUJrgYndrj26472871 = bUJrgYndrj462212;     bUJrgYndrj462212 = bUJrgYndrj37402331;     bUJrgYndrj37402331 = bUJrgYndrj5460774;     bUJrgYndrj5460774 = bUJrgYndrj99866388;     bUJrgYndrj99866388 = bUJrgYndrj6259450;     bUJrgYndrj6259450 = bUJrgYndrj94589729;     bUJrgYndrj94589729 = bUJrgYndrj18720014;     bUJrgYndrj18720014 = bUJrgYndrj81766481;     bUJrgYndrj81766481 = bUJrgYndrj89927750;     bUJrgYndrj89927750 = bUJrgYndrj9153715;     bUJrgYndrj9153715 = bUJrgYndrj63549949;     bUJrgYndrj63549949 = bUJrgYndrj32554673;     bUJrgYndrj32554673 = bUJrgYndrj7552855;     bUJrgYndrj7552855 = bUJrgYndrj72470543;     bUJrgYndrj72470543 = bUJrgYndrj57392181;     bUJrgYndrj57392181 = bUJrgYndrj1026301;     bUJrgYndrj1026301 = bUJrgYndrj63971805;     bUJrgYndrj63971805 = bUJrgYndrj48753655;     bUJrgYndrj48753655 = bUJrgYndrj45435179;     bUJrgYndrj45435179 = bUJrgYndrj59786700;     bUJrgYndrj59786700 = bUJrgYndrj6416690;     bUJrgYndrj6416690 = bUJrgYndrj97630608;     bUJrgYndrj97630608 = bUJrgYndrj79488138;     bUJrgYndrj79488138 = bUJrgYndrj82473968;     bUJrgYndrj82473968 = bUJrgYndrj82669737;     bUJrgYndrj82669737 = bUJrgYndrj8602499;     bUJrgYndrj8602499 = bUJrgYndrj91965393;     bUJrgYndrj91965393 = bUJrgYndrj29237672;     bUJrgYndrj29237672 = bUJrgYndrj81688354;     bUJrgYndrj81688354 = bUJrgYndrj16618128;     bUJrgYndrj16618128 = bUJrgYndrj39707935;     bUJrgYndrj39707935 = bUJrgYndrj57629556;     bUJrgYndrj57629556 = bUJrgYndrj94868647;     bUJrgYndrj94868647 = bUJrgYndrj75949379;     bUJrgYndrj75949379 = bUJrgYndrj50800767;     bUJrgYndrj50800767 = bUJrgYndrj27028152;     bUJrgYndrj27028152 = bUJrgYndrj25718556;     bUJrgYndrj25718556 = bUJrgYndrj36657541;     bUJrgYndrj36657541 = bUJrgYndrj8237949;     bUJrgYndrj8237949 = bUJrgYndrj99295702;     bUJrgYndrj99295702 = bUJrgYndrj31388051;     bUJrgYndrj31388051 = bUJrgYndrj91961075;     bUJrgYndrj91961075 = bUJrgYndrj74677340;     bUJrgYndrj74677340 = bUJrgYndrj6978311;     bUJrgYndrj6978311 = bUJrgYndrj88867430;     bUJrgYndrj88867430 = bUJrgYndrj60676135;     bUJrgYndrj60676135 = bUJrgYndrj78616432;     bUJrgYndrj78616432 = bUJrgYndrj76037570;     bUJrgYndrj76037570 = bUJrgYndrj22239428;     bUJrgYndrj22239428 = bUJrgYndrj66621726;     bUJrgYndrj66621726 = bUJrgYndrj54067721;     bUJrgYndrj54067721 = bUJrgYndrj2981746;     bUJrgYndrj2981746 = bUJrgYndrj83469544;     bUJrgYndrj83469544 = bUJrgYndrj75192858;     bUJrgYndrj75192858 = bUJrgYndrj20056182;     bUJrgYndrj20056182 = bUJrgYndrj2831604;     bUJrgYndrj2831604 = bUJrgYndrj57914193;     bUJrgYndrj57914193 = bUJrgYndrj22986806;     bUJrgYndrj22986806 = bUJrgYndrj17196651;     bUJrgYndrj17196651 = bUJrgYndrj97656951;     bUJrgYndrj97656951 = bUJrgYndrj2624337;     bUJrgYndrj2624337 = bUJrgYndrj89482341;     bUJrgYndrj89482341 = bUJrgYndrj78128;     bUJrgYndrj78128 = bUJrgYndrj73309622;     bUJrgYndrj73309622 = bUJrgYndrj69445779;     bUJrgYndrj69445779 = bUJrgYndrj5920394;     bUJrgYndrj5920394 = bUJrgYndrj37686025;     bUJrgYndrj37686025 = bUJrgYndrj31603475;     bUJrgYndrj31603475 = bUJrgYndrj21669776;     bUJrgYndrj21669776 = bUJrgYndrj30364030;     bUJrgYndrj30364030 = bUJrgYndrj75307745;     bUJrgYndrj75307745 = bUJrgYndrj27314265;     bUJrgYndrj27314265 = bUJrgYndrj40515706;     bUJrgYndrj40515706 = bUJrgYndrj46139476;     bUJrgYndrj46139476 = bUJrgYndrj28398649;     bUJrgYndrj28398649 = bUJrgYndrj14455615;     bUJrgYndrj14455615 = bUJrgYndrj22953268;     bUJrgYndrj22953268 = bUJrgYndrj72509827;     bUJrgYndrj72509827 = bUJrgYndrj93606537;     bUJrgYndrj93606537 = bUJrgYndrj21993603;     bUJrgYndrj21993603 = bUJrgYndrj64430483;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void oOLJXCSQyx27275099() {     double UhQuriDfkZ51352160 = -810804471;    double UhQuriDfkZ40827068 = -935380514;    double UhQuriDfkZ29219943 = -354134691;    double UhQuriDfkZ42360593 = -840064392;    double UhQuriDfkZ44614519 = -165758994;    double UhQuriDfkZ63573741 = -993705211;    double UhQuriDfkZ23773226 = -966037912;    double UhQuriDfkZ47321439 = -214106121;    double UhQuriDfkZ94517715 = -594685528;    double UhQuriDfkZ25917766 = -187200399;    double UhQuriDfkZ12911538 = -393835199;    double UhQuriDfkZ55795764 = 7946369;    double UhQuriDfkZ9134563 = -874293504;    double UhQuriDfkZ22949718 = -680107424;    double UhQuriDfkZ91360897 = -123380503;    double UhQuriDfkZ53318685 = -517688777;    double UhQuriDfkZ1575576 = -659207417;    double UhQuriDfkZ58121481 = -660341348;    double UhQuriDfkZ96139128 = 76336878;    double UhQuriDfkZ83225698 = -525680424;    double UhQuriDfkZ67927880 = -791787786;    double UhQuriDfkZ76092291 = -310979377;    double UhQuriDfkZ75107113 = -299360515;    double UhQuriDfkZ76223339 = 76540600;    double UhQuriDfkZ62224435 = -19813976;    double UhQuriDfkZ88768628 = -269235334;    double UhQuriDfkZ75610441 = -564975627;    double UhQuriDfkZ82353031 = -817018512;    double UhQuriDfkZ62830544 = -901496342;    double UhQuriDfkZ88867353 = -499613838;    double UhQuriDfkZ22670202 = -37767216;    double UhQuriDfkZ36996703 = -231473687;    double UhQuriDfkZ61825937 = -160508779;    double UhQuriDfkZ2393642 = -88881243;    double UhQuriDfkZ50171130 = -627590371;    double UhQuriDfkZ33986516 = -226934191;    double UhQuriDfkZ19941069 = -60580231;    double UhQuriDfkZ21082032 = 54893936;    double UhQuriDfkZ98666345 = -144499811;    double UhQuriDfkZ87573775 = -920477683;    double UhQuriDfkZ89578120 = -372558454;    double UhQuriDfkZ82471533 = -59042910;    double UhQuriDfkZ55938312 = 39333000;    double UhQuriDfkZ1018648 = -239288954;    double UhQuriDfkZ34654598 = -782140938;    double UhQuriDfkZ11169421 = -157014198;    double UhQuriDfkZ12615420 = -389317384;    double UhQuriDfkZ31493616 = -869487965;    double UhQuriDfkZ71913982 = -305594493;    double UhQuriDfkZ9217708 = -386595997;    double UhQuriDfkZ81065797 = -811055386;    double UhQuriDfkZ62542598 = -261871631;    double UhQuriDfkZ80010251 = -423169429;    double UhQuriDfkZ30494349 = -398837668;    double UhQuriDfkZ59347219 = -562074822;    double UhQuriDfkZ75259869 = -399825095;    double UhQuriDfkZ65719954 = -536020000;    double UhQuriDfkZ52996604 = -330675291;    double UhQuriDfkZ80136158 = -720250416;    double UhQuriDfkZ55845890 = -896523661;    double UhQuriDfkZ87963300 = -328729585;    double UhQuriDfkZ41420194 = -49019400;    double UhQuriDfkZ84490895 = -312609779;    double UhQuriDfkZ5650363 = 4928309;    double UhQuriDfkZ3247565 = -49433183;    double UhQuriDfkZ75914835 = -62361512;    double UhQuriDfkZ93969827 = -831544852;    double UhQuriDfkZ6740922 = -685412262;    double UhQuriDfkZ72778587 = 47482947;    double UhQuriDfkZ57374381 = -896446312;    double UhQuriDfkZ33377616 = -357108546;    double UhQuriDfkZ80493544 = -614101354;    double UhQuriDfkZ59455136 = -415841537;    double UhQuriDfkZ8565354 = -3185439;    double UhQuriDfkZ93647578 = -53121971;    double UhQuriDfkZ85456346 = -632744876;    double UhQuriDfkZ20153979 = -250312378;    double UhQuriDfkZ74088466 = 39928439;    double UhQuriDfkZ41568741 = -141318463;    double UhQuriDfkZ51055014 = -862799779;    double UhQuriDfkZ76153208 = -879917951;    double UhQuriDfkZ44116825 = -695487663;    double UhQuriDfkZ10439050 = -411424020;    double UhQuriDfkZ53612836 = -414900345;    double UhQuriDfkZ7801556 = -688558452;    double UhQuriDfkZ60127603 = -775895586;    double UhQuriDfkZ56986452 = -808304259;    double UhQuriDfkZ31331589 = -761671112;    double UhQuriDfkZ43046423 = -526806422;    double UhQuriDfkZ74911260 = -127765277;    double UhQuriDfkZ68266561 = -690914192;    double UhQuriDfkZ66944465 = -729904941;    double UhQuriDfkZ40945874 = -224855648;    double UhQuriDfkZ42820455 = -247976151;    double UhQuriDfkZ99610475 = -491748099;    double UhQuriDfkZ48157927 = -223539055;    double UhQuriDfkZ97980638 = -746433132;    double UhQuriDfkZ50287950 = -965595309;    double UhQuriDfkZ97771082 = -89855772;    double UhQuriDfkZ58739763 = -810804471;     UhQuriDfkZ51352160 = UhQuriDfkZ40827068;     UhQuriDfkZ40827068 = UhQuriDfkZ29219943;     UhQuriDfkZ29219943 = UhQuriDfkZ42360593;     UhQuriDfkZ42360593 = UhQuriDfkZ44614519;     UhQuriDfkZ44614519 = UhQuriDfkZ63573741;     UhQuriDfkZ63573741 = UhQuriDfkZ23773226;     UhQuriDfkZ23773226 = UhQuriDfkZ47321439;     UhQuriDfkZ47321439 = UhQuriDfkZ94517715;     UhQuriDfkZ94517715 = UhQuriDfkZ25917766;     UhQuriDfkZ25917766 = UhQuriDfkZ12911538;     UhQuriDfkZ12911538 = UhQuriDfkZ55795764;     UhQuriDfkZ55795764 = UhQuriDfkZ9134563;     UhQuriDfkZ9134563 = UhQuriDfkZ22949718;     UhQuriDfkZ22949718 = UhQuriDfkZ91360897;     UhQuriDfkZ91360897 = UhQuriDfkZ53318685;     UhQuriDfkZ53318685 = UhQuriDfkZ1575576;     UhQuriDfkZ1575576 = UhQuriDfkZ58121481;     UhQuriDfkZ58121481 = UhQuriDfkZ96139128;     UhQuriDfkZ96139128 = UhQuriDfkZ83225698;     UhQuriDfkZ83225698 = UhQuriDfkZ67927880;     UhQuriDfkZ67927880 = UhQuriDfkZ76092291;     UhQuriDfkZ76092291 = UhQuriDfkZ75107113;     UhQuriDfkZ75107113 = UhQuriDfkZ76223339;     UhQuriDfkZ76223339 = UhQuriDfkZ62224435;     UhQuriDfkZ62224435 = UhQuriDfkZ88768628;     UhQuriDfkZ88768628 = UhQuriDfkZ75610441;     UhQuriDfkZ75610441 = UhQuriDfkZ82353031;     UhQuriDfkZ82353031 = UhQuriDfkZ62830544;     UhQuriDfkZ62830544 = UhQuriDfkZ88867353;     UhQuriDfkZ88867353 = UhQuriDfkZ22670202;     UhQuriDfkZ22670202 = UhQuriDfkZ36996703;     UhQuriDfkZ36996703 = UhQuriDfkZ61825937;     UhQuriDfkZ61825937 = UhQuriDfkZ2393642;     UhQuriDfkZ2393642 = UhQuriDfkZ50171130;     UhQuriDfkZ50171130 = UhQuriDfkZ33986516;     UhQuriDfkZ33986516 = UhQuriDfkZ19941069;     UhQuriDfkZ19941069 = UhQuriDfkZ21082032;     UhQuriDfkZ21082032 = UhQuriDfkZ98666345;     UhQuriDfkZ98666345 = UhQuriDfkZ87573775;     UhQuriDfkZ87573775 = UhQuriDfkZ89578120;     UhQuriDfkZ89578120 = UhQuriDfkZ82471533;     UhQuriDfkZ82471533 = UhQuriDfkZ55938312;     UhQuriDfkZ55938312 = UhQuriDfkZ1018648;     UhQuriDfkZ1018648 = UhQuriDfkZ34654598;     UhQuriDfkZ34654598 = UhQuriDfkZ11169421;     UhQuriDfkZ11169421 = UhQuriDfkZ12615420;     UhQuriDfkZ12615420 = UhQuriDfkZ31493616;     UhQuriDfkZ31493616 = UhQuriDfkZ71913982;     UhQuriDfkZ71913982 = UhQuriDfkZ9217708;     UhQuriDfkZ9217708 = UhQuriDfkZ81065797;     UhQuriDfkZ81065797 = UhQuriDfkZ62542598;     UhQuriDfkZ62542598 = UhQuriDfkZ80010251;     UhQuriDfkZ80010251 = UhQuriDfkZ30494349;     UhQuriDfkZ30494349 = UhQuriDfkZ59347219;     UhQuriDfkZ59347219 = UhQuriDfkZ75259869;     UhQuriDfkZ75259869 = UhQuriDfkZ65719954;     UhQuriDfkZ65719954 = UhQuriDfkZ52996604;     UhQuriDfkZ52996604 = UhQuriDfkZ80136158;     UhQuriDfkZ80136158 = UhQuriDfkZ55845890;     UhQuriDfkZ55845890 = UhQuriDfkZ87963300;     UhQuriDfkZ87963300 = UhQuriDfkZ41420194;     UhQuriDfkZ41420194 = UhQuriDfkZ84490895;     UhQuriDfkZ84490895 = UhQuriDfkZ5650363;     UhQuriDfkZ5650363 = UhQuriDfkZ3247565;     UhQuriDfkZ3247565 = UhQuriDfkZ75914835;     UhQuriDfkZ75914835 = UhQuriDfkZ93969827;     UhQuriDfkZ93969827 = UhQuriDfkZ6740922;     UhQuriDfkZ6740922 = UhQuriDfkZ72778587;     UhQuriDfkZ72778587 = UhQuriDfkZ57374381;     UhQuriDfkZ57374381 = UhQuriDfkZ33377616;     UhQuriDfkZ33377616 = UhQuriDfkZ80493544;     UhQuriDfkZ80493544 = UhQuriDfkZ59455136;     UhQuriDfkZ59455136 = UhQuriDfkZ8565354;     UhQuriDfkZ8565354 = UhQuriDfkZ93647578;     UhQuriDfkZ93647578 = UhQuriDfkZ85456346;     UhQuriDfkZ85456346 = UhQuriDfkZ20153979;     UhQuriDfkZ20153979 = UhQuriDfkZ74088466;     UhQuriDfkZ74088466 = UhQuriDfkZ41568741;     UhQuriDfkZ41568741 = UhQuriDfkZ51055014;     UhQuriDfkZ51055014 = UhQuriDfkZ76153208;     UhQuriDfkZ76153208 = UhQuriDfkZ44116825;     UhQuriDfkZ44116825 = UhQuriDfkZ10439050;     UhQuriDfkZ10439050 = UhQuriDfkZ53612836;     UhQuriDfkZ53612836 = UhQuriDfkZ7801556;     UhQuriDfkZ7801556 = UhQuriDfkZ60127603;     UhQuriDfkZ60127603 = UhQuriDfkZ56986452;     UhQuriDfkZ56986452 = UhQuriDfkZ31331589;     UhQuriDfkZ31331589 = UhQuriDfkZ43046423;     UhQuriDfkZ43046423 = UhQuriDfkZ74911260;     UhQuriDfkZ74911260 = UhQuriDfkZ68266561;     UhQuriDfkZ68266561 = UhQuriDfkZ66944465;     UhQuriDfkZ66944465 = UhQuriDfkZ40945874;     UhQuriDfkZ40945874 = UhQuriDfkZ42820455;     UhQuriDfkZ42820455 = UhQuriDfkZ99610475;     UhQuriDfkZ99610475 = UhQuriDfkZ48157927;     UhQuriDfkZ48157927 = UhQuriDfkZ97980638;     UhQuriDfkZ97980638 = UhQuriDfkZ50287950;     UhQuriDfkZ50287950 = UhQuriDfkZ97771082;     UhQuriDfkZ97771082 = UhQuriDfkZ58739763;     UhQuriDfkZ58739763 = UhQuriDfkZ51352160;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void JPctTRPSCA72970791() {     double pEfNEwbSQw70541118 = -654579296;    double pEfNEwbSQw13124926 = -576988847;    double pEfNEwbSQw53879325 = 69019157;    double pEfNEwbSQw26652040 = -845556562;    double pEfNEwbSQw71621022 = -200950710;    double pEfNEwbSQw76330699 = -559940964;    double pEfNEwbSQw2763859 = -27741317;    double pEfNEwbSQw83579032 = -774617530;    double pEfNEwbSQw63514685 = -966853562;    double pEfNEwbSQw90286553 = -438121541;    double pEfNEwbSQw73661703 = -76656114;    double pEfNEwbSQw32284006 = -49835311;    double pEfNEwbSQw66570146 = -147479778;    double pEfNEwbSQw16619506 = -661000644;    double pEfNEwbSQw8562563 = -589789971;    double pEfNEwbSQw34109468 = -844832385;    double pEfNEwbSQw1424164 = -425447263;    double pEfNEwbSQw48958409 = 52450632;    double pEfNEwbSQw17237670 = -510893515;    double pEfNEwbSQw79096798 = 43490044;    double pEfNEwbSQw68684829 = -313062622;    double pEfNEwbSQw2219621 = -900779074;    double pEfNEwbSQw77929962 = -607429594;    double pEfNEwbSQw27508871 = -92596135;    double pEfNEwbSQw78401789 = -795436083;    double pEfNEwbSQw35885481 = -492573241;    double pEfNEwbSQw87401373 = -514335478;    double pEfNEwbSQw30277132 = -164265187;    double pEfNEwbSQw68738453 = -253716730;    double pEfNEwbSQw53859221 = -431296009;    double pEfNEwbSQw89205177 = -421752489;    double pEfNEwbSQw14889153 = -161820563;    double pEfNEwbSQw2716226 = -771374632;    double pEfNEwbSQw67583945 = -680989972;    double pEfNEwbSQw1224872 = -53667304;    double pEfNEwbSQw12580404 = -340130228;    double pEfNEwbSQw57175108 = -145964190;    double pEfNEwbSQw52761547 = -931642183;    double pEfNEwbSQw54359029 = -488253552;    double pEfNEwbSQw19161531 = -722955423;    double pEfNEwbSQw99941581 = -803853597;    double pEfNEwbSQw15478727 = -165821788;    double pEfNEwbSQw17838477 = -807121786;    double pEfNEwbSQw29113607 = -865600653;    double pEfNEwbSQw78640255 = -461406776;    double pEfNEwbSQw60969662 = 1075149;    double pEfNEwbSQw37441761 = -413669030;    double pEfNEwbSQw12578194 = -189375100;    double pEfNEwbSQw10022473 = -105351663;    double pEfNEwbSQw49575204 = -822613362;    double pEfNEwbSQw88667983 = -172549338;    double pEfNEwbSQw82986533 = -945431015;    double pEfNEwbSQw18565454 = -299093340;    double pEfNEwbSQw82168530 = -635135504;    double pEfNEwbSQw4285963 = -368392635;    double pEfNEwbSQw68321497 = -753800223;    double pEfNEwbSQw35194964 = -969559254;    double pEfNEwbSQw26370454 = -838384708;    double pEfNEwbSQw48250251 = 49879521;    double pEfNEwbSQw35735542 = -708377470;    double pEfNEwbSQw88929325 = 54394514;    double pEfNEwbSQw72486727 = -863476131;    double pEfNEwbSQw14840580 = -420900801;    double pEfNEwbSQw9655464 = -435557553;    double pEfNEwbSQw1081376 = 83630948;    double pEfNEwbSQw58772551 = -914835552;    double pEfNEwbSQw29567780 = -278460680;    double pEfNEwbSQw98986201 = -466489807;    double pEfNEwbSQw15394635 = -507333340;    double pEfNEwbSQw95982159 = -149659744;    double pEfNEwbSQw76934359 = -598868196;    double pEfNEwbSQw48662616 = -493805080;    double pEfNEwbSQw94599380 = -459295816;    double pEfNEwbSQw98076138 = -787938093;    double pEfNEwbSQw79155217 = -152656360;    double pEfNEwbSQw53206103 = -47240834;    double pEfNEwbSQw84381144 = 6342711;    double pEfNEwbSQw48816355 = -741828941;    double pEfNEwbSQw48868616 = -631189360;    double pEfNEwbSQw17432128 = -696511233;    double pEfNEwbSQw98443720 = 21095789;    double pEfNEwbSQw74823179 = -224960379;    double pEfNEwbSQw20254660 = 41086476;    double pEfNEwbSQw19163249 = -431103368;    double pEfNEwbSQw65191238 = -158746671;    double pEfNEwbSQw6218645 = -476321474;    double pEfNEwbSQw96323698 = -862727223;    double pEfNEwbSQw20547696 = -36239128;    double pEfNEwbSQw63297982 = -212597338;    double pEfNEwbSQw32903374 = -299867082;    double pEfNEwbSQw77385440 = -370570975;    double pEfNEwbSQw30804654 = -307579482;    double pEfNEwbSQw4511297 = -881521704;    double pEfNEwbSQw18623488 = -779876083;    double pEfNEwbSQw30232205 = -677349937;    double pEfNEwbSQw27454855 = -940377466;    double pEfNEwbSQw638148 = -744920988;    double pEfNEwbSQw8183014 = -271564233;    double pEfNEwbSQw28032231 = -849231602;    double pEfNEwbSQw19867704 = -654579296;     pEfNEwbSQw70541118 = pEfNEwbSQw13124926;     pEfNEwbSQw13124926 = pEfNEwbSQw53879325;     pEfNEwbSQw53879325 = pEfNEwbSQw26652040;     pEfNEwbSQw26652040 = pEfNEwbSQw71621022;     pEfNEwbSQw71621022 = pEfNEwbSQw76330699;     pEfNEwbSQw76330699 = pEfNEwbSQw2763859;     pEfNEwbSQw2763859 = pEfNEwbSQw83579032;     pEfNEwbSQw83579032 = pEfNEwbSQw63514685;     pEfNEwbSQw63514685 = pEfNEwbSQw90286553;     pEfNEwbSQw90286553 = pEfNEwbSQw73661703;     pEfNEwbSQw73661703 = pEfNEwbSQw32284006;     pEfNEwbSQw32284006 = pEfNEwbSQw66570146;     pEfNEwbSQw66570146 = pEfNEwbSQw16619506;     pEfNEwbSQw16619506 = pEfNEwbSQw8562563;     pEfNEwbSQw8562563 = pEfNEwbSQw34109468;     pEfNEwbSQw34109468 = pEfNEwbSQw1424164;     pEfNEwbSQw1424164 = pEfNEwbSQw48958409;     pEfNEwbSQw48958409 = pEfNEwbSQw17237670;     pEfNEwbSQw17237670 = pEfNEwbSQw79096798;     pEfNEwbSQw79096798 = pEfNEwbSQw68684829;     pEfNEwbSQw68684829 = pEfNEwbSQw2219621;     pEfNEwbSQw2219621 = pEfNEwbSQw77929962;     pEfNEwbSQw77929962 = pEfNEwbSQw27508871;     pEfNEwbSQw27508871 = pEfNEwbSQw78401789;     pEfNEwbSQw78401789 = pEfNEwbSQw35885481;     pEfNEwbSQw35885481 = pEfNEwbSQw87401373;     pEfNEwbSQw87401373 = pEfNEwbSQw30277132;     pEfNEwbSQw30277132 = pEfNEwbSQw68738453;     pEfNEwbSQw68738453 = pEfNEwbSQw53859221;     pEfNEwbSQw53859221 = pEfNEwbSQw89205177;     pEfNEwbSQw89205177 = pEfNEwbSQw14889153;     pEfNEwbSQw14889153 = pEfNEwbSQw2716226;     pEfNEwbSQw2716226 = pEfNEwbSQw67583945;     pEfNEwbSQw67583945 = pEfNEwbSQw1224872;     pEfNEwbSQw1224872 = pEfNEwbSQw12580404;     pEfNEwbSQw12580404 = pEfNEwbSQw57175108;     pEfNEwbSQw57175108 = pEfNEwbSQw52761547;     pEfNEwbSQw52761547 = pEfNEwbSQw54359029;     pEfNEwbSQw54359029 = pEfNEwbSQw19161531;     pEfNEwbSQw19161531 = pEfNEwbSQw99941581;     pEfNEwbSQw99941581 = pEfNEwbSQw15478727;     pEfNEwbSQw15478727 = pEfNEwbSQw17838477;     pEfNEwbSQw17838477 = pEfNEwbSQw29113607;     pEfNEwbSQw29113607 = pEfNEwbSQw78640255;     pEfNEwbSQw78640255 = pEfNEwbSQw60969662;     pEfNEwbSQw60969662 = pEfNEwbSQw37441761;     pEfNEwbSQw37441761 = pEfNEwbSQw12578194;     pEfNEwbSQw12578194 = pEfNEwbSQw10022473;     pEfNEwbSQw10022473 = pEfNEwbSQw49575204;     pEfNEwbSQw49575204 = pEfNEwbSQw88667983;     pEfNEwbSQw88667983 = pEfNEwbSQw82986533;     pEfNEwbSQw82986533 = pEfNEwbSQw18565454;     pEfNEwbSQw18565454 = pEfNEwbSQw82168530;     pEfNEwbSQw82168530 = pEfNEwbSQw4285963;     pEfNEwbSQw4285963 = pEfNEwbSQw68321497;     pEfNEwbSQw68321497 = pEfNEwbSQw35194964;     pEfNEwbSQw35194964 = pEfNEwbSQw26370454;     pEfNEwbSQw26370454 = pEfNEwbSQw48250251;     pEfNEwbSQw48250251 = pEfNEwbSQw35735542;     pEfNEwbSQw35735542 = pEfNEwbSQw88929325;     pEfNEwbSQw88929325 = pEfNEwbSQw72486727;     pEfNEwbSQw72486727 = pEfNEwbSQw14840580;     pEfNEwbSQw14840580 = pEfNEwbSQw9655464;     pEfNEwbSQw9655464 = pEfNEwbSQw1081376;     pEfNEwbSQw1081376 = pEfNEwbSQw58772551;     pEfNEwbSQw58772551 = pEfNEwbSQw29567780;     pEfNEwbSQw29567780 = pEfNEwbSQw98986201;     pEfNEwbSQw98986201 = pEfNEwbSQw15394635;     pEfNEwbSQw15394635 = pEfNEwbSQw95982159;     pEfNEwbSQw95982159 = pEfNEwbSQw76934359;     pEfNEwbSQw76934359 = pEfNEwbSQw48662616;     pEfNEwbSQw48662616 = pEfNEwbSQw94599380;     pEfNEwbSQw94599380 = pEfNEwbSQw98076138;     pEfNEwbSQw98076138 = pEfNEwbSQw79155217;     pEfNEwbSQw79155217 = pEfNEwbSQw53206103;     pEfNEwbSQw53206103 = pEfNEwbSQw84381144;     pEfNEwbSQw84381144 = pEfNEwbSQw48816355;     pEfNEwbSQw48816355 = pEfNEwbSQw48868616;     pEfNEwbSQw48868616 = pEfNEwbSQw17432128;     pEfNEwbSQw17432128 = pEfNEwbSQw98443720;     pEfNEwbSQw98443720 = pEfNEwbSQw74823179;     pEfNEwbSQw74823179 = pEfNEwbSQw20254660;     pEfNEwbSQw20254660 = pEfNEwbSQw19163249;     pEfNEwbSQw19163249 = pEfNEwbSQw65191238;     pEfNEwbSQw65191238 = pEfNEwbSQw6218645;     pEfNEwbSQw6218645 = pEfNEwbSQw96323698;     pEfNEwbSQw96323698 = pEfNEwbSQw20547696;     pEfNEwbSQw20547696 = pEfNEwbSQw63297982;     pEfNEwbSQw63297982 = pEfNEwbSQw32903374;     pEfNEwbSQw32903374 = pEfNEwbSQw77385440;     pEfNEwbSQw77385440 = pEfNEwbSQw30804654;     pEfNEwbSQw30804654 = pEfNEwbSQw4511297;     pEfNEwbSQw4511297 = pEfNEwbSQw18623488;     pEfNEwbSQw18623488 = pEfNEwbSQw30232205;     pEfNEwbSQw30232205 = pEfNEwbSQw27454855;     pEfNEwbSQw27454855 = pEfNEwbSQw638148;     pEfNEwbSQw638148 = pEfNEwbSQw8183014;     pEfNEwbSQw8183014 = pEfNEwbSQw28032231;     pEfNEwbSQw28032231 = pEfNEwbSQw19867704;     pEfNEwbSQw19867704 = pEfNEwbSQw70541118;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TAuDABjmYq9945964() {     double TeYvVGRbLZ38267367 = -823979819;    double TeYvVGRbLZ89047122 = 11437781;    double TeYvVGRbLZ23229170 = -842378687;    double TeYvVGRbLZ55303648 = -666402510;    double TeYvVGRbLZ4183709 = -487927801;    double TeYvVGRbLZ11844851 = 9168683;    double TeYvVGRbLZ42366230 = -352372417;    double TeYvVGRbLZ99251837 = -448823275;    double TeYvVGRbLZ55643569 = -517842301;    double TeYvVGRbLZ96057665 = -638868476;    double TeYvVGRbLZ52912873 = -507396016;    double TeYvVGRbLZ39713919 = -949634887;    double TeYvVGRbLZ1613926 = -984315268;    double TeYvVGRbLZ20112047 = -821758875;    double TeYvVGRbLZ60245876 = -272858923;    double TeYvVGRbLZ63162392 = -829136014;    double TeYvVGRbLZ7268511 = -48741824;    double TeYvVGRbLZ10338772 = -732799218;    double TeYvVGRbLZ77726969 = -978166358;    double TeYvVGRbLZ12052290 = -475802715;    double TeYvVGRbLZ15062973 = -546933158;    double TeYvVGRbLZ93078516 = -649093162;    double TeYvVGRbLZ81631850 = -655724607;    double TeYvVGRbLZ96230397 = -838468699;    double TeYvVGRbLZ46632110 = -470775621;    double TeYvVGRbLZ99521606 = -840964604;    double TeYvVGRbLZ2722451 = -17877405;    double TeYvVGRbLZ18343107 = -553634774;    double TeYvVGRbLZ48411784 = -863799273;    double TeYvVGRbLZ44670862 = -836265281;    double TeYvVGRbLZ15203133 = -341289350;    double TeYvVGRbLZ26162591 = -794217148;    double TeYvVGRbLZ1551443 = -946144725;    double TeYvVGRbLZ32599577 = -101677745;    double TeYvVGRbLZ25308117 = -158874389;    double TeYvVGRbLZ9774921 = -487295736;    double TeYvVGRbLZ87851700 = -128138899;    double TeYvVGRbLZ63328088 = -920601316;    double TeYvVGRbLZ87211108 = -120032386;    double TeYvVGRbLZ87233742 = -810302539;    double TeYvVGRbLZ60595219 = -182425003;    double TeYvVGRbLZ53010120 = -609990455;    double TeYvVGRbLZ92671846 = -313277304;    double TeYvVGRbLZ78795225 = -946249080;    double TeYvVGRbLZ70657510 = -611292056;    double TeYvVGRbLZ54164310 = 75591016;    double TeYvVGRbLZ60963238 = -346302521;    double TeYvVGRbLZ9133141 = -577019073;    double TeYvVGRbLZ80798000 = -819102009;    double TeYvVGRbLZ21862710 = -311639218;    double TeYvVGRbLZ35683030 = -448965121;    double TeYvVGRbLZ82002898 = -143330059;    double TeYvVGRbLZ40600202 = -123143125;    double TeYvVGRbLZ95289588 = -446849680;    double TeYvVGRbLZ94882990 = -970087341;    double TeYvVGRbLZ45188851 = -74886657;    double TeYvVGRbLZ7415273 = -332837613;    double TeYvVGRbLZ26998772 = 96090011;    double TeYvVGRbLZ8671538 = -95626889;    double TeYvVGRbLZ4662103 = -646963198;    double TeYvVGRbLZ9122401 = -972953913;    double TeYvVGRbLZ24023123 = -798737644;    double TeYvVGRbLZ50840054 = -585024002;    double TeYvVGRbLZ10972708 = -681577020;    double TeYvVGRbLZ80854533 = -197579126;    double TeYvVGRbLZ26750283 = -713178868;    double TeYvVGRbLZ38162476 = 96509838;    double TeYvVGRbLZ69014349 = -782637524;    double TeYvVGRbLZ94803930 = -562884487;    double TeYvVGRbLZ50470955 = -785563188;    double TeYvVGRbLZ75310691 = -600997115;    double TeYvVGRbLZ43940423 = -128140508;    double TeYvVGRbLZ23127663 = -512766832;    double TeYvVGRbLZ90493226 = -67863820;    double TeYvVGRbLZ51457071 = -193377712;    double TeYvVGRbLZ62052852 = -936942704;    double TeYvVGRbLZ406671 = -235815859;    double TeYvVGRbLZ2836626 = -709475527;    double TeYvVGRbLZ25572887 = -127176643;    double TeYvVGRbLZ92467799 = -446366637;    double TeYvVGRbLZ38558368 = -394662083;    double TeYvVGRbLZ93589310 = -440858332;    double TeYvVGRbLZ37545107 = -734532766;    double TeYvVGRbLZ26549074 = -452160056;    double TeYvVGRbLZ8987832 = -287300161;    double TeYvVGRbLZ33200235 = -97959292;    double TeYvVGRbLZ85562388 = -571074024;    double TeYvVGRbLZ6261855 = -399295046;    double TeYvVGRbLZ37716586 = -131590404;    double TeYvVGRbLZ80119266 = 16012268;    double TeYvVGRbLZ2359649 = -54458124;    double TeYvVGRbLZ60852928 = -124228910;    double TeYvVGRbLZ54656550 = -724974427;    double TeYvVGRbLZ82549006 = -473069189;    double TeYvVGRbLZ78111342 = -837348626;    double TeYvVGRbLZ36572096 = -383687360;    double TeYvVGRbLZ2170066 = 75033547;    double TeYvVGRbLZ81699138 = -631700284;    double TeYvVGRbLZ97940691 = -648669955;    double TeYvVGRbLZ43907227 = -823979819;     TeYvVGRbLZ38267367 = TeYvVGRbLZ89047122;     TeYvVGRbLZ89047122 = TeYvVGRbLZ23229170;     TeYvVGRbLZ23229170 = TeYvVGRbLZ55303648;     TeYvVGRbLZ55303648 = TeYvVGRbLZ4183709;     TeYvVGRbLZ4183709 = TeYvVGRbLZ11844851;     TeYvVGRbLZ11844851 = TeYvVGRbLZ42366230;     TeYvVGRbLZ42366230 = TeYvVGRbLZ99251837;     TeYvVGRbLZ99251837 = TeYvVGRbLZ55643569;     TeYvVGRbLZ55643569 = TeYvVGRbLZ96057665;     TeYvVGRbLZ96057665 = TeYvVGRbLZ52912873;     TeYvVGRbLZ52912873 = TeYvVGRbLZ39713919;     TeYvVGRbLZ39713919 = TeYvVGRbLZ1613926;     TeYvVGRbLZ1613926 = TeYvVGRbLZ20112047;     TeYvVGRbLZ20112047 = TeYvVGRbLZ60245876;     TeYvVGRbLZ60245876 = TeYvVGRbLZ63162392;     TeYvVGRbLZ63162392 = TeYvVGRbLZ7268511;     TeYvVGRbLZ7268511 = TeYvVGRbLZ10338772;     TeYvVGRbLZ10338772 = TeYvVGRbLZ77726969;     TeYvVGRbLZ77726969 = TeYvVGRbLZ12052290;     TeYvVGRbLZ12052290 = TeYvVGRbLZ15062973;     TeYvVGRbLZ15062973 = TeYvVGRbLZ93078516;     TeYvVGRbLZ93078516 = TeYvVGRbLZ81631850;     TeYvVGRbLZ81631850 = TeYvVGRbLZ96230397;     TeYvVGRbLZ96230397 = TeYvVGRbLZ46632110;     TeYvVGRbLZ46632110 = TeYvVGRbLZ99521606;     TeYvVGRbLZ99521606 = TeYvVGRbLZ2722451;     TeYvVGRbLZ2722451 = TeYvVGRbLZ18343107;     TeYvVGRbLZ18343107 = TeYvVGRbLZ48411784;     TeYvVGRbLZ48411784 = TeYvVGRbLZ44670862;     TeYvVGRbLZ44670862 = TeYvVGRbLZ15203133;     TeYvVGRbLZ15203133 = TeYvVGRbLZ26162591;     TeYvVGRbLZ26162591 = TeYvVGRbLZ1551443;     TeYvVGRbLZ1551443 = TeYvVGRbLZ32599577;     TeYvVGRbLZ32599577 = TeYvVGRbLZ25308117;     TeYvVGRbLZ25308117 = TeYvVGRbLZ9774921;     TeYvVGRbLZ9774921 = TeYvVGRbLZ87851700;     TeYvVGRbLZ87851700 = TeYvVGRbLZ63328088;     TeYvVGRbLZ63328088 = TeYvVGRbLZ87211108;     TeYvVGRbLZ87211108 = TeYvVGRbLZ87233742;     TeYvVGRbLZ87233742 = TeYvVGRbLZ60595219;     TeYvVGRbLZ60595219 = TeYvVGRbLZ53010120;     TeYvVGRbLZ53010120 = TeYvVGRbLZ92671846;     TeYvVGRbLZ92671846 = TeYvVGRbLZ78795225;     TeYvVGRbLZ78795225 = TeYvVGRbLZ70657510;     TeYvVGRbLZ70657510 = TeYvVGRbLZ54164310;     TeYvVGRbLZ54164310 = TeYvVGRbLZ60963238;     TeYvVGRbLZ60963238 = TeYvVGRbLZ9133141;     TeYvVGRbLZ9133141 = TeYvVGRbLZ80798000;     TeYvVGRbLZ80798000 = TeYvVGRbLZ21862710;     TeYvVGRbLZ21862710 = TeYvVGRbLZ35683030;     TeYvVGRbLZ35683030 = TeYvVGRbLZ82002898;     TeYvVGRbLZ82002898 = TeYvVGRbLZ40600202;     TeYvVGRbLZ40600202 = TeYvVGRbLZ95289588;     TeYvVGRbLZ95289588 = TeYvVGRbLZ94882990;     TeYvVGRbLZ94882990 = TeYvVGRbLZ45188851;     TeYvVGRbLZ45188851 = TeYvVGRbLZ7415273;     TeYvVGRbLZ7415273 = TeYvVGRbLZ26998772;     TeYvVGRbLZ26998772 = TeYvVGRbLZ8671538;     TeYvVGRbLZ8671538 = TeYvVGRbLZ4662103;     TeYvVGRbLZ4662103 = TeYvVGRbLZ9122401;     TeYvVGRbLZ9122401 = TeYvVGRbLZ24023123;     TeYvVGRbLZ24023123 = TeYvVGRbLZ50840054;     TeYvVGRbLZ50840054 = TeYvVGRbLZ10972708;     TeYvVGRbLZ10972708 = TeYvVGRbLZ80854533;     TeYvVGRbLZ80854533 = TeYvVGRbLZ26750283;     TeYvVGRbLZ26750283 = TeYvVGRbLZ38162476;     TeYvVGRbLZ38162476 = TeYvVGRbLZ69014349;     TeYvVGRbLZ69014349 = TeYvVGRbLZ94803930;     TeYvVGRbLZ94803930 = TeYvVGRbLZ50470955;     TeYvVGRbLZ50470955 = TeYvVGRbLZ75310691;     TeYvVGRbLZ75310691 = TeYvVGRbLZ43940423;     TeYvVGRbLZ43940423 = TeYvVGRbLZ23127663;     TeYvVGRbLZ23127663 = TeYvVGRbLZ90493226;     TeYvVGRbLZ90493226 = TeYvVGRbLZ51457071;     TeYvVGRbLZ51457071 = TeYvVGRbLZ62052852;     TeYvVGRbLZ62052852 = TeYvVGRbLZ406671;     TeYvVGRbLZ406671 = TeYvVGRbLZ2836626;     TeYvVGRbLZ2836626 = TeYvVGRbLZ25572887;     TeYvVGRbLZ25572887 = TeYvVGRbLZ92467799;     TeYvVGRbLZ92467799 = TeYvVGRbLZ38558368;     TeYvVGRbLZ38558368 = TeYvVGRbLZ93589310;     TeYvVGRbLZ93589310 = TeYvVGRbLZ37545107;     TeYvVGRbLZ37545107 = TeYvVGRbLZ26549074;     TeYvVGRbLZ26549074 = TeYvVGRbLZ8987832;     TeYvVGRbLZ8987832 = TeYvVGRbLZ33200235;     TeYvVGRbLZ33200235 = TeYvVGRbLZ85562388;     TeYvVGRbLZ85562388 = TeYvVGRbLZ6261855;     TeYvVGRbLZ6261855 = TeYvVGRbLZ37716586;     TeYvVGRbLZ37716586 = TeYvVGRbLZ80119266;     TeYvVGRbLZ80119266 = TeYvVGRbLZ2359649;     TeYvVGRbLZ2359649 = TeYvVGRbLZ60852928;     TeYvVGRbLZ60852928 = TeYvVGRbLZ54656550;     TeYvVGRbLZ54656550 = TeYvVGRbLZ82549006;     TeYvVGRbLZ82549006 = TeYvVGRbLZ78111342;     TeYvVGRbLZ78111342 = TeYvVGRbLZ36572096;     TeYvVGRbLZ36572096 = TeYvVGRbLZ2170066;     TeYvVGRbLZ2170066 = TeYvVGRbLZ81699138;     TeYvVGRbLZ81699138 = TeYvVGRbLZ97940691;     TeYvVGRbLZ97940691 = TeYvVGRbLZ43907227;     TeYvVGRbLZ43907227 = TeYvVGRbLZ38267367;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZyKbWuZgQB64362176() {     double GYiHerRlRO8919034 = -342128947;    double GYiHerRlRO57720640 = -960205512;    double GYiHerRlRO3198089 = -184673147;    double GYiHerRlRO95234934 = -856540902;    double GYiHerRlRO25634030 = -271334141;    double GYiHerRlRO1844614 = -792412469;    double GYiHerRlRO60745125 = -351148127;    double GYiHerRlRO56094218 = -795640349;    double GYiHerRlRO1508624 = -611189629;    double GYiHerRlRO19024127 = -939963826;    double GYiHerRlRO95162034 = -542297944;    double GYiHerRlRO85260489 = -165398671;    double GYiHerRlRO81441313 = -893852326;    double GYiHerRlRO3959081 = -622787084;    double GYiHerRlRO42965895 = -422608908;    double GYiHerRlRO95691031 = -399119601;    double GYiHerRlRO1121338 = 42073046;    double GYiHerRlRO30632266 = -721965409;    double GYiHerRlRO59434751 = -585354301;    double GYiHerRlRO70838998 = 81830981;    double GYiHerRlRO70198727 = -455612293;    double GYiHerRlRO54474281 = -980378468;    double GYiHerRlRO83575658 = -123567751;    double GYiHerRlRO30079935 = -430869605;    double GYiHerRlRO10756499 = -146680297;    double GYiHerRlRO30119186 = -939249053;    double GYiHerRlRO10983237 = -413055180;    double GYiHerRlRO26125332 = 41241464;    double GYiHerRlRO80554271 = -58157506;    double GYiHerRlRO83842957 = -294660352;    double GYiHerRlRO22275129 = -89723036;    double GYiHerRlRO70674051 = -22514314;    double GYiHerRlRO84496803 = -893106337;    double GYiHerRlRO97964551 = -765207429;    double GYiHerRlRO3332354 = -5821172;    double GYiHerRlRO69768179 = -566522302;    double GYiHerRlRO31643186 = -316732107;    double GYiHerRlRO16120577 = -704714420;    double GYiHerRlRO65744397 = -75761035;    double GYiHerRlRO82337041 = -327910901;    double GYiHerRlRO20668503 = -566443881;    double GYiHerRlRO81493111 = -379379544;    double GYiHerRlRO41638806 = -300031358;    double GYiHerRlRO85303525 = 81775948;    double GYiHerRlRO66611569 = -919938452;    double GYiHerRlRO60570145 = -782746157;    double GYiHerRlRO87094442 = -462372320;    double GYiHerRlRO74747350 = 70850630;    double GYiHerRlRO86239452 = -804866003;    double GYiHerRlRO30290197 = -594648092;    double GYiHerRlRO3872355 = 4462758;    double GYiHerRlRO23874403 = -112549785;    double GYiHerRlRO95675859 = -50941163;    double GYiHerRlRO85516893 = -7731176;    double GYiHerRlRO94163449 = 18971739;    double GYiHerRlRO54444753 = -361750480;    double GYiHerRlRO74144981 = -736637761;    double GYiHerRlRO73118154 = -753803543;    double GYiHerRlRO84478436 = -609860606;    double GYiHerRlRO95514843 = -332085088;    double GYiHerRlRO90861377 = -279357289;    double GYiHerRlRO34619793 = -292389592;    double GYiHerRlRO75539947 = -637482844;    double GYiHerRlRO17665667 = -216529277;    double GYiHerRlRO96748997 = -750240791;    double GYiHerRlRO24487983 = -419783631;    double GYiHerRlRO763687 = -272292334;    double GYiHerRlRO83476761 = -28644898;    double GYiHerRlRO626728 = -516965913;    double GYiHerRlRO73197716 = -856086606;    double GYiHerRlRO64047845 = 17612505;    double GYiHerRlRO85000760 = -253212534;    double GYiHerRlRO64887868 = -546204375;    double GYiHerRlRO77097709 = -157443400;    double GYiHerRlRO50170496 = -351725139;    double GYiHerRlRO88705615 = 23767251;    double GYiHerRlRO12835475 = -580347111;    double GYiHerRlRO98272133 = -105343700;    double GYiHerRlRO63468365 = -510931154;    double GYiHerRlRO50186353 = -363934140;    double GYiHerRlRO43024744 = -376876733;    double GYiHerRlRO36235886 = -383905811;    double GYiHerRlRO39885880 = -153892533;    double GYiHerRlRO50264075 = -463509414;    double GYiHerRlRO79970603 = -199123111;    double GYiHerRlRO98400726 = -977173252;    double GYiHerRlRO74998191 = -971573151;    double GYiHerRlRO98979910 = -785375162;    double GYiHerRlRO3801103 = -684179169;    double GYiHerRlRO48887601 = -644070692;    double GYiHerRlRO95623197 = -829884542;    double GYiHerRlRO58525032 = -562928564;    double GYiHerRlRO31642141 = 5146185;    double GYiHerRlRO70229553 = -743675948;    double GYiHerRlRO91475664 = 51446387;    double GYiHerRlRO86048709 = -174054289;    double GYiHerRlRO5953165 = -741896701;    double GYiHerRlRO23973140 = 16497919;    double GYiHerRlRO88554527 = -167983261;    double GYiHerRlRO42123586 = -342128947;     GYiHerRlRO8919034 = GYiHerRlRO57720640;     GYiHerRlRO57720640 = GYiHerRlRO3198089;     GYiHerRlRO3198089 = GYiHerRlRO95234934;     GYiHerRlRO95234934 = GYiHerRlRO25634030;     GYiHerRlRO25634030 = GYiHerRlRO1844614;     GYiHerRlRO1844614 = GYiHerRlRO60745125;     GYiHerRlRO60745125 = GYiHerRlRO56094218;     GYiHerRlRO56094218 = GYiHerRlRO1508624;     GYiHerRlRO1508624 = GYiHerRlRO19024127;     GYiHerRlRO19024127 = GYiHerRlRO95162034;     GYiHerRlRO95162034 = GYiHerRlRO85260489;     GYiHerRlRO85260489 = GYiHerRlRO81441313;     GYiHerRlRO81441313 = GYiHerRlRO3959081;     GYiHerRlRO3959081 = GYiHerRlRO42965895;     GYiHerRlRO42965895 = GYiHerRlRO95691031;     GYiHerRlRO95691031 = GYiHerRlRO1121338;     GYiHerRlRO1121338 = GYiHerRlRO30632266;     GYiHerRlRO30632266 = GYiHerRlRO59434751;     GYiHerRlRO59434751 = GYiHerRlRO70838998;     GYiHerRlRO70838998 = GYiHerRlRO70198727;     GYiHerRlRO70198727 = GYiHerRlRO54474281;     GYiHerRlRO54474281 = GYiHerRlRO83575658;     GYiHerRlRO83575658 = GYiHerRlRO30079935;     GYiHerRlRO30079935 = GYiHerRlRO10756499;     GYiHerRlRO10756499 = GYiHerRlRO30119186;     GYiHerRlRO30119186 = GYiHerRlRO10983237;     GYiHerRlRO10983237 = GYiHerRlRO26125332;     GYiHerRlRO26125332 = GYiHerRlRO80554271;     GYiHerRlRO80554271 = GYiHerRlRO83842957;     GYiHerRlRO83842957 = GYiHerRlRO22275129;     GYiHerRlRO22275129 = GYiHerRlRO70674051;     GYiHerRlRO70674051 = GYiHerRlRO84496803;     GYiHerRlRO84496803 = GYiHerRlRO97964551;     GYiHerRlRO97964551 = GYiHerRlRO3332354;     GYiHerRlRO3332354 = GYiHerRlRO69768179;     GYiHerRlRO69768179 = GYiHerRlRO31643186;     GYiHerRlRO31643186 = GYiHerRlRO16120577;     GYiHerRlRO16120577 = GYiHerRlRO65744397;     GYiHerRlRO65744397 = GYiHerRlRO82337041;     GYiHerRlRO82337041 = GYiHerRlRO20668503;     GYiHerRlRO20668503 = GYiHerRlRO81493111;     GYiHerRlRO81493111 = GYiHerRlRO41638806;     GYiHerRlRO41638806 = GYiHerRlRO85303525;     GYiHerRlRO85303525 = GYiHerRlRO66611569;     GYiHerRlRO66611569 = GYiHerRlRO60570145;     GYiHerRlRO60570145 = GYiHerRlRO87094442;     GYiHerRlRO87094442 = GYiHerRlRO74747350;     GYiHerRlRO74747350 = GYiHerRlRO86239452;     GYiHerRlRO86239452 = GYiHerRlRO30290197;     GYiHerRlRO30290197 = GYiHerRlRO3872355;     GYiHerRlRO3872355 = GYiHerRlRO23874403;     GYiHerRlRO23874403 = GYiHerRlRO95675859;     GYiHerRlRO95675859 = GYiHerRlRO85516893;     GYiHerRlRO85516893 = GYiHerRlRO94163449;     GYiHerRlRO94163449 = GYiHerRlRO54444753;     GYiHerRlRO54444753 = GYiHerRlRO74144981;     GYiHerRlRO74144981 = GYiHerRlRO73118154;     GYiHerRlRO73118154 = GYiHerRlRO84478436;     GYiHerRlRO84478436 = GYiHerRlRO95514843;     GYiHerRlRO95514843 = GYiHerRlRO90861377;     GYiHerRlRO90861377 = GYiHerRlRO34619793;     GYiHerRlRO34619793 = GYiHerRlRO75539947;     GYiHerRlRO75539947 = GYiHerRlRO17665667;     GYiHerRlRO17665667 = GYiHerRlRO96748997;     GYiHerRlRO96748997 = GYiHerRlRO24487983;     GYiHerRlRO24487983 = GYiHerRlRO763687;     GYiHerRlRO763687 = GYiHerRlRO83476761;     GYiHerRlRO83476761 = GYiHerRlRO626728;     GYiHerRlRO626728 = GYiHerRlRO73197716;     GYiHerRlRO73197716 = GYiHerRlRO64047845;     GYiHerRlRO64047845 = GYiHerRlRO85000760;     GYiHerRlRO85000760 = GYiHerRlRO64887868;     GYiHerRlRO64887868 = GYiHerRlRO77097709;     GYiHerRlRO77097709 = GYiHerRlRO50170496;     GYiHerRlRO50170496 = GYiHerRlRO88705615;     GYiHerRlRO88705615 = GYiHerRlRO12835475;     GYiHerRlRO12835475 = GYiHerRlRO98272133;     GYiHerRlRO98272133 = GYiHerRlRO63468365;     GYiHerRlRO63468365 = GYiHerRlRO50186353;     GYiHerRlRO50186353 = GYiHerRlRO43024744;     GYiHerRlRO43024744 = GYiHerRlRO36235886;     GYiHerRlRO36235886 = GYiHerRlRO39885880;     GYiHerRlRO39885880 = GYiHerRlRO50264075;     GYiHerRlRO50264075 = GYiHerRlRO79970603;     GYiHerRlRO79970603 = GYiHerRlRO98400726;     GYiHerRlRO98400726 = GYiHerRlRO74998191;     GYiHerRlRO74998191 = GYiHerRlRO98979910;     GYiHerRlRO98979910 = GYiHerRlRO3801103;     GYiHerRlRO3801103 = GYiHerRlRO48887601;     GYiHerRlRO48887601 = GYiHerRlRO95623197;     GYiHerRlRO95623197 = GYiHerRlRO58525032;     GYiHerRlRO58525032 = GYiHerRlRO31642141;     GYiHerRlRO31642141 = GYiHerRlRO70229553;     GYiHerRlRO70229553 = GYiHerRlRO91475664;     GYiHerRlRO91475664 = GYiHerRlRO86048709;     GYiHerRlRO86048709 = GYiHerRlRO5953165;     GYiHerRlRO5953165 = GYiHerRlRO23973140;     GYiHerRlRO23973140 = GYiHerRlRO88554527;     GYiHerRlRO88554527 = GYiHerRlRO42123586;     GYiHerRlRO42123586 = GYiHerRlRO8919034;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void oHvwAdtTYW10057869() {     double cauhcirHuk28107992 = -185903775;    double cauhcirHuk30018498 = -601813851;    double cauhcirHuk27857470 = -861519299;    double cauhcirHuk79526382 = -862033069;    double cauhcirHuk52640533 = -306525864;    double cauhcirHuk14601571 = -358648221;    double cauhcirHuk39735759 = -512851533;    double cauhcirHuk92351811 = -256151774;    double cauhcirHuk70505593 = -983357663;    double cauhcirHuk83392913 = -90884969;    double cauhcirHuk55912200 = -225118860;    double cauhcirHuk61748732 = -223180360;    double cauhcirHuk38876896 = -167038600;    double cauhcirHuk97628868 = -603680305;    double cauhcirHuk60167561 = -889018395;    double cauhcirHuk76481814 = -726263209;    double cauhcirHuk969925 = -824166799;    double cauhcirHuk21469194 = -9173434;    double cauhcirHuk80533292 = -72584693;    double cauhcirHuk66710098 = -448998551;    double cauhcirHuk70955676 = 23112866;    double cauhcirHuk80601611 = -470178165;    double cauhcirHuk86398507 = -431636830;    double cauhcirHuk81365466 = -600006340;    double cauhcirHuk26933853 = -922302416;    double cauhcirHuk77236039 = -62586955;    double cauhcirHuk22774169 = -362415031;    double cauhcirHuk74049432 = -406005216;    double cauhcirHuk86462180 = -510377878;    double cauhcirHuk48834825 = -226342523;    double cauhcirHuk88810105 = -473708309;    double cauhcirHuk48566500 = 47138814;    double cauhcirHuk25387093 = -403972187;    double cauhcirHuk63154855 = -257316158;    double cauhcirHuk54386094 = -531898102;    double cauhcirHuk48362067 = -679718321;    double cauhcirHuk68877225 = -402116065;    double cauhcirHuk47800092 = -591250542;    double cauhcirHuk21437081 = -419514765;    double cauhcirHuk13924797 = -130388641;    double cauhcirHuk31031964 = -997739024;    double cauhcirHuk14500304 = -486158402;    double cauhcirHuk3538971 = -46486142;    double cauhcirHuk13398485 = -544535751;    double cauhcirHuk10597227 = -599204289;    double cauhcirHuk10370387 = -624656792;    double cauhcirHuk11920782 = -486723966;    double cauhcirHuk55831929 = -349036504;    double cauhcirHuk24347943 = -604623148;    double cauhcirHuk70647692 = 69334538;    double cauhcirHuk11474540 = -457031193;    double cauhcirHuk44318337 = -796109165;    double cauhcirHuk34231062 = 73134922;    double cauhcirHuk37191075 = -244029012;    double cauhcirHuk39102193 = -887346069;    double cauhcirHuk47506380 = -715725610;    double cauhcirHuk43619990 = -70177021;    double cauhcirHuk46492004 = -161512960;    double cauhcirHuk52592529 = -939730654;    double cauhcirHuk75404494 = -143938910;    double cauhcirHuk91827402 = -996233191;    double cauhcirHuk65686326 = -6846317;    double cauhcirHuk5889631 = -745773896;    double cauhcirHuk21670768 = -657015141;    double cauhcirHuk94582808 = -617176660;    double cauhcirHuk7345700 = -172257674;    double cauhcirHuk36361639 = -819208174;    double cauhcirHuk75722041 = -909722443;    double cauhcirHuk43242774 = 28217796;    double cauhcirHuk11805494 = -109300075;    double cauhcirHuk7604590 = -224147144;    double cauhcirHuk53169833 = -132916258;    double cauhcirHuk32113 = -589658670;    double cauhcirHuk66608496 = -942196053;    double cauhcirHuk35678135 = -451259528;    double cauhcirHuk56455372 = -490728733;    double cauhcirHuk77062640 = -323692024;    double cauhcirHuk73000023 = -887101080;    double cauhcirHuk70768240 = 99197949;    double cauhcirHuk16563466 = -197645624;    double cauhcirHuk65315257 = -575862990;    double cauhcirHuk66942240 = 86621472;    double cauhcirHuk49701490 = -801382069;    double cauhcirHuk15814489 = -479712417;    double cauhcirHuk37360286 = -769311330;    double cauhcirHuk44491768 = -677599144;    double cauhcirHuk14335439 = 74003892;    double cauhcirHuk88196017 = -59943175;    double cauhcirHuk24052662 = -369970090;    double cauhcirHuk6879714 = -816172492;    double cauhcirHuk4742078 = -509541300;    double cauhcirHuk22385222 = -140603105;    double cauhcirHuk95207562 = -651519889;    double cauhcirHuk46032587 = -175575855;    double cauhcirHuk22097394 = -134155451;    double cauhcirHuk65345637 = -890892707;    double cauhcirHuk8610673 = -740384506;    double cauhcirHuk81868202 = -389471002;    double cauhcirHuk18815676 = -927359091;    double cauhcirHuk3251528 = -185903775;     cauhcirHuk28107992 = cauhcirHuk30018498;     cauhcirHuk30018498 = cauhcirHuk27857470;     cauhcirHuk27857470 = cauhcirHuk79526382;     cauhcirHuk79526382 = cauhcirHuk52640533;     cauhcirHuk52640533 = cauhcirHuk14601571;     cauhcirHuk14601571 = cauhcirHuk39735759;     cauhcirHuk39735759 = cauhcirHuk92351811;     cauhcirHuk92351811 = cauhcirHuk70505593;     cauhcirHuk70505593 = cauhcirHuk83392913;     cauhcirHuk83392913 = cauhcirHuk55912200;     cauhcirHuk55912200 = cauhcirHuk61748732;     cauhcirHuk61748732 = cauhcirHuk38876896;     cauhcirHuk38876896 = cauhcirHuk97628868;     cauhcirHuk97628868 = cauhcirHuk60167561;     cauhcirHuk60167561 = cauhcirHuk76481814;     cauhcirHuk76481814 = cauhcirHuk969925;     cauhcirHuk969925 = cauhcirHuk21469194;     cauhcirHuk21469194 = cauhcirHuk80533292;     cauhcirHuk80533292 = cauhcirHuk66710098;     cauhcirHuk66710098 = cauhcirHuk70955676;     cauhcirHuk70955676 = cauhcirHuk80601611;     cauhcirHuk80601611 = cauhcirHuk86398507;     cauhcirHuk86398507 = cauhcirHuk81365466;     cauhcirHuk81365466 = cauhcirHuk26933853;     cauhcirHuk26933853 = cauhcirHuk77236039;     cauhcirHuk77236039 = cauhcirHuk22774169;     cauhcirHuk22774169 = cauhcirHuk74049432;     cauhcirHuk74049432 = cauhcirHuk86462180;     cauhcirHuk86462180 = cauhcirHuk48834825;     cauhcirHuk48834825 = cauhcirHuk88810105;     cauhcirHuk88810105 = cauhcirHuk48566500;     cauhcirHuk48566500 = cauhcirHuk25387093;     cauhcirHuk25387093 = cauhcirHuk63154855;     cauhcirHuk63154855 = cauhcirHuk54386094;     cauhcirHuk54386094 = cauhcirHuk48362067;     cauhcirHuk48362067 = cauhcirHuk68877225;     cauhcirHuk68877225 = cauhcirHuk47800092;     cauhcirHuk47800092 = cauhcirHuk21437081;     cauhcirHuk21437081 = cauhcirHuk13924797;     cauhcirHuk13924797 = cauhcirHuk31031964;     cauhcirHuk31031964 = cauhcirHuk14500304;     cauhcirHuk14500304 = cauhcirHuk3538971;     cauhcirHuk3538971 = cauhcirHuk13398485;     cauhcirHuk13398485 = cauhcirHuk10597227;     cauhcirHuk10597227 = cauhcirHuk10370387;     cauhcirHuk10370387 = cauhcirHuk11920782;     cauhcirHuk11920782 = cauhcirHuk55831929;     cauhcirHuk55831929 = cauhcirHuk24347943;     cauhcirHuk24347943 = cauhcirHuk70647692;     cauhcirHuk70647692 = cauhcirHuk11474540;     cauhcirHuk11474540 = cauhcirHuk44318337;     cauhcirHuk44318337 = cauhcirHuk34231062;     cauhcirHuk34231062 = cauhcirHuk37191075;     cauhcirHuk37191075 = cauhcirHuk39102193;     cauhcirHuk39102193 = cauhcirHuk47506380;     cauhcirHuk47506380 = cauhcirHuk43619990;     cauhcirHuk43619990 = cauhcirHuk46492004;     cauhcirHuk46492004 = cauhcirHuk52592529;     cauhcirHuk52592529 = cauhcirHuk75404494;     cauhcirHuk75404494 = cauhcirHuk91827402;     cauhcirHuk91827402 = cauhcirHuk65686326;     cauhcirHuk65686326 = cauhcirHuk5889631;     cauhcirHuk5889631 = cauhcirHuk21670768;     cauhcirHuk21670768 = cauhcirHuk94582808;     cauhcirHuk94582808 = cauhcirHuk7345700;     cauhcirHuk7345700 = cauhcirHuk36361639;     cauhcirHuk36361639 = cauhcirHuk75722041;     cauhcirHuk75722041 = cauhcirHuk43242774;     cauhcirHuk43242774 = cauhcirHuk11805494;     cauhcirHuk11805494 = cauhcirHuk7604590;     cauhcirHuk7604590 = cauhcirHuk53169833;     cauhcirHuk53169833 = cauhcirHuk32113;     cauhcirHuk32113 = cauhcirHuk66608496;     cauhcirHuk66608496 = cauhcirHuk35678135;     cauhcirHuk35678135 = cauhcirHuk56455372;     cauhcirHuk56455372 = cauhcirHuk77062640;     cauhcirHuk77062640 = cauhcirHuk73000023;     cauhcirHuk73000023 = cauhcirHuk70768240;     cauhcirHuk70768240 = cauhcirHuk16563466;     cauhcirHuk16563466 = cauhcirHuk65315257;     cauhcirHuk65315257 = cauhcirHuk66942240;     cauhcirHuk66942240 = cauhcirHuk49701490;     cauhcirHuk49701490 = cauhcirHuk15814489;     cauhcirHuk15814489 = cauhcirHuk37360286;     cauhcirHuk37360286 = cauhcirHuk44491768;     cauhcirHuk44491768 = cauhcirHuk14335439;     cauhcirHuk14335439 = cauhcirHuk88196017;     cauhcirHuk88196017 = cauhcirHuk24052662;     cauhcirHuk24052662 = cauhcirHuk6879714;     cauhcirHuk6879714 = cauhcirHuk4742078;     cauhcirHuk4742078 = cauhcirHuk22385222;     cauhcirHuk22385222 = cauhcirHuk95207562;     cauhcirHuk95207562 = cauhcirHuk46032587;     cauhcirHuk46032587 = cauhcirHuk22097394;     cauhcirHuk22097394 = cauhcirHuk65345637;     cauhcirHuk65345637 = cauhcirHuk8610673;     cauhcirHuk8610673 = cauhcirHuk81868202;     cauhcirHuk81868202 = cauhcirHuk18815676;     cauhcirHuk18815676 = cauhcirHuk3251528;     cauhcirHuk3251528 = cauhcirHuk28107992;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FtktkMBepu16716614() {     double PNgHOQEIFp28101520 = -58427916;    double PNgHOQEIFp68590814 = -644408976;    double PNgHOQEIFp29170682 = -818647913;    double PNgHOQEIFp64606242 = -682573899;    double PNgHOQEIFp61480635 = -774881186;    double PNgHOQEIFp49407003 = -58081033;    double PNgHOQEIFp8283095 = -217387999;    double PNgHOQEIFp61565862 = 39670909;    double PNgHOQEIFp53245757 = -697003733;    double PNgHOQEIFp80032427 = -827691840;    double PNgHOQEIFp76232805 = -184590932;    double PNgHOQEIFp76040409 = 41341277;    double PNgHOQEIFp70729810 = -188697075;    double PNgHOQEIFp51473088 = -948833357;    double PNgHOQEIFp5339671 = -851731247;    double PNgHOQEIFp51046361 = 40941141;    double PNgHOQEIFp56822684 = -643781369;    double PNgHOQEIFp44469727 = -406245056;    double PNgHOQEIFp961562 = -690566960;    double PNgHOQEIFp38783862 = 38976998;    double PNgHOQEIFp50625100 = -787353508;    double PNgHOQEIFp58897877 = -980170048;    double PNgHOQEIFp23276904 = -890594672;    double PNgHOQEIFp25015575 = -480926862;    double PNgHOQEIFp55376542 = -248996269;    double PNgHOQEIFp93810116 = 29207117;    double PNgHOQEIFp92995750 = -663214744;    double PNgHOQEIFp9452958 = 85027795;    double PNgHOQEIFp99140627 = -56448193;    double PNgHOQEIFp24924696 = 98226103;    double PNgHOQEIFp22222784 = -860801543;    double PNgHOQEIFp33290358 = -344682948;    double PNgHOQEIFp10839516 = -605916403;    double PNgHOQEIFp63437692 = -133997890;    double PNgHOQEIFp36744132 = -791212027;    double PNgHOQEIFp91190256 = -759484067;    double PNgHOQEIFp80818592 = -501769443;    double PNgHOQEIFp17717772 = -586513221;    double PNgHOQEIFp56750678 = -32196181;    double PNgHOQEIFp85797689 = -839820327;    double PNgHOQEIFp18887631 = 14317078;    double PNgHOQEIFp197965 = -679950484;    double PNgHOQEIFp63822330 = -605616396;    double PNgHOQEIFp72630383 = -101500194;    double PNgHOQEIFp94615278 = 27536310;    double PNgHOQEIFp50798355 = -436701463;    double PNgHOQEIFp39618575 = -968004588;    double PNgHOQEIFp31215510 = -957797860;    double PNgHOQEIFp43006332 = -168387009;    double PNgHOQEIFp90693115 = -617690348;    double PNgHOQEIFp24733910 = -35586201;    double PNgHOQEIFp58865595 = -78254913;    double PNgHOQEIFp9679410 = -918919086;    double PNgHOQEIFp47441346 = -775948864;    double PNgHOQEIFp66091513 = -644245346;    double PNgHOQEIFp69203643 = -78257868;    double PNgHOQEIFp45313910 = -753814304;    double PNgHOQEIFp4155108 = -237721051;    double PNgHOQEIFp9229701 = -333577631;    double PNgHOQEIFp67670519 = -704088303;    double PNgHOQEIFp56411253 = -394866290;    double PNgHOQEIFp98830137 = -202415795;    double PNgHOQEIFp62425234 = -903880898;    double PNgHOQEIFp28321062 = -695229837;    double PNgHOQEIFp57809643 = -966890297;    double PNgHOQEIFp42942447 = -839907985;    double PNgHOQEIFp65200893 = -352742320;    double PNgHOQEIFp7292118 = 45300815;    double PNgHOQEIFp14728956 = -57621331;    double PNgHOQEIFp14149414 = 7752820;    double PNgHOQEIFp70227768 = -457289416;    double PNgHOQEIFp39104913 = 42731852;    double PNgHOQEIFp87719049 = -274048876;    double PNgHOQEIFp15163873 = -850746633;    double PNgHOQEIFp19896231 = -975340081;    double PNgHOQEIFp50427135 = -7403024;    double PNgHOQEIFp95075546 = -274553653;    double PNgHOQEIFp50646521 = -689094479;    double PNgHOQEIFp30400296 = -408463172;    double PNgHOQEIFp4578188 = -812294806;    double PNgHOQEIFp54191542 = -2788296;    double PNgHOQEIFp61780240 = -705416884;    double PNgHOQEIFp66446625 = -746585196;    double PNgHOQEIFp8447513 = -438757845;    double PNgHOQEIFp190786 = -866187697;    double PNgHOQEIFp63357189 = -682546631;    double PNgHOQEIFp23610948 = -425763862;    double PNgHOQEIFp63398170 = -829967540;    double PNgHOQEIFp97346178 = -489752544;    double PNgHOQEIFp67540489 = -612954159;    double PNgHOQEIFp45876347 = 94330237;    double PNgHOQEIFp76663485 = -164048393;    double PNgHOQEIFp8488072 = -152935591;    double PNgHOQEIFp89080158 = -328107878;    double PNgHOQEIFp29386436 = -344954038;    double PNgHOQEIFp20057494 = -783267127;    double PNgHOQEIFp37772730 = -776069587;    double PNgHOQEIFp35501269 = -910386560;    double PNgHOQEIFp14820740 = -134609898;    double PNgHOQEIFp51672832 = -58427916;     PNgHOQEIFp28101520 = PNgHOQEIFp68590814;     PNgHOQEIFp68590814 = PNgHOQEIFp29170682;     PNgHOQEIFp29170682 = PNgHOQEIFp64606242;     PNgHOQEIFp64606242 = PNgHOQEIFp61480635;     PNgHOQEIFp61480635 = PNgHOQEIFp49407003;     PNgHOQEIFp49407003 = PNgHOQEIFp8283095;     PNgHOQEIFp8283095 = PNgHOQEIFp61565862;     PNgHOQEIFp61565862 = PNgHOQEIFp53245757;     PNgHOQEIFp53245757 = PNgHOQEIFp80032427;     PNgHOQEIFp80032427 = PNgHOQEIFp76232805;     PNgHOQEIFp76232805 = PNgHOQEIFp76040409;     PNgHOQEIFp76040409 = PNgHOQEIFp70729810;     PNgHOQEIFp70729810 = PNgHOQEIFp51473088;     PNgHOQEIFp51473088 = PNgHOQEIFp5339671;     PNgHOQEIFp5339671 = PNgHOQEIFp51046361;     PNgHOQEIFp51046361 = PNgHOQEIFp56822684;     PNgHOQEIFp56822684 = PNgHOQEIFp44469727;     PNgHOQEIFp44469727 = PNgHOQEIFp961562;     PNgHOQEIFp961562 = PNgHOQEIFp38783862;     PNgHOQEIFp38783862 = PNgHOQEIFp50625100;     PNgHOQEIFp50625100 = PNgHOQEIFp58897877;     PNgHOQEIFp58897877 = PNgHOQEIFp23276904;     PNgHOQEIFp23276904 = PNgHOQEIFp25015575;     PNgHOQEIFp25015575 = PNgHOQEIFp55376542;     PNgHOQEIFp55376542 = PNgHOQEIFp93810116;     PNgHOQEIFp93810116 = PNgHOQEIFp92995750;     PNgHOQEIFp92995750 = PNgHOQEIFp9452958;     PNgHOQEIFp9452958 = PNgHOQEIFp99140627;     PNgHOQEIFp99140627 = PNgHOQEIFp24924696;     PNgHOQEIFp24924696 = PNgHOQEIFp22222784;     PNgHOQEIFp22222784 = PNgHOQEIFp33290358;     PNgHOQEIFp33290358 = PNgHOQEIFp10839516;     PNgHOQEIFp10839516 = PNgHOQEIFp63437692;     PNgHOQEIFp63437692 = PNgHOQEIFp36744132;     PNgHOQEIFp36744132 = PNgHOQEIFp91190256;     PNgHOQEIFp91190256 = PNgHOQEIFp80818592;     PNgHOQEIFp80818592 = PNgHOQEIFp17717772;     PNgHOQEIFp17717772 = PNgHOQEIFp56750678;     PNgHOQEIFp56750678 = PNgHOQEIFp85797689;     PNgHOQEIFp85797689 = PNgHOQEIFp18887631;     PNgHOQEIFp18887631 = PNgHOQEIFp197965;     PNgHOQEIFp197965 = PNgHOQEIFp63822330;     PNgHOQEIFp63822330 = PNgHOQEIFp72630383;     PNgHOQEIFp72630383 = PNgHOQEIFp94615278;     PNgHOQEIFp94615278 = PNgHOQEIFp50798355;     PNgHOQEIFp50798355 = PNgHOQEIFp39618575;     PNgHOQEIFp39618575 = PNgHOQEIFp31215510;     PNgHOQEIFp31215510 = PNgHOQEIFp43006332;     PNgHOQEIFp43006332 = PNgHOQEIFp90693115;     PNgHOQEIFp90693115 = PNgHOQEIFp24733910;     PNgHOQEIFp24733910 = PNgHOQEIFp58865595;     PNgHOQEIFp58865595 = PNgHOQEIFp9679410;     PNgHOQEIFp9679410 = PNgHOQEIFp47441346;     PNgHOQEIFp47441346 = PNgHOQEIFp66091513;     PNgHOQEIFp66091513 = PNgHOQEIFp69203643;     PNgHOQEIFp69203643 = PNgHOQEIFp45313910;     PNgHOQEIFp45313910 = PNgHOQEIFp4155108;     PNgHOQEIFp4155108 = PNgHOQEIFp9229701;     PNgHOQEIFp9229701 = PNgHOQEIFp67670519;     PNgHOQEIFp67670519 = PNgHOQEIFp56411253;     PNgHOQEIFp56411253 = PNgHOQEIFp98830137;     PNgHOQEIFp98830137 = PNgHOQEIFp62425234;     PNgHOQEIFp62425234 = PNgHOQEIFp28321062;     PNgHOQEIFp28321062 = PNgHOQEIFp57809643;     PNgHOQEIFp57809643 = PNgHOQEIFp42942447;     PNgHOQEIFp42942447 = PNgHOQEIFp65200893;     PNgHOQEIFp65200893 = PNgHOQEIFp7292118;     PNgHOQEIFp7292118 = PNgHOQEIFp14728956;     PNgHOQEIFp14728956 = PNgHOQEIFp14149414;     PNgHOQEIFp14149414 = PNgHOQEIFp70227768;     PNgHOQEIFp70227768 = PNgHOQEIFp39104913;     PNgHOQEIFp39104913 = PNgHOQEIFp87719049;     PNgHOQEIFp87719049 = PNgHOQEIFp15163873;     PNgHOQEIFp15163873 = PNgHOQEIFp19896231;     PNgHOQEIFp19896231 = PNgHOQEIFp50427135;     PNgHOQEIFp50427135 = PNgHOQEIFp95075546;     PNgHOQEIFp95075546 = PNgHOQEIFp50646521;     PNgHOQEIFp50646521 = PNgHOQEIFp30400296;     PNgHOQEIFp30400296 = PNgHOQEIFp4578188;     PNgHOQEIFp4578188 = PNgHOQEIFp54191542;     PNgHOQEIFp54191542 = PNgHOQEIFp61780240;     PNgHOQEIFp61780240 = PNgHOQEIFp66446625;     PNgHOQEIFp66446625 = PNgHOQEIFp8447513;     PNgHOQEIFp8447513 = PNgHOQEIFp190786;     PNgHOQEIFp190786 = PNgHOQEIFp63357189;     PNgHOQEIFp63357189 = PNgHOQEIFp23610948;     PNgHOQEIFp23610948 = PNgHOQEIFp63398170;     PNgHOQEIFp63398170 = PNgHOQEIFp97346178;     PNgHOQEIFp97346178 = PNgHOQEIFp67540489;     PNgHOQEIFp67540489 = PNgHOQEIFp45876347;     PNgHOQEIFp45876347 = PNgHOQEIFp76663485;     PNgHOQEIFp76663485 = PNgHOQEIFp8488072;     PNgHOQEIFp8488072 = PNgHOQEIFp89080158;     PNgHOQEIFp89080158 = PNgHOQEIFp29386436;     PNgHOQEIFp29386436 = PNgHOQEIFp20057494;     PNgHOQEIFp20057494 = PNgHOQEIFp37772730;     PNgHOQEIFp37772730 = PNgHOQEIFp35501269;     PNgHOQEIFp35501269 = PNgHOQEIFp14820740;     PNgHOQEIFp14820740 = PNgHOQEIFp51672832;     PNgHOQEIFp51672832 = PNgHOQEIFp28101520;}
// Junk Finished
