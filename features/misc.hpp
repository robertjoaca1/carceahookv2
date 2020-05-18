#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../singleton.hpp"
#include "../helpers/input.hpp"
#include "../hooks.hpp"

class Misc : public Singleton<Misc>
{
	friend class Singleton<Misc>;
public:
	void DoBhop(CUserCmd* cmd);
	void DoAutostrafe(CUserCmd* cmd);
	void DoClantag();
	void DoFakeDuck(CUserCmd* cmd);
};