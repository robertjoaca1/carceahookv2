#pragma once
#pragma once

#include <string>
#include "../singleton.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/input.hpp"
#include "../options.hpp"
#include "../ui.hpp"
#include "../hooks.hpp"
#include "../zgui/zgui.hh"
#include "visuals.hpp"

struct IDirect3DDevice9;

class Menu
	: public Singleton<Menu>
{
private:
	/*void RenderTabs(int& SelectedTab, const char* Tabs[], size_t ArraySize);
	void CreateStyle();

	ImGuiStyle	Style;
	bool		Visible;
	int			SelectedTab;
	int			RagebotTab;
	int			LegitbotTab;
	int			VisualsTab;
	int			VisualsTab2;
	int			SkinsTab;*/
public:
	bool IsVisible;
	void RenderMenu();
	void Initialize();
	/*void Initialize();
	void Shutdown();

	void OnDeviceLost();
	void OnDeviceReset();

	void RenderMenu();
	void RenderSpectatorList();
	void RenderLocalplayerFlags();
	void RenderBombTimer();

	void Show();
	void Hide();
	void Toggle();

	bool IsVisible() const { return Visible; }*/
};