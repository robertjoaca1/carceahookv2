#pragma once

#include "valve_sdk/csgostructs.hpp"
#include "helpers/vfunc_hook.hpp"
#include <d3d9.h>

struct LayerRecord
{
	LayerRecord()
	{
		m_nOrder = 0;
		m_nSequence = 0;
		m_flWeight = 0.f;
		m_flCycle = 0.f;
	}

	LayerRecord(const LayerRecord& src)
	{
		m_nOrder = src.m_nOrder;
		m_nSequence = src.m_nSequence;
		m_flWeight = src.m_flWeight;
		m_flCycle = src.m_flCycle;
	}

	uint32_t m_nOrder;
	uint32_t m_nSequence;
	float_t m_flWeight;
	float_t m_flCycle;
};

namespace index
{
	constexpr auto EndScene					= 42;
	constexpr auto Reset					= 16;
    constexpr auto PaintTraverse            = 41;
    constexpr auto CreateMove               = 24;
    constexpr auto FrameStageNotify         = 37;
    constexpr auto DrawModelExecute         = 21;
    constexpr auto DoPostScreenSpaceEffects = 44;
	constexpr auto SvCheatsGetBool          = 13;
	constexpr auto OverrideView             = 18;
	constexpr auto LockCursor               = 67;
	constexpr auto ListLeavesInBox			= 6;
	constexpr auto WriteUserCmdDeltaToBuffer = 24;
	constexpr auto BeginFrame = 9;
}

namespace Hooks
{
	extern QAngle RealAngle;
	extern QAngle FakeAngle;
	extern bool SendPacket;
	extern int TickCount;
	extern matrix3x4_t FakeLagMatrix[128];
	extern matrix3x4_t FakeAngleMatrix[128];
	extern Vector FakeOrigin;
	extern int ShotsFired[65];
	extern int ShotsHit[65];
	extern int TickBaseShift;
	extern CUserCmd* cmd;

    void Initialize();
    void Shutdown();

	inline vfunc_hook direct3d_hook;
    inline vfunc_hook hlclient_hook;
	inline vfunc_hook vguipanel_hook;
	inline vfunc_hook vguisurf_hook;
	inline vfunc_hook mdlrender_hook;
	inline vfunc_hook viewrender_hook;
	inline vfunc_hook clientmode_hook;
	inline vfunc_hook sv_cheats;
	inline vfunc_hook bsp_query_hook;
	inline vfunc_hook studio_render_hook;

	long __stdcall hkEndScene(IDirect3DDevice9* device);
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
    //void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket);
	bool __fastcall hkCreateMove(IClientMode* thisptr, void* edx, float sample_frametime, CUserCmd* cmd);
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce);
	void __fastcall hkDrawModelExecute(void* _this, int, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
    void __fastcall hkFrameStageNotify(void* _this, int, ClientFrameStage_t stage);
	void __fastcall hkOverrideView(void* _this, int, CViewSetup * vsView);
	void __fastcall hkLockCursor(void* _this);
    int  __fastcall hkDoPostScreenEffects(void* _this, int, int a1);
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx);
	int __fastcall hkListLeavesInBox(void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax);
	bool __fastcall hkWriteUserCmdDeltaToBuffer(IBaseClientDLL* ECX, void* EDX, int nSlot, bf_write* buf, int from, int to, bool isNewCmd);
	void __fastcall hkBeginFrame(void* thisptr);












	struct TickInfo
	{
		TickInfo()
		{
			HeadPosition.Init();
			SimulationTime = -1.f;
			Origin.Init();
			Angles.Init();
			Mins.Init();
			Maxs.Init();
			Velocity.Init();
			MatrixBuilt = false;
		}
		TickInfo(C_BasePlayer* Player)
		{
			HeadPosition = Player->GetHitboxPos(HITBOX_HEAD);
			SimulationTime = Player->m_flSimulationTime();

			Origin = Player->m_vecOrigin();
			Angles = Player->m_angEyeAngles();
			Mins = Player->GetCollideable()->OBBMins();
			Maxs = Player->GetCollideable()->OBBMaxs();
			Flags = Player->m_fFlags();
			Velocity = Player->m_vecVelocity();

			int layerCount = Player->GetNumAnimOverlays();
			for (int i = 0; i < layerCount; i++)
			{
				AnimationLayer* currentLayer = Player->GetAnimOverlay(i);
				LayerRecords[i].m_nOrder = currentLayer->m_nOrder;
				LayerRecords[i].m_nSequence = currentLayer->m_nSequence;
				LayerRecords[i].m_flWeight = currentLayer->m_flWeight;
				LayerRecords[i].m_flCycle = currentLayer->m_flCycle;
			}
			PoseParams = Player->m_flPoseParameter();

			
			MatrixBuilt = false;
			if (Player->SetupBones2(BoneMatrix, 128, BONE_USED_BY_ANYTHING, g_GlobalVars->curtime))
				MatrixBuilt = true;
		}
		Vector HeadPosition;
		float SimulationTime;
		int32_t Flags;
		Vector Origin;
		QAngle Angles;
		Vector Mins;
		Vector Maxs;
		Vector Velocity;

		std::array<float_t, 24> PoseParams;
		std::array<LayerRecord, 15> LayerRecords;

		bool MatrixBuilt;
		matrix3x4_t BoneMatrix[128];
	};
}
