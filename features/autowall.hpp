#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../singleton.hpp"

#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E' ///< the egg sacs in the tunnels in ep2.
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
//#define CHAR_TEX_UNUSED		'J'
#define CHAR_TEX_SNOW			'K'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
//#define CHAR_TEX_UNUSED		'Q'
#define CHAR_TEX_REFLECTIVE		'R'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
//#define CHAR_TEX_UNUSED		'X' ///< do not use - "fake" materials use this (ladders, wading, clips, etc)
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z' ///< wierd-looking jello effect for advisor shield.

#define CHAR_TEX_STEAM_PIPE		11


struct FireBulletData
{
	FireBulletData(const Vector& eyePos, C_BasePlayer* entity) : src(eyePos)
	{

	}

	Vector          src;
	trace_t        enter_trace;
	Vector          direction;
	CTraceFilter   filter;
	float           trace_length;
	float           trace_length_remaining;
	float           current_damage;
	int             penetrate_count;
	float max_range;
};

class Autowall : public Singleton<Autowall>
{
public:
	void TraceLine(Vector& start, Vector& end, unsigned int mask, C_BasePlayer* ignore, trace_t* ptr);
	bool VectortoVectorVisible(Vector src, Vector point, C_BasePlayer* pEntity);
	bool Breakable(C_BasePlayer* pEntity);
	void ScaleDamage(C_BasePlayer* pEntity, int, CCSWeaponInfo* weapon_data, float& current_damage);
	bool TraceToExit(trace_t& enter_trace, trace_t& exit_trace, Vector start_position, Vector dir);
	bool HandleBulletPen(CCSWeaponInfo* wpn_data, FireBulletData& data, bool extracheck, Vector point, C_BasePlayer* pEntity);
	bool FireBullet(C_BaseCombatWeapon* weapon, FireBulletData& data);
	float GetDamage(Vector point);
	bool CanHitFloatingPoint(const Vector& point, const Vector& source, C_BasePlayer* pEntity = nullptr);
};