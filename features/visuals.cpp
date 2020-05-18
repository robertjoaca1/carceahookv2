#include "visuals.hpp"
#include "../hooks.hpp"
#include "legitbot.hpp"
#include "ragebot.hpp"
#include "Autowall.hpp"

void Render::CreateFonts()
{
	TahomaBold12 = g_VGuiSurface->CreateFont_();
	Visitor9 = g_VGuiSurface->CreateFont_();
	Tahoma12 = g_VGuiSurface->CreateFont_();
	SmallFonts8 = g_VGuiSurface->CreateFont_();

	g_VGuiSurface->SetFontGlyphSet(TahomaBold12, "Tahoma Bold", 12, 400, 0, 0, FONTFLAG_DROPSHADOW);
	g_VGuiSurface->SetFontGlyphSet(Visitor9, "Visitor TT2 BRK", 9, 400, 0, 0, FONTFLAG_OUTLINE);
	g_VGuiSurface->SetFontGlyphSet(Tahoma12, "Tahoma", 12, 700, 0, 0, FONTFLAG_DROPSHADOW);
	g_VGuiSurface->SetFontGlyphSet(SmallFonts8, "Small Fonts", 8, 400, 0, 0, FONTFLAG_OUTLINE);
}
void Render::Text(int X, int Y, const char* Text, vgui::HFont Font, Color DrawColor, bool Center/*, bool eatmyasscheeks*/)
{
	std::wstring WText = std::wstring(std::string_view(Text).begin(), std::string_view(Text).end());
	g_VGuiSurface->DrawSetTextFont(Font);
	g_VGuiSurface->DrawSetTextColor(DrawColor);
	if (Center)
	{
		int TextWidth, TextHeight;
		Render::Get().TextSize(TextWidth, TextHeight, Text, Font);
		g_VGuiSurface->DrawSetTextPos(X - TextWidth / 2, Y);
	}
	else
		g_VGuiSurface->DrawSetTextPos(X, Y);
	g_VGuiSurface->DrawPrintText(WText.c_str(), wcslen(WText.c_str()));
}
void Render::TextSize(int& Width, int& Height, const char* Text, vgui::HFont Font)
{
	std::wstring WText = std::wstring(std::string_view(Text).begin(), std::string_view(Text).end());
	g_VGuiSurface->GetTextSize(Font, WText.c_str(), Width, Height);
}
void Render::FilledRectange(int X1, int Y1, int X2, int Y2, Color DrawColor)
{
	g_VGuiSurface->DrawSetColor(DrawColor);
	g_VGuiSurface->DrawFilledRect(X1, Y1, X2, Y2);
}
void Render::OutlinedRectange(int X1, int Y1, int X2, int Y2, Color DrawColor)
{
	g_VGuiSurface->DrawSetColor(DrawColor);
	g_VGuiSurface->DrawOutlinedRect(X1, Y1, X2, Y2);
}
void Render::Line(int X1, int Y1, int X2, int Y2, Color DrawColor)
{
	g_VGuiSurface->DrawSetColor(DrawColor);
	g_VGuiSurface->DrawLine(X1, Y1, X2, Y2);
}
void AngleMatrix(const QAngle angles, matrix3x4_t& matrix)
{
	float sr, sp, sy, cr, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	sr = sin(DEG2RAD(angles[2]));
	cr = cos(DEG2RAD(angles[2]));

	//matrix = (YAW * PITCH) * ROLL
	matrix[0][0] = cp * cy;
	matrix[1][0] = cp * sy;
	matrix[2][0] = -sp;

	float crcy = cr * cy;
	float crsy = cr * sy;
	float srcy = sr * cy;
	float srsy = sr * sy;

	matrix[0][1] = sp * srcy - crsy;
	matrix[1][1] = sp * srsy + crcy;
	matrix[2][1] = sr * cp;

	matrix[0][2] = (sp * crcy + srsy);
	matrix[1][2] = (sp * crsy - srcy);
	matrix[2][2] = cr * cp;

	matrix[0][3] = 0.0f;
	matrix[1][3] = 0.0f;
	matrix[2][3] = 0.0f;
}

void MatrixSetColumn(const Vector& in, int column, matrix3x4_t& out)
{
	out[0][column] = in.x;
	out[1][column] = in.y;
	out[2][column] = in.z;
}

void AngleMatrix(const QAngle& angles, const Vector& position, matrix3x4_t& matrix_out)
{
	AngleMatrix(angles, matrix_out);
	MatrixSetColumn(position, 3, matrix_out);
}

void MatrixCopy(const matrix3x4_t& source, matrix3x4_t& target)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			target[i][j] = source[i][j];
		}
	}
}

void MatrixMultiply(matrix3x4_t& in1, const matrix3x4_t& in2)
{
	matrix3x4_t out;
	if (&in1 == &out)
	{
		matrix3x4_t in1b;
		MatrixCopy(in1, in1b);
		MatrixMultiply(in1b, in2);
		return;
	}
	if (&in2 == &out)
	{
		matrix3x4_t in2b;
		MatrixCopy(in2, in2b);
		MatrixMultiply(in1, in2b);
		return;
	}
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
		in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
		in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
		in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
		in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
		in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
		in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
		in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
		in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
		in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
		in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
		in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
		in1[2][2] * in2[2][3] + in1[2][3];

	in1 = out;
}
FORCEINLINE float DotProduct(const float* v1, const float* v2) {
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}
void VectorRotate(const float* in1, const matrix3x4_t& in2, float* out)
{
	out[0] = DotProduct(in1, in2[0]);
	out[1] = DotProduct(in1, in2[1]);
	out[2] = DotProduct(in1, in2[2]);
}

void VectorRotate(const Vector& in1, const matrix3x4_t& in2, Vector& out)
{
	VectorRotate(&in1.x, in2, &out.x);
}

void VectorRotate(const Vector& in1, const QAngle& in2, Vector& out)
{
	matrix3x4_t matRotate;
	AngleMatrix(in2, matRotate);
	VectorRotate(in1, matRotate, out);
}
Chams::Chams()
{
	std::ofstream("csgo\\materials\\material_textured.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\material_textured_ignorez.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\material_flat.vmt") << R"#("UnlitGeneric"
{
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\material_flat_ignorez.vmt") << R"#("UnlitGeneric"
{
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

	materialRegular = g_MatSystem->FindMaterial("material_textured", TEXTURE_GROUP_MODEL);
	materialRegularIgnoreZ = g_MatSystem->FindMaterial("material_textured_ignorez", TEXTURE_GROUP_MODEL);
	materialFlat = g_MatSystem->FindMaterial("material_flat", TEXTURE_GROUP_MODEL);
	materialFlatIgnoreZ = g_MatSystem->FindMaterial("material_flat_ignorez", TEXTURE_GROUP_MODEL);
	materialDogtag = g_MatSystem->FindMaterial("models\\inventory_items\\dogtags\\dogtags_outline", TEXTURE_GROUP_OTHER);
}
Chams::~Chams()
{
	std::remove("csgo\\materials\\material_textured.vmt");
	std::remove("csgo\\materials\\material_textured_ignorez.vmt");
	std::remove("csgo\\materials\\material_flat.vmt");
	std::remove("csgo\\materials\\material_flat_ignorez.vmt");
	std::remove("csgo\\materials\\material_reflective.vmt");
	std::remove("csgo\\materials\\material_reflective_ignorez.vmt");
}
std::vector<const char*> vistasmoke_mats =
{
		//"particle/vistasmokev1/vistasmokev1_fire",
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
};
void Chams::OnDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* matrix)
{
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);
	const auto mdl = info.pModel;

	if (g_MdlRender->IsForcedMaterialOverride())
		return fnDME(g_MdlRender, 0, ctx, state, info, matrix);

	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	bool is_arm = strstr(mdl->szName, "arms") != nullptr;

	static std::vector<IMaterial*> Materials =
	{
		nullptr,
		materialRegular,
		materialFlat,
		materialDogtag,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};
	static std::vector<IMaterial*> ZMaterials =
	{
		nullptr,
		materialRegularIgnoreZ,
		materialFlatIgnoreZ,
		materialDogtag,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};

	float color[3] = {
		Variables.VisualsChamsColor[0],
		Variables.VisualsChamsColor[1],
		Variables.VisualsChamsColor[2] };
	float zcolor[3] = {
		Variables.VisualsChamsColorIgnoreZ[0],
		Variables.VisualsChamsColorIgnoreZ[1],
		Variables.VisualsChamsColorIgnoreZ[2] };
	float backtrackcolor[3] = {
		Variables.VisualsChamsBacktrackColor[0],
		Variables.VisualsChamsBacktrackColor[1],
		Variables.VisualsChamsBacktrackColor[2] };

	for (auto mat_s : vistasmoke_mats)
	{
		IMaterial* mat = g_MatSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
		mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, Variables.VisualsNoSmoke);
	}

	if (is_player)
	{
		auto entity = C_BasePlayer::GetPlayerByIndex(info.entity_index);

		if (g_LocalPlayer && entity && entity->IsAlive() && !entity->IsDormant())
		{
			if (entity->IsEnemy() && Variables.VisualsChamsEnabled)
			{
				if (Variables.RageAimbotEnabled && RageAimbot::Get().BacktrackRecords[info.entity_index].size() > 0)
				{
					switch (Variables.VisualsChamsBacktrack)
					{
					case 1:
						for (int t = 0; t < RageAimbot::Get().BacktrackRecords[info.entity_index].size(); t++)
						{
							if (!RageAimbot::Get().BacktrackRecords[info.entity_index].at(t).MatrixBuilt
								|| !RageAimbot::Get().BacktrackRecords[info.entity_index].at(t).BoneMatrix)
								continue;
							g_RenderView->SetColorModulation(backtrackcolor);
							g_RenderView->SetBlend(float(Variables.VisualsChamsBacktrackAlpha) / 255.f);
							g_MdlRender->ForcedMaterialOverride(Variables.VisualsChamsIgnoreZ ? ZMaterials.at(Variables.VisualsChamsBacktrackMaterial) : Materials.at(Variables.VisualsChamsBacktrackMaterial));
							fnDME(g_MdlRender, 0, ctx, state, info, RageAimbot::Get().BacktrackRecords[info.entity_index].at(t).BoneMatrix);
						}
						break;
					case 2:
						if (RageAimbot::Get().BacktrackRecords[info.entity_index].back().MatrixBuilt
							&& RageAimbot::Get().BacktrackRecords[info.entity_index].back().BoneMatrix)
						{
							g_RenderView->SetColorModulation(backtrackcolor);
							g_RenderView->SetBlend(float(Variables.VisualsChamsBacktrackAlpha) / 255.f);
							g_MdlRender->ForcedMaterialOverride(Variables.VisualsChamsIgnoreZ ? ZMaterials.at(Variables.VisualsChamsBacktrackMaterial) : Materials.at(Variables.VisualsChamsBacktrackMaterial));
							fnDME(g_MdlRender, 0, ctx, state, info, RageAimbot::Get().BacktrackRecords[info.entity_index].back().BoneMatrix);
						}
						break;
					}
				}
				else if (Variables.LegitBacktrackEnabled && LegitBacktrack::Get().BacktrackRecords[info.entity_index].size() > 0)
				{
					switch (Variables.VisualsChamsBacktrack)
					{
					case 1:
						for (int t = 0; t < LegitBacktrack::Get().BacktrackRecords[info.entity_index].size(); t++)
						{
							if (!LegitBacktrack::Get().BacktrackRecords[info.entity_index].at(t).MatrixBuilt
								|| !LegitBacktrack::Get().BacktrackRecords[info.entity_index].at(t).BoneMatrix)
								continue;
							g_RenderView->SetColorModulation(backtrackcolor);
							g_RenderView->SetBlend(float(Variables.VisualsChamsBacktrackAlpha) / 255.f);
							g_MdlRender->ForcedMaterialOverride(Variables.VisualsChamsIgnoreZ ? ZMaterials.at(Variables.VisualsChamsBacktrackMaterial) : Materials.at(Variables.VisualsChamsBacktrackMaterial));
							fnDME(g_MdlRender, 0, ctx, state, info, LegitBacktrack::Get().BacktrackRecords[info.entity_index].at(t).BoneMatrix);
						}
						break;
					case 2:
						if (LegitBacktrack::Get().BacktrackRecords[info.entity_index].back().MatrixBuilt
							&& LegitBacktrack::Get().BacktrackRecords[info.entity_index].back().BoneMatrix)
						{
							g_RenderView->SetColorModulation(backtrackcolor);
							g_RenderView->SetBlend(float(Variables.VisualsChamsBacktrackAlpha) / 255.f);
							g_MdlRender->ForcedMaterialOverride(Variables.VisualsChamsIgnoreZ ? ZMaterials.at(Variables.VisualsChamsBacktrackMaterial) : Materials.at(Variables.VisualsChamsBacktrackMaterial));
							fnDME(g_MdlRender, 0, ctx, state, info, LegitBacktrack::Get().BacktrackRecords[info.entity_index].back().BoneMatrix);
						}
						break;
					}
				}
				if (Variables.VisualsChamsIgnoreZ)
				{
					g_RenderView->SetColorModulation(zcolor);
					g_RenderView->SetBlend(float(Variables.VisualsChamsAlpha) / 255.f);
					g_MdlRender->ForcedMaterialOverride(ZMaterials.at(Variables.VisualsChamsMaterial));
					fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				}
				g_RenderView->SetColorModulation(color);
				g_RenderView->SetBlend(float(Variables.VisualsChamsAlpha) / 255.f);
				g_MdlRender->ForcedMaterialOverride(Materials.at(Variables.VisualsChamsMaterial));
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			}
			else if (!entity->IsEnemy() && entity != g_LocalPlayer && false)
			{

			}
			else if (entity == g_LocalPlayer && Variables.VisualsLocalChamsEnabled)
			{
				if (Variables.VisualsLocalChamsEnabledFakelag && Variables.MiscFakelagChoke)
				{
					g_RenderView->SetColorModulation(Variables.VisualsLocalChamsColorFakelag);
					g_RenderView->SetBlend(float(Variables.VisualsLocalChamsAlphaFakelag) / 255.f);
					g_MdlRender->ForcedMaterialOverride(Materials.at(Variables.VisualsLocalChamsMaterialFakelag));
					fnDME(g_MdlRender, 0, ctx, state, info, Hooks::FakeLagMatrix);
				}
				if ((Variables.LegitAntiaimEnabled || Variables.RageAntiaimEnabled) && Variables.VisualsLocalChamsEnabledDesync)
				{
					if (RageAimbot::Get().m_got_fake_matrix)
					{
						for (auto& i : Hooks::FakeAngleMatrix)
						{
							i[0][3] += info.origin.x;
							i[1][3] += info.origin.y;
							i[2][3] += info.origin.z;
						}

						g_RenderView->SetColorModulation(Variables.VisualsLocalChamsColorDesync);
						g_RenderView->SetBlend(float(Variables.VisualsLocalChamsAlphaDesync) / 255.f);
						g_MdlRender->ForcedMaterialOverride(Materials.at(Variables.VisualsLocalChamsMaterialDesync));
						fnDME(g_MdlRender, 0, ctx, state, info, Hooks::FakeAngleMatrix);

						for (auto& i : Hooks::FakeAngleMatrix)
						{
							i[0][3] -= info.origin.x;
							i[1][3] -= info.origin.y;
							i[2][3] -= info.origin.z;
						}
					}
				}
				g_RenderView->SetColorModulation(Variables.VisualsLocalChamsColor);
				g_RenderView->SetBlend(float(Variables.VisualsLocalChamsAlpha) / 255.f);
				g_MdlRender->ForcedMaterialOverride(Materials.at(Variables.VisualsLocalChamsMaterial));
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			}
		}
	}
	fnDME(g_MdlRender, 0, ctx, state, info, matrix);
	g_MdlRender->ForcedMaterialOverride(nullptr);
}

Glow::Glow()
{
}

Glow::~Glow()
{
	// We cannot call shutdown here unfortunately.
	// Reason is not very straightforward but anyways:
	// - This destructor will be called when the dll unloads
	//   but it cannot distinguish between manual unload 
	//   (pressing the Unload button or calling FreeLibrary)
	//   or unload due to game exit.
	//   What that means is that this destructor will be called
	//   when the game exits.
	// - When the game is exiting, other dlls might already 
	//   have been unloaded before us, so it is not safe to 
	//   access intermodular variables or functions.
	//   
	//   Trying to call Shutdown here will crash CSGO when it is
	//   exiting (because we try to access g_GlowObjManager).
	//
}

void Glow::Shutdown()
{
	// Remove glow from all entities
	for (auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++) {
		auto& glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
		auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

		if (glowObject.IsUnused())
			continue;

		if (!entity || entity->IsDormant())
			continue;

		glowObject.m_flAlpha = 0.0f;
	}
}

void Glow::Run()
{
	if (Variables.VisualsGlowEnabled || Variables.VisualsLocalGlowEnabled)
	{
		for (auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++) {
			auto& glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
			auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

			if (glowObject.IsUnused())
				continue;

			if (!entity || entity->IsDormant())
				continue;

			auto class_id = entity->GetClientClass()->m_ClassID;
			auto color = Color{};
			int style = 0;
			switch (class_id) {
			case ClassId_CCSPlayer:
				if (!entity->IsAlive())
					continue;
				if (entity->IsEnemy() && Variables.VisualsGlowEnabled)
				{
					color = Color(
						int(Variables.VisualsGlowColor[0] * 255),
						int(Variables.VisualsGlowColor[1] * 255),
						int(Variables.VisualsGlowColor[2] * 255),
						int(Variables.VisualsGlowAlpha));
					style = Variables.VisualsGlowGlowstyle;
				}
				else if (entity == g_LocalPlayer && Variables.VisualsLocalGlowEnabled)
				{
					color = Color(
						int(Variables.VisualsLocalGlowColor[0] * 255),
						int(Variables.VisualsLocalGlowColor[1] * 255),
						int(Variables.VisualsLocalGlowColor[2] * 255),
						int(Variables.VisualsLocalGlowAlpha));
					style = Variables.VisualsLocalGlowGlowstyle;
				}
				break;
				/*case ClassId_CPlantedC4:
					color = Color(255, 255, 255, 170);
					break;
				default:
					if (entity->IsWeapon())
						color = Color(255, 255, 255, 170);*/
			}

			glowObject.m_flRed = color.r() / 255.0f;
			glowObject.m_flGreen = color.g() / 255.0f;
			glowObject.m_flBlue = color.b() / 255.0f;
			glowObject.m_flAlpha = color.a() / 255.0f;
			glowObject.m_bRenderWhenOccluded = true;
			glowObject.m_bRenderWhenUnoccluded = false;
			glowObject.m_nGlowStyle = style;
		}
	}
}

RECT Visuals::GetBBox(C_BasePlayer* Player, Vector TransformedPoints[]) //not pasted ;))
{
	RECT rect{};
	auto collideable = Player->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = Player->m_rgflCoordinateFrame();

	Vector points[] =
	{
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector pos = Player->GetAbsOrigin();
	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++)
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
		else
			TransformedPoints[i] = screen_points[i];

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++)
	{
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}
bool Visuals::Begin(C_BasePlayer* Player)
{
	Context.Player = Player;

	if (!Context.Player->IsEnemy())
		return false;

	if (Context.Player->IsDormant())
		return false;

	auto head = Context.Player->GetHitboxPos(HITBOX_HEAD);
	auto origin = Context.Player->GetAbsOrigin();

	head.z += 15;

	if (!Math::WorldToScreen(head, Context.HeadPos) ||
		!Math::WorldToScreen(origin, Context.Origin))
		return false;

	Vector points_transformed[8];
	RECT Box = GetBBox(Context.Player, points_transformed);

	Context.Box = Box;
	Context.Box.top = Box.bottom;
	Context.Box.bottom = Box.top;

	return true;
}

void Visuals::Box()
{
	Render::Get().OutlinedRectange(Context.Box.left - 1, Context.Box.top - 1, Context.Box.right + 1, Context.Box.bottom + 1, Color(0, 0, 0, 100));
	Render::Get().OutlinedRectange(Context.Box.left + 1, Context.Box.top + 1, Context.Box.right - 1, Context.Box.bottom - 1, Color(0, 0, 0, 100));
	Render::Get().OutlinedRectange(Context.Box.left, Context.Box.top, Context.Box.right, Context.Box.bottom, Color(175, 175, 175, 255));
}
void Visuals::Name()
{
	player_info_t PlayerInfo;
	g_EngineClient->GetPlayerInfo(Context.Player->EntIndex(), &PlayerInfo);

	int TextWidth, TextHeight;
	Render::Get().TextSize(TextWidth, TextHeight, PlayerInfo.szName, Render::Get().Visitor9);
	Render::Get().Text(Context.Box.left + (Context.Box.right - Context.Box.left) / 2, Context.Box.top - TextHeight + 1, PlayerInfo.szName, Render::Get().Visitor9, Color(255, 255, 255, 100), true);
}
void Visuals::Weapon()
{
	C_BaseCombatWeapon* Weapon = Context.Player->m_hActiveWeapon();
	if (!Weapon) return;
	std::string WeaponName = std::string(Weapon->GetCSWeaponData()->szHudName + std::string("(") + std::to_string(Weapon->m_iClip1()) + std::string("/") + std::to_string(Weapon->m_iPrimaryReserveAmmoCount()) + std::string(")"));
	WeaponName.erase(0, 13);
	int TextWidth, TextHeight;
	Render::Get().TextSize(TextWidth, TextHeight, WeaponName.c_str(), Render::Get().Visitor9);
	Render::Get().Text(Context.Box.left + (Context.Box.right - Context.Box.left) / 2, Context.Box.bottom + 1, WeaponName.c_str(), Render::Get().Visitor9, Color(255, 255, 255, 100), true);
}
void Visuals::Health()
{
	int HealthValue = std::clamp(Context.Player->m_iHealth(), 0, 100);
	
	float Height = (Context.Box.bottom - Context.Box.top - 1) * float(HealthValue / 100.f);

	Render::Get().FilledRectange(Context.Box.left - 7, Context.Box.top, Context.Box.left - 3, Context.Box.bottom, Color(0, 0, 0, 100));
	Render::Get().FilledRectange(Context.Box.left - 6, Context.Box.top + 1, Context.Box.left - 4, Context.Box.top + Height, Color(66, 207, 66, 255));
}
void Visuals::Skeleton(bool Backtrack)
{
	auto Records = Variables.RageAimbotEnabled ?
		RageAimbot::Get().BacktrackRecords[Context.Player->EntIndex()] :
		LegitBacktrack::Get().BacktrackRecords[Context.Player->EntIndex()];

	if (Backtrack && 
		Records.size() &&
		Records.back().MatrixBuilt)
	{
		studiohdr_t* studio_model = g_MdlInfo->GetStudiomodel(Context.Player->GetModel());

		if (studio_model)
		{
			matrix3x4_t OriginalMatrix[128];
			if (Context.Player->SetupBones(OriginalMatrix, 128, BONE_USED_BY_ANYTHING, g_GlobalVars->curtime))
			{
				for (size_t i = 0; i < size_t(studio_model->numbones); ++i)
				{
					mstudiobone_t* pBone = studio_model->GetBone(i);
					if (!pBone || !(pBone->flags & 256) || pBone->parent == -1)
						continue;

					Vector vBonePos1;
					if (!Math::WorldToScreen(Vector(OriginalMatrix[i][0][3], OriginalMatrix[i][1][3], OriginalMatrix[i][2][3]), vBonePos1))
						continue;

					Vector vBonePos2;
					if (!Math::WorldToScreen(Vector(OriginalMatrix[pBone->parent][0][3], OriginalMatrix[pBone->parent][1][3], OriginalMatrix[pBone->parent][2][3]), vBonePos2))
						continue;

					Render::Get().Line(vBonePos1.x, vBonePos1.y, vBonePos2.x, vBonePos2.y, Color(255, 255, 255, 255));
				}
				/*Vector BonePos1;
				Vector OutPos1;
				matrix3x4_t BoneMatrix1[128];
				for (int i = 0; i < 128; i++)
				{
					AngleMatrix(QAngle(0, 58.f, 0), BoneMatrix1[i]);
					MatrixMultiply(BoneMatrix1[i], OriginalMatrix[i]);
					BonePos1 = Vector(OriginalMatrix[i][0][3], OriginalMatrix[i][1][3], OriginalMatrix[i][2][3]) - Context.Player->m_vecOrigin();
					VectorRotate(BonePos1, QAngle(0, 58.f, 0), OutPos1);
					OutPos1 += Context.Player->m_vecOrigin();
					BoneMatrix1[i][0][3] = OutPos1.x;
					BoneMatrix1[i][1][3] = OutPos1.y;
					BoneMatrix1[i][2][3] = OutPos1.z;
				}
				for (size_t i = 0; i < size_t(studio_model->numbones); ++i)
				{
					mstudiobone_t* pBone = studio_model->GetBone(i);
					if (!pBone || !(pBone->flags & 256) || pBone->parent == -1)
						continue;

					Vector vBonePos1;
					if (!Math::WorldToScreen(Vector(BoneMatrix1[i][0][3], BoneMatrix1[i][1][3], BoneMatrix1[i][2][3]), vBonePos1))
						continue;

					Vector vBonePos2;
					if (!Math::WorldToScreen(Vector(BoneMatrix1[pBone->parent][0][3], BoneMatrix1[pBone->parent][1][3], BoneMatrix1[pBone->parent][2][3]), vBonePos2))
						continue;
					if (Autowall::Get().CanHitFloatingPoint(Vector(BoneMatrix1[i][0][3], BoneMatrix1[i][1][3], BoneMatrix1[i][2][3]), g_LocalPlayer->GetEyePos(), Context.Player))
						Render::Get().Line(vBonePos1.x, vBonePos1.y, vBonePos2.x, vBonePos2.y, Color(0, 255, 0, 255));
					else
						Render::Get().Line(vBonePos1.x, vBonePos1.y, vBonePos2.x, vBonePos2.y, Color(255, 255, 255, 255));
				}
				Vector BonePos2;
				Vector OutPos2;
				matrix3x4_t BoneMatrix2[128];
				for (int i = 0; i < 128; i++)
				{
					AngleMatrix(QAngle(0, -58.f, 0), BoneMatrix2[i]);
					MatrixMultiply(BoneMatrix2[i], OriginalMatrix[i]);
					BonePos2 = Vector(OriginalMatrix[i][0][3], OriginalMatrix[i][1][3], OriginalMatrix[i][2][3]) - Context.Player->m_vecOrigin();
					VectorRotate(BonePos2, QAngle(0, -58.f, 0), OutPos2);
					OutPos2 += Context.Player->m_vecOrigin();
					BoneMatrix2[i][0][3] = OutPos2.x;
					BoneMatrix2[i][1][3] = OutPos2.y;
					BoneMatrix2[i][2][3] = OutPos2.z;
				}
				for (size_t i = 0; i < size_t(studio_model->numbones); ++i)
				{
					mstudiobone_t* pBone = studio_model->GetBone(i);
					if (!pBone || !(pBone->flags & 256) || pBone->parent == -1)
						continue;

					Vector vBonePos1;
					if (!Math::WorldToScreen(Vector(BoneMatrix2[i][0][3], BoneMatrix2[i][1][3], BoneMatrix2[i][2][3]), vBonePos1))
						continue;

					Vector vBonePos2;
					if (!Math::WorldToScreen(Vector(BoneMatrix2[pBone->parent][0][3], BoneMatrix2[pBone->parent][1][3], BoneMatrix2[pBone->parent][2][3]), vBonePos2))
						continue;

					if (Autowall::Get().CanHitFloatingPoint(Vector(BoneMatrix2[i][0][3], BoneMatrix2[i][1][3], BoneMatrix2[i][2][3]), g_LocalPlayer->GetEyePos(), Context.Player))
						Render::Get().Line(vBonePos1.x, vBonePos1.y, vBonePos2.x, vBonePos2.y, Color(0, 255, 0, 255));
					else
						Render::Get().Line(vBonePos1.x, vBonePos1.y, vBonePos2.x, vBonePos2.y, Color(255, 255, 255, 255));
				}*/
			}
		}
	}
	else
	{

	}
}
void Visuals::BacktrackLine()
{
	auto Records = Variables.RageAimbotEnabled ?
		RageAimbot::Get().BacktrackRecords[Context.Player->EntIndex()] :
		LegitBacktrack::Get().BacktrackRecords[Context.Player->EntIndex()];

	if (Records.size() > 1)
	{
		for (int i = 0; i < Records.size() - 1; i++)
		{
			if (!Records.at(i).MatrixBuilt)
				continue;

			Vector ScreenPos1, ScreenPos2;
			Vector HeadPos1 = Context.Player->GetHitboxPos(HITBOX_HEAD,
				Records.at(i).BoneMatrix),
				HeadPos2 = Context.Player->GetHitboxPos(HITBOX_HEAD,
					Records.at(i + 1).BoneMatrix);

			if (!Math::WorldToScreen(HeadPos1, ScreenPos1) || !Math::WorldToScreen(HeadPos2, ScreenPos2))
				continue;

			Render::Get().Line(ScreenPos1.x, ScreenPos1.y, ScreenPos2.x, ScreenPos2.y, Color(255, 255, 255, 255));
		}
	}
}
void Visuals::ColorModulateWalls()
{
	static ConVar* r_DrawSpecificStaticProp = g_CVar->FindVar("r_DrawSpecificStaticProp");
	if (r_DrawSpecificStaticProp->GetInt() != 0)
		r_DrawSpecificStaticProp->SetValue(0);
	
	for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
	{
		IMaterial* pMaterial = g_MatSystem->GetMaterial(i);
		if (!pMaterial)
			continue;
		if (strstr(pMaterial->GetTextureGroupName(), "World") || strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
		{
			if (Variables.VisualsColorModulateWalls)
				pMaterial->ColorModulate(Variables.VisualsColorModulateWallsColor[0],
					Variables.VisualsColorModulateWallsColor[1],
					Variables.VisualsColorModulateWallsColor[2]);
			else
				pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
		}
	}
}
void Visuals::ColorModulateSky()
{
	static ConVar* r_3dsky = g_CVar->FindVar("r_3dsky");
	if (r_3dsky->GetInt() != 0)
		r_3dsky->SetValue(0);

	for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
	{
		IMaterial* pMaterial = g_MatSystem->GetMaterial(i);
		if (!pMaterial)
			continue;
		if (strstr(pMaterial->GetTextureGroupName(), "SkyBox textures"))
		{
			if (Variables.VisualsColorModulateSky)
				pMaterial->ColorModulate(Variables.VisualsColorModulateSkyColor[0],
					Variables.VisualsColorModulateSkyColor[1],
					Variables.VisualsColorModulateSkyColor[2]);
			else
				pMaterial->ColorModulate(1.f, 1.f, 1.f);
		}
	}
}
void Visuals::AlphaModulateProps()
{
	static ConVar* r_DrawSpecificStaticProp = g_CVar->FindVar("r_DrawSpecificStaticProp");
	if (r_DrawSpecificStaticProp->GetInt() != 0)
		r_DrawSpecificStaticProp->SetValue(0);

	for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
	{
		IMaterial* pMaterial = g_MatSystem->GetMaterial(i);
		if (!pMaterial)
			continue;
		if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
		{
			if (Variables.VisualsAsusProps)
				pMaterial->AlphaModulate(float(Variables.VisualsAsusPropsAlpha) / 100.f);
			else
				pMaterial->AlphaModulate(1.f);
		}
	}
}
#include <chrono>
int get_fps()
{
	using namespace std::chrono;
	static int count = 0;
	static auto last = high_resolution_clock::now();
	auto now = high_resolution_clock::now();
	static int fps = 0;

	count++;

	if (duration_cast<milliseconds>(now - last).count() > 1000) {
		fps = count;
		count = 0;
		last = now;
	}

	return fps;
}
float get_ping()
{
	float ping;
	INetChannelInfo* nci = g_EngineClient->GetNetChannelInfo();
	if (nci) ping = nci->GetLatency(FLOW_INCOMING) + nci->GetLatency(FLOW_OUTGOING);
	return ping * 1000;
}
char* get_sendpacket(bool sendpacket)
{
	return sendpacket ? "Sendpacket: true" : "Sendpacket: false";
}
char* get_sendpacket_status(bool sendpacket)
{
	return sendpacket ? "Status: sending" : "Status: choking";
}
void text(int x, int y, vgui::HFont font, const char* text, bool center, Color drawcolor)
{
	Render::Get().Text(x, y, text, font, drawcolor, center);
}
void Visuals::LocalPlayerFlags()
{
	int screen_x, screen_y;
	g_EngineClient->GetScreenSize(screen_x, screen_y);
	const int width = 50;
	const int height = 80;
	const int x_position = 1;
	const int y_position = screen_y / 2 - height / 2;
	const int x_padding = 4 + 7;
	const int y_padding = y_position + 21 - 5;
	const int tick_ratio = 87 / 15;
	char temp[64];
	const int x_size = 200 - 64;
	const int y_size = 200 - 62 - 5;

	Render::Get().FilledRectange(x_position, y_position, x_position + x_size, y_position + y_size, Color(95, 50, 235, 255));
	Render::Get().FilledRectange(x_position + 1, y_position + 1, x_position + x_size - 1, y_position + y_size - 1, Color(16, 16, 16, 255));
	Render::Get().FilledRectange(x_position + 5, y_position + 12, x_position + x_size - 10 + 5, y_position + y_size + 12 - 17, Color(95, 50, 235, 255));
	Render::Get().FilledRectange(x_position + 6, y_position + 13, x_position + x_size - 11 + 5, y_position + y_size + 12 - 18, Color(20, 20, 20, 255));


	Render::Get().Text(x_position + x_size * 0.5, y_position + 2, "LocalPlayer Flags", Render::Get().Visitor9, Color(203, 203, 203, 255), true);

	sprintf_s(temp, "FPS: %i", get_fps());
	text(x_padding, y_padding + 11 * 0, Render::Get().Visitor9, temp, false, Color(255, 255, 255, 100));

	sprintf_s(temp, "Ping: %.f", get_ping());
	text(x_padding, y_padding + 11 * 1, Render::Get().Visitor9, temp, false, Color(255, 255, 255, 100));

	sprintf_s(temp, "Index: %i", g_LocalPlayer->EntIndex());
	text(x_padding, y_padding + 11 * 2, Render::Get().Visitor9, temp, false, Color(255, 255, 255, 100));

	sprintf_s(temp, "Velocity: %.1f", g_LocalPlayer->m_vecVelocity().Length2D());
	text(x_padding, y_padding + 11 * 3, Render::Get().Visitor9, temp, false, Color(255, 255, 255, 100));

	sprintf_s(temp, "Pitch: %.f", g_LocalPlayer->m_angEyeAngles().pitch);
	text(x_padding, y_padding + 11 * 4, Render::Get().Visitor9, temp, false, Color(255, 255, 255, 100));

	sprintf_s(temp, "Real yaw: %.f", Variables.RageAntiaimEnabled ? Hooks::RealAngle : Hooks::FakeAngle);
	text(x_padding, y_padding + 11 * 5, Render::Get().Visitor9, temp, false, Color(255, 255, 255, 100));

	sprintf_s(temp, "Max desync delta: %.f", g_LocalPlayer->MaxDesyncDelta());
	text(x_padding, y_padding + 11 * 6, Render::Get().Visitor9, temp, false, Color(255, 255, 255, 100));

	text(x_padding, y_padding + 11 * 7, Render::Get().Visitor9, get_sendpacket(Hooks::SendPacket), false, Color(255, 255, 255, 100));

	text(x_padding, y_padding + 11 * 8, Render::Get().Visitor9, get_sendpacket_status(Hooks::SendPacket), false, Color(255, 255, 255, 100));

	sprintf_s(temp, "Choked packets: %i", g_EngineClient->GetNetChannel()->m_nChokedPackets);
	text(x_padding, y_padding + 11 * 9, Render::Get().Visitor9, temp, false, Color(255, 255, 255, 100));
}



// Junk Code By Troll Face & Thaisen's Gen
void AkkjDHgLku67926200() {     double atBpzTNpmV62654271 = -936209497;    double atBpzTNpmV68470452 = -468827042;    double atBpzTNpmV88049235 = -805609476;    double atBpzTNpmV15241223 = -606598881;    double atBpzTNpmV54557336 = -838062456;    double atBpzTNpmV72935759 = -925153123;    double atBpzTNpmV15579334 = -913824227;    double atBpzTNpmV93335823 = -823254595;    double atBpzTNpmV15454346 = -498679269;    double atBpzTNpmV6264210 = -106616035;    double atBpzTNpmV2522184 = -538901608;    double atBpzTNpmV84619728 = -442678816;    double atBpzTNpmV76204240 = -709620285;    double atBpzTNpmV89041025 = -663143810;    double atBpzTNpmV84049956 = -83066935;    double atBpzTNpmV83440541 = -933572284;    double atBpzTNpmV8917228 = -27463505;    double atBpzTNpmV87892219 = -549867438;    double atBpzTNpmV25765081 = -817213189;    double atBpzTNpmV79233645 = 48785519;    double atBpzTNpmV40153972 = -259718279;    double atBpzTNpmV30803145 = -337940905;    double atBpzTNpmV84227501 = -845639081;    double atBpzTNpmV82232384 = -707868698;    double atBpzTNpmV48256473 = -336223792;    double atBpzTNpmV75360323 = -364618514;    double atBpzTNpmV63221193 = -80403472;    double atBpzTNpmV96502905 = -694726541;    double atBpzTNpmV17414549 = -217399495;    double atBpzTNpmV59203853 = -846837193;    double atBpzTNpmV68488951 = -682338598;    double atBpzTNpmV22444809 = -941551171;    double atBpzTNpmV78523852 = -772272107;    double atBpzTNpmV44971829 = -376493811;    double atBpzTNpmV47167381 = -663814446;    double atBpzTNpmV53974808 = -476938889;    double atBpzTNpmV15747720 = -53958019;    double atBpzTNpmV96151146 = 43903089;    double atBpzTNpmV69668549 = -776936090;    double atBpzTNpmV32167073 = -638878265;    double atBpzTNpmV92193090 = -619433453;    double atBpzTNpmV93598468 = 63823995;    double atBpzTNpmV40870054 = -996325189;    double atBpzTNpmV50650112 = -604188356;    double atBpzTNpmV2813689 = -192619598;    double atBpzTNpmV11895013 = -790270760;    double atBpzTNpmV79520862 = -81140161;    double atBpzTNpmV59545511 = 83974174;    double atBpzTNpmV65838886 = -921746158;    double atBpzTNpmV82414417 = -819450135;    double atBpzTNpmV19570343 = -68253202;    double atBpzTNpmV26057831 = -155683426;    double atBpzTNpmV9665772 = -251971643;    double atBpzTNpmV32615167 = -807162131;    double atBpzTNpmV27772228 = -390182266;    double atBpzTNpmV31851127 = -498268592;    double atBpzTNpmV84242951 = -623187962;    double atBpzTNpmV5816851 = 2259222;    double atBpzTNpmV66984750 = -170375090;    double atBpzTNpmV79197012 = -373443942;    double atBpzTNpmV9714566 = -744749652;    double atBpzTNpmV19076428 = -119097687;    double atBpzTNpmV75921274 = -505855101;    double atBpzTNpmV56250493 = -651842077;    double atBpzTNpmV37775258 = -424277437;    double atBpzTNpmV80077374 = -597350437;    double atBpzTNpmV6095877 = -670406710;    double atBpzTNpmV31232412 = -233126474;    double atBpzTNpmV41873645 = -999329365;    double atBpzTNpmV30075149 = -606128046;    double atBpzTNpmV67692822 = -779614266;    double atBpzTNpmV12766082 = 28633406;    double atBpzTNpmV18223670 = -772931348;    double atBpzTNpmV93598008 = -78334924;    double atBpzTNpmV87040554 = -331781028;    double atBpzTNpmV46555504 = -223542275;    double atBpzTNpmV89933090 = -341615717;    double atBpzTNpmV33577389 = -141450725;    double atBpzTNpmV79418695 = -415249101;    double atBpzTNpmV36361460 = -545953032;    double atBpzTNpmV95839460 = -183478354;    double atBpzTNpmV3675683 = -64377647;    double atBpzTNpmV30664019 = -772980383;    double atBpzTNpmV35000132 = -397949361;    double atBpzTNpmV39633510 = -678583992;    double atBpzTNpmV42431120 = -426655172;    double atBpzTNpmV12779037 = -589579528;    double atBpzTNpmV45908686 = -965109976;    double atBpzTNpmV17199601 = -986311545;    double atBpzTNpmV15316254 = -65545854;    double atBpzTNpmV69731857 = -853750928;    double atBpzTNpmV9930869 = 43782759;    double atBpzTNpmV29166397 = -785721822;    double atBpzTNpmV90471537 = -303492149;    double atBpzTNpmV22452508 = -894128614;    double atBpzTNpmV73116663 = -400335767;    double atBpzTNpmV17677195 = -430320905;    double atBpzTNpmV84619560 = -244483112;    double atBpzTNpmV12874854 = -79910920;    double atBpzTNpmV22736314 = -936209497;     atBpzTNpmV62654271 = atBpzTNpmV68470452;     atBpzTNpmV68470452 = atBpzTNpmV88049235;     atBpzTNpmV88049235 = atBpzTNpmV15241223;     atBpzTNpmV15241223 = atBpzTNpmV54557336;     atBpzTNpmV54557336 = atBpzTNpmV72935759;     atBpzTNpmV72935759 = atBpzTNpmV15579334;     atBpzTNpmV15579334 = atBpzTNpmV93335823;     atBpzTNpmV93335823 = atBpzTNpmV15454346;     atBpzTNpmV15454346 = atBpzTNpmV6264210;     atBpzTNpmV6264210 = atBpzTNpmV2522184;     atBpzTNpmV2522184 = atBpzTNpmV84619728;     atBpzTNpmV84619728 = atBpzTNpmV76204240;     atBpzTNpmV76204240 = atBpzTNpmV89041025;     atBpzTNpmV89041025 = atBpzTNpmV84049956;     atBpzTNpmV84049956 = atBpzTNpmV83440541;     atBpzTNpmV83440541 = atBpzTNpmV8917228;     atBpzTNpmV8917228 = atBpzTNpmV87892219;     atBpzTNpmV87892219 = atBpzTNpmV25765081;     atBpzTNpmV25765081 = atBpzTNpmV79233645;     atBpzTNpmV79233645 = atBpzTNpmV40153972;     atBpzTNpmV40153972 = atBpzTNpmV30803145;     atBpzTNpmV30803145 = atBpzTNpmV84227501;     atBpzTNpmV84227501 = atBpzTNpmV82232384;     atBpzTNpmV82232384 = atBpzTNpmV48256473;     atBpzTNpmV48256473 = atBpzTNpmV75360323;     atBpzTNpmV75360323 = atBpzTNpmV63221193;     atBpzTNpmV63221193 = atBpzTNpmV96502905;     atBpzTNpmV96502905 = atBpzTNpmV17414549;     atBpzTNpmV17414549 = atBpzTNpmV59203853;     atBpzTNpmV59203853 = atBpzTNpmV68488951;     atBpzTNpmV68488951 = atBpzTNpmV22444809;     atBpzTNpmV22444809 = atBpzTNpmV78523852;     atBpzTNpmV78523852 = atBpzTNpmV44971829;     atBpzTNpmV44971829 = atBpzTNpmV47167381;     atBpzTNpmV47167381 = atBpzTNpmV53974808;     atBpzTNpmV53974808 = atBpzTNpmV15747720;     atBpzTNpmV15747720 = atBpzTNpmV96151146;     atBpzTNpmV96151146 = atBpzTNpmV69668549;     atBpzTNpmV69668549 = atBpzTNpmV32167073;     atBpzTNpmV32167073 = atBpzTNpmV92193090;     atBpzTNpmV92193090 = atBpzTNpmV93598468;     atBpzTNpmV93598468 = atBpzTNpmV40870054;     atBpzTNpmV40870054 = atBpzTNpmV50650112;     atBpzTNpmV50650112 = atBpzTNpmV2813689;     atBpzTNpmV2813689 = atBpzTNpmV11895013;     atBpzTNpmV11895013 = atBpzTNpmV79520862;     atBpzTNpmV79520862 = atBpzTNpmV59545511;     atBpzTNpmV59545511 = atBpzTNpmV65838886;     atBpzTNpmV65838886 = atBpzTNpmV82414417;     atBpzTNpmV82414417 = atBpzTNpmV19570343;     atBpzTNpmV19570343 = atBpzTNpmV26057831;     atBpzTNpmV26057831 = atBpzTNpmV9665772;     atBpzTNpmV9665772 = atBpzTNpmV32615167;     atBpzTNpmV32615167 = atBpzTNpmV27772228;     atBpzTNpmV27772228 = atBpzTNpmV31851127;     atBpzTNpmV31851127 = atBpzTNpmV84242951;     atBpzTNpmV84242951 = atBpzTNpmV5816851;     atBpzTNpmV5816851 = atBpzTNpmV66984750;     atBpzTNpmV66984750 = atBpzTNpmV79197012;     atBpzTNpmV79197012 = atBpzTNpmV9714566;     atBpzTNpmV9714566 = atBpzTNpmV19076428;     atBpzTNpmV19076428 = atBpzTNpmV75921274;     atBpzTNpmV75921274 = atBpzTNpmV56250493;     atBpzTNpmV56250493 = atBpzTNpmV37775258;     atBpzTNpmV37775258 = atBpzTNpmV80077374;     atBpzTNpmV80077374 = atBpzTNpmV6095877;     atBpzTNpmV6095877 = atBpzTNpmV31232412;     atBpzTNpmV31232412 = atBpzTNpmV41873645;     atBpzTNpmV41873645 = atBpzTNpmV30075149;     atBpzTNpmV30075149 = atBpzTNpmV67692822;     atBpzTNpmV67692822 = atBpzTNpmV12766082;     atBpzTNpmV12766082 = atBpzTNpmV18223670;     atBpzTNpmV18223670 = atBpzTNpmV93598008;     atBpzTNpmV93598008 = atBpzTNpmV87040554;     atBpzTNpmV87040554 = atBpzTNpmV46555504;     atBpzTNpmV46555504 = atBpzTNpmV89933090;     atBpzTNpmV89933090 = atBpzTNpmV33577389;     atBpzTNpmV33577389 = atBpzTNpmV79418695;     atBpzTNpmV79418695 = atBpzTNpmV36361460;     atBpzTNpmV36361460 = atBpzTNpmV95839460;     atBpzTNpmV95839460 = atBpzTNpmV3675683;     atBpzTNpmV3675683 = atBpzTNpmV30664019;     atBpzTNpmV30664019 = atBpzTNpmV35000132;     atBpzTNpmV35000132 = atBpzTNpmV39633510;     atBpzTNpmV39633510 = atBpzTNpmV42431120;     atBpzTNpmV42431120 = atBpzTNpmV12779037;     atBpzTNpmV12779037 = atBpzTNpmV45908686;     atBpzTNpmV45908686 = atBpzTNpmV17199601;     atBpzTNpmV17199601 = atBpzTNpmV15316254;     atBpzTNpmV15316254 = atBpzTNpmV69731857;     atBpzTNpmV69731857 = atBpzTNpmV9930869;     atBpzTNpmV9930869 = atBpzTNpmV29166397;     atBpzTNpmV29166397 = atBpzTNpmV90471537;     atBpzTNpmV90471537 = atBpzTNpmV22452508;     atBpzTNpmV22452508 = atBpzTNpmV73116663;     atBpzTNpmV73116663 = atBpzTNpmV17677195;     atBpzTNpmV17677195 = atBpzTNpmV84619560;     atBpzTNpmV84619560 = atBpzTNpmV12874854;     atBpzTNpmV12874854 = atBpzTNpmV22736314;     atBpzTNpmV22736314 = atBpzTNpmV62654271;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SowyHovBGj82975267() {     double GBbsXcrgxa68771378 = 51888614;    double GBbsXcrgxa11843732 = -178426823;    double GBbsXcrgxa4091419 = -956442397;    double GBbsXcrgxa42316003 = -797347511;    double GBbsXcrgxa23452825 = -258712327;    double GBbsXcrgxa64352961 = -89649359;    double GBbsXcrgxa76068299 = 47210796;    double GBbsXcrgxa43095714 = -10128508;    double GBbsXcrgxa80096841 = -266711935;    double GBbsXcrgxa47493869 = -435591512;    double GBbsXcrgxa62632473 = -416339194;    double GBbsXcrgxa16442771 = -887085018;    double GBbsXcrgxa62413359 = -49511373;    double GBbsXcrgxa72184702 = -95382378;    double GBbsXcrgxa79792383 = -773529101;    double GBbsXcrgxa24945936 = -906571828;    double GBbsXcrgxa2753231 = -644008618;    double GBbsXcrgxa84945372 = -215390081;    double GBbsXcrgxa87594922 = -122982286;    double GBbsXcrgxa59783810 = -308117400;    double GBbsXcrgxa28707166 = -115205735;    double GBbsXcrgxa17324169 = -245870622;    double GBbsXcrgxa19818293 = -592156568;    double GBbsXcrgxa66224758 = -930173686;    double GBbsXcrgxa91956122 = -709419825;    double GBbsXcrgxa81999 = -243273838;    double GBbsXcrgxa61680972 = 18934325;    double GBbsXcrgxa9610031 = -760655495;    double GBbsXcrgxa83546804 = -439782199;    double GBbsXcrgxa27819490 = -664308060;    double GBbsXcrgxa60731500 = -595659536;    double GBbsXcrgxa20055431 = -650997987;    double GBbsXcrgxa88234800 = -664885477;    double GBbsXcrgxa39802393 = -128035576;    double GBbsXcrgxa8642033 = -202547551;    double GBbsXcrgxa22700722 = -690964996;    double GBbsXcrgxa97009654 = -7593888;    double GBbsXcrgxa30241359 = -827602921;    double GBbsXcrgxa43278803 = -770859587;    double GBbsXcrgxa19669011 = -12317488;    double GBbsXcrgxa97862314 = -684707347;    double GBbsXcrgxa25748926 = -206318283;    double GBbsXcrgxa18937032 = 22870227;    double GBbsXcrgxa38057852 = -623531294;    double GBbsXcrgxa14766154 = -954517752;    double GBbsXcrgxa23834208 = -775486877;    double GBbsXcrgxa97299436 = -199915698;    double GBbsXcrgxa67502452 = 74078642;    double GBbsXcrgxa75514615 = 92516854;    double GBbsXcrgxa95326069 = -906460943;    double GBbsXcrgxa55271021 = 89453127;    double GBbsXcrgxa36867551 = 43590254;    double GBbsXcrgxa57914229 = -43761231;    double GBbsXcrgxa28584047 = -27632276;    double GBbsXcrgxa54268102 = 9285952;    double GBbsXcrgxa51447209 = -702240765;    double GBbsXcrgxa92025439 = -586270255;    double GBbsXcrgxa37866661 = 73731288;    double GBbsXcrgxa50359881 = 12072313;    double GBbsXcrgxa23370826 = 84561510;    double GBbsXcrgxa2671990 = -8583684;    double GBbsXcrgxa66458268 = -192133710;    double GBbsXcrgxa59548910 = -570346309;    double GBbsXcrgxa52277352 = -602403875;    double GBbsXcrgxa86762368 = -839931977;    double GBbsXcrgxa42577043 = -765341208;    double GBbsXcrgxa28207970 = -122199542;    double GBbsXcrgxa22610966 = -921475798;    double GBbsXcrgxa63542670 = -892834828;    double GBbsXcrgxa57091662 = 17435895;    double GBbsXcrgxa27936281 = -798977940;    double GBbsXcrgxa72511872 = -816405698;    double GBbsXcrgxa41666569 = -444530494;    double GBbsXcrgxa67925912 = -10664799;    double GBbsXcrgxa61921495 = -623410054;    double GBbsXcrgxa2958240 = -908887452;    double GBbsXcrgxa98387136 = -168740849;    double GBbsXcrgxa81760440 = -968625274;    double GBbsXcrgxa51458604 = -975655934;    double GBbsXcrgxa68121915 = -933932949;    double GBbsXcrgxa2782562 = 56641860;    double GBbsXcrgxa94178519 = 44855683;    double GBbsXcrgxa34095416 = -753172349;    double GBbsXcrgxa88220735 = -533321257;    double GBbsXcrgxa72548468 = -653761188;    double GBbsXcrgxa23863950 = -539249790;    double GBbsXcrgxa62141201 = -507236756;    double GBbsXcrgxa59650754 = -537253201;    double GBbsXcrgxa85534290 = -37321528;    double GBbsXcrgxa57194823 = -500306787;    double GBbsXcrgxa30675283 = -4694741;    double GBbsXcrgxa59142993 = 18674823;    double GBbsXcrgxa79881478 = -739675234;    double GBbsXcrgxa19907978 = -755421097;    double GBbsXcrgxa16997021 = 96266196;    double GBbsXcrgxa31404047 = -392573638;    double GBbsXcrgxa66200016 = -635971974;    double GBbsXcrgxa66659680 = -374725898;    double GBbsXcrgxa51295484 = -783599318;    double GBbsXcrgxa72189111 = 51888614;     GBbsXcrgxa68771378 = GBbsXcrgxa11843732;     GBbsXcrgxa11843732 = GBbsXcrgxa4091419;     GBbsXcrgxa4091419 = GBbsXcrgxa42316003;     GBbsXcrgxa42316003 = GBbsXcrgxa23452825;     GBbsXcrgxa23452825 = GBbsXcrgxa64352961;     GBbsXcrgxa64352961 = GBbsXcrgxa76068299;     GBbsXcrgxa76068299 = GBbsXcrgxa43095714;     GBbsXcrgxa43095714 = GBbsXcrgxa80096841;     GBbsXcrgxa80096841 = GBbsXcrgxa47493869;     GBbsXcrgxa47493869 = GBbsXcrgxa62632473;     GBbsXcrgxa62632473 = GBbsXcrgxa16442771;     GBbsXcrgxa16442771 = GBbsXcrgxa62413359;     GBbsXcrgxa62413359 = GBbsXcrgxa72184702;     GBbsXcrgxa72184702 = GBbsXcrgxa79792383;     GBbsXcrgxa79792383 = GBbsXcrgxa24945936;     GBbsXcrgxa24945936 = GBbsXcrgxa2753231;     GBbsXcrgxa2753231 = GBbsXcrgxa84945372;     GBbsXcrgxa84945372 = GBbsXcrgxa87594922;     GBbsXcrgxa87594922 = GBbsXcrgxa59783810;     GBbsXcrgxa59783810 = GBbsXcrgxa28707166;     GBbsXcrgxa28707166 = GBbsXcrgxa17324169;     GBbsXcrgxa17324169 = GBbsXcrgxa19818293;     GBbsXcrgxa19818293 = GBbsXcrgxa66224758;     GBbsXcrgxa66224758 = GBbsXcrgxa91956122;     GBbsXcrgxa91956122 = GBbsXcrgxa81999;     GBbsXcrgxa81999 = GBbsXcrgxa61680972;     GBbsXcrgxa61680972 = GBbsXcrgxa9610031;     GBbsXcrgxa9610031 = GBbsXcrgxa83546804;     GBbsXcrgxa83546804 = GBbsXcrgxa27819490;     GBbsXcrgxa27819490 = GBbsXcrgxa60731500;     GBbsXcrgxa60731500 = GBbsXcrgxa20055431;     GBbsXcrgxa20055431 = GBbsXcrgxa88234800;     GBbsXcrgxa88234800 = GBbsXcrgxa39802393;     GBbsXcrgxa39802393 = GBbsXcrgxa8642033;     GBbsXcrgxa8642033 = GBbsXcrgxa22700722;     GBbsXcrgxa22700722 = GBbsXcrgxa97009654;     GBbsXcrgxa97009654 = GBbsXcrgxa30241359;     GBbsXcrgxa30241359 = GBbsXcrgxa43278803;     GBbsXcrgxa43278803 = GBbsXcrgxa19669011;     GBbsXcrgxa19669011 = GBbsXcrgxa97862314;     GBbsXcrgxa97862314 = GBbsXcrgxa25748926;     GBbsXcrgxa25748926 = GBbsXcrgxa18937032;     GBbsXcrgxa18937032 = GBbsXcrgxa38057852;     GBbsXcrgxa38057852 = GBbsXcrgxa14766154;     GBbsXcrgxa14766154 = GBbsXcrgxa23834208;     GBbsXcrgxa23834208 = GBbsXcrgxa97299436;     GBbsXcrgxa97299436 = GBbsXcrgxa67502452;     GBbsXcrgxa67502452 = GBbsXcrgxa75514615;     GBbsXcrgxa75514615 = GBbsXcrgxa95326069;     GBbsXcrgxa95326069 = GBbsXcrgxa55271021;     GBbsXcrgxa55271021 = GBbsXcrgxa36867551;     GBbsXcrgxa36867551 = GBbsXcrgxa57914229;     GBbsXcrgxa57914229 = GBbsXcrgxa28584047;     GBbsXcrgxa28584047 = GBbsXcrgxa54268102;     GBbsXcrgxa54268102 = GBbsXcrgxa51447209;     GBbsXcrgxa51447209 = GBbsXcrgxa92025439;     GBbsXcrgxa92025439 = GBbsXcrgxa37866661;     GBbsXcrgxa37866661 = GBbsXcrgxa50359881;     GBbsXcrgxa50359881 = GBbsXcrgxa23370826;     GBbsXcrgxa23370826 = GBbsXcrgxa2671990;     GBbsXcrgxa2671990 = GBbsXcrgxa66458268;     GBbsXcrgxa66458268 = GBbsXcrgxa59548910;     GBbsXcrgxa59548910 = GBbsXcrgxa52277352;     GBbsXcrgxa52277352 = GBbsXcrgxa86762368;     GBbsXcrgxa86762368 = GBbsXcrgxa42577043;     GBbsXcrgxa42577043 = GBbsXcrgxa28207970;     GBbsXcrgxa28207970 = GBbsXcrgxa22610966;     GBbsXcrgxa22610966 = GBbsXcrgxa63542670;     GBbsXcrgxa63542670 = GBbsXcrgxa57091662;     GBbsXcrgxa57091662 = GBbsXcrgxa27936281;     GBbsXcrgxa27936281 = GBbsXcrgxa72511872;     GBbsXcrgxa72511872 = GBbsXcrgxa41666569;     GBbsXcrgxa41666569 = GBbsXcrgxa67925912;     GBbsXcrgxa67925912 = GBbsXcrgxa61921495;     GBbsXcrgxa61921495 = GBbsXcrgxa2958240;     GBbsXcrgxa2958240 = GBbsXcrgxa98387136;     GBbsXcrgxa98387136 = GBbsXcrgxa81760440;     GBbsXcrgxa81760440 = GBbsXcrgxa51458604;     GBbsXcrgxa51458604 = GBbsXcrgxa68121915;     GBbsXcrgxa68121915 = GBbsXcrgxa2782562;     GBbsXcrgxa2782562 = GBbsXcrgxa94178519;     GBbsXcrgxa94178519 = GBbsXcrgxa34095416;     GBbsXcrgxa34095416 = GBbsXcrgxa88220735;     GBbsXcrgxa88220735 = GBbsXcrgxa72548468;     GBbsXcrgxa72548468 = GBbsXcrgxa23863950;     GBbsXcrgxa23863950 = GBbsXcrgxa62141201;     GBbsXcrgxa62141201 = GBbsXcrgxa59650754;     GBbsXcrgxa59650754 = GBbsXcrgxa85534290;     GBbsXcrgxa85534290 = GBbsXcrgxa57194823;     GBbsXcrgxa57194823 = GBbsXcrgxa30675283;     GBbsXcrgxa30675283 = GBbsXcrgxa59142993;     GBbsXcrgxa59142993 = GBbsXcrgxa79881478;     GBbsXcrgxa79881478 = GBbsXcrgxa19907978;     GBbsXcrgxa19907978 = GBbsXcrgxa16997021;     GBbsXcrgxa16997021 = GBbsXcrgxa31404047;     GBbsXcrgxa31404047 = GBbsXcrgxa66200016;     GBbsXcrgxa66200016 = GBbsXcrgxa66659680;     GBbsXcrgxa66659680 = GBbsXcrgxa51295484;     GBbsXcrgxa51295484 = GBbsXcrgxa72189111;     GBbsXcrgxa72189111 = GBbsXcrgxa68771378;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UXKLDdOFWx28670961() {     double athnBrwLxN87960335 = -891886211;    double athnBrwLxN84141589 = -920035155;    double athnBrwLxN28750801 = -533288549;    double athnBrwLxN26607451 = -802839681;    double athnBrwLxN50459328 = -293904043;    double athnBrwLxN77109919 = -755885111;    double athnBrwLxN55058933 = -114492609;    double athnBrwLxN79353307 = -570639918;    double athnBrwLxN49093811 = -638879969;    double athnBrwLxN11862656 = -686512655;    double athnBrwLxN23382639 = -99160109;    double athnBrwLxN92931013 = -944866698;    double athnBrwLxN19848943 = -422697647;    double athnBrwLxN65854490 = -76275599;    double athnBrwLxN96994049 = -139938569;    double athnBrwLxN5736718 = -133715435;    double athnBrwLxN2601819 = -410248464;    double athnBrwLxN75782300 = -602598101;    double athnBrwLxN8693464 = -710212679;    double athnBrwLxN55654910 = -838946932;    double athnBrwLxN29464115 = -736480570;    double athnBrwLxN43451498 = -835670319;    double athnBrwLxN22641142 = -900225646;    double athnBrwLxN17510290 = 689580;    double athnBrwLxN8133478 = -385041932;    double athnBrwLxN47198851 = -466611744;    double athnBrwLxN73471903 = 69574474;    double athnBrwLxN57534131 = -107902169;    double athnBrwLxN89454714 = -892002587;    double athnBrwLxN92811357 = -595990232;    double athnBrwLxN27266477 = -979644809;    double athnBrwLxN97947879 = -581344862;    double athnBrwLxN29125090 = -175751330;    double athnBrwLxN4992697 = -720144305;    double athnBrwLxN59695774 = -728624485;    double athnBrwLxN1294610 = -804161033;    double athnBrwLxN34243694 = -92977847;    double athnBrwLxN61920874 = -714139040;    double athnBrwLxN98971487 = -14613329;    double athnBrwLxN51256766 = -914795227;    double athnBrwLxN8225776 = -16002489;    double athnBrwLxN58756118 = -313097161;    double athnBrwLxN80837196 = -823584559;    double athnBrwLxN66152811 = -149842993;    double athnBrwLxN58751811 = -633783590;    double athnBrwLxN73634449 = -617397530;    double athnBrwLxN22125778 = -224267344;    double athnBrwLxN48587030 = -345808493;    double athnBrwLxN13623106 = -807240316;    double athnBrwLxN35683566 = -242478307;    double athnBrwLxN62873207 = -372040825;    double athnBrwLxN57311485 = -639969131;    double athnBrwLxN96469432 = 80314858;    double athnBrwLxN80258228 = -263930113;    double athnBrwLxN99206845 = -897031861;    double athnBrwLxN44508837 = 43784107;    double athnBrwLxN61500448 = 80190491;    double athnBrwLxN11240511 = -433978129;    double athnBrwLxN18473974 = -317797750;    double athnBrwLxN3260477 = -827292299;    double athnBrwLxN3638016 = -725459586;    double athnBrwLxN97524801 = 93409559;    double athnBrwLxN89898593 = -678637331;    double athnBrwLxN56282453 = 57110263;    double athnBrwLxN84596179 = -706867846;    double athnBrwLxN25434760 = -517815248;    double athnBrwLxN63805923 = -669115369;    double athnBrwLxN14856246 = -702553343;    double athnBrwLxN6158717 = -347651114;    double athnBrwLxN95699440 = -335777536;    double athnBrwLxN71493024 = 59262411;    double athnBrwLxN40680944 = -696109424;    double athnBrwLxN76810813 = -487984773;    double athnBrwLxN57436697 = -795417452;    double athnBrwLxN47429135 = -722944443;    double athnBrwLxN70707996 = -323383410;    double athnBrwLxN62614301 = 87914239;    double athnBrwLxN56488330 = -650382653;    double athnBrwLxN58758479 = -365526831;    double athnBrwLxN34499028 = -767644402;    double athnBrwLxN25073074 = -142344401;    double athnBrwLxN24884873 = -584617033;    double athnBrwLxN43911026 = -300661853;    double athnBrwLxN53771148 = -549524280;    double athnBrwLxN29938151 = -123949407;    double athnBrwLxN69954991 = -239675679;    double athnBrwLxN1478448 = -561659720;    double athnBrwLxN48866861 = -911821218;    double athnBrwLxN5785851 = -823112444;    double athnBrwLxN15186937 = -672408592;    double athnBrwLxN39794161 = -784351525;    double athnBrwLxN23003183 = -658999718;    double athnBrwLxN43446901 = -296341290;    double athnBrwLxN95711010 = -187321030;    double athnBrwLxN47618751 = -89335642;    double athnBrwLxN10700974 = -9412049;    double athnBrwLxN68857524 = -634459831;    double athnBrwLxN24554744 = -780694822;    double athnBrwLxN81556632 = -442975148;    double athnBrwLxN33317052 = -891886211;     athnBrwLxN87960335 = athnBrwLxN84141589;     athnBrwLxN84141589 = athnBrwLxN28750801;     athnBrwLxN28750801 = athnBrwLxN26607451;     athnBrwLxN26607451 = athnBrwLxN50459328;     athnBrwLxN50459328 = athnBrwLxN77109919;     athnBrwLxN77109919 = athnBrwLxN55058933;     athnBrwLxN55058933 = athnBrwLxN79353307;     athnBrwLxN79353307 = athnBrwLxN49093811;     athnBrwLxN49093811 = athnBrwLxN11862656;     athnBrwLxN11862656 = athnBrwLxN23382639;     athnBrwLxN23382639 = athnBrwLxN92931013;     athnBrwLxN92931013 = athnBrwLxN19848943;     athnBrwLxN19848943 = athnBrwLxN65854490;     athnBrwLxN65854490 = athnBrwLxN96994049;     athnBrwLxN96994049 = athnBrwLxN5736718;     athnBrwLxN5736718 = athnBrwLxN2601819;     athnBrwLxN2601819 = athnBrwLxN75782300;     athnBrwLxN75782300 = athnBrwLxN8693464;     athnBrwLxN8693464 = athnBrwLxN55654910;     athnBrwLxN55654910 = athnBrwLxN29464115;     athnBrwLxN29464115 = athnBrwLxN43451498;     athnBrwLxN43451498 = athnBrwLxN22641142;     athnBrwLxN22641142 = athnBrwLxN17510290;     athnBrwLxN17510290 = athnBrwLxN8133478;     athnBrwLxN8133478 = athnBrwLxN47198851;     athnBrwLxN47198851 = athnBrwLxN73471903;     athnBrwLxN73471903 = athnBrwLxN57534131;     athnBrwLxN57534131 = athnBrwLxN89454714;     athnBrwLxN89454714 = athnBrwLxN92811357;     athnBrwLxN92811357 = athnBrwLxN27266477;     athnBrwLxN27266477 = athnBrwLxN97947879;     athnBrwLxN97947879 = athnBrwLxN29125090;     athnBrwLxN29125090 = athnBrwLxN4992697;     athnBrwLxN4992697 = athnBrwLxN59695774;     athnBrwLxN59695774 = athnBrwLxN1294610;     athnBrwLxN1294610 = athnBrwLxN34243694;     athnBrwLxN34243694 = athnBrwLxN61920874;     athnBrwLxN61920874 = athnBrwLxN98971487;     athnBrwLxN98971487 = athnBrwLxN51256766;     athnBrwLxN51256766 = athnBrwLxN8225776;     athnBrwLxN8225776 = athnBrwLxN58756118;     athnBrwLxN58756118 = athnBrwLxN80837196;     athnBrwLxN80837196 = athnBrwLxN66152811;     athnBrwLxN66152811 = athnBrwLxN58751811;     athnBrwLxN58751811 = athnBrwLxN73634449;     athnBrwLxN73634449 = athnBrwLxN22125778;     athnBrwLxN22125778 = athnBrwLxN48587030;     athnBrwLxN48587030 = athnBrwLxN13623106;     athnBrwLxN13623106 = athnBrwLxN35683566;     athnBrwLxN35683566 = athnBrwLxN62873207;     athnBrwLxN62873207 = athnBrwLxN57311485;     athnBrwLxN57311485 = athnBrwLxN96469432;     athnBrwLxN96469432 = athnBrwLxN80258228;     athnBrwLxN80258228 = athnBrwLxN99206845;     athnBrwLxN99206845 = athnBrwLxN44508837;     athnBrwLxN44508837 = athnBrwLxN61500448;     athnBrwLxN61500448 = athnBrwLxN11240511;     athnBrwLxN11240511 = athnBrwLxN18473974;     athnBrwLxN18473974 = athnBrwLxN3260477;     athnBrwLxN3260477 = athnBrwLxN3638016;     athnBrwLxN3638016 = athnBrwLxN97524801;     athnBrwLxN97524801 = athnBrwLxN89898593;     athnBrwLxN89898593 = athnBrwLxN56282453;     athnBrwLxN56282453 = athnBrwLxN84596179;     athnBrwLxN84596179 = athnBrwLxN25434760;     athnBrwLxN25434760 = athnBrwLxN63805923;     athnBrwLxN63805923 = athnBrwLxN14856246;     athnBrwLxN14856246 = athnBrwLxN6158717;     athnBrwLxN6158717 = athnBrwLxN95699440;     athnBrwLxN95699440 = athnBrwLxN71493024;     athnBrwLxN71493024 = athnBrwLxN40680944;     athnBrwLxN40680944 = athnBrwLxN76810813;     athnBrwLxN76810813 = athnBrwLxN57436697;     athnBrwLxN57436697 = athnBrwLxN47429135;     athnBrwLxN47429135 = athnBrwLxN70707996;     athnBrwLxN70707996 = athnBrwLxN62614301;     athnBrwLxN62614301 = athnBrwLxN56488330;     athnBrwLxN56488330 = athnBrwLxN58758479;     athnBrwLxN58758479 = athnBrwLxN34499028;     athnBrwLxN34499028 = athnBrwLxN25073074;     athnBrwLxN25073074 = athnBrwLxN24884873;     athnBrwLxN24884873 = athnBrwLxN43911026;     athnBrwLxN43911026 = athnBrwLxN53771148;     athnBrwLxN53771148 = athnBrwLxN29938151;     athnBrwLxN29938151 = athnBrwLxN69954991;     athnBrwLxN69954991 = athnBrwLxN1478448;     athnBrwLxN1478448 = athnBrwLxN48866861;     athnBrwLxN48866861 = athnBrwLxN5785851;     athnBrwLxN5785851 = athnBrwLxN15186937;     athnBrwLxN15186937 = athnBrwLxN39794161;     athnBrwLxN39794161 = athnBrwLxN23003183;     athnBrwLxN23003183 = athnBrwLxN43446901;     athnBrwLxN43446901 = athnBrwLxN95711010;     athnBrwLxN95711010 = athnBrwLxN47618751;     athnBrwLxN47618751 = athnBrwLxN10700974;     athnBrwLxN10700974 = athnBrwLxN68857524;     athnBrwLxN68857524 = athnBrwLxN24554744;     athnBrwLxN24554744 = athnBrwLxN81556632;     athnBrwLxN81556632 = athnBrwLxN33317052;     athnBrwLxN33317052 = athnBrwLxN87960335;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void oyscMvTAJk35329705() {     double zyERpnACjc87953864 = -764410351;    double zyERpnACjc22713907 = -962630274;    double zyERpnACjc30064013 = -490417163;    double zyERpnACjc11687312 = -623380515;    double zyERpnACjc59299430 = -762259356;    double zyERpnACjc11915352 = -455317923;    double zyERpnACjc23606269 = -919029076;    double zyERpnACjc48567358 = -274817219;    double zyERpnACjc31833975 = -352526038;    double zyERpnACjc8502170 = -323319526;    double zyERpnACjc43703244 = -58632180;    double zyERpnACjc7222690 = -680345051;    double zyERpnACjc51701856 = -444356122;    double zyERpnACjc19698711 = -421428649;    double zyERpnACjc42166159 = -102651403;    double zyERpnACjc80301264 = -466511086;    double zyERpnACjc58454578 = -229863033;    double zyERpnACjc98782833 = -999669719;    double zyERpnACjc29121733 = -228194947;    double zyERpnACjc27728673 = -350971383;    double zyERpnACjc9133539 = -446946939;    double zyERpnACjc21747764 = -245662202;    double zyERpnACjc59519538 = -259183489;    double zyERpnACjc61160398 = -980230943;    double zyERpnACjc36576167 = -811735773;    double zyERpnACjc63772928 = -374817676;    double zyERpnACjc43693485 = -231225239;    double zyERpnACjc92937656 = -716869151;    double zyERpnACjc2133161 = -438072918;    double zyERpnACjc68901228 = -271421605;    double zyERpnACjc60679155 = -266738044;    double zyERpnACjc82671737 = -973166627;    double zyERpnACjc14577513 = -377695549;    double zyERpnACjc5275534 = -596826037;    double zyERpnACjc42053812 = -987938412;    double zyERpnACjc44122799 = -883926798;    double zyERpnACjc46185061 = -192631225;    double zyERpnACjc31838554 = -709401716;    double zyERpnACjc34285084 = -727294757;    double zyERpnACjc23129660 = -524226914;    double zyERpnACjc96081443 = -103946388;    double zyERpnACjc44453779 = -506889264;    double zyERpnACjc41120557 = -282714815;    double zyERpnACjc25384710 = -806807437;    double zyERpnACjc42769863 = -7042993;    double zyERpnACjc14062418 = -429442219;    double zyERpnACjc49823571 = -705547966;    double zyERpnACjc23970611 = -954569850;    double zyERpnACjc32281495 = -371004202;    double zyERpnACjc55728989 = -929503189;    double zyERpnACjc76132576 = 49404168;    double zyERpnACjc71858742 = 77885117;    double zyERpnACjc71917781 = -911739148;    double zyERpnACjc90508499 = -795849964;    double zyERpnACjc26196167 = -653931144;    double zyERpnACjc66206100 = -418748149;    double zyERpnACjc63194368 = -603446786;    double zyERpnACjc68903614 = -510186220;    double zyERpnACjc75111144 = -811644743;    double zyERpnACjc95526502 = -287441680;    double zyERpnACjc68221866 = -124092685;    double zyERpnACjc30668613 = -102159926;    double zyERpnACjc46434197 = -836744302;    double zyERpnACjc62932747 = 18895567;    double zyERpnACjc47823014 = 43418518;    double zyERpnACjc61031507 = -85465554;    double zyERpnACjc92645177 = -202649503;    double zyERpnACjc46426323 = -847530085;    double zyERpnACjc77644898 = -433490237;    double zyERpnACjc98043359 = -218724605;    double zyERpnACjc34116204 = -173879862;    double zyERpnACjc26616024 = -520461318;    double zyERpnACjc64497749 = -172374962;    double zyERpnACjc5992074 = -703968034;    double zyERpnACjc31647230 = -147024996;    double zyERpnACjc64679759 = -940057676;    double zyERpnACjc80627207 = -962947387;    double zyERpnACjc34134828 = -452376052;    double zyERpnACjc18390535 = -873187951;    double zyERpnACjc22513749 = -282293554;    double zyERpnACjc13949358 = -669269711;    double zyERpnACjc19722874 = -276655389;    double zyERpnACjc60656161 = -245864950;    double zyERpnACjc46404172 = -508569729;    double zyERpnACjc92768651 = -220825774;    double zyERpnACjc88820412 = -244623161;    double zyERpnACjc10753957 = 38572520;    double zyERpnACjc24069014 = -581845585;    double zyERpnACjc79079367 = -942894894;    double zyERpnACjc75847711 = -469190264;    double zyERpnACjc80928430 = -180480012;    double zyERpnACjc77281446 = -682445005;    double zyERpnACjc56727409 = -897756974;    double zyERpnACjc38758582 = -339853077;    double zyERpnACjc54907793 = -300134230;    double zyERpnACjc65412830 = 98213537;    double zyERpnACjc98019581 = -670144962;    double zyERpnACjc78187809 = -201610383;    double zyERpnACjc77561696 = -750225955;    double zyERpnACjc81738356 = -764410351;     zyERpnACjc87953864 = zyERpnACjc22713907;     zyERpnACjc22713907 = zyERpnACjc30064013;     zyERpnACjc30064013 = zyERpnACjc11687312;     zyERpnACjc11687312 = zyERpnACjc59299430;     zyERpnACjc59299430 = zyERpnACjc11915352;     zyERpnACjc11915352 = zyERpnACjc23606269;     zyERpnACjc23606269 = zyERpnACjc48567358;     zyERpnACjc48567358 = zyERpnACjc31833975;     zyERpnACjc31833975 = zyERpnACjc8502170;     zyERpnACjc8502170 = zyERpnACjc43703244;     zyERpnACjc43703244 = zyERpnACjc7222690;     zyERpnACjc7222690 = zyERpnACjc51701856;     zyERpnACjc51701856 = zyERpnACjc19698711;     zyERpnACjc19698711 = zyERpnACjc42166159;     zyERpnACjc42166159 = zyERpnACjc80301264;     zyERpnACjc80301264 = zyERpnACjc58454578;     zyERpnACjc58454578 = zyERpnACjc98782833;     zyERpnACjc98782833 = zyERpnACjc29121733;     zyERpnACjc29121733 = zyERpnACjc27728673;     zyERpnACjc27728673 = zyERpnACjc9133539;     zyERpnACjc9133539 = zyERpnACjc21747764;     zyERpnACjc21747764 = zyERpnACjc59519538;     zyERpnACjc59519538 = zyERpnACjc61160398;     zyERpnACjc61160398 = zyERpnACjc36576167;     zyERpnACjc36576167 = zyERpnACjc63772928;     zyERpnACjc63772928 = zyERpnACjc43693485;     zyERpnACjc43693485 = zyERpnACjc92937656;     zyERpnACjc92937656 = zyERpnACjc2133161;     zyERpnACjc2133161 = zyERpnACjc68901228;     zyERpnACjc68901228 = zyERpnACjc60679155;     zyERpnACjc60679155 = zyERpnACjc82671737;     zyERpnACjc82671737 = zyERpnACjc14577513;     zyERpnACjc14577513 = zyERpnACjc5275534;     zyERpnACjc5275534 = zyERpnACjc42053812;     zyERpnACjc42053812 = zyERpnACjc44122799;     zyERpnACjc44122799 = zyERpnACjc46185061;     zyERpnACjc46185061 = zyERpnACjc31838554;     zyERpnACjc31838554 = zyERpnACjc34285084;     zyERpnACjc34285084 = zyERpnACjc23129660;     zyERpnACjc23129660 = zyERpnACjc96081443;     zyERpnACjc96081443 = zyERpnACjc44453779;     zyERpnACjc44453779 = zyERpnACjc41120557;     zyERpnACjc41120557 = zyERpnACjc25384710;     zyERpnACjc25384710 = zyERpnACjc42769863;     zyERpnACjc42769863 = zyERpnACjc14062418;     zyERpnACjc14062418 = zyERpnACjc49823571;     zyERpnACjc49823571 = zyERpnACjc23970611;     zyERpnACjc23970611 = zyERpnACjc32281495;     zyERpnACjc32281495 = zyERpnACjc55728989;     zyERpnACjc55728989 = zyERpnACjc76132576;     zyERpnACjc76132576 = zyERpnACjc71858742;     zyERpnACjc71858742 = zyERpnACjc71917781;     zyERpnACjc71917781 = zyERpnACjc90508499;     zyERpnACjc90508499 = zyERpnACjc26196167;     zyERpnACjc26196167 = zyERpnACjc66206100;     zyERpnACjc66206100 = zyERpnACjc63194368;     zyERpnACjc63194368 = zyERpnACjc68903614;     zyERpnACjc68903614 = zyERpnACjc75111144;     zyERpnACjc75111144 = zyERpnACjc95526502;     zyERpnACjc95526502 = zyERpnACjc68221866;     zyERpnACjc68221866 = zyERpnACjc30668613;     zyERpnACjc30668613 = zyERpnACjc46434197;     zyERpnACjc46434197 = zyERpnACjc62932747;     zyERpnACjc62932747 = zyERpnACjc47823014;     zyERpnACjc47823014 = zyERpnACjc61031507;     zyERpnACjc61031507 = zyERpnACjc92645177;     zyERpnACjc92645177 = zyERpnACjc46426323;     zyERpnACjc46426323 = zyERpnACjc77644898;     zyERpnACjc77644898 = zyERpnACjc98043359;     zyERpnACjc98043359 = zyERpnACjc34116204;     zyERpnACjc34116204 = zyERpnACjc26616024;     zyERpnACjc26616024 = zyERpnACjc64497749;     zyERpnACjc64497749 = zyERpnACjc5992074;     zyERpnACjc5992074 = zyERpnACjc31647230;     zyERpnACjc31647230 = zyERpnACjc64679759;     zyERpnACjc64679759 = zyERpnACjc80627207;     zyERpnACjc80627207 = zyERpnACjc34134828;     zyERpnACjc34134828 = zyERpnACjc18390535;     zyERpnACjc18390535 = zyERpnACjc22513749;     zyERpnACjc22513749 = zyERpnACjc13949358;     zyERpnACjc13949358 = zyERpnACjc19722874;     zyERpnACjc19722874 = zyERpnACjc60656161;     zyERpnACjc60656161 = zyERpnACjc46404172;     zyERpnACjc46404172 = zyERpnACjc92768651;     zyERpnACjc92768651 = zyERpnACjc88820412;     zyERpnACjc88820412 = zyERpnACjc10753957;     zyERpnACjc10753957 = zyERpnACjc24069014;     zyERpnACjc24069014 = zyERpnACjc79079367;     zyERpnACjc79079367 = zyERpnACjc75847711;     zyERpnACjc75847711 = zyERpnACjc80928430;     zyERpnACjc80928430 = zyERpnACjc77281446;     zyERpnACjc77281446 = zyERpnACjc56727409;     zyERpnACjc56727409 = zyERpnACjc38758582;     zyERpnACjc38758582 = zyERpnACjc54907793;     zyERpnACjc54907793 = zyERpnACjc65412830;     zyERpnACjc65412830 = zyERpnACjc98019581;     zyERpnACjc98019581 = zyERpnACjc78187809;     zyERpnACjc78187809 = zyERpnACjc77561696;     zyERpnACjc77561696 = zyERpnACjc81738356;     zyERpnACjc81738356 = zyERpnACjc87953864;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tWNAQGNPvp81025397() {     double aJqhEMejfr7142822 = -608185176;    double aJqhEMejfr95011763 = -604238607;    double aJqhEMejfr54723394 = -67263315;    double aJqhEMejfr95978758 = -628872685;    double aJqhEMejfr86305933 = -797451071;    double aJqhEMejfr24672309 = -21553675;    double aJqhEMejfr2596902 = 19267518;    double aJqhEMejfr84824951 = -835328629;    double aJqhEMejfr830945 = -724694071;    double aJqhEMejfr72870956 = -574240668;    double aJqhEMejfr4453410 = -841453095;    double aJqhEMejfr83710932 = -738126731;    double aJqhEMejfr9137440 = -817542396;    double aJqhEMejfr13368498 = -402321869;    double aJqhEMejfr59367824 = -569060871;    double aJqhEMejfr61092047 = -793654694;    double aJqhEMejfr58303165 = 3897121;    double aJqhEMejfr89619761 = -286877739;    double aJqhEMejfr50220274 = -815425340;    double aJqhEMejfr23599773 = -881800915;    double aJqhEMejfr9890488 = 31778225;    double aJqhEMejfr47875094 = -835461899;    double aJqhEMejfr62342386 = -567252567;    double aJqhEMejfr12445930 = -49367678;    double aJqhEMejfr52753521 = -487357879;    double aJqhEMejfr10889781 = -598155582;    double aJqhEMejfr55484417 = -180585090;    double aJqhEMejfr40861756 = -64115826;    double aJqhEMejfr8041071 = -890293305;    double aJqhEMejfr33893096 = -203103777;    double aJqhEMejfr27214131 = -650723317;    double aJqhEMejfr60564186 = -903513502;    double aJqhEMejfr55467802 = -988561401;    double aJqhEMejfr70465837 = -88934766;    double aJqhEMejfr93107552 = -414015346;    double aJqhEMejfr22716687 = -997122835;    double aJqhEMejfr83419100 = -278015183;    double aJqhEMejfr63518069 = -595937834;    double aJqhEMejfr89977767 = 28951501;    double aJqhEMejfr54717415 = -326704653;    double aJqhEMejfr6444904 = -535241530;    double aJqhEMejfr77460971 = -613668142;    double aJqhEMejfr3020722 = -29169601;    double aJqhEMejfr53479669 = -333119136;    double aJqhEMejfr86755520 = -786308831;    double aJqhEMejfr63862659 = -271352872;    double aJqhEMejfr74649911 = -729899611;    double aJqhEMejfr5055190 = -274456985;    double aJqhEMejfr70389985 = -170761372;    double aJqhEMejfr96086485 = -265520554;    double aJqhEMejfr83734762 = -412089784;    double aJqhEMejfr92302677 = -605674268;    double aJqhEMejfr10472984 = -787663059;    double aJqhEMejfr42182681 = 67852200;    double aJqhEMejfr71134910 = -460248957;    double aJqhEMejfr59267728 = -772723278;    double aJqhEMejfr32669378 = 63013960;    double aJqhEMejfr42277464 = 82104363;    double aJqhEMejfr43225237 = -41514806;    double aJqhEMejfr75416153 = -99295489;    double aJqhEMejfr69187892 = -840968586;    double aJqhEMejfr61735146 = -916616656;    double aJqhEMejfr76783881 = -945035324;    double aJqhEMejfr66937848 = -421590295;    double aJqhEMejfr45656825 = -923517352;    double aJqhEMejfr43889223 = -937939594;    double aJqhEMejfr28243130 = -749565331;    double aJqhEMejfr38671603 = -628607630;    double aJqhEMejfr20260946 = -988306524;    double aJqhEMejfr36651138 = -571938037;    double aJqhEMejfr77672946 = -415639511;    double aJqhEMejfr94785096 = -400165045;    double aJqhEMejfr99641993 = -215829241;    double aJqhEMejfr95502859 = -388720688;    double aJqhEMejfr17154870 = -246559385;    double aJqhEMejfr32429516 = -354553633;    double aJqhEMejfr44854373 = -706292299;    double aJqhEMejfr8862717 = -134133432;    double aJqhEMejfr25690410 = -263058847;    double aJqhEMejfr88890861 = -116005008;    double aJqhEMejfr36239869 = -868255972;    double aJqhEMejfr50429228 = -906128105;    double aJqhEMejfr70471771 = -893354454;    double aJqhEMejfr11954585 = -524772752;    double aJqhEMejfr50158334 = -791013993;    double aJqhEMejfr34911454 = 54950951;    double aJqhEMejfr50091203 = -15850443;    double aJqhEMejfr13285121 = -956413602;    double aJqhEMejfr99330926 = -628685809;    double aJqhEMejfr33839825 = -641292069;    double aJqhEMejfr90047308 = -960136796;    double aJqhEMejfr41141636 = -260119546;    double aJqhEMejfr20292832 = -454423029;    double aJqhEMejfr14561615 = -871753010;    double aJqhEMejfr85529523 = -485736067;    double aJqhEMejfr44709758 = -618624874;    double aJqhEMejfr677091 = -668632818;    double aJqhEMejfr36082873 = -607579307;    double aJqhEMejfr7822845 = -409601785;    double aJqhEMejfr42866297 = -608185176;     aJqhEMejfr7142822 = aJqhEMejfr95011763;     aJqhEMejfr95011763 = aJqhEMejfr54723394;     aJqhEMejfr54723394 = aJqhEMejfr95978758;     aJqhEMejfr95978758 = aJqhEMejfr86305933;     aJqhEMejfr86305933 = aJqhEMejfr24672309;     aJqhEMejfr24672309 = aJqhEMejfr2596902;     aJqhEMejfr2596902 = aJqhEMejfr84824951;     aJqhEMejfr84824951 = aJqhEMejfr830945;     aJqhEMejfr830945 = aJqhEMejfr72870956;     aJqhEMejfr72870956 = aJqhEMejfr4453410;     aJqhEMejfr4453410 = aJqhEMejfr83710932;     aJqhEMejfr83710932 = aJqhEMejfr9137440;     aJqhEMejfr9137440 = aJqhEMejfr13368498;     aJqhEMejfr13368498 = aJqhEMejfr59367824;     aJqhEMejfr59367824 = aJqhEMejfr61092047;     aJqhEMejfr61092047 = aJqhEMejfr58303165;     aJqhEMejfr58303165 = aJqhEMejfr89619761;     aJqhEMejfr89619761 = aJqhEMejfr50220274;     aJqhEMejfr50220274 = aJqhEMejfr23599773;     aJqhEMejfr23599773 = aJqhEMejfr9890488;     aJqhEMejfr9890488 = aJqhEMejfr47875094;     aJqhEMejfr47875094 = aJqhEMejfr62342386;     aJqhEMejfr62342386 = aJqhEMejfr12445930;     aJqhEMejfr12445930 = aJqhEMejfr52753521;     aJqhEMejfr52753521 = aJqhEMejfr10889781;     aJqhEMejfr10889781 = aJqhEMejfr55484417;     aJqhEMejfr55484417 = aJqhEMejfr40861756;     aJqhEMejfr40861756 = aJqhEMejfr8041071;     aJqhEMejfr8041071 = aJqhEMejfr33893096;     aJqhEMejfr33893096 = aJqhEMejfr27214131;     aJqhEMejfr27214131 = aJqhEMejfr60564186;     aJqhEMejfr60564186 = aJqhEMejfr55467802;     aJqhEMejfr55467802 = aJqhEMejfr70465837;     aJqhEMejfr70465837 = aJqhEMejfr93107552;     aJqhEMejfr93107552 = aJqhEMejfr22716687;     aJqhEMejfr22716687 = aJqhEMejfr83419100;     aJqhEMejfr83419100 = aJqhEMejfr63518069;     aJqhEMejfr63518069 = aJqhEMejfr89977767;     aJqhEMejfr89977767 = aJqhEMejfr54717415;     aJqhEMejfr54717415 = aJqhEMejfr6444904;     aJqhEMejfr6444904 = aJqhEMejfr77460971;     aJqhEMejfr77460971 = aJqhEMejfr3020722;     aJqhEMejfr3020722 = aJqhEMejfr53479669;     aJqhEMejfr53479669 = aJqhEMejfr86755520;     aJqhEMejfr86755520 = aJqhEMejfr63862659;     aJqhEMejfr63862659 = aJqhEMejfr74649911;     aJqhEMejfr74649911 = aJqhEMejfr5055190;     aJqhEMejfr5055190 = aJqhEMejfr70389985;     aJqhEMejfr70389985 = aJqhEMejfr96086485;     aJqhEMejfr96086485 = aJqhEMejfr83734762;     aJqhEMejfr83734762 = aJqhEMejfr92302677;     aJqhEMejfr92302677 = aJqhEMejfr10472984;     aJqhEMejfr10472984 = aJqhEMejfr42182681;     aJqhEMejfr42182681 = aJqhEMejfr71134910;     aJqhEMejfr71134910 = aJqhEMejfr59267728;     aJqhEMejfr59267728 = aJqhEMejfr32669378;     aJqhEMejfr32669378 = aJqhEMejfr42277464;     aJqhEMejfr42277464 = aJqhEMejfr43225237;     aJqhEMejfr43225237 = aJqhEMejfr75416153;     aJqhEMejfr75416153 = aJqhEMejfr69187892;     aJqhEMejfr69187892 = aJqhEMejfr61735146;     aJqhEMejfr61735146 = aJqhEMejfr76783881;     aJqhEMejfr76783881 = aJqhEMejfr66937848;     aJqhEMejfr66937848 = aJqhEMejfr45656825;     aJqhEMejfr45656825 = aJqhEMejfr43889223;     aJqhEMejfr43889223 = aJqhEMejfr28243130;     aJqhEMejfr28243130 = aJqhEMejfr38671603;     aJqhEMejfr38671603 = aJqhEMejfr20260946;     aJqhEMejfr20260946 = aJqhEMejfr36651138;     aJqhEMejfr36651138 = aJqhEMejfr77672946;     aJqhEMejfr77672946 = aJqhEMejfr94785096;     aJqhEMejfr94785096 = aJqhEMejfr99641993;     aJqhEMejfr99641993 = aJqhEMejfr95502859;     aJqhEMejfr95502859 = aJqhEMejfr17154870;     aJqhEMejfr17154870 = aJqhEMejfr32429516;     aJqhEMejfr32429516 = aJqhEMejfr44854373;     aJqhEMejfr44854373 = aJqhEMejfr8862717;     aJqhEMejfr8862717 = aJqhEMejfr25690410;     aJqhEMejfr25690410 = aJqhEMejfr88890861;     aJqhEMejfr88890861 = aJqhEMejfr36239869;     aJqhEMejfr36239869 = aJqhEMejfr50429228;     aJqhEMejfr50429228 = aJqhEMejfr70471771;     aJqhEMejfr70471771 = aJqhEMejfr11954585;     aJqhEMejfr11954585 = aJqhEMejfr50158334;     aJqhEMejfr50158334 = aJqhEMejfr34911454;     aJqhEMejfr34911454 = aJqhEMejfr50091203;     aJqhEMejfr50091203 = aJqhEMejfr13285121;     aJqhEMejfr13285121 = aJqhEMejfr99330926;     aJqhEMejfr99330926 = aJqhEMejfr33839825;     aJqhEMejfr33839825 = aJqhEMejfr90047308;     aJqhEMejfr90047308 = aJqhEMejfr41141636;     aJqhEMejfr41141636 = aJqhEMejfr20292832;     aJqhEMejfr20292832 = aJqhEMejfr14561615;     aJqhEMejfr14561615 = aJqhEMejfr85529523;     aJqhEMejfr85529523 = aJqhEMejfr44709758;     aJqhEMejfr44709758 = aJqhEMejfr677091;     aJqhEMejfr677091 = aJqhEMejfr36082873;     aJqhEMejfr36082873 = aJqhEMejfr7822845;     aJqhEMejfr7822845 = aJqhEMejfr42866297;     aJqhEMejfr42866297 = aJqhEMejfr7142822;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TaXLzWAzQM26721090() {     double JYYhxHdCRU26331780 = -451960002;    double JYYhxHdCRU67309621 = -245846939;    double JYYhxHdCRU79382776 = -744109467;    double JYYhxHdCRU80270205 = -634364855;    double JYYhxHdCRU13312438 = -832642787;    double JYYhxHdCRU37429266 = -687789428;    double JYYhxHdCRU81587535 = -142435887;    double JYYhxHdCRU21082545 = -295840038;    double JYYhxHdCRU69827914 = 3137895;    double JYYhxHdCRU37239743 = -825161811;    double JYYhxHdCRU65203575 = -524274010;    double JYYhxHdCRU60199174 = -795908411;    double JYYhxHdCRU66573023 = -90728670;    double JYYhxHdCRU7038286 = -383215089;    double JYYhxHdCRU76569490 = 64529661;    double JYYhxHdCRU41882829 = -20798302;    double JYYhxHdCRU58151752 = -862342725;    double JYYhxHdCRU80456690 = -674085760;    double JYYhxHdCRU71318814 = -302655733;    double JYYhxHdCRU19470873 = -312630447;    double JYYhxHdCRU10647437 = -589496611;    double JYYhxHdCRU74002424 = -325261596;    double JYYhxHdCRU65165234 = -875321646;    double JYYhxHdCRU63731462 = -218504412;    double JYYhxHdCRU68930875 = -162979986;    double JYYhxHdCRU58006633 = -821493489;    double JYYhxHdCRU67275349 = -129944941;    double JYYhxHdCRU88785856 = -511362500;    double JYYhxHdCRU13948980 = -242513693;    double JYYhxHdCRU98884964 = -134785948;    double JYYhxHdCRU93749106 = 65291410;    double JYYhxHdCRU38456636 = -833860378;    double JYYhxHdCRU96358090 = -499427254;    double JYYhxHdCRU35656140 = -681043495;    double JYYhxHdCRU44161294 = -940092280;    double JYYhxHdCRU1310575 = -10318872;    double JYYhxHdCRU20653139 = -363399142;    double JYYhxHdCRU95197583 = -482473953;    double JYYhxHdCRU45670452 = -314802240;    double JYYhxHdCRU86305170 = -129182392;    double JYYhxHdCRU16808365 = -966536673;    double JYYhxHdCRU10468164 = -720447020;    double JYYhxHdCRU64920886 = -875624387;    double JYYhxHdCRU81574628 = -959430835;    double JYYhxHdCRU30741178 = -465574669;    double JYYhxHdCRU13662902 = -113263525;    double JYYhxHdCRU99476251 = -754251257;    double JYYhxHdCRU86139767 = -694344120;    double JYYhxHdCRU8498475 = 29481458;    double JYYhxHdCRU36443982 = -701537918;    double JYYhxHdCRU91336947 = -873583736;    double JYYhxHdCRU12746613 = -189233653;    double JYYhxHdCRU49028186 = -663586971;    double JYYhxHdCRU93856862 = -168445636;    double JYYhxHdCRU16073654 = -266566770;    double JYYhxHdCRU52329355 = -26698406;    double JYYhxHdCRU2144387 = -370525293;    double JYYhxHdCRU15651315 = -425605055;    double JYYhxHdCRU11339330 = -371384869;    double JYYhxHdCRU55305805 = 88850701;    double JYYhxHdCRU70153917 = -457844488;    double JYYhxHdCRU92801679 = -631073387;    double JYYhxHdCRU7133565 = 46673655;    double JYYhxHdCRU70942950 = -862076157;    double JYYhxHdCRU43490636 = -790453221;    double JYYhxHdCRU26746940 = -690413633;    double JYYhxHdCRU63841083 = -196481158;    double JYYhxHdCRU30916883 = -409685176;    double JYYhxHdCRU62876992 = -443122810;    double JYYhxHdCRU75258916 = -925151468;    double JYYhxHdCRU21229690 = -657399161;    double JYYhxHdCRU62954168 = -279868772;    double JYYhxHdCRU34786238 = -259283520;    double JYYhxHdCRU85013644 = -73473341;    double JYYhxHdCRU2662509 = -346093775;    double JYYhxHdCRU179273 = -869049591;    double JYYhxHdCRU9081539 = -449637210;    double JYYhxHdCRU83590606 = -915890812;    double JYYhxHdCRU32990284 = -752929744;    double JYYhxHdCRU55267974 = 50283539;    double JYYhxHdCRU58530381 = 32757768;    double JYYhxHdCRU81135581 = -435600821;    double JYYhxHdCRU80287381 = -440843959;    double JYYhxHdCRU77504997 = -540975775;    double JYYhxHdCRU7548017 = -261202213;    double JYYhxHdCRU81002494 = -745474938;    double JYYhxHdCRU89428449 = -70273407;    double JYYhxHdCRU2501229 = -230981619;    double JYYhxHdCRU19582487 = -314476725;    double JYYhxHdCRU91831938 = -813393874;    double JYYhxHdCRU99166187 = -639793579;    double JYYhxHdCRU5001825 = -937794087;    double JYYhxHdCRU83858253 = -11089085;    double JYYhxHdCRU90364647 = -303652942;    double JYYhxHdCRU16151253 = -671337905;    double JYYhxHdCRU24006686 = -235463286;    double JYYhxHdCRU3334599 = -667120675;    double JYYhxHdCRU93977936 = 86451769;    double JYYhxHdCRU38083993 = -68977614;    double JYYhxHdCRU3994239 = -451960002;     JYYhxHdCRU26331780 = JYYhxHdCRU67309621;     JYYhxHdCRU67309621 = JYYhxHdCRU79382776;     JYYhxHdCRU79382776 = JYYhxHdCRU80270205;     JYYhxHdCRU80270205 = JYYhxHdCRU13312438;     JYYhxHdCRU13312438 = JYYhxHdCRU37429266;     JYYhxHdCRU37429266 = JYYhxHdCRU81587535;     JYYhxHdCRU81587535 = JYYhxHdCRU21082545;     JYYhxHdCRU21082545 = JYYhxHdCRU69827914;     JYYhxHdCRU69827914 = JYYhxHdCRU37239743;     JYYhxHdCRU37239743 = JYYhxHdCRU65203575;     JYYhxHdCRU65203575 = JYYhxHdCRU60199174;     JYYhxHdCRU60199174 = JYYhxHdCRU66573023;     JYYhxHdCRU66573023 = JYYhxHdCRU7038286;     JYYhxHdCRU7038286 = JYYhxHdCRU76569490;     JYYhxHdCRU76569490 = JYYhxHdCRU41882829;     JYYhxHdCRU41882829 = JYYhxHdCRU58151752;     JYYhxHdCRU58151752 = JYYhxHdCRU80456690;     JYYhxHdCRU80456690 = JYYhxHdCRU71318814;     JYYhxHdCRU71318814 = JYYhxHdCRU19470873;     JYYhxHdCRU19470873 = JYYhxHdCRU10647437;     JYYhxHdCRU10647437 = JYYhxHdCRU74002424;     JYYhxHdCRU74002424 = JYYhxHdCRU65165234;     JYYhxHdCRU65165234 = JYYhxHdCRU63731462;     JYYhxHdCRU63731462 = JYYhxHdCRU68930875;     JYYhxHdCRU68930875 = JYYhxHdCRU58006633;     JYYhxHdCRU58006633 = JYYhxHdCRU67275349;     JYYhxHdCRU67275349 = JYYhxHdCRU88785856;     JYYhxHdCRU88785856 = JYYhxHdCRU13948980;     JYYhxHdCRU13948980 = JYYhxHdCRU98884964;     JYYhxHdCRU98884964 = JYYhxHdCRU93749106;     JYYhxHdCRU93749106 = JYYhxHdCRU38456636;     JYYhxHdCRU38456636 = JYYhxHdCRU96358090;     JYYhxHdCRU96358090 = JYYhxHdCRU35656140;     JYYhxHdCRU35656140 = JYYhxHdCRU44161294;     JYYhxHdCRU44161294 = JYYhxHdCRU1310575;     JYYhxHdCRU1310575 = JYYhxHdCRU20653139;     JYYhxHdCRU20653139 = JYYhxHdCRU95197583;     JYYhxHdCRU95197583 = JYYhxHdCRU45670452;     JYYhxHdCRU45670452 = JYYhxHdCRU86305170;     JYYhxHdCRU86305170 = JYYhxHdCRU16808365;     JYYhxHdCRU16808365 = JYYhxHdCRU10468164;     JYYhxHdCRU10468164 = JYYhxHdCRU64920886;     JYYhxHdCRU64920886 = JYYhxHdCRU81574628;     JYYhxHdCRU81574628 = JYYhxHdCRU30741178;     JYYhxHdCRU30741178 = JYYhxHdCRU13662902;     JYYhxHdCRU13662902 = JYYhxHdCRU99476251;     JYYhxHdCRU99476251 = JYYhxHdCRU86139767;     JYYhxHdCRU86139767 = JYYhxHdCRU8498475;     JYYhxHdCRU8498475 = JYYhxHdCRU36443982;     JYYhxHdCRU36443982 = JYYhxHdCRU91336947;     JYYhxHdCRU91336947 = JYYhxHdCRU12746613;     JYYhxHdCRU12746613 = JYYhxHdCRU49028186;     JYYhxHdCRU49028186 = JYYhxHdCRU93856862;     JYYhxHdCRU93856862 = JYYhxHdCRU16073654;     JYYhxHdCRU16073654 = JYYhxHdCRU52329355;     JYYhxHdCRU52329355 = JYYhxHdCRU2144387;     JYYhxHdCRU2144387 = JYYhxHdCRU15651315;     JYYhxHdCRU15651315 = JYYhxHdCRU11339330;     JYYhxHdCRU11339330 = JYYhxHdCRU55305805;     JYYhxHdCRU55305805 = JYYhxHdCRU70153917;     JYYhxHdCRU70153917 = JYYhxHdCRU92801679;     JYYhxHdCRU92801679 = JYYhxHdCRU7133565;     JYYhxHdCRU7133565 = JYYhxHdCRU70942950;     JYYhxHdCRU70942950 = JYYhxHdCRU43490636;     JYYhxHdCRU43490636 = JYYhxHdCRU26746940;     JYYhxHdCRU26746940 = JYYhxHdCRU63841083;     JYYhxHdCRU63841083 = JYYhxHdCRU30916883;     JYYhxHdCRU30916883 = JYYhxHdCRU62876992;     JYYhxHdCRU62876992 = JYYhxHdCRU75258916;     JYYhxHdCRU75258916 = JYYhxHdCRU21229690;     JYYhxHdCRU21229690 = JYYhxHdCRU62954168;     JYYhxHdCRU62954168 = JYYhxHdCRU34786238;     JYYhxHdCRU34786238 = JYYhxHdCRU85013644;     JYYhxHdCRU85013644 = JYYhxHdCRU2662509;     JYYhxHdCRU2662509 = JYYhxHdCRU179273;     JYYhxHdCRU179273 = JYYhxHdCRU9081539;     JYYhxHdCRU9081539 = JYYhxHdCRU83590606;     JYYhxHdCRU83590606 = JYYhxHdCRU32990284;     JYYhxHdCRU32990284 = JYYhxHdCRU55267974;     JYYhxHdCRU55267974 = JYYhxHdCRU58530381;     JYYhxHdCRU58530381 = JYYhxHdCRU81135581;     JYYhxHdCRU81135581 = JYYhxHdCRU80287381;     JYYhxHdCRU80287381 = JYYhxHdCRU77504997;     JYYhxHdCRU77504997 = JYYhxHdCRU7548017;     JYYhxHdCRU7548017 = JYYhxHdCRU81002494;     JYYhxHdCRU81002494 = JYYhxHdCRU89428449;     JYYhxHdCRU89428449 = JYYhxHdCRU2501229;     JYYhxHdCRU2501229 = JYYhxHdCRU19582487;     JYYhxHdCRU19582487 = JYYhxHdCRU91831938;     JYYhxHdCRU91831938 = JYYhxHdCRU99166187;     JYYhxHdCRU99166187 = JYYhxHdCRU5001825;     JYYhxHdCRU5001825 = JYYhxHdCRU83858253;     JYYhxHdCRU83858253 = JYYhxHdCRU90364647;     JYYhxHdCRU90364647 = JYYhxHdCRU16151253;     JYYhxHdCRU16151253 = JYYhxHdCRU24006686;     JYYhxHdCRU24006686 = JYYhxHdCRU3334599;     JYYhxHdCRU3334599 = JYYhxHdCRU93977936;     JYYhxHdCRU93977936 = JYYhxHdCRU38083993;     JYYhxHdCRU38083993 = JYYhxHdCRU3994239;     JYYhxHdCRU3994239 = JYYhxHdCRU26331780;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void kBivmEKhoL42100355() {     double uzmbKBXTuP77788017 = 1141552;    double uzmbKBXTuP2257598 = -518477031;    double uzmbKBXTuP36005526 = -466686388;    double uzmbKBXTuP20989906 = -639551904;    double uzmbKBXTuP16596358 = 50787260;    double uzmbKBXTuP49477504 = -522567638;    double uzmbKBXTuP89523133 = -784044659;    double uzmbKBXTuP10881383 = -886323036;    double uzmbKBXTuP29436163 = -531687470;    double uzmbKBXTuP92476930 = -512142890;    double uzmbKBXTuP67023176 = -835827097;    double uzmbKBXTuP43549180 = -789368887;    double uzmbKBXTuP20817741 = -748737929;    double uzmbKBXTuP51059752 = -548503131;    double uzmbKBXTuP87259953 = -681523726;    double uzmbKBXTuP68185234 = -696433931;    double uzmbKBXTuP8008752 = -824902579;    double uzmbKBXTuP32913789 = -673115557;    double uzmbKBXTuP52356325 = 59404451;    double uzmbKBXTuP54460245 = -936191671;    double uzmbKBXTuP44695666 = -687367289;    double uzmbKBXTuP87567124 = -576739088;    double uzmbKBXTuP1164592 = -494053554;    double uzmbKBXTuP89945575 = -622689106;    double uzmbKBXTuP45320599 = -589956420;    double uzmbKBXTuP58061438 = -604645956;    double uzmbKBXTuP33966784 = -876562578;    double uzmbKBXTuP84047506 = -78206582;    double uzmbKBXTuP52862005 = -730721837;    double uzmbKBXTuP49155062 = -436930221;    double uzmbKBXTuP67698806 = -786250237;    double uzmbKBXTuP89799504 = -523632427;    double uzmbKBXTuP23865586 = -37467226;    double uzmbKBXTuP36113649 = -629146183;    double uzmbKBXTuP53489827 = -520276050;    double uzmbKBXTuP25538135 = -56115129;    double uzmbKBXTuP39151954 = -566261769;    double uzmbKBXTuP86228237 = -375313621;    double uzmbKBXTuP3824654 = -639458551;    double uzmbKBXTuP21693606 = -553744702;    double uzmbKBXTuP54373855 = 92795693;    double uzmbKBXTuP91641623 = -576849294;    double uzmbKBXTuP12271042 = -575053907;    double uzmbKBXTuP19219868 = 37941449;    double uzmbKBXTuP66727631 = -468214627;    double uzmbKBXTuP10696463 = -941734698;    double uzmbKBXTuP28478907 = -227250033;    double uzmbKBXTuP46052980 = -235348637;    double uzmbKBXTuP94489826 = -820289203;    double uzmbKBXTuP24559395 = -135554319;    double uzmbKBXTuP65183456 = -637216912;    double uzmbKBXTuP48721439 = -957039738;    double uzmbKBXTuP40996989 = -607515109;    double uzmbKBXTuP42660256 = -24949148;    double uzmbKBXTuP97404689 = -328089149;    double uzmbKBXTuP90220892 = -422119361;    double uzmbKBXTuP1093007 = 75576522;    double uzmbKBXTuP46059950 = -843997282;    double uzmbKBXTuP75669306 = 50404516;    double uzmbKBXTuP58534919 = -344566785;    double uzmbKBXTuP15510720 = -646005061;    double uzmbKBXTuP5475628 = -605838077;    double uzmbKBXTuP58019377 = -55601199;    double uzmbKBXTuP80281101 = 5242751;    double uzmbKBXTuP24778124 = -725892653;    double uzmbKBXTuP77223671 = -212194671;    double uzmbKBXTuP19683595 = -651901661;    double uzmbKBXTuP84704091 = -19591746;    double uzmbKBXTuP97569924 = 71772919;    double uzmbKBXTuP61721818 = -525408598;    double uzmbKBXTuP29033281 = -30172163;    double uzmbKBXTuP21780514 = -349588958;    double uzmbKBXTuP29089136 = 66342994;    double uzmbKBXTuP30662720 = -386850847;    double uzmbKBXTuP86390 = -928987364;    double uzmbKBXTuP53054043 = -10517996;    double uzmbKBXTuP75296083 = 98314819;    double uzmbKBXTuP81944723 = -431995004;    double uzmbKBXTuP23217944 = -54474480;    double uzmbKBXTuP34624137 = -648221723;    double uzmbKBXTuP29582531 = -277395923;    double uzmbKBXTuP87913804 = -541213942;    double uzmbKBXTuP89557679 = -257917380;    double uzmbKBXTuP28302610 = -495167519;    double uzmbKBXTuP83971605 = -799713309;    double uzmbKBXTuP18977367 = -829210499;    double uzmbKBXTuP48802516 = -916117318;    double uzmbKBXTuP81205329 = 87481921;    double uzmbKBXTuP38708960 = -201057034;    double uzmbKBXTuP63268934 = 1843310;    double uzmbKBXTuP24445129 = -31691652;    double uzmbKBXTuP93092003 = -722264487;    double uzmbKBXTuP10558931 = -325718137;    double uzmbKBXTuP45289734 = -194891767;    double uzmbKBXTuP6182887 = -907739641;    double uzmbKBXTuP48898228 = -301366230;    double uzmbKBXTuP33622246 = -421248095;    double uzmbKBXTuP31989940 = -480296659;    double uzmbKBXTuP94441743 = -236165898;    double uzmbKBXTuP89503960 = 1141552;     uzmbKBXTuP77788017 = uzmbKBXTuP2257598;     uzmbKBXTuP2257598 = uzmbKBXTuP36005526;     uzmbKBXTuP36005526 = uzmbKBXTuP20989906;     uzmbKBXTuP20989906 = uzmbKBXTuP16596358;     uzmbKBXTuP16596358 = uzmbKBXTuP49477504;     uzmbKBXTuP49477504 = uzmbKBXTuP89523133;     uzmbKBXTuP89523133 = uzmbKBXTuP10881383;     uzmbKBXTuP10881383 = uzmbKBXTuP29436163;     uzmbKBXTuP29436163 = uzmbKBXTuP92476930;     uzmbKBXTuP92476930 = uzmbKBXTuP67023176;     uzmbKBXTuP67023176 = uzmbKBXTuP43549180;     uzmbKBXTuP43549180 = uzmbKBXTuP20817741;     uzmbKBXTuP20817741 = uzmbKBXTuP51059752;     uzmbKBXTuP51059752 = uzmbKBXTuP87259953;     uzmbKBXTuP87259953 = uzmbKBXTuP68185234;     uzmbKBXTuP68185234 = uzmbKBXTuP8008752;     uzmbKBXTuP8008752 = uzmbKBXTuP32913789;     uzmbKBXTuP32913789 = uzmbKBXTuP52356325;     uzmbKBXTuP52356325 = uzmbKBXTuP54460245;     uzmbKBXTuP54460245 = uzmbKBXTuP44695666;     uzmbKBXTuP44695666 = uzmbKBXTuP87567124;     uzmbKBXTuP87567124 = uzmbKBXTuP1164592;     uzmbKBXTuP1164592 = uzmbKBXTuP89945575;     uzmbKBXTuP89945575 = uzmbKBXTuP45320599;     uzmbKBXTuP45320599 = uzmbKBXTuP58061438;     uzmbKBXTuP58061438 = uzmbKBXTuP33966784;     uzmbKBXTuP33966784 = uzmbKBXTuP84047506;     uzmbKBXTuP84047506 = uzmbKBXTuP52862005;     uzmbKBXTuP52862005 = uzmbKBXTuP49155062;     uzmbKBXTuP49155062 = uzmbKBXTuP67698806;     uzmbKBXTuP67698806 = uzmbKBXTuP89799504;     uzmbKBXTuP89799504 = uzmbKBXTuP23865586;     uzmbKBXTuP23865586 = uzmbKBXTuP36113649;     uzmbKBXTuP36113649 = uzmbKBXTuP53489827;     uzmbKBXTuP53489827 = uzmbKBXTuP25538135;     uzmbKBXTuP25538135 = uzmbKBXTuP39151954;     uzmbKBXTuP39151954 = uzmbKBXTuP86228237;     uzmbKBXTuP86228237 = uzmbKBXTuP3824654;     uzmbKBXTuP3824654 = uzmbKBXTuP21693606;     uzmbKBXTuP21693606 = uzmbKBXTuP54373855;     uzmbKBXTuP54373855 = uzmbKBXTuP91641623;     uzmbKBXTuP91641623 = uzmbKBXTuP12271042;     uzmbKBXTuP12271042 = uzmbKBXTuP19219868;     uzmbKBXTuP19219868 = uzmbKBXTuP66727631;     uzmbKBXTuP66727631 = uzmbKBXTuP10696463;     uzmbKBXTuP10696463 = uzmbKBXTuP28478907;     uzmbKBXTuP28478907 = uzmbKBXTuP46052980;     uzmbKBXTuP46052980 = uzmbKBXTuP94489826;     uzmbKBXTuP94489826 = uzmbKBXTuP24559395;     uzmbKBXTuP24559395 = uzmbKBXTuP65183456;     uzmbKBXTuP65183456 = uzmbKBXTuP48721439;     uzmbKBXTuP48721439 = uzmbKBXTuP40996989;     uzmbKBXTuP40996989 = uzmbKBXTuP42660256;     uzmbKBXTuP42660256 = uzmbKBXTuP97404689;     uzmbKBXTuP97404689 = uzmbKBXTuP90220892;     uzmbKBXTuP90220892 = uzmbKBXTuP1093007;     uzmbKBXTuP1093007 = uzmbKBXTuP46059950;     uzmbKBXTuP46059950 = uzmbKBXTuP75669306;     uzmbKBXTuP75669306 = uzmbKBXTuP58534919;     uzmbKBXTuP58534919 = uzmbKBXTuP15510720;     uzmbKBXTuP15510720 = uzmbKBXTuP5475628;     uzmbKBXTuP5475628 = uzmbKBXTuP58019377;     uzmbKBXTuP58019377 = uzmbKBXTuP80281101;     uzmbKBXTuP80281101 = uzmbKBXTuP24778124;     uzmbKBXTuP24778124 = uzmbKBXTuP77223671;     uzmbKBXTuP77223671 = uzmbKBXTuP19683595;     uzmbKBXTuP19683595 = uzmbKBXTuP84704091;     uzmbKBXTuP84704091 = uzmbKBXTuP97569924;     uzmbKBXTuP97569924 = uzmbKBXTuP61721818;     uzmbKBXTuP61721818 = uzmbKBXTuP29033281;     uzmbKBXTuP29033281 = uzmbKBXTuP21780514;     uzmbKBXTuP21780514 = uzmbKBXTuP29089136;     uzmbKBXTuP29089136 = uzmbKBXTuP30662720;     uzmbKBXTuP30662720 = uzmbKBXTuP86390;     uzmbKBXTuP86390 = uzmbKBXTuP53054043;     uzmbKBXTuP53054043 = uzmbKBXTuP75296083;     uzmbKBXTuP75296083 = uzmbKBXTuP81944723;     uzmbKBXTuP81944723 = uzmbKBXTuP23217944;     uzmbKBXTuP23217944 = uzmbKBXTuP34624137;     uzmbKBXTuP34624137 = uzmbKBXTuP29582531;     uzmbKBXTuP29582531 = uzmbKBXTuP87913804;     uzmbKBXTuP87913804 = uzmbKBXTuP89557679;     uzmbKBXTuP89557679 = uzmbKBXTuP28302610;     uzmbKBXTuP28302610 = uzmbKBXTuP83971605;     uzmbKBXTuP83971605 = uzmbKBXTuP18977367;     uzmbKBXTuP18977367 = uzmbKBXTuP48802516;     uzmbKBXTuP48802516 = uzmbKBXTuP81205329;     uzmbKBXTuP81205329 = uzmbKBXTuP38708960;     uzmbKBXTuP38708960 = uzmbKBXTuP63268934;     uzmbKBXTuP63268934 = uzmbKBXTuP24445129;     uzmbKBXTuP24445129 = uzmbKBXTuP93092003;     uzmbKBXTuP93092003 = uzmbKBXTuP10558931;     uzmbKBXTuP10558931 = uzmbKBXTuP45289734;     uzmbKBXTuP45289734 = uzmbKBXTuP6182887;     uzmbKBXTuP6182887 = uzmbKBXTuP48898228;     uzmbKBXTuP48898228 = uzmbKBXTuP33622246;     uzmbKBXTuP33622246 = uzmbKBXTuP31989940;     uzmbKBXTuP31989940 = uzmbKBXTuP94441743;     uzmbKBXTuP94441743 = uzmbKBXTuP89503960;     uzmbKBXTuP89503960 = uzmbKBXTuP77788017;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IRjcPmkAHb48428902() {     double coJFNErVhI32442415 = -436386034;    double coJFNErVhI49255217 = 1958149;    double coJFNErVhI96738171 = -852071002;    double coJFNErVhI92424847 = -645654312;    double coJFNErVhI91048028 = -721647988;    double coJFNErVhI63651901 = -651718474;    double coJFNErVhI10623837 = 14062669;    double coJFNErVhI40056486 = -286891284;    double coJFNErVhI17210574 = -578540841;    double coJFNErVhI75108915 = -790944159;    double coJFNErVhI45634471 = -361183670;    double coJFNErVhI6313894 = -975792985;    double coJFNErVhI84635055 = -552278233;    double coJFNErVhI44026183 = -160606710;    double coJFNErVhI17484027 = -588645376;    double coJFNErVhI57952770 = -326593495;    double coJFNErVhI7840515 = -198502407;    double coJFNErVhI510376 = -736680029;    double coJFNErVhI53576927 = -226407095;    double coJFNErVhI72094801 = -181557817;    double coJFNErVhI78870054 = -155450445;    double coJFNErVhI38819714 = -743183196;    double coJFNErVhI37634423 = 19203025;    double coJFNErVhI91373943 = -321729923;    double coJFNErVhI41073215 = -962869884;    double coJFNErVhI99302385 = -608354736;    double coJFNErVhI35956709 = -331406857;    double coJFNErVhI37296507 = -86258448;    double coJFNErVhI92759681 = -10966697;    double coJFNErVhI43590471 = -727688189;    double coJFNErVhI19404335 = -235122763;    double coJFNErVhI20791115 = -935128952;    double coJFNErVhI91521462 = -593984837;    double coJFNErVhI30769542 = -309266992;    double coJFNErVhI87993983 = -738139307;    double coJFNErVhI12864678 = -304110707;    double coJFNErVhI13856442 = -416688389;    double coJFNErVhI99205475 = -249242645;    double coJFNErVhI54594302 = 78592859;    double coJFNErVhI45680000 = -212053301;    double coJFNErVhI10333256 = -19754465;    double coJFNErVhI28316282 = -84381360;    double coJFNErVhI3271225 = -415559223;    double coJFNErVhI28214267 = -535738217;    double coJFNErVhI26711695 = -600732224;    double coJFNErVhI66030064 = 89475706;    double coJFNErVhI44952618 = -254307416;    double coJFNErVhI69480289 = -213001008;    double coJFNErVhI36832593 = -720019367;    double coJFNErVhI69401057 = -375573618;    double coJFNErVhI40296996 = -294432414;    double coJFNErVhI38103589 = -372105717;    double coJFNErVhI72724991 = -347430570;    double coJFNErVhI76014 = 79164368;    double coJFNErVhI69558848 = -723997825;    double coJFNErVhI93622701 = -693202839;    double coJFNErVhI11620794 = 82755123;    double coJFNErVhI5364228 = -430341079;    double coJFNErVhI51351632 = -682784428;    double coJFNErVhI91745642 = -13293252;    double coJFNErVhI27695192 = -220311618;    double coJFNErVhI73327330 = -899678883;    double coJFNErVhI47296804 = -175924587;    double coJFNErVhI73620103 = -850852652;    double coJFNErVhI55704581 = -455821397;    double coJFNErVhI24843356 = -426054719;    double coJFNErVhI14792431 = -281808148;    double coJFNErVhI53865514 = -143011241;    double coJFNErVhI56032199 = -422467404;    double coJFNErVhI4619349 = -184534669;    double coJFNErVhI44096329 = -909905107;    double coJFNErVhI8635039 = -949259763;    double coJFNErVhI45916074 = -715272888;    double coJFNErVhI7896927 = 85646205;    double coJFNErVhI61761545 = -61803353;    double coJFNErVhI50553772 = 28930914;    double coJFNErVhI35548490 = -227623973;    double coJFNErVhI9420156 = -445058759;    double coJFNErVhI64662249 = -720997700;    double coJFNErVhI75043151 = -952345591;    double coJFNErVhI54349767 = -254047320;    double coJFNErVhI66476419 = -18405849;    double coJFNErVhI463914 = -366239082;    double coJFNErVhI23358625 = -635393080;    double coJFNErVhI3293476 = -333255776;    double coJFNErVhI81300746 = -863017046;    double coJFNErVhI48066123 = -487698382;    double coJFNErVhI91445448 = -573149205;    double coJFNErVhI61210693 = -585269167;    double coJFNErVhI94371282 = 55063531;    double coJFNErVhI1243884 = -286865831;    double coJFNErVhI8492214 = -986347311;    double coJFNErVhI47853844 = -566458217;    double coJFNErVhI62848660 = -908113889;    double coJFNErVhI17984808 = -991741683;    double coJFNErVhI37005926 = -120075583;    double coJFNErVhI81019477 = -908456773;    double coJFNErVhI29651122 = -564706571;    double coJFNErVhI72509686 = 20083180;    double coJFNErVhI1868340 = -436386034;     coJFNErVhI32442415 = coJFNErVhI49255217;     coJFNErVhI49255217 = coJFNErVhI96738171;     coJFNErVhI96738171 = coJFNErVhI92424847;     coJFNErVhI92424847 = coJFNErVhI91048028;     coJFNErVhI91048028 = coJFNErVhI63651901;     coJFNErVhI63651901 = coJFNErVhI10623837;     coJFNErVhI10623837 = coJFNErVhI40056486;     coJFNErVhI40056486 = coJFNErVhI17210574;     coJFNErVhI17210574 = coJFNErVhI75108915;     coJFNErVhI75108915 = coJFNErVhI45634471;     coJFNErVhI45634471 = coJFNErVhI6313894;     coJFNErVhI6313894 = coJFNErVhI84635055;     coJFNErVhI84635055 = coJFNErVhI44026183;     coJFNErVhI44026183 = coJFNErVhI17484027;     coJFNErVhI17484027 = coJFNErVhI57952770;     coJFNErVhI57952770 = coJFNErVhI7840515;     coJFNErVhI7840515 = coJFNErVhI510376;     coJFNErVhI510376 = coJFNErVhI53576927;     coJFNErVhI53576927 = coJFNErVhI72094801;     coJFNErVhI72094801 = coJFNErVhI78870054;     coJFNErVhI78870054 = coJFNErVhI38819714;     coJFNErVhI38819714 = coJFNErVhI37634423;     coJFNErVhI37634423 = coJFNErVhI91373943;     coJFNErVhI91373943 = coJFNErVhI41073215;     coJFNErVhI41073215 = coJFNErVhI99302385;     coJFNErVhI99302385 = coJFNErVhI35956709;     coJFNErVhI35956709 = coJFNErVhI37296507;     coJFNErVhI37296507 = coJFNErVhI92759681;     coJFNErVhI92759681 = coJFNErVhI43590471;     coJFNErVhI43590471 = coJFNErVhI19404335;     coJFNErVhI19404335 = coJFNErVhI20791115;     coJFNErVhI20791115 = coJFNErVhI91521462;     coJFNErVhI91521462 = coJFNErVhI30769542;     coJFNErVhI30769542 = coJFNErVhI87993983;     coJFNErVhI87993983 = coJFNErVhI12864678;     coJFNErVhI12864678 = coJFNErVhI13856442;     coJFNErVhI13856442 = coJFNErVhI99205475;     coJFNErVhI99205475 = coJFNErVhI54594302;     coJFNErVhI54594302 = coJFNErVhI45680000;     coJFNErVhI45680000 = coJFNErVhI10333256;     coJFNErVhI10333256 = coJFNErVhI28316282;     coJFNErVhI28316282 = coJFNErVhI3271225;     coJFNErVhI3271225 = coJFNErVhI28214267;     coJFNErVhI28214267 = coJFNErVhI26711695;     coJFNErVhI26711695 = coJFNErVhI66030064;     coJFNErVhI66030064 = coJFNErVhI44952618;     coJFNErVhI44952618 = coJFNErVhI69480289;     coJFNErVhI69480289 = coJFNErVhI36832593;     coJFNErVhI36832593 = coJFNErVhI69401057;     coJFNErVhI69401057 = coJFNErVhI40296996;     coJFNErVhI40296996 = coJFNErVhI38103589;     coJFNErVhI38103589 = coJFNErVhI72724991;     coJFNErVhI72724991 = coJFNErVhI76014;     coJFNErVhI76014 = coJFNErVhI69558848;     coJFNErVhI69558848 = coJFNErVhI93622701;     coJFNErVhI93622701 = coJFNErVhI11620794;     coJFNErVhI11620794 = coJFNErVhI5364228;     coJFNErVhI5364228 = coJFNErVhI51351632;     coJFNErVhI51351632 = coJFNErVhI91745642;     coJFNErVhI91745642 = coJFNErVhI27695192;     coJFNErVhI27695192 = coJFNErVhI73327330;     coJFNErVhI73327330 = coJFNErVhI47296804;     coJFNErVhI47296804 = coJFNErVhI73620103;     coJFNErVhI73620103 = coJFNErVhI55704581;     coJFNErVhI55704581 = coJFNErVhI24843356;     coJFNErVhI24843356 = coJFNErVhI14792431;     coJFNErVhI14792431 = coJFNErVhI53865514;     coJFNErVhI53865514 = coJFNErVhI56032199;     coJFNErVhI56032199 = coJFNErVhI4619349;     coJFNErVhI4619349 = coJFNErVhI44096329;     coJFNErVhI44096329 = coJFNErVhI8635039;     coJFNErVhI8635039 = coJFNErVhI45916074;     coJFNErVhI45916074 = coJFNErVhI7896927;     coJFNErVhI7896927 = coJFNErVhI61761545;     coJFNErVhI61761545 = coJFNErVhI50553772;     coJFNErVhI50553772 = coJFNErVhI35548490;     coJFNErVhI35548490 = coJFNErVhI9420156;     coJFNErVhI9420156 = coJFNErVhI64662249;     coJFNErVhI64662249 = coJFNErVhI75043151;     coJFNErVhI75043151 = coJFNErVhI54349767;     coJFNErVhI54349767 = coJFNErVhI66476419;     coJFNErVhI66476419 = coJFNErVhI463914;     coJFNErVhI463914 = coJFNErVhI23358625;     coJFNErVhI23358625 = coJFNErVhI3293476;     coJFNErVhI3293476 = coJFNErVhI81300746;     coJFNErVhI81300746 = coJFNErVhI48066123;     coJFNErVhI48066123 = coJFNErVhI91445448;     coJFNErVhI91445448 = coJFNErVhI61210693;     coJFNErVhI61210693 = coJFNErVhI94371282;     coJFNErVhI94371282 = coJFNErVhI1243884;     coJFNErVhI1243884 = coJFNErVhI8492214;     coJFNErVhI8492214 = coJFNErVhI47853844;     coJFNErVhI47853844 = coJFNErVhI62848660;     coJFNErVhI62848660 = coJFNErVhI17984808;     coJFNErVhI17984808 = coJFNErVhI37005926;     coJFNErVhI37005926 = coJFNErVhI81019477;     coJFNErVhI81019477 = coJFNErVhI29651122;     coJFNErVhI29651122 = coJFNErVhI72509686;     coJFNErVhI72509686 = coJFNErVhI1868340;     coJFNErVhI1868340 = coJFNErVhI32442415;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VhAsyrBqtD63808168() {     double GrXtyMIXkQ83898653 = 16715520;    double GrXtyMIXkQ84203193 = -270671943;    double GrXtyMIXkQ53360921 = -574647923;    double GrXtyMIXkQ33144548 = -650841361;    double GrXtyMIXkQ94331948 = -938217941;    double GrXtyMIXkQ75700138 = -486496685;    double GrXtyMIXkQ18559436 = -627546103;    double GrXtyMIXkQ29855324 = -877374281;    double GrXtyMIXkQ76818822 = -13366206;    double GrXtyMIXkQ30346103 = -477925239;    double GrXtyMIXkQ47454071 = -672736756;    double GrXtyMIXkQ89663899 = -969253461;    double GrXtyMIXkQ38879773 = -110287492;    double GrXtyMIXkQ88047648 = -325894751;    double GrXtyMIXkQ28174489 = -234698763;    double GrXtyMIXkQ84255175 = 97770875;    double GrXtyMIXkQ57697514 = -161062261;    double GrXtyMIXkQ52967475 = -735709826;    double GrXtyMIXkQ34614438 = -964346911;    double GrXtyMIXkQ7084173 = -805119042;    double GrXtyMIXkQ12918284 = -253321123;    double GrXtyMIXkQ52384414 = -994660687;    double GrXtyMIXkQ73633780 = -699528883;    double GrXtyMIXkQ17588058 = -725914617;    double GrXtyMIXkQ17462939 = -289846319;    double GrXtyMIXkQ99357190 = -391507203;    double GrXtyMIXkQ2648145 = 21975506;    double GrXtyMIXkQ32558157 = -753102530;    double GrXtyMIXkQ31672708 = -499174841;    double GrXtyMIXkQ93860568 = 70167538;    double GrXtyMIXkQ93354034 = 13335590;    double GrXtyMIXkQ72133984 = -624901001;    double GrXtyMIXkQ19028958 = -132024809;    double GrXtyMIXkQ31227051 = -257369680;    double GrXtyMIXkQ97322517 = -318323077;    double GrXtyMIXkQ37092238 = -349906964;    double GrXtyMIXkQ32355256 = -619551017;    double GrXtyMIXkQ90236129 = -142082313;    double GrXtyMIXkQ12748504 = -246063452;    double GrXtyMIXkQ81068436 = -636615610;    double GrXtyMIXkQ47898747 = -60422100;    double GrXtyMIXkQ9489742 = 59216367;    double GrXtyMIXkQ50621380 = -114988743;    double GrXtyMIXkQ65859506 = -638365933;    double GrXtyMIXkQ62698149 = -603372182;    double GrXtyMIXkQ63063626 = -738995467;    double GrXtyMIXkQ73955273 = -827306193;    double GrXtyMIXkQ29393502 = -854005524;    double GrXtyMIXkQ22823945 = -469790027;    double GrXtyMIXkQ57516470 = -909590018;    double GrXtyMIXkQ14143505 = -58065591;    double GrXtyMIXkQ74078416 = -39911802;    double GrXtyMIXkQ64693794 = -291358708;    double GrXtyMIXkQ48879407 = -877339144;    double GrXtyMIXkQ50889884 = -785520204;    double GrXtyMIXkQ31514239 = 11376207;    double GrXtyMIXkQ10569414 = -571143061;    double GrXtyMIXkQ35772864 = -848733307;    double GrXtyMIXkQ15681609 = -260995043;    double GrXtyMIXkQ94974757 = -446710739;    double GrXtyMIXkQ73051994 = -408472192;    double GrXtyMIXkQ86001278 = -874443573;    double GrXtyMIXkQ98182616 = -278199441;    double GrXtyMIXkQ82958254 = 16466256;    double GrXtyMIXkQ36992069 = -391260829;    double GrXtyMIXkQ75320087 = 52164244;    double GrXtyMIXkQ70634942 = -737228652;    double GrXtyMIXkQ7652723 = -852917812;    double GrXtyMIXkQ90725131 = 92428326;    double GrXtyMIXkQ91082250 = -884791799;    double GrXtyMIXkQ51899919 = -282678109;    double GrXtyMIXkQ67461385 = 81020051;    double GrXtyMIXkQ40218971 = -389646374;    double GrXtyMIXkQ53546002 = -227731301;    double GrXtyMIXkQ59185426 = -644696942;    double GrXtyMIXkQ3428543 = -212537490;    double GrXtyMIXkQ1763035 = -779671945;    double GrXtyMIXkQ7774274 = 38837049;    double GrXtyMIXkQ54889908 = -22542435;    double GrXtyMIXkQ54399313 = -550850852;    double GrXtyMIXkQ25401918 = -564201011;    double GrXtyMIXkQ73254642 = -124018970;    double GrXtyMIXkQ9734212 = -183312503;    double GrXtyMIXkQ74156237 = -589584824;    double GrXtyMIXkQ79717064 = -871766872;    double GrXtyMIXkQ19275618 = -946752608;    double GrXtyMIXkQ7440190 = -233542293;    double GrXtyMIXkQ70149550 = -254685666;    double GrXtyMIXkQ80337166 = -471849477;    double GrXtyMIXkQ65808278 = -229699285;    double GrXtyMIXkQ26522825 = -778763904;    double GrXtyMIXkQ96582392 = -770817711;    double GrXtyMIXkQ74554520 = -881087270;    double GrXtyMIXkQ17773747 = -799352714;    double GrXtyMIXkQ8016442 = -128143419;    double GrXtyMIXkQ61897468 = -185978527;    double GrXtyMIXkQ11307125 = -662584193;    double GrXtyMIXkQ67663125 = -31454999;    double GrXtyMIXkQ28867438 = -147105104;    double GrXtyMIXkQ87378061 = 16715520;     GrXtyMIXkQ83898653 = GrXtyMIXkQ84203193;     GrXtyMIXkQ84203193 = GrXtyMIXkQ53360921;     GrXtyMIXkQ53360921 = GrXtyMIXkQ33144548;     GrXtyMIXkQ33144548 = GrXtyMIXkQ94331948;     GrXtyMIXkQ94331948 = GrXtyMIXkQ75700138;     GrXtyMIXkQ75700138 = GrXtyMIXkQ18559436;     GrXtyMIXkQ18559436 = GrXtyMIXkQ29855324;     GrXtyMIXkQ29855324 = GrXtyMIXkQ76818822;     GrXtyMIXkQ76818822 = GrXtyMIXkQ30346103;     GrXtyMIXkQ30346103 = GrXtyMIXkQ47454071;     GrXtyMIXkQ47454071 = GrXtyMIXkQ89663899;     GrXtyMIXkQ89663899 = GrXtyMIXkQ38879773;     GrXtyMIXkQ38879773 = GrXtyMIXkQ88047648;     GrXtyMIXkQ88047648 = GrXtyMIXkQ28174489;     GrXtyMIXkQ28174489 = GrXtyMIXkQ84255175;     GrXtyMIXkQ84255175 = GrXtyMIXkQ57697514;     GrXtyMIXkQ57697514 = GrXtyMIXkQ52967475;     GrXtyMIXkQ52967475 = GrXtyMIXkQ34614438;     GrXtyMIXkQ34614438 = GrXtyMIXkQ7084173;     GrXtyMIXkQ7084173 = GrXtyMIXkQ12918284;     GrXtyMIXkQ12918284 = GrXtyMIXkQ52384414;     GrXtyMIXkQ52384414 = GrXtyMIXkQ73633780;     GrXtyMIXkQ73633780 = GrXtyMIXkQ17588058;     GrXtyMIXkQ17588058 = GrXtyMIXkQ17462939;     GrXtyMIXkQ17462939 = GrXtyMIXkQ99357190;     GrXtyMIXkQ99357190 = GrXtyMIXkQ2648145;     GrXtyMIXkQ2648145 = GrXtyMIXkQ32558157;     GrXtyMIXkQ32558157 = GrXtyMIXkQ31672708;     GrXtyMIXkQ31672708 = GrXtyMIXkQ93860568;     GrXtyMIXkQ93860568 = GrXtyMIXkQ93354034;     GrXtyMIXkQ93354034 = GrXtyMIXkQ72133984;     GrXtyMIXkQ72133984 = GrXtyMIXkQ19028958;     GrXtyMIXkQ19028958 = GrXtyMIXkQ31227051;     GrXtyMIXkQ31227051 = GrXtyMIXkQ97322517;     GrXtyMIXkQ97322517 = GrXtyMIXkQ37092238;     GrXtyMIXkQ37092238 = GrXtyMIXkQ32355256;     GrXtyMIXkQ32355256 = GrXtyMIXkQ90236129;     GrXtyMIXkQ90236129 = GrXtyMIXkQ12748504;     GrXtyMIXkQ12748504 = GrXtyMIXkQ81068436;     GrXtyMIXkQ81068436 = GrXtyMIXkQ47898747;     GrXtyMIXkQ47898747 = GrXtyMIXkQ9489742;     GrXtyMIXkQ9489742 = GrXtyMIXkQ50621380;     GrXtyMIXkQ50621380 = GrXtyMIXkQ65859506;     GrXtyMIXkQ65859506 = GrXtyMIXkQ62698149;     GrXtyMIXkQ62698149 = GrXtyMIXkQ63063626;     GrXtyMIXkQ63063626 = GrXtyMIXkQ73955273;     GrXtyMIXkQ73955273 = GrXtyMIXkQ29393502;     GrXtyMIXkQ29393502 = GrXtyMIXkQ22823945;     GrXtyMIXkQ22823945 = GrXtyMIXkQ57516470;     GrXtyMIXkQ57516470 = GrXtyMIXkQ14143505;     GrXtyMIXkQ14143505 = GrXtyMIXkQ74078416;     GrXtyMIXkQ74078416 = GrXtyMIXkQ64693794;     GrXtyMIXkQ64693794 = GrXtyMIXkQ48879407;     GrXtyMIXkQ48879407 = GrXtyMIXkQ50889884;     GrXtyMIXkQ50889884 = GrXtyMIXkQ31514239;     GrXtyMIXkQ31514239 = GrXtyMIXkQ10569414;     GrXtyMIXkQ10569414 = GrXtyMIXkQ35772864;     GrXtyMIXkQ35772864 = GrXtyMIXkQ15681609;     GrXtyMIXkQ15681609 = GrXtyMIXkQ94974757;     GrXtyMIXkQ94974757 = GrXtyMIXkQ73051994;     GrXtyMIXkQ73051994 = GrXtyMIXkQ86001278;     GrXtyMIXkQ86001278 = GrXtyMIXkQ98182616;     GrXtyMIXkQ98182616 = GrXtyMIXkQ82958254;     GrXtyMIXkQ82958254 = GrXtyMIXkQ36992069;     GrXtyMIXkQ36992069 = GrXtyMIXkQ75320087;     GrXtyMIXkQ75320087 = GrXtyMIXkQ70634942;     GrXtyMIXkQ70634942 = GrXtyMIXkQ7652723;     GrXtyMIXkQ7652723 = GrXtyMIXkQ90725131;     GrXtyMIXkQ90725131 = GrXtyMIXkQ91082250;     GrXtyMIXkQ91082250 = GrXtyMIXkQ51899919;     GrXtyMIXkQ51899919 = GrXtyMIXkQ67461385;     GrXtyMIXkQ67461385 = GrXtyMIXkQ40218971;     GrXtyMIXkQ40218971 = GrXtyMIXkQ53546002;     GrXtyMIXkQ53546002 = GrXtyMIXkQ59185426;     GrXtyMIXkQ59185426 = GrXtyMIXkQ3428543;     GrXtyMIXkQ3428543 = GrXtyMIXkQ1763035;     GrXtyMIXkQ1763035 = GrXtyMIXkQ7774274;     GrXtyMIXkQ7774274 = GrXtyMIXkQ54889908;     GrXtyMIXkQ54889908 = GrXtyMIXkQ54399313;     GrXtyMIXkQ54399313 = GrXtyMIXkQ25401918;     GrXtyMIXkQ25401918 = GrXtyMIXkQ73254642;     GrXtyMIXkQ73254642 = GrXtyMIXkQ9734212;     GrXtyMIXkQ9734212 = GrXtyMIXkQ74156237;     GrXtyMIXkQ74156237 = GrXtyMIXkQ79717064;     GrXtyMIXkQ79717064 = GrXtyMIXkQ19275618;     GrXtyMIXkQ19275618 = GrXtyMIXkQ7440190;     GrXtyMIXkQ7440190 = GrXtyMIXkQ70149550;     GrXtyMIXkQ70149550 = GrXtyMIXkQ80337166;     GrXtyMIXkQ80337166 = GrXtyMIXkQ65808278;     GrXtyMIXkQ65808278 = GrXtyMIXkQ26522825;     GrXtyMIXkQ26522825 = GrXtyMIXkQ96582392;     GrXtyMIXkQ96582392 = GrXtyMIXkQ74554520;     GrXtyMIXkQ74554520 = GrXtyMIXkQ17773747;     GrXtyMIXkQ17773747 = GrXtyMIXkQ8016442;     GrXtyMIXkQ8016442 = GrXtyMIXkQ61897468;     GrXtyMIXkQ61897468 = GrXtyMIXkQ11307125;     GrXtyMIXkQ11307125 = GrXtyMIXkQ67663125;     GrXtyMIXkQ67663125 = GrXtyMIXkQ28867438;     GrXtyMIXkQ28867438 = GrXtyMIXkQ87378061;     GrXtyMIXkQ87378061 = GrXtyMIXkQ83898653;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void xIpCctkxbT39820288() {     double pPglhkdHQd70820330 = -123935685;    double pPglhkdHQd93850931 = -381258517;    double pPglhkdHQd46056935 = -5763306;    double pPglhkdHQd61007742 = -656638652;    double pPglhkdHQd45061035 = -792031419;    double pPglhkdHQd89165815 = -884189979;    double pPglhkdHQd68605104 = -309344141;    double pPglhkdHQd12571673 = -307914102;    double pPglhkdHQd55204513 = -222876908;    double pPglhkdHQd3846490 = -192786445;    double pPglhkdHQd67134801 = -826825500;    double pPglhkdHQd59290377 = 8643655;    double pPglhkdHQd99506222 = -198650781;    double pPglhkdHQd31365758 = -122393150;    double pPglhkdHQd51887358 = -421464313;    double pPglhkdHQd19534335 = -980880711;    double pPglhkdHQd7537690 = -830982098;    double pPglhkdHQd82184232 = -411096070;    double pPglhkdHQd95774008 = -300867882;    double pPglhkdHQd63837001 = -143216881;    double pPglhkdHQd80383952 = -298000116;    double pPglhkdHQd91074373 = -822782590;    double pPglhkdHQd43280120 = -596935133;    double pPglhkdHQd93945007 = -660003392;    double pPglhkdHQd73427924 = -314114098;    double pPglhkdHQd93536090 = 44969451;    double pPglhkdHQd59538572 = -230126559;    double pPglhkdHQd33144707 = -980751797;    double pPglhkdHQd4575501 = -915407473;    double pPglhkdHQd73574207 = -591052532;    double pPglhkdHQd52474287 = 96906691;    double pPglhkdHQd76576013 = -795822703;    double pPglhkdHQd73302040 = -715716543;    double pPglhkdHQd61150148 = -393484450;    double pPglhkdHQd90101466 = -690293174;    double pPglhkdHQd70052453 = -530502781;    double pPglhkdHQd88324519 = -587456306;    double pPglhkdHQd62564506 = -22314883;    double pPglhkdHQd65979670 = -608914624;    double pPglhkdHQd8855511 = -917008780;    double pPglhkdHQd31060178 = -882344750;    double pPglhkdHQd94330666 = -297939116;    double pPglhkdHQd27071554 = 91531205;    double pPglhkdHQd84404186 = -688361615;    double pPglhkdHQd14683009 = 40736100;    double pPglhkdHQd65630547 = -694345601;    double pPglhkdHQd94605299 = -303010707;    double pPglhkdHQd31649446 = 47224722;    double pPglhkdHQd13049574 = -319533707;    double pPglhkdHQd50116050 = -147608348;    double pPglhkdHQd55501367 = -117420318;    double pPglhkdHQd78991458 = -639224486;    double pPglhkdHQd49835397 = -99278393;    double pPglhkdHQd3424377 = -393431304;    double pPglhkdHQd59436336 = -336633451;    double pPglhkdHQd79745957 = -301153095;    double pPglhkdHQd50570812 = -784323384;    double pPglhkdHQd52111928 = -345759914;    double pPglhkdHQd87579817 = -242524554;    double pPglhkdHQd51524945 = -737000870;    double pPglhkdHQd29627244 = -554063421;    double pPglhkdHQd35460397 = -328592345;    double pPglhkdHQd7996172 = -392506630;    double pPglhkdHQd81630305 = -631824377;    double pPglhkdHQd51372203 = -189693136;    double pPglhkdHQd90558787 = 68997202;    double pPglhkdHQd85988337 = -275639803;    double pPglhkdHQd38356074 = -805166332;    double pPglhkdHQd41264292 = -432099977;    double pPglhkdHQd81834905 = -890961532;    double pPglhkdHQd31209815 = -293424405;    double pPglhkdHQd44973183 = -708667216;    double pPglhkdHQd16204563 = -802181447;    double pPglhkdHQd86918497 = -383859102;    double pPglhkdHQd32776823 = -260872131;    double pPglhkdHQd86053285 = 99938999;    double pPglhkdHQd64002820 = -814313795;    double pPglhkdHQd58875934 = -908573519;    double pPglhkdHQd79261998 = -600739493;    double pPglhkdHQd7797377 = -619768498;    double pPglhkdHQd98930791 = -652019842;    double pPglhkdHQd27889126 = -177351281;    double pPglhkdHQd20095134 = -561218091;    double pPglhkdHQd54459451 = -667799126;    double pPglhkdHQd18072840 = -373632215;    double pPglhkdHQd73482828 = -263868823;    double pPglhkdHQd26740617 = -596544310;    double pPglhkdHQd69877663 = -222285239;    double pPglhkdHQd1713813 = 43149001;    double pPglhkdHQd10355510 = -289140079;    double pPglhkdHQd19481642 = -746179398;    double pPglhkdHQd36212592 = -141696393;    double pPglhkdHQd74984688 = -779790329;    double pPglhkdHQd14454726 = -871913754;    double pPglhkdHQd79228267 = -262945359;    double pPglhkdHQd95599780 = -453752406;    double pPglhkdHQd86334495 = -905432486;    double pPglhkdHQd45441248 = -276644419;    double pPglhkdHQd33031983 = -398668479;    double pPglhkdHQd24124221 = -123935685;     pPglhkdHQd70820330 = pPglhkdHQd93850931;     pPglhkdHQd93850931 = pPglhkdHQd46056935;     pPglhkdHQd46056935 = pPglhkdHQd61007742;     pPglhkdHQd61007742 = pPglhkdHQd45061035;     pPglhkdHQd45061035 = pPglhkdHQd89165815;     pPglhkdHQd89165815 = pPglhkdHQd68605104;     pPglhkdHQd68605104 = pPglhkdHQd12571673;     pPglhkdHQd12571673 = pPglhkdHQd55204513;     pPglhkdHQd55204513 = pPglhkdHQd3846490;     pPglhkdHQd3846490 = pPglhkdHQd67134801;     pPglhkdHQd67134801 = pPglhkdHQd59290377;     pPglhkdHQd59290377 = pPglhkdHQd99506222;     pPglhkdHQd99506222 = pPglhkdHQd31365758;     pPglhkdHQd31365758 = pPglhkdHQd51887358;     pPglhkdHQd51887358 = pPglhkdHQd19534335;     pPglhkdHQd19534335 = pPglhkdHQd7537690;     pPglhkdHQd7537690 = pPglhkdHQd82184232;     pPglhkdHQd82184232 = pPglhkdHQd95774008;     pPglhkdHQd95774008 = pPglhkdHQd63837001;     pPglhkdHQd63837001 = pPglhkdHQd80383952;     pPglhkdHQd80383952 = pPglhkdHQd91074373;     pPglhkdHQd91074373 = pPglhkdHQd43280120;     pPglhkdHQd43280120 = pPglhkdHQd93945007;     pPglhkdHQd93945007 = pPglhkdHQd73427924;     pPglhkdHQd73427924 = pPglhkdHQd93536090;     pPglhkdHQd93536090 = pPglhkdHQd59538572;     pPglhkdHQd59538572 = pPglhkdHQd33144707;     pPglhkdHQd33144707 = pPglhkdHQd4575501;     pPglhkdHQd4575501 = pPglhkdHQd73574207;     pPglhkdHQd73574207 = pPglhkdHQd52474287;     pPglhkdHQd52474287 = pPglhkdHQd76576013;     pPglhkdHQd76576013 = pPglhkdHQd73302040;     pPglhkdHQd73302040 = pPglhkdHQd61150148;     pPglhkdHQd61150148 = pPglhkdHQd90101466;     pPglhkdHQd90101466 = pPglhkdHQd70052453;     pPglhkdHQd70052453 = pPglhkdHQd88324519;     pPglhkdHQd88324519 = pPglhkdHQd62564506;     pPglhkdHQd62564506 = pPglhkdHQd65979670;     pPglhkdHQd65979670 = pPglhkdHQd8855511;     pPglhkdHQd8855511 = pPglhkdHQd31060178;     pPglhkdHQd31060178 = pPglhkdHQd94330666;     pPglhkdHQd94330666 = pPglhkdHQd27071554;     pPglhkdHQd27071554 = pPglhkdHQd84404186;     pPglhkdHQd84404186 = pPglhkdHQd14683009;     pPglhkdHQd14683009 = pPglhkdHQd65630547;     pPglhkdHQd65630547 = pPglhkdHQd94605299;     pPglhkdHQd94605299 = pPglhkdHQd31649446;     pPglhkdHQd31649446 = pPglhkdHQd13049574;     pPglhkdHQd13049574 = pPglhkdHQd50116050;     pPglhkdHQd50116050 = pPglhkdHQd55501367;     pPglhkdHQd55501367 = pPglhkdHQd78991458;     pPglhkdHQd78991458 = pPglhkdHQd49835397;     pPglhkdHQd49835397 = pPglhkdHQd3424377;     pPglhkdHQd3424377 = pPglhkdHQd59436336;     pPglhkdHQd59436336 = pPglhkdHQd79745957;     pPglhkdHQd79745957 = pPglhkdHQd50570812;     pPglhkdHQd50570812 = pPglhkdHQd52111928;     pPglhkdHQd52111928 = pPglhkdHQd87579817;     pPglhkdHQd87579817 = pPglhkdHQd51524945;     pPglhkdHQd51524945 = pPglhkdHQd29627244;     pPglhkdHQd29627244 = pPglhkdHQd35460397;     pPglhkdHQd35460397 = pPglhkdHQd7996172;     pPglhkdHQd7996172 = pPglhkdHQd81630305;     pPglhkdHQd81630305 = pPglhkdHQd51372203;     pPglhkdHQd51372203 = pPglhkdHQd90558787;     pPglhkdHQd90558787 = pPglhkdHQd85988337;     pPglhkdHQd85988337 = pPglhkdHQd38356074;     pPglhkdHQd38356074 = pPglhkdHQd41264292;     pPglhkdHQd41264292 = pPglhkdHQd81834905;     pPglhkdHQd81834905 = pPglhkdHQd31209815;     pPglhkdHQd31209815 = pPglhkdHQd44973183;     pPglhkdHQd44973183 = pPglhkdHQd16204563;     pPglhkdHQd16204563 = pPglhkdHQd86918497;     pPglhkdHQd86918497 = pPglhkdHQd32776823;     pPglhkdHQd32776823 = pPglhkdHQd86053285;     pPglhkdHQd86053285 = pPglhkdHQd64002820;     pPglhkdHQd64002820 = pPglhkdHQd58875934;     pPglhkdHQd58875934 = pPglhkdHQd79261998;     pPglhkdHQd79261998 = pPglhkdHQd7797377;     pPglhkdHQd7797377 = pPglhkdHQd98930791;     pPglhkdHQd98930791 = pPglhkdHQd27889126;     pPglhkdHQd27889126 = pPglhkdHQd20095134;     pPglhkdHQd20095134 = pPglhkdHQd54459451;     pPglhkdHQd54459451 = pPglhkdHQd18072840;     pPglhkdHQd18072840 = pPglhkdHQd73482828;     pPglhkdHQd73482828 = pPglhkdHQd26740617;     pPglhkdHQd26740617 = pPglhkdHQd69877663;     pPglhkdHQd69877663 = pPglhkdHQd1713813;     pPglhkdHQd1713813 = pPglhkdHQd10355510;     pPglhkdHQd10355510 = pPglhkdHQd19481642;     pPglhkdHQd19481642 = pPglhkdHQd36212592;     pPglhkdHQd36212592 = pPglhkdHQd74984688;     pPglhkdHQd74984688 = pPglhkdHQd14454726;     pPglhkdHQd14454726 = pPglhkdHQd79228267;     pPglhkdHQd79228267 = pPglhkdHQd95599780;     pPglhkdHQd95599780 = pPglhkdHQd86334495;     pPglhkdHQd86334495 = pPglhkdHQd45441248;     pPglhkdHQd45441248 = pPglhkdHQd33031983;     pPglhkdHQd33031983 = pPglhkdHQd24124221;     pPglhkdHQd24124221 = pPglhkdHQd70820330;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QFwpRcHVsr85515980() {     double OZnTCrodTv90009288 = 32289490;    double OZnTCrodTv66148789 = -22866849;    double OZnTCrodTv70716317 = -682609458;    double OZnTCrodTv45299189 = -662130822;    double OZnTCrodTv72067539 = -827223134;    double OZnTCrodTv1922773 = -450425732;    double OZnTCrodTv47595737 = -471047547;    double OZnTCrodTv48829265 = -868425512;    double OZnTCrodTv24201482 = -595044942;    double OZnTCrodTv68215276 = -443707587;    double OZnTCrodTv27884967 = -509646415;    double OZnTCrodTv35778620 = -49138025;    double OZnTCrodTv56941805 = -571837055;    double OZnTCrodTv25035546 = -103286371;    double OZnTCrodTv69089024 = -887873781;    double OZnTCrodTv325117 = -208024319;    double OZnTCrodTv7386277 = -597221944;    double OZnTCrodTv73021160 = -798304090;    double OZnTCrodTv16872549 = -888098275;    double OZnTCrodTv59708101 = -674046412;    double OZnTCrodTv81140901 = -919274952;    double OZnTCrodTv17201704 = -312582287;    double OZnTCrodTv46102968 = -905004212;    double OZnTCrodTv45230540 = -829140127;    double OZnTCrodTv89605278 = 10263795;    double OZnTCrodTv40652943 = -178368455;    double OZnTCrodTv71329504 = -179486410;    double OZnTCrodTv81068806 = -327998472;    double OZnTCrodTv10483410 = -267627861;    double OZnTCrodTv38566075 = -522734703;    double OZnTCrodTv19009263 = -287078582;    double OZnTCrodTv54468463 = -726169578;    double OZnTCrodTv14192330 = -226582396;    double OZnTCrodTv26340452 = -985593178;    double OZnTCrodTv41155207 = -116370107;    double OZnTCrodTv48646341 = -643698818;    double OZnTCrodTv25558559 = -672840265;    double OZnTCrodTv94244021 = 91148999;    double OZnTCrodTv21672354 = -952668365;    double OZnTCrodTv40443266 = -719486519;    double OZnTCrodTv41423638 = -213639892;    double OZnTCrodTv27337859 = -404717994;    double OZnTCrodTv88971718 = -754923581;    double OZnTCrodTv12499146 = -214673314;    double OZnTCrodTv58668666 = -738529738;    double OZnTCrodTv15430790 = -536256254;    double OZnTCrodTv19431640 = -327362352;    double OZnTCrodTv12734024 = -372662413;    double OZnTCrodTv51158063 = -119290877;    double OZnTCrodTv90473546 = -583625712;    double OZnTCrodTv63103552 = -578914269;    double OZnTCrodTv99435393 = -222783871;    double OZnTCrodTv88390599 = 24797696;    double OZnTCrodTv55098558 = -629729141;    double OZnTCrodTv4375080 = -142951265;    double OZnTCrodTv72807584 = -655128224;    double OZnTCrodTv20045821 = -117862638;    double OZnTCrodTv25485778 = -853469331;    double OZnTCrodTv55693910 = -572394618;    double OZnTCrodTv31414596 = -548854680;    double OZnTCrodTv30593269 = -170939323;    double OZnTCrodTv66526930 = -43049075;    double OZnTCrodTv38345856 = -500797652;    double OZnTCrodTv85635407 = 27689761;    double OZnTCrodTv49206014 = -56629005;    double OZnTCrodTv73416504 = -783476838;    double OZnTCrodTv21586290 = -822555630;    double OZnTCrodTv30601354 = -586243877;    double OZnTCrodTv83880338 = -986916264;    double OZnTCrodTv20442684 = -144174964;    double OZnTCrodTv74766558 = -535184055;    double OZnTCrodTv13142256 = -588370943;    double OZnTCrodTv51348807 = -845635726;    double OZnTCrodTv76429282 = -68611756;    double OZnTCrodTv18284463 = -360406521;    double OZnTCrodTv53803042 = -414556959;    double OZnTCrodTv28229986 = -557658706;    double OZnTCrodTv33603823 = -590330898;    double OZnTCrodTv86561873 = 9389610;    double OZnTCrodTv74174489 = -453479951;    double OZnTCrodTv21221303 = -851006103;    double OZnTCrodTv58595480 = -806823997;    double OZnTCrodTv29910744 = -108707596;    double OZnTCrodTv20009864 = -684002149;    double OZnTCrodTv75462522 = -943820435;    double OZnTCrodTv19573870 = 35705288;    double OZnTCrodTv66077863 = -650967274;    double OZnTCrodTv59093771 = -596853256;    double OZnTCrodTv21965373 = -742641914;    double OZnTCrodTv68347622 = -461241884;    double OZnTCrodTv28600520 = -425836181;    double OZnTCrodTv72782 = -819370934;    double OZnTCrodTv38550111 = -336456384;    double OZnTCrodTv90257758 = -303813686;    double OZnTCrodTv9849998 = -448547197;    double OZnTCrodTv74896708 = -70590817;    double OZnTCrodTv88992003 = -903920343;    double OZnTCrodTv3336312 = -682613343;    double OZnTCrodTv63293132 = -58044309;    double OZnTCrodTv85252162 = 32289490;     OZnTCrodTv90009288 = OZnTCrodTv66148789;     OZnTCrodTv66148789 = OZnTCrodTv70716317;     OZnTCrodTv70716317 = OZnTCrodTv45299189;     OZnTCrodTv45299189 = OZnTCrodTv72067539;     OZnTCrodTv72067539 = OZnTCrodTv1922773;     OZnTCrodTv1922773 = OZnTCrodTv47595737;     OZnTCrodTv47595737 = OZnTCrodTv48829265;     OZnTCrodTv48829265 = OZnTCrodTv24201482;     OZnTCrodTv24201482 = OZnTCrodTv68215276;     OZnTCrodTv68215276 = OZnTCrodTv27884967;     OZnTCrodTv27884967 = OZnTCrodTv35778620;     OZnTCrodTv35778620 = OZnTCrodTv56941805;     OZnTCrodTv56941805 = OZnTCrodTv25035546;     OZnTCrodTv25035546 = OZnTCrodTv69089024;     OZnTCrodTv69089024 = OZnTCrodTv325117;     OZnTCrodTv325117 = OZnTCrodTv7386277;     OZnTCrodTv7386277 = OZnTCrodTv73021160;     OZnTCrodTv73021160 = OZnTCrodTv16872549;     OZnTCrodTv16872549 = OZnTCrodTv59708101;     OZnTCrodTv59708101 = OZnTCrodTv81140901;     OZnTCrodTv81140901 = OZnTCrodTv17201704;     OZnTCrodTv17201704 = OZnTCrodTv46102968;     OZnTCrodTv46102968 = OZnTCrodTv45230540;     OZnTCrodTv45230540 = OZnTCrodTv89605278;     OZnTCrodTv89605278 = OZnTCrodTv40652943;     OZnTCrodTv40652943 = OZnTCrodTv71329504;     OZnTCrodTv71329504 = OZnTCrodTv81068806;     OZnTCrodTv81068806 = OZnTCrodTv10483410;     OZnTCrodTv10483410 = OZnTCrodTv38566075;     OZnTCrodTv38566075 = OZnTCrodTv19009263;     OZnTCrodTv19009263 = OZnTCrodTv54468463;     OZnTCrodTv54468463 = OZnTCrodTv14192330;     OZnTCrodTv14192330 = OZnTCrodTv26340452;     OZnTCrodTv26340452 = OZnTCrodTv41155207;     OZnTCrodTv41155207 = OZnTCrodTv48646341;     OZnTCrodTv48646341 = OZnTCrodTv25558559;     OZnTCrodTv25558559 = OZnTCrodTv94244021;     OZnTCrodTv94244021 = OZnTCrodTv21672354;     OZnTCrodTv21672354 = OZnTCrodTv40443266;     OZnTCrodTv40443266 = OZnTCrodTv41423638;     OZnTCrodTv41423638 = OZnTCrodTv27337859;     OZnTCrodTv27337859 = OZnTCrodTv88971718;     OZnTCrodTv88971718 = OZnTCrodTv12499146;     OZnTCrodTv12499146 = OZnTCrodTv58668666;     OZnTCrodTv58668666 = OZnTCrodTv15430790;     OZnTCrodTv15430790 = OZnTCrodTv19431640;     OZnTCrodTv19431640 = OZnTCrodTv12734024;     OZnTCrodTv12734024 = OZnTCrodTv51158063;     OZnTCrodTv51158063 = OZnTCrodTv90473546;     OZnTCrodTv90473546 = OZnTCrodTv63103552;     OZnTCrodTv63103552 = OZnTCrodTv99435393;     OZnTCrodTv99435393 = OZnTCrodTv88390599;     OZnTCrodTv88390599 = OZnTCrodTv55098558;     OZnTCrodTv55098558 = OZnTCrodTv4375080;     OZnTCrodTv4375080 = OZnTCrodTv72807584;     OZnTCrodTv72807584 = OZnTCrodTv20045821;     OZnTCrodTv20045821 = OZnTCrodTv25485778;     OZnTCrodTv25485778 = OZnTCrodTv55693910;     OZnTCrodTv55693910 = OZnTCrodTv31414596;     OZnTCrodTv31414596 = OZnTCrodTv30593269;     OZnTCrodTv30593269 = OZnTCrodTv66526930;     OZnTCrodTv66526930 = OZnTCrodTv38345856;     OZnTCrodTv38345856 = OZnTCrodTv85635407;     OZnTCrodTv85635407 = OZnTCrodTv49206014;     OZnTCrodTv49206014 = OZnTCrodTv73416504;     OZnTCrodTv73416504 = OZnTCrodTv21586290;     OZnTCrodTv21586290 = OZnTCrodTv30601354;     OZnTCrodTv30601354 = OZnTCrodTv83880338;     OZnTCrodTv83880338 = OZnTCrodTv20442684;     OZnTCrodTv20442684 = OZnTCrodTv74766558;     OZnTCrodTv74766558 = OZnTCrodTv13142256;     OZnTCrodTv13142256 = OZnTCrodTv51348807;     OZnTCrodTv51348807 = OZnTCrodTv76429282;     OZnTCrodTv76429282 = OZnTCrodTv18284463;     OZnTCrodTv18284463 = OZnTCrodTv53803042;     OZnTCrodTv53803042 = OZnTCrodTv28229986;     OZnTCrodTv28229986 = OZnTCrodTv33603823;     OZnTCrodTv33603823 = OZnTCrodTv86561873;     OZnTCrodTv86561873 = OZnTCrodTv74174489;     OZnTCrodTv74174489 = OZnTCrodTv21221303;     OZnTCrodTv21221303 = OZnTCrodTv58595480;     OZnTCrodTv58595480 = OZnTCrodTv29910744;     OZnTCrodTv29910744 = OZnTCrodTv20009864;     OZnTCrodTv20009864 = OZnTCrodTv75462522;     OZnTCrodTv75462522 = OZnTCrodTv19573870;     OZnTCrodTv19573870 = OZnTCrodTv66077863;     OZnTCrodTv66077863 = OZnTCrodTv59093771;     OZnTCrodTv59093771 = OZnTCrodTv21965373;     OZnTCrodTv21965373 = OZnTCrodTv68347622;     OZnTCrodTv68347622 = OZnTCrodTv28600520;     OZnTCrodTv28600520 = OZnTCrodTv72782;     OZnTCrodTv72782 = OZnTCrodTv38550111;     OZnTCrodTv38550111 = OZnTCrodTv90257758;     OZnTCrodTv90257758 = OZnTCrodTv9849998;     OZnTCrodTv9849998 = OZnTCrodTv74896708;     OZnTCrodTv74896708 = OZnTCrodTv88992003;     OZnTCrodTv88992003 = OZnTCrodTv3336312;     OZnTCrodTv3336312 = OZnTCrodTv63293132;     OZnTCrodTv63293132 = OZnTCrodTv85252162;     OZnTCrodTv85252162 = OZnTCrodTv90009288;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LemNEmjYgl70248620() {     double cyJGEtDdNu28393675 = -882736020;    double cyJGEtDdNu72172187 = -363488389;    double cyJGEtDdNu18721869 = -979173148;    double cyJGEtDdNu28802223 = -852574332;    double cyJGEtDdNu17240444 = -429251243;    double cyJGEtDdNu92631255 = -983464425;    double cyJGEtDdNu37029668 = 10082110;    double cyJGEtDdNu52130401 = -85271013;    double cyJGEtDdNu79455257 = -525734939;    double cyJGEtDdNu313337 = -208743001;    double cyJGEtDdNu29064693 = 84183827;    double cyJGEtDdNu74463426 = -429223023;    double cyJGEtDdNu39960058 = -196551129;    double cyJGEtDdNu58530901 = -819919760;    double cyJGEtDdNu58320247 = -757979866;    double cyJGEtDdNu87342133 = -529515884;    double cyJGEtDdNu51230691 = -310087065;    double cyJGEtDdNu31694485 = -75648510;    double cyJGEtDdNu38641361 = -344576794;    double cyJGEtDdNu79376537 = -23681024;    double cyJGEtDdNu2985375 = -251358250;    double cyJGEtDdNu91160098 = -982189798;    double cyJGEtDdNu14870269 = 37815472;    double cyJGEtDdNu4151495 = -186493074;    double cyJGEtDdNu93517298 = -14286565;    double cyJGEtDdNu18312571 = -716838339;    double cyJGEtDdNu24689786 = -877406399;    double cyJGEtDdNu41513482 = -613524832;    double cyJGEtDdNu9620782 = -525998321;    double cyJGEtDdNu92459941 = -710667672;    double cyJGEtDdNu18666536 = -667955894;    double cyJGEtDdNu25529505 = -195041567;    double cyJGEtDdNu10520484 = -146369885;    double cyJGEtDdNu56438221 = -93128903;    double cyJGEtDdNu60904651 = -909210050;    double cyJGEtDdNu63005927 = -790325146;    double cyJGEtDdNu88085269 = -743954803;    double cyJGEtDdNu87685371 = -786660560;    double cyJGEtDdNu97744125 = -927494432;    double cyJGEtDdNu31745884 = -715010312;    double cyJGEtDdNu74294892 = -988286278;    double cyJGEtDdNu7654584 = -424483668;    double cyJGEtDdNu52488687 = -788702899;    double cyJGEtDdNu9457166 = -810332269;    double cyJGEtDdNu62621928 = -723802012;    double cyJGEtDdNu74603304 = -408032889;    double cyJGEtDdNu41386529 = -994785021;    double cyJGEtDdNu99519599 = -603675327;    double cyJGEtDdNu8716654 = -155041355;    double cyJGEtDdNu51143115 = -768635556;    double cyJGEtDdNu65048554 = -823347165;    double cyJGEtDdNu25776006 = -107756892;    double cyJGEtDdNu90052657 = -934996119;    double cyJGEtDdNu48196651 = -570404961;    double cyJGEtDdNu67263246 = 1312387;    double cyJGEtDdNu37233577 = -900546222;    double cyJGEtDdNu57301919 = -301303862;    double cyJGEtDdNu14570374 = -692680075;    double cyJGEtDdNu35284925 = -738287767;    double cyJGEtDdNu98927872 = -712412905;    double cyJGEtDdNu67941469 = -6058027;    double cyJGEtDdNu95516185 = -376393058;    double cyJGEtDdNu42509620 = -559272692;    double cyJGEtDdNu86995315 = -815067267;    double cyJGEtDdNu81646800 = -540787108;    double cyJGEtDdNu3535189 = -720774606;    double cyJGEtDdNu63942942 = -182853138;    double cyJGEtDdNu83521836 = -3422226;    double cyJGEtDdNu97626249 = -910709710;    double cyJGEtDdNu95314320 = -967654721;    double cyJGEtDdNu99256864 = -785561081;    double cyJGEtDdNu63545320 = -523426506;    double cyJGEtDdNu33950359 = -148154078;    double cyJGEtDdNu6895477 = -629566483;    double cyJGEtDdNu5081646 = -35394746;    double cyJGEtDdNu95330791 = -826874583;    double cyJGEtDdNu38671411 = -93486899;    double cyJGEtDdNu5413103 = -151852259;    double cyJGEtDdNu41529567 = -462691062;    double cyJGEtDdNu18913994 = -606253676;    double cyJGEtDdNu76926042 = -722053318;    double cyJGEtDdNu25170187 = -173731072;    double cyJGEtDdNu32796828 = -358483477;    double cyJGEtDdNu58477666 = -757362766;    double cyJGEtDdNu27411388 = -887320507;    double cyJGEtDdNu92890530 = -460199003;    double cyJGEtDdNu35476847 = -260045448;    double cyJGEtDdNu62323833 = -575964924;    double cyJGEtDdNu89174975 = 5558709;    double cyJGEtDdNu23671074 = 91336172;    double cyJGEtDdNu5704009 = -389021285;    double cyJGEtDdNu73514895 = 48725271;    double cyJGEtDdNu52400447 = 51627207;    double cyJGEtDdNu98816252 = -115081528;    double cyJGEtDdNu63804415 = -608952285;    double cyJGEtDdNu78778706 = -511893221;    double cyJGEtDdNu65144963 = -865210976;    double cyJGEtDdNu65493371 = -973635633;    double cyJGEtDdNu27810365 = -169545162;    double cyJGEtDdNu59086740 = -882736020;     cyJGEtDdNu28393675 = cyJGEtDdNu72172187;     cyJGEtDdNu72172187 = cyJGEtDdNu18721869;     cyJGEtDdNu18721869 = cyJGEtDdNu28802223;     cyJGEtDdNu28802223 = cyJGEtDdNu17240444;     cyJGEtDdNu17240444 = cyJGEtDdNu92631255;     cyJGEtDdNu92631255 = cyJGEtDdNu37029668;     cyJGEtDdNu37029668 = cyJGEtDdNu52130401;     cyJGEtDdNu52130401 = cyJGEtDdNu79455257;     cyJGEtDdNu79455257 = cyJGEtDdNu313337;     cyJGEtDdNu313337 = cyJGEtDdNu29064693;     cyJGEtDdNu29064693 = cyJGEtDdNu74463426;     cyJGEtDdNu74463426 = cyJGEtDdNu39960058;     cyJGEtDdNu39960058 = cyJGEtDdNu58530901;     cyJGEtDdNu58530901 = cyJGEtDdNu58320247;     cyJGEtDdNu58320247 = cyJGEtDdNu87342133;     cyJGEtDdNu87342133 = cyJGEtDdNu51230691;     cyJGEtDdNu51230691 = cyJGEtDdNu31694485;     cyJGEtDdNu31694485 = cyJGEtDdNu38641361;     cyJGEtDdNu38641361 = cyJGEtDdNu79376537;     cyJGEtDdNu79376537 = cyJGEtDdNu2985375;     cyJGEtDdNu2985375 = cyJGEtDdNu91160098;     cyJGEtDdNu91160098 = cyJGEtDdNu14870269;     cyJGEtDdNu14870269 = cyJGEtDdNu4151495;     cyJGEtDdNu4151495 = cyJGEtDdNu93517298;     cyJGEtDdNu93517298 = cyJGEtDdNu18312571;     cyJGEtDdNu18312571 = cyJGEtDdNu24689786;     cyJGEtDdNu24689786 = cyJGEtDdNu41513482;     cyJGEtDdNu41513482 = cyJGEtDdNu9620782;     cyJGEtDdNu9620782 = cyJGEtDdNu92459941;     cyJGEtDdNu92459941 = cyJGEtDdNu18666536;     cyJGEtDdNu18666536 = cyJGEtDdNu25529505;     cyJGEtDdNu25529505 = cyJGEtDdNu10520484;     cyJGEtDdNu10520484 = cyJGEtDdNu56438221;     cyJGEtDdNu56438221 = cyJGEtDdNu60904651;     cyJGEtDdNu60904651 = cyJGEtDdNu63005927;     cyJGEtDdNu63005927 = cyJGEtDdNu88085269;     cyJGEtDdNu88085269 = cyJGEtDdNu87685371;     cyJGEtDdNu87685371 = cyJGEtDdNu97744125;     cyJGEtDdNu97744125 = cyJGEtDdNu31745884;     cyJGEtDdNu31745884 = cyJGEtDdNu74294892;     cyJGEtDdNu74294892 = cyJGEtDdNu7654584;     cyJGEtDdNu7654584 = cyJGEtDdNu52488687;     cyJGEtDdNu52488687 = cyJGEtDdNu9457166;     cyJGEtDdNu9457166 = cyJGEtDdNu62621928;     cyJGEtDdNu62621928 = cyJGEtDdNu74603304;     cyJGEtDdNu74603304 = cyJGEtDdNu41386529;     cyJGEtDdNu41386529 = cyJGEtDdNu99519599;     cyJGEtDdNu99519599 = cyJGEtDdNu8716654;     cyJGEtDdNu8716654 = cyJGEtDdNu51143115;     cyJGEtDdNu51143115 = cyJGEtDdNu65048554;     cyJGEtDdNu65048554 = cyJGEtDdNu25776006;     cyJGEtDdNu25776006 = cyJGEtDdNu90052657;     cyJGEtDdNu90052657 = cyJGEtDdNu48196651;     cyJGEtDdNu48196651 = cyJGEtDdNu67263246;     cyJGEtDdNu67263246 = cyJGEtDdNu37233577;     cyJGEtDdNu37233577 = cyJGEtDdNu57301919;     cyJGEtDdNu57301919 = cyJGEtDdNu14570374;     cyJGEtDdNu14570374 = cyJGEtDdNu35284925;     cyJGEtDdNu35284925 = cyJGEtDdNu98927872;     cyJGEtDdNu98927872 = cyJGEtDdNu67941469;     cyJGEtDdNu67941469 = cyJGEtDdNu95516185;     cyJGEtDdNu95516185 = cyJGEtDdNu42509620;     cyJGEtDdNu42509620 = cyJGEtDdNu86995315;     cyJGEtDdNu86995315 = cyJGEtDdNu81646800;     cyJGEtDdNu81646800 = cyJGEtDdNu3535189;     cyJGEtDdNu3535189 = cyJGEtDdNu63942942;     cyJGEtDdNu63942942 = cyJGEtDdNu83521836;     cyJGEtDdNu83521836 = cyJGEtDdNu97626249;     cyJGEtDdNu97626249 = cyJGEtDdNu95314320;     cyJGEtDdNu95314320 = cyJGEtDdNu99256864;     cyJGEtDdNu99256864 = cyJGEtDdNu63545320;     cyJGEtDdNu63545320 = cyJGEtDdNu33950359;     cyJGEtDdNu33950359 = cyJGEtDdNu6895477;     cyJGEtDdNu6895477 = cyJGEtDdNu5081646;     cyJGEtDdNu5081646 = cyJGEtDdNu95330791;     cyJGEtDdNu95330791 = cyJGEtDdNu38671411;     cyJGEtDdNu38671411 = cyJGEtDdNu5413103;     cyJGEtDdNu5413103 = cyJGEtDdNu41529567;     cyJGEtDdNu41529567 = cyJGEtDdNu18913994;     cyJGEtDdNu18913994 = cyJGEtDdNu76926042;     cyJGEtDdNu76926042 = cyJGEtDdNu25170187;     cyJGEtDdNu25170187 = cyJGEtDdNu32796828;     cyJGEtDdNu32796828 = cyJGEtDdNu58477666;     cyJGEtDdNu58477666 = cyJGEtDdNu27411388;     cyJGEtDdNu27411388 = cyJGEtDdNu92890530;     cyJGEtDdNu92890530 = cyJGEtDdNu35476847;     cyJGEtDdNu35476847 = cyJGEtDdNu62323833;     cyJGEtDdNu62323833 = cyJGEtDdNu89174975;     cyJGEtDdNu89174975 = cyJGEtDdNu23671074;     cyJGEtDdNu23671074 = cyJGEtDdNu5704009;     cyJGEtDdNu5704009 = cyJGEtDdNu73514895;     cyJGEtDdNu73514895 = cyJGEtDdNu52400447;     cyJGEtDdNu52400447 = cyJGEtDdNu98816252;     cyJGEtDdNu98816252 = cyJGEtDdNu63804415;     cyJGEtDdNu63804415 = cyJGEtDdNu78778706;     cyJGEtDdNu78778706 = cyJGEtDdNu65144963;     cyJGEtDdNu65144963 = cyJGEtDdNu65493371;     cyJGEtDdNu65493371 = cyJGEtDdNu27810365;     cyJGEtDdNu27810365 = cyJGEtDdNu59086740;     cyJGEtDdNu59086740 = cyJGEtDdNu28393675;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RmKGddXXWy76907365() {     double BtNeINBZIZ28387204 = -755260161;    double BtNeINBZIZ10744504 = -406083514;    double BtNeINBZIZ20035081 = -936301762;    double BtNeINBZIZ13882084 = -673115162;    double BtNeINBZIZ26080546 = -897606565;    double BtNeINBZIZ27436688 = -682897237;    double BtNeINBZIZ5577004 = -794454357;    double BtNeINBZIZ21344452 = -889448330;    double BtNeINBZIZ62195421 = -239381009;    double BtNeINBZIZ96952849 = -945549872;    double BtNeINBZIZ49385298 = -975288245;    double BtNeINBZIZ88755103 = -164701385;    double BtNeINBZIZ71812972 = -218209603;    double BtNeINBZIZ12375121 = -65072811;    double BtNeINBZIZ3492357 = -720692718;    double BtNeINBZIZ61906681 = -862311535;    double BtNeINBZIZ7083451 = -129701635;    double BtNeINBZIZ54695017 = -472720132;    double BtNeINBZIZ59069630 = -962559061;    double BtNeINBZIZ51450301 = -635705476;    double BtNeINBZIZ82654799 = 38175376;    double BtNeINBZIZ69456364 = -392181681;    double BtNeINBZIZ51748665 = -421142370;    double BtNeINBZIZ47801603 = -67413597;    double BtNeINBZIZ21959988 = -440980418;    double BtNeINBZIZ34886648 = -625044267;    double BtNeINBZIZ94911367 = -78206111;    double BtNeINBZIZ76917007 = -122491821;    double BtNeINBZIZ22299229 = -72068636;    double BtNeINBZIZ68549811 = -386099046;    double BtNeINBZIZ52079214 = 44950871;    double BtNeINBZIZ10253362 = -586863329;    double BtNeINBZIZ95972907 = -348314101;    double BtNeINBZIZ56721059 = 30189364;    double BtNeINBZIZ43262689 = -68523975;    double BtNeINBZIZ5834117 = -870090892;    double BtNeINBZIZ26637 = -843608182;    double BtNeINBZIZ57603051 = -781923239;    double BtNeINBZIZ33057722 = -540175848;    double BtNeINBZIZ3618778 = -324441998;    double BtNeINBZIZ62150560 = 23769823;    double BtNeINBZIZ93352244 = -618275750;    double BtNeINBZIZ12772048 = -247833153;    double BtNeINBZIZ68689064 = -367296712;    double BtNeINBZIZ46639980 = -97061414;    double BtNeINBZIZ15031273 = -220077560;    double BtNeINBZIZ69084321 = -376065643;    double BtNeINBZIZ74903180 = -112436683;    double BtNeINBZIZ27375044 = -818805217;    double BtNeINBZIZ71188538 = -355660442;    double BtNeINBZIZ78307923 = -401902173;    double BtNeINBZIZ40323263 = -489902640;    double BtNeINBZIZ65501005 = -827050128;    double BtNeINBZIZ58446922 = -2324813;    double BtNeINBZIZ94252566 = -855586891;    double BtNeINBZIZ58930840 = -263078481;    double BtNeINBZIZ58995839 = -984941145;    double BtNeINBZIZ72233477 = -768888166;    double BtNeINBZIZ91922095 = -132134744;    double BtNeINBZIZ91193898 = -172562298;    double BtNeINBZIZ32525320 = -504691126;    double BtNeINBZIZ28659997 = -571962537;    double BtNeINBZIZ99045223 = -717379695;    double BtNeINBZIZ93645609 = -853281963;    double BtNeINBZIZ44873636 = -890500744;    double BtNeINBZIZ39131936 = -288424917;    double BtNeINBZIZ92782196 = -816387285;    double BtNeINBZIZ15091914 = -148398968;    double BtNeINBZIZ69112431 = -996548837;    double BtNeINBZIZ97658239 = -850601826;    double BtNeINBZIZ61880044 = 81296646;    double BtNeINBZIZ49480400 = -347778397;    double BtNeINBZIZ21637295 = -932544284;    double BtNeINBZIZ55450853 = -538117063;    double BtNeINBZIZ89299740 = -559475299;    double BtNeINBZIZ89302554 = -343548875;    double BtNeINBZIZ56684317 = -44348528;    double BtNeINBZIZ83059601 = 46154342;    double BtNeINBZIZ1161623 = -970352183;    double BtNeINBZIZ6928715 = -120902858;    double BtNeINBZIZ65802327 = -148978625;    double BtNeINBZIZ20008187 = -965769429;    double BtNeINBZIZ49541964 = -303686604;    double BtNeINBZIZ51110690 = -716408195;    double BtNeINBZIZ90241887 = -984196874;    double BtNeINBZIZ11755952 = -465146489;    double BtNeINBZIZ44752356 = -759813202;    double BtNeINBZIZ37525985 = -245989289;    double BtNeINBZIZ62468492 = -114223745;    double BtNeINBZIZ84331849 = -805445494;    double BtNeINBZIZ46838278 = -885149748;    double BtNeINBZIZ27793160 = 25279984;    double BtNeINBZIZ65680955 = -549788495;    double BtNeINBZIZ41863824 = -267613551;    double BtNeINBZIZ71093457 = -819750873;    double BtNeINBZIZ33490563 = -404267640;    double BtNeINBZIZ94307020 = -900896056;    double BtNeINBZIZ19126438 = -394551191;    double BtNeINBZIZ23815429 = -476795969;    double BtNeINBZIZ7508045 = -755260161;     BtNeINBZIZ28387204 = BtNeINBZIZ10744504;     BtNeINBZIZ10744504 = BtNeINBZIZ20035081;     BtNeINBZIZ20035081 = BtNeINBZIZ13882084;     BtNeINBZIZ13882084 = BtNeINBZIZ26080546;     BtNeINBZIZ26080546 = BtNeINBZIZ27436688;     BtNeINBZIZ27436688 = BtNeINBZIZ5577004;     BtNeINBZIZ5577004 = BtNeINBZIZ21344452;     BtNeINBZIZ21344452 = BtNeINBZIZ62195421;     BtNeINBZIZ62195421 = BtNeINBZIZ96952849;     BtNeINBZIZ96952849 = BtNeINBZIZ49385298;     BtNeINBZIZ49385298 = BtNeINBZIZ88755103;     BtNeINBZIZ88755103 = BtNeINBZIZ71812972;     BtNeINBZIZ71812972 = BtNeINBZIZ12375121;     BtNeINBZIZ12375121 = BtNeINBZIZ3492357;     BtNeINBZIZ3492357 = BtNeINBZIZ61906681;     BtNeINBZIZ61906681 = BtNeINBZIZ7083451;     BtNeINBZIZ7083451 = BtNeINBZIZ54695017;     BtNeINBZIZ54695017 = BtNeINBZIZ59069630;     BtNeINBZIZ59069630 = BtNeINBZIZ51450301;     BtNeINBZIZ51450301 = BtNeINBZIZ82654799;     BtNeINBZIZ82654799 = BtNeINBZIZ69456364;     BtNeINBZIZ69456364 = BtNeINBZIZ51748665;     BtNeINBZIZ51748665 = BtNeINBZIZ47801603;     BtNeINBZIZ47801603 = BtNeINBZIZ21959988;     BtNeINBZIZ21959988 = BtNeINBZIZ34886648;     BtNeINBZIZ34886648 = BtNeINBZIZ94911367;     BtNeINBZIZ94911367 = BtNeINBZIZ76917007;     BtNeINBZIZ76917007 = BtNeINBZIZ22299229;     BtNeINBZIZ22299229 = BtNeINBZIZ68549811;     BtNeINBZIZ68549811 = BtNeINBZIZ52079214;     BtNeINBZIZ52079214 = BtNeINBZIZ10253362;     BtNeINBZIZ10253362 = BtNeINBZIZ95972907;     BtNeINBZIZ95972907 = BtNeINBZIZ56721059;     BtNeINBZIZ56721059 = BtNeINBZIZ43262689;     BtNeINBZIZ43262689 = BtNeINBZIZ5834117;     BtNeINBZIZ5834117 = BtNeINBZIZ26637;     BtNeINBZIZ26637 = BtNeINBZIZ57603051;     BtNeINBZIZ57603051 = BtNeINBZIZ33057722;     BtNeINBZIZ33057722 = BtNeINBZIZ3618778;     BtNeINBZIZ3618778 = BtNeINBZIZ62150560;     BtNeINBZIZ62150560 = BtNeINBZIZ93352244;     BtNeINBZIZ93352244 = BtNeINBZIZ12772048;     BtNeINBZIZ12772048 = BtNeINBZIZ68689064;     BtNeINBZIZ68689064 = BtNeINBZIZ46639980;     BtNeINBZIZ46639980 = BtNeINBZIZ15031273;     BtNeINBZIZ15031273 = BtNeINBZIZ69084321;     BtNeINBZIZ69084321 = BtNeINBZIZ74903180;     BtNeINBZIZ74903180 = BtNeINBZIZ27375044;     BtNeINBZIZ27375044 = BtNeINBZIZ71188538;     BtNeINBZIZ71188538 = BtNeINBZIZ78307923;     BtNeINBZIZ78307923 = BtNeINBZIZ40323263;     BtNeINBZIZ40323263 = BtNeINBZIZ65501005;     BtNeINBZIZ65501005 = BtNeINBZIZ58446922;     BtNeINBZIZ58446922 = BtNeINBZIZ94252566;     BtNeINBZIZ94252566 = BtNeINBZIZ58930840;     BtNeINBZIZ58930840 = BtNeINBZIZ58995839;     BtNeINBZIZ58995839 = BtNeINBZIZ72233477;     BtNeINBZIZ72233477 = BtNeINBZIZ91922095;     BtNeINBZIZ91922095 = BtNeINBZIZ91193898;     BtNeINBZIZ91193898 = BtNeINBZIZ32525320;     BtNeINBZIZ32525320 = BtNeINBZIZ28659997;     BtNeINBZIZ28659997 = BtNeINBZIZ99045223;     BtNeINBZIZ99045223 = BtNeINBZIZ93645609;     BtNeINBZIZ93645609 = BtNeINBZIZ44873636;     BtNeINBZIZ44873636 = BtNeINBZIZ39131936;     BtNeINBZIZ39131936 = BtNeINBZIZ92782196;     BtNeINBZIZ92782196 = BtNeINBZIZ15091914;     BtNeINBZIZ15091914 = BtNeINBZIZ69112431;     BtNeINBZIZ69112431 = BtNeINBZIZ97658239;     BtNeINBZIZ97658239 = BtNeINBZIZ61880044;     BtNeINBZIZ61880044 = BtNeINBZIZ49480400;     BtNeINBZIZ49480400 = BtNeINBZIZ21637295;     BtNeINBZIZ21637295 = BtNeINBZIZ55450853;     BtNeINBZIZ55450853 = BtNeINBZIZ89299740;     BtNeINBZIZ89299740 = BtNeINBZIZ89302554;     BtNeINBZIZ89302554 = BtNeINBZIZ56684317;     BtNeINBZIZ56684317 = BtNeINBZIZ83059601;     BtNeINBZIZ83059601 = BtNeINBZIZ1161623;     BtNeINBZIZ1161623 = BtNeINBZIZ6928715;     BtNeINBZIZ6928715 = BtNeINBZIZ65802327;     BtNeINBZIZ65802327 = BtNeINBZIZ20008187;     BtNeINBZIZ20008187 = BtNeINBZIZ49541964;     BtNeINBZIZ49541964 = BtNeINBZIZ51110690;     BtNeINBZIZ51110690 = BtNeINBZIZ90241887;     BtNeINBZIZ90241887 = BtNeINBZIZ11755952;     BtNeINBZIZ11755952 = BtNeINBZIZ44752356;     BtNeINBZIZ44752356 = BtNeINBZIZ37525985;     BtNeINBZIZ37525985 = BtNeINBZIZ62468492;     BtNeINBZIZ62468492 = BtNeINBZIZ84331849;     BtNeINBZIZ84331849 = BtNeINBZIZ46838278;     BtNeINBZIZ46838278 = BtNeINBZIZ27793160;     BtNeINBZIZ27793160 = BtNeINBZIZ65680955;     BtNeINBZIZ65680955 = BtNeINBZIZ41863824;     BtNeINBZIZ41863824 = BtNeINBZIZ71093457;     BtNeINBZIZ71093457 = BtNeINBZIZ33490563;     BtNeINBZIZ33490563 = BtNeINBZIZ94307020;     BtNeINBZIZ94307020 = BtNeINBZIZ19126438;     BtNeINBZIZ19126438 = BtNeINBZIZ23815429;     BtNeINBZIZ23815429 = BtNeINBZIZ7508045;     BtNeINBZIZ7508045 = BtNeINBZIZ28387204;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RWuwzHOveg22603058() {     double rnjnfwJube47576162 = -599034987;    double rnjnfwJube83042361 = -47691847;    double rnjnfwJube44694462 = -513147914;    double rnjnfwJube98173530 = -678607332;    double rnjnfwJube53087049 = -932798280;    double rnjnfwJube40193645 = -249132989;    double rnjnfwJube84567637 = -956157762;    double rnjnfwJube57602045 = -349959740;    double rnjnfwJube31192391 = -611549042;    double rnjnfwJube61321637 = -96471015;    double rnjnfwJube10135464 = -658109160;    double rnjnfwJube65243345 = -222483065;    double rnjnfwJube29248556 = -591395877;    double rnjnfwJube6044909 = -45966031;    double rnjnfwJube20694023 = -87102186;    double rnjnfwJube42697463 = -89455142;    double rnjnfwJube6932038 = -995941481;    double rnjnfwJube45531946 = -859928152;    double rnjnfwJube80168171 = -449789454;    double rnjnfwJube47321401 = -66535007;    double rnjnfwJube83411748 = -583099460;    double rnjnfwJube95583694 = -981981378;    double rnjnfwJube54571514 = -729211449;    double rnjnfwJube99087135 = -236550331;    double rnjnfwJube38137342 = -116602525;    double rnjnfwJube82003500 = -848382173;    double rnjnfwJube6702300 = -27565962;    double rnjnfwJube24841107 = -569738495;    double rnjnfwJube28207138 = -524289024;    double rnjnfwJube33541680 = -317781217;    double rnjnfwJube18614190 = -339034402;    double rnjnfwJube88145811 = -517210204;    double rnjnfwJube36863196 = -959179954;    double rnjnfwJube21911362 = -561919364;    double rnjnfwJube94316430 = -594600908;    double rnjnfwJube84428004 = -983286929;    double rnjnfwJube37260676 = -928992140;    double rnjnfwJube89282566 = -668459358;    double rnjnfwJube88750406 = -883929590;    double rnjnfwJube35206533 = -126919737;    double rnjnfwJube72514020 = -407525319;    double rnjnfwJube26359437 = -725054628;    double rnjnfwJube74672211 = 5712061;    double rnjnfwJube96784023 = -993608411;    double rnjnfwJube90625637 = -876327252;    double rnjnfwJube64831514 = -61988213;    double rnjnfwJube93910661 = -400417288;    double rnjnfwJube55987759 = -532323818;    double rnjnfwJube65483533 = -618562387;    double rnjnfwJube11546035 = -791677807;    double rnjnfwJube85910109 = -863396124;    double rnjnfwJube60767197 = -73462025;    double rnjnfwJube4056209 = -702974039;    double rnjnfwJube10121104 = -238622649;    double rnjnfwJube39191310 = -661904704;    double rnjnfwJube51992468 = -617053609;    double rnjnfwJube28470848 = -318480399;    double rnjnfwJube45607327 = -176597583;    double rnjnfwJube60036189 = -462004807;    double rnjnfwJube71083549 = 15583893;    double rnjnfwJube33491346 = -121567027;    double rnjnfwJube59726530 = -286419268;    double rnjnfwJube29394907 = -825670716;    double rnjnfwJube97650711 = -193767825;    double rnjnfwJube42707447 = -757436614;    double rnjnfwJube21989652 = -40898956;    double rnjnfwJube28380149 = -263303112;    double rnjnfwJube7337194 = 70523487;    double rnjnfwJube11728478 = -451365124;    double rnjnfwJube36266018 = -103815258;    double rnjnfwJube5436788 = -160463003;    double rnjnfwJube17649472 = -227482124;    double rnjnfwJube56781539 = -975998563;    double rnjnfwJube44961639 = -222869717;    double rnjnfwJube74807380 = -659009689;    double rnjnfwJube57052311 = -858044832;    double rnjnfwJube20911483 = -887693439;    double rnjnfwJube57787490 = -735603038;    double rnjnfwJube8461498 = -360223080;    double rnjnfwJube73305828 = 45385688;    double rnjnfwJube88092838 = -347964885;    double rnjnfwJube50714541 = -495242145;    double rnjnfwJube59357574 = -951176109;    double rnjnfwJube16661103 = -732611218;    double rnjnfwJube47631570 = -454385094;    double rnjnfwJube57846993 = -165572378;    double rnjnfwJube84089603 = -814236166;    double rnjnfwJube26742093 = -620557306;    double rnjnfwJube82720052 = -900014661;    double rnjnfwJube42323963 = -977547300;    double rnjnfwJube55957157 = -564806531;    double rnjnfwJube91653348 = -652394557;    double rnjnfwJube29246378 = -106454551;    double rnjnfwJube17666857 = -799513483;    double rnjnfwJube1715187 = 94647290;    double rnjnfwJube12787491 = -21106052;    double rnjnfwJube96964529 = -899383912;    double rnjnfwJube77021500 = -800520115;    double rnjnfwJube54076577 = -136171798;    double rnjnfwJube68635985 = -599034987;     rnjnfwJube47576162 = rnjnfwJube83042361;     rnjnfwJube83042361 = rnjnfwJube44694462;     rnjnfwJube44694462 = rnjnfwJube98173530;     rnjnfwJube98173530 = rnjnfwJube53087049;     rnjnfwJube53087049 = rnjnfwJube40193645;     rnjnfwJube40193645 = rnjnfwJube84567637;     rnjnfwJube84567637 = rnjnfwJube57602045;     rnjnfwJube57602045 = rnjnfwJube31192391;     rnjnfwJube31192391 = rnjnfwJube61321637;     rnjnfwJube61321637 = rnjnfwJube10135464;     rnjnfwJube10135464 = rnjnfwJube65243345;     rnjnfwJube65243345 = rnjnfwJube29248556;     rnjnfwJube29248556 = rnjnfwJube6044909;     rnjnfwJube6044909 = rnjnfwJube20694023;     rnjnfwJube20694023 = rnjnfwJube42697463;     rnjnfwJube42697463 = rnjnfwJube6932038;     rnjnfwJube6932038 = rnjnfwJube45531946;     rnjnfwJube45531946 = rnjnfwJube80168171;     rnjnfwJube80168171 = rnjnfwJube47321401;     rnjnfwJube47321401 = rnjnfwJube83411748;     rnjnfwJube83411748 = rnjnfwJube95583694;     rnjnfwJube95583694 = rnjnfwJube54571514;     rnjnfwJube54571514 = rnjnfwJube99087135;     rnjnfwJube99087135 = rnjnfwJube38137342;     rnjnfwJube38137342 = rnjnfwJube82003500;     rnjnfwJube82003500 = rnjnfwJube6702300;     rnjnfwJube6702300 = rnjnfwJube24841107;     rnjnfwJube24841107 = rnjnfwJube28207138;     rnjnfwJube28207138 = rnjnfwJube33541680;     rnjnfwJube33541680 = rnjnfwJube18614190;     rnjnfwJube18614190 = rnjnfwJube88145811;     rnjnfwJube88145811 = rnjnfwJube36863196;     rnjnfwJube36863196 = rnjnfwJube21911362;     rnjnfwJube21911362 = rnjnfwJube94316430;     rnjnfwJube94316430 = rnjnfwJube84428004;     rnjnfwJube84428004 = rnjnfwJube37260676;     rnjnfwJube37260676 = rnjnfwJube89282566;     rnjnfwJube89282566 = rnjnfwJube88750406;     rnjnfwJube88750406 = rnjnfwJube35206533;     rnjnfwJube35206533 = rnjnfwJube72514020;     rnjnfwJube72514020 = rnjnfwJube26359437;     rnjnfwJube26359437 = rnjnfwJube74672211;     rnjnfwJube74672211 = rnjnfwJube96784023;     rnjnfwJube96784023 = rnjnfwJube90625637;     rnjnfwJube90625637 = rnjnfwJube64831514;     rnjnfwJube64831514 = rnjnfwJube93910661;     rnjnfwJube93910661 = rnjnfwJube55987759;     rnjnfwJube55987759 = rnjnfwJube65483533;     rnjnfwJube65483533 = rnjnfwJube11546035;     rnjnfwJube11546035 = rnjnfwJube85910109;     rnjnfwJube85910109 = rnjnfwJube60767197;     rnjnfwJube60767197 = rnjnfwJube4056209;     rnjnfwJube4056209 = rnjnfwJube10121104;     rnjnfwJube10121104 = rnjnfwJube39191310;     rnjnfwJube39191310 = rnjnfwJube51992468;     rnjnfwJube51992468 = rnjnfwJube28470848;     rnjnfwJube28470848 = rnjnfwJube45607327;     rnjnfwJube45607327 = rnjnfwJube60036189;     rnjnfwJube60036189 = rnjnfwJube71083549;     rnjnfwJube71083549 = rnjnfwJube33491346;     rnjnfwJube33491346 = rnjnfwJube59726530;     rnjnfwJube59726530 = rnjnfwJube29394907;     rnjnfwJube29394907 = rnjnfwJube97650711;     rnjnfwJube97650711 = rnjnfwJube42707447;     rnjnfwJube42707447 = rnjnfwJube21989652;     rnjnfwJube21989652 = rnjnfwJube28380149;     rnjnfwJube28380149 = rnjnfwJube7337194;     rnjnfwJube7337194 = rnjnfwJube11728478;     rnjnfwJube11728478 = rnjnfwJube36266018;     rnjnfwJube36266018 = rnjnfwJube5436788;     rnjnfwJube5436788 = rnjnfwJube17649472;     rnjnfwJube17649472 = rnjnfwJube56781539;     rnjnfwJube56781539 = rnjnfwJube44961639;     rnjnfwJube44961639 = rnjnfwJube74807380;     rnjnfwJube74807380 = rnjnfwJube57052311;     rnjnfwJube57052311 = rnjnfwJube20911483;     rnjnfwJube20911483 = rnjnfwJube57787490;     rnjnfwJube57787490 = rnjnfwJube8461498;     rnjnfwJube8461498 = rnjnfwJube73305828;     rnjnfwJube73305828 = rnjnfwJube88092838;     rnjnfwJube88092838 = rnjnfwJube50714541;     rnjnfwJube50714541 = rnjnfwJube59357574;     rnjnfwJube59357574 = rnjnfwJube16661103;     rnjnfwJube16661103 = rnjnfwJube47631570;     rnjnfwJube47631570 = rnjnfwJube57846993;     rnjnfwJube57846993 = rnjnfwJube84089603;     rnjnfwJube84089603 = rnjnfwJube26742093;     rnjnfwJube26742093 = rnjnfwJube82720052;     rnjnfwJube82720052 = rnjnfwJube42323963;     rnjnfwJube42323963 = rnjnfwJube55957157;     rnjnfwJube55957157 = rnjnfwJube91653348;     rnjnfwJube91653348 = rnjnfwJube29246378;     rnjnfwJube29246378 = rnjnfwJube17666857;     rnjnfwJube17666857 = rnjnfwJube1715187;     rnjnfwJube1715187 = rnjnfwJube12787491;     rnjnfwJube12787491 = rnjnfwJube96964529;     rnjnfwJube96964529 = rnjnfwJube77021500;     rnjnfwJube77021500 = rnjnfwJube54076577;     rnjnfwJube54076577 = rnjnfwJube68635985;     rnjnfwJube68635985 = rnjnfwJube47576162;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RQuleCEeMh77019271() {     double sWgdLBofSn18227829 = -117184117;    double sWgdLBofSn51715879 = 80664854;    double sWgdLBofSn24663381 = -955442374;    double sWgdLBofSn38104817 = -868745721;    double sWgdLBofSn74537371 = -716204627;    double sWgdLBofSn30193408 = 49285859;    double sWgdLBofSn2946533 = -954933472;    double sWgdLBofSn14444426 = -696776830;    double sWgdLBofSn77057445 = -704896371;    double sWgdLBofSn84288097 = -397566365;    double sWgdLBofSn52384624 = -693011089;    double sWgdLBofSn10789917 = -538246858;    double sWgdLBofSn9075943 = -500932935;    double sWgdLBofSn89891942 = -946994241;    double sWgdLBofSn3414042 = -236852190;    double sWgdLBofSn75226103 = -759438729;    double sWgdLBofSn784865 = -905126611;    double sWgdLBofSn65825440 = -849094348;    double sWgdLBofSn61875953 = -56977396;    double sWgdLBofSn6108110 = -608901312;    double sWgdLBofSn38547502 = -491778600;    double sWgdLBofSn56979459 = -213266684;    double sWgdLBofSn56515322 = -197054593;    double sWgdLBofSn32936672 = -928951238;    double sWgdLBofSn2261731 = -892507213;    double sWgdLBofSn12601081 = -946666618;    double sWgdLBofSn14963085 = -422743738;    double sWgdLBofSn32623333 = 25137737;    double sWgdLBofSn60349625 = -818647241;    double sWgdLBofSn72713775 = -876176288;    double sWgdLBofSn25686187 = -87468087;    double sWgdLBofSn32657272 = -845507367;    double sWgdLBofSn19808557 = -906141563;    double sWgdLBofSn87276337 = -125449048;    double sWgdLBofSn72340666 = -441547687;    double sWgdLBofSn44421264 = 37486523;    double sWgdLBofSn81052161 = -17585347;    double sWgdLBofSn42075055 = -452572465;    double sWgdLBofSn67283695 = -839658226;    double sWgdLBofSn30309831 = -744528100;    double sWgdLBofSn32587305 = -791544198;    double sWgdLBofSn54842428 = -494443697;    double sWgdLBofSn23639172 = 18958008;    double sWgdLBofSn3292324 = 34416617;    double sWgdLBofSn86579696 = -84973646;    double sWgdLBofSn71237349 = -920325368;    double sWgdLBofSn20041865 = -516487088;    double sWgdLBofSn21601969 = -984454113;    double sWgdLBofSn70924985 = -604326356;    double sWgdLBofSn19973522 = 25313314;    double sWgdLBofSn54099434 = -409968245;    double sWgdLBofSn2638702 = -42681746;    double sWgdLBofSn59131865 = -630772081;    double sWgdLBofSn348408 = -899504145;    double sWgdLBofSn38471769 = -772845618;    double sWgdLBofSn61248370 = -903917434;    double sWgdLBofSn95200556 = -722280553;    double sWgdLBofSn91726709 = 73508863;    double sWgdLBofSn35843087 = -976238509;    double sWgdLBofSn61936290 = -769538010;    double sWgdLBofSn15230323 = -527970404;    double sWgdLBofSn70323200 = -880071210;    double sWgdLBofSn54094800 = -878129589;    double sWgdLBofSn4343670 = -828720083;    double sWgdLBofSn58601911 = -210098279;    double sWgdLBofSn19727353 = -847503723;    double sWgdLBofSn90981359 = -632105296;    double sWgdLBofSn21799606 = -275483887;    double sWgdLBofSn17551276 = -405446554;    double sWgdLBofSn58992778 = -174338713;    double sWgdLBofSn94173941 = -641853382;    double sWgdLBofSn58709810 = -352554146;    double sWgdLBofSn98541744 = 90563878;    double sWgdLBofSn31566123 = -312449296;    double sWgdLBofSn73520805 = -817357115;    double sWgdLBofSn83705074 = -997334903;    double sWgdLBofSn33340287 = -132224693;    double sWgdLBofSn53222998 = -131471211;    double sWgdLBofSn46356976 = -743977592;    double sWgdLBofSn31024382 = -972181845;    double sWgdLBofSn92559216 = -330179531;    double sWgdLBofSn93361116 = -438289625;    double sWgdLBofSn61698347 = -370535907;    double sWgdLBofSn40376104 = -743960556;    double sWgdLBofSn18614342 = -366208043;    double sWgdLBofSn23047485 = 55213659;    double sWgdLBofSn73525406 = -114735286;    double sWgdLBofSn19460149 = 93362582;    double sWgdLBofSn48804568 = -352603431;    double sWgdLBofSn11092297 = -537630254;    double sWgdLBofSn49220707 = -240232924;    double sWgdLBofSn89325452 = 8905789;    double sWgdLBofSn6231968 = -476333957;    double sWgdLBofSn5347405 = 29879783;    double sWgdLBofSn15079508 = -116557697;    double sWgdLBofSn62264104 = -911472988;    double sWgdLBofSn747628 = -616314109;    double sWgdLBofSn19295502 = -152321909;    double sWgdLBofSn44690413 = -755485105;    double sWgdLBofSn66852344 = -117184117;     sWgdLBofSn18227829 = sWgdLBofSn51715879;     sWgdLBofSn51715879 = sWgdLBofSn24663381;     sWgdLBofSn24663381 = sWgdLBofSn38104817;     sWgdLBofSn38104817 = sWgdLBofSn74537371;     sWgdLBofSn74537371 = sWgdLBofSn30193408;     sWgdLBofSn30193408 = sWgdLBofSn2946533;     sWgdLBofSn2946533 = sWgdLBofSn14444426;     sWgdLBofSn14444426 = sWgdLBofSn77057445;     sWgdLBofSn77057445 = sWgdLBofSn84288097;     sWgdLBofSn84288097 = sWgdLBofSn52384624;     sWgdLBofSn52384624 = sWgdLBofSn10789917;     sWgdLBofSn10789917 = sWgdLBofSn9075943;     sWgdLBofSn9075943 = sWgdLBofSn89891942;     sWgdLBofSn89891942 = sWgdLBofSn3414042;     sWgdLBofSn3414042 = sWgdLBofSn75226103;     sWgdLBofSn75226103 = sWgdLBofSn784865;     sWgdLBofSn784865 = sWgdLBofSn65825440;     sWgdLBofSn65825440 = sWgdLBofSn61875953;     sWgdLBofSn61875953 = sWgdLBofSn6108110;     sWgdLBofSn6108110 = sWgdLBofSn38547502;     sWgdLBofSn38547502 = sWgdLBofSn56979459;     sWgdLBofSn56979459 = sWgdLBofSn56515322;     sWgdLBofSn56515322 = sWgdLBofSn32936672;     sWgdLBofSn32936672 = sWgdLBofSn2261731;     sWgdLBofSn2261731 = sWgdLBofSn12601081;     sWgdLBofSn12601081 = sWgdLBofSn14963085;     sWgdLBofSn14963085 = sWgdLBofSn32623333;     sWgdLBofSn32623333 = sWgdLBofSn60349625;     sWgdLBofSn60349625 = sWgdLBofSn72713775;     sWgdLBofSn72713775 = sWgdLBofSn25686187;     sWgdLBofSn25686187 = sWgdLBofSn32657272;     sWgdLBofSn32657272 = sWgdLBofSn19808557;     sWgdLBofSn19808557 = sWgdLBofSn87276337;     sWgdLBofSn87276337 = sWgdLBofSn72340666;     sWgdLBofSn72340666 = sWgdLBofSn44421264;     sWgdLBofSn44421264 = sWgdLBofSn81052161;     sWgdLBofSn81052161 = sWgdLBofSn42075055;     sWgdLBofSn42075055 = sWgdLBofSn67283695;     sWgdLBofSn67283695 = sWgdLBofSn30309831;     sWgdLBofSn30309831 = sWgdLBofSn32587305;     sWgdLBofSn32587305 = sWgdLBofSn54842428;     sWgdLBofSn54842428 = sWgdLBofSn23639172;     sWgdLBofSn23639172 = sWgdLBofSn3292324;     sWgdLBofSn3292324 = sWgdLBofSn86579696;     sWgdLBofSn86579696 = sWgdLBofSn71237349;     sWgdLBofSn71237349 = sWgdLBofSn20041865;     sWgdLBofSn20041865 = sWgdLBofSn21601969;     sWgdLBofSn21601969 = sWgdLBofSn70924985;     sWgdLBofSn70924985 = sWgdLBofSn19973522;     sWgdLBofSn19973522 = sWgdLBofSn54099434;     sWgdLBofSn54099434 = sWgdLBofSn2638702;     sWgdLBofSn2638702 = sWgdLBofSn59131865;     sWgdLBofSn59131865 = sWgdLBofSn348408;     sWgdLBofSn348408 = sWgdLBofSn38471769;     sWgdLBofSn38471769 = sWgdLBofSn61248370;     sWgdLBofSn61248370 = sWgdLBofSn95200556;     sWgdLBofSn95200556 = sWgdLBofSn91726709;     sWgdLBofSn91726709 = sWgdLBofSn35843087;     sWgdLBofSn35843087 = sWgdLBofSn61936290;     sWgdLBofSn61936290 = sWgdLBofSn15230323;     sWgdLBofSn15230323 = sWgdLBofSn70323200;     sWgdLBofSn70323200 = sWgdLBofSn54094800;     sWgdLBofSn54094800 = sWgdLBofSn4343670;     sWgdLBofSn4343670 = sWgdLBofSn58601911;     sWgdLBofSn58601911 = sWgdLBofSn19727353;     sWgdLBofSn19727353 = sWgdLBofSn90981359;     sWgdLBofSn90981359 = sWgdLBofSn21799606;     sWgdLBofSn21799606 = sWgdLBofSn17551276;     sWgdLBofSn17551276 = sWgdLBofSn58992778;     sWgdLBofSn58992778 = sWgdLBofSn94173941;     sWgdLBofSn94173941 = sWgdLBofSn58709810;     sWgdLBofSn58709810 = sWgdLBofSn98541744;     sWgdLBofSn98541744 = sWgdLBofSn31566123;     sWgdLBofSn31566123 = sWgdLBofSn73520805;     sWgdLBofSn73520805 = sWgdLBofSn83705074;     sWgdLBofSn83705074 = sWgdLBofSn33340287;     sWgdLBofSn33340287 = sWgdLBofSn53222998;     sWgdLBofSn53222998 = sWgdLBofSn46356976;     sWgdLBofSn46356976 = sWgdLBofSn31024382;     sWgdLBofSn31024382 = sWgdLBofSn92559216;     sWgdLBofSn92559216 = sWgdLBofSn93361116;     sWgdLBofSn93361116 = sWgdLBofSn61698347;     sWgdLBofSn61698347 = sWgdLBofSn40376104;     sWgdLBofSn40376104 = sWgdLBofSn18614342;     sWgdLBofSn18614342 = sWgdLBofSn23047485;     sWgdLBofSn23047485 = sWgdLBofSn73525406;     sWgdLBofSn73525406 = sWgdLBofSn19460149;     sWgdLBofSn19460149 = sWgdLBofSn48804568;     sWgdLBofSn48804568 = sWgdLBofSn11092297;     sWgdLBofSn11092297 = sWgdLBofSn49220707;     sWgdLBofSn49220707 = sWgdLBofSn89325452;     sWgdLBofSn89325452 = sWgdLBofSn6231968;     sWgdLBofSn6231968 = sWgdLBofSn5347405;     sWgdLBofSn5347405 = sWgdLBofSn15079508;     sWgdLBofSn15079508 = sWgdLBofSn62264104;     sWgdLBofSn62264104 = sWgdLBofSn747628;     sWgdLBofSn747628 = sWgdLBofSn19295502;     sWgdLBofSn19295502 = sWgdLBofSn44690413;     sWgdLBofSn44690413 = sWgdLBofSn66852344;     sWgdLBofSn66852344 = sWgdLBofSn18227829;}
// Junk Finished
