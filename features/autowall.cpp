#include "autowall.hpp"

void Autowall::TraceLine(Vector& start, Vector& end, unsigned int mask, C_BasePlayer* ignore, trace_t* ptr) {

	CTraceFilter filter;
	filter.pSkip = ignore;
	g_EngineTrace->TraceRay(Ray_t(start, end), mask, &filter, ptr);
}

bool Autowall::VectortoVectorVisible(Vector src, Vector point, C_BasePlayer* pEntity = nullptr)
{
	trace_t TraceInit;
	TraceInit.hit_entity = nullptr;
	CTraceFilter filter1;
	filter1.pSkip = g_LocalPlayer;
	g_EngineTrace->TraceRay(Ray_t(src, point), MASK_SOLID, &filter1, &TraceInit);

	if (TraceInit.fraction == 1.0f)
		return true;

	if (pEntity != nullptr && TraceInit.hit_entity == pEntity)
		return true;

	return false;
}

// idk monarch sent me the autowall the floating point is mine tho :)

// i will never be able to make my own autowall

bool Autowall::Breakable(C_BasePlayer* pEntity) {
	if (!pEntity)
		return false;

	const int take_damage = pEntity->get_take_damage();

	auto* cclass = g_CHLClient->GetAllClasses();
	if (!cclass) 
		return false;

	//				 '       ''     '      '   '
	//			    01234567890123456     012345678
	//  check against CBreakableSurface and CBaseDoor

	//  windows etc. are CBrekableSurface
	//  large garage door in office is CBaseDoor and it get's reported as a breakable when it is not one
	//  this is seperate from "CPropDoorRotating", which is a normal door.
	//  normally you would also check for "CFuncBrush" but it was acting oddly
	//  ((cclass->m_pNetworkName[1]) != 'F' || (cclass->m_pNetworkName[4]) != 'c' || (cclass->m_pNetworkName[5]) != 'B' || (cclass->m_pNetworkName[9]) != 'h')

	if ((cclass->m_pNetworkName[1] == 'B' && cclass->m_pNetworkName[9] == 'e' && cclass->m_pNetworkName[10] == 'S' && cclass->m_pNetworkName[16] == 'e')
		|| (cclass->m_pNetworkName[1] != 'B' || cclass->m_pNetworkName[5] != 'D'))
		pEntity->get_take_damage() = 2;

	using IsBreakableEntity_t = bool(__thiscall*)(C_BasePlayer*);
	static IsBreakableEntity_t  IsBreakableEntityEx = (IsBreakableEntity_t)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE");

	pEntity->get_take_damage() = take_damage;

	return IsBreakableEntityEx(pEntity);
}

void Autowall::ScaleDamage(C_BasePlayer* pEntity, int hitgroup, CCSWeaponInfo* weapon_data, float& current_damage) {
	bool   has_heavy_armor = pEntity->m_bHasHeavyArmor();
	int    armor = pEntity->m_ArmorValue();
	int    hit_group = hitgroup;

	auto armored = [&]() -> bool {
		C_BasePlayer* target = pEntity;

		switch (hitgroup) {
		case HITGROUP_HEAD:
			return target->m_bHasHelmet();
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	switch (hit_group) {
	case HITGROUP_HEAD:
		current_damage *= has_heavy_armor ? 2.f : 4.f;
		break;
	case HITGROUP_STOMACH:
		current_damage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		current_damage *= 0.75f;
		break;
	default:
		break;
	}

	if (armor > 0 && armored()) {
		float bonus_value = 1.f, armor_bonus_ratio = 0.5f, armor_ratio = weapon_data->flArmorRatio / 2.f;

		if (has_heavy_armor) {
			armor_bonus_ratio = 0.33f;
			armor_ratio *= 0.5f;
			bonus_value = 0.33f;
		}

		auto new_damage = current_damage * armor_ratio;

		if (has_heavy_armor) {
			new_damage *= 0.85f;
		}

		if (((current_damage - (current_damage * armor_ratio)) * (bonus_value * armor_bonus_ratio)) > armor) {
			new_damage = current_damage - (armor / armor_bonus_ratio);
		}

		current_damage = new_damage;
	}
}

bool Autowall::TraceToExit(trace_t& enter_trace, trace_t& exit_trace, Vector start_position, Vector dir) {
	Vector   start, end;
	float  max_distance = 90.f,
		ray_extension = 4.f,
		current_distance = 0;
	int    first_contents = 0;

	while (current_distance <= max_distance) {
		//  add extra distance to our ray
		current_distance += ray_extension;

		//  multiply the direction vector to the distance so we go outwards, add our position to it.
		start = start_position + dir * current_distance;

		if (!first_contents) {
			first_contents = g_EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr); /*0x4600400B*/
		}

		int point_contents = g_EngineTrace->GetPointContents(start, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr);

		if (!(point_contents & MASK_SHOT_HULL) || point_contents & CONTENTS_HITBOX && point_contents != first_contents) /*0x600400B, *0x40000000*/ {

			//  let's setup our end position by deducting the direction by the extra added distance
			end = start - (dir * ray_extension);

			//  let's cast a ray from our start pos to the end pos
			TraceLine(start, end, MASK_SHOT_HULL | CONTENTS_HITBOX, nullptr, &exit_trace);

			//  let's check if a hitbox is in-front of our enemy and if they are behind of a solid wall
			if (exit_trace.startsolid && exit_trace.surface.flags & SURF_HITBOX) {
				TraceLine(start, start_position, MASK_SHOT_HULL, (C_BasePlayer *)exit_trace.hit_entity, &exit_trace);

				if (exit_trace.DidHit() && !exit_trace.startsolid) {
					start = exit_trace.endpos;

					return true;
				}

				continue;
			}

			//  can we hit? is the wall solid?
			if (exit_trace.DidHit() && !exit_trace.startsolid) {

				// is the wall a breakable? if so, let's shoot through it.
				if (Breakable((C_BasePlayer*)exit_trace.hit_entity) && Breakable((C_BasePlayer*)exit_trace.hit_entity)) {
					return true;
				}

				if (enter_trace.surface.flags & SURF_NODRAW || !(exit_trace.surface.flags & SURF_NODRAW) && (exit_trace.plane.normal.Dot(dir) <= 1.f)) {
					float mult_amount = exit_trace.fraction * 4.f;

					start -= dir * mult_amount;

					return true;
				}

				continue;
			}

			if (!exit_trace.DidHit() || exit_trace.startsolid) {
				if ((enter_trace.hit_entity != nullptr && enter_trace.hit_entity->EntIndex() != 0) && Breakable((C_BasePlayer*)exit_trace.hit_entity)) {
					exit_trace = enter_trace;

					exit_trace.endpos = start + dir;

					return true;
				}

				continue;
			}
		}
	}

	return false;
}

bool Autowall::HandleBulletPen(CCSWeaponInfo* wpn_data, FireBulletData& data, bool extracheck = false, Vector point = Vector(0, 0, 0), C_BasePlayer* pEntity = nullptr) {
	trace_t       trace_exit;
	surfacedata_t* enter_surface_data = g_PhysSurface->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int           enter_material = enter_surface_data->game.material;

	C_BasePlayer* entity = (C_BasePlayer*)data.enter_trace.hit_entity;

	float         enter_surf_penetration_modifier = enter_surface_data->game.penetrationmodifier;
	float         final_damage_modifier = 0.f;
	float         combined_penetration_modifier = 0.f;
	bool          solid_surf = ((data.enter_trace.contents >> 3)& CONTENTS_SOLID);
	bool          light_surf = ((data.enter_trace.surface.flags >> 7)& SURF_LIGHT);

	if (data.penetrate_count <= 0
		|| (!data.penetrate_count && !light_surf && !solid_surf && enter_material != CHAR_TEX_GLASS && enter_material != CHAR_TEX_GRATE)
		|| wpn_data->flPenetration <= 0.f
		|| !TraceToExit(data.enter_trace, trace_exit, data.enter_trace.endpos, data.direction)
		&& !(g_EngineTrace->GetPointContents(data.enter_trace.endpos, MASK_SHOT_HULL, nullptr) & (MASK_SHOT_HULL)))
		return false;

	surfacedata_t* exit_surface_data = g_PhysSurface->GetSurfaceData(trace_exit.surface.surfaceProps);
	int           exit_material = exit_surface_data->game.material;
	float         exit_surf_penetration_modifier = exit_surface_data->game.penetrationmodifier;

	static ConVar* damage_reduction_bullets = g_CVar->FindVar("ff_damage_reduction_bullets");
	static ConVar* damage_bullet_penetration = g_CVar->FindVar("ff_damage_bullet_penetration");

	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE) {
		combined_penetration_modifier = 3.f;
		final_damage_modifier = 0.05f;
	}
	else if (light_surf || solid_surf) {
		combined_penetration_modifier = 1.f;
		final_damage_modifier = 0.16f;
	}
	else if (data.enter_trace.hit_entity && enter_material == CHAR_TEX_FLESH && (g_LocalPlayer->m_iTeamNum() == entity->m_iTeamNum() && damage_reduction_bullets->GetFloat() == 0.f)) {
		//  look's like you aren't shooting through your teammate today
		if (damage_bullet_penetration->GetFloat() == 0.f) {
			return false;
		}

		combined_penetration_modifier = damage_bullet_penetration->GetFloat();
		final_damage_modifier = 0.16f;
	}

	else {
		combined_penetration_modifier = (enter_surf_penetration_modifier + exit_surf_penetration_modifier) * 0.5f;
		final_damage_modifier = 0.16f;
	}

	//  do our materials line up?
	if (enter_material == exit_material) {
		if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD) {
			combined_penetration_modifier = 3.f;
		}

		else if (exit_material == CHAR_TEX_PLASTIC) {
			combined_penetration_modifier = 2.f;
		}
	}

	//  calculate thickness of the wall by getting the length of the range of the trace and squaring
	float thickness = (trace_exit.endpos - data.enter_trace.endpos).LengthSqr();

	if (extracheck)
		if (!VectortoVectorVisible(trace_exit.endpos, point, pEntity))
			return false;

	float modifier = fmaxf(1.f / combined_penetration_modifier, 0.f);

	//  this calculates how much damage we've lost depending on thickness of the wall, our penetration, damage, and the modifiers set earlier
	float lost_damage = fmaxf(
		((modifier * thickness) / 24.f) //* 0.041666668
		+ ((data.current_damage * final_damage_modifier)
			+ (fmaxf(3.75 / wpn_data->flPenetration, 0.f) * 3.f * modifier)), 0.f);

	//  did we loose too much damage?
	if (lost_damage > data.current_damage) {
		return false;
	}

	//  we can't use any of the damage that we've lost
	if (lost_damage > 0.f) {
		data.current_damage -= lost_damage;
	}

	//  do we still have enough damage to deal?
	if (data.current_damage < 1.f) {
		return false;
	}

	data.src = trace_exit.endpos;
	--data.penetrate_count;

	return true;
}


bool Autowall::FireBullet(C_BaseCombatWeapon* weapon, FireBulletData& data) {
	auto local = g_LocalPlayer;

	trace_t     exit_trace;
	CCSWeaponInfo* weapon_info = weapon->GetCSWeaponData();

	data.penetrate_count = 4;
	data.trace_length = 0.f;

	if (weapon_info == nullptr) {
		return false;
	}

	data.current_damage = weapon_info->iDamage;
	data.max_range = weapon_info->flRange;

	//  if our damage is greater than (or equal to) 1, and we can shoot, let's shoot.
	while (data.penetrate_count > 0 && data.current_damage >= 1.f) {
		data.max_range -= data.trace_length;

		//  create endpoint of bullet
		auto end = (data.direction * data.max_range) + data.src;

		TraceLine(data.src, end, MASK_SHOT_HULL | CONTENTS_HITBOX, local, &data.enter_trace);

		//	UtilClipTraceToPlayers( data.src, end + data.direction * 40.f, MASK_SHOT_HULL | CONTENTS_HITBOX, &data.filter, &data.enter_trace );

			//  "fraction == 1" means that we didn't hit anything. we don't want that- so let's break on it.
		if (data.enter_trace.fraction == 1.f) {
			break;
		}

		//  calculate the damage based on the distance the bullet traveled.
		data.trace_length += data.enter_trace.fraction * data.max_range;

		//  let's make our damage drops off the further away the bullet is.
		data.current_damage *= pow(weapon_info->flRangeModifier, (data.trace_length / 500.f));

		//  get surface data
		auto enter_surface_data = g_PhysSurface->GetSurfaceData(data.enter_trace.surface.surfaceProps);
		auto      enter_pen_mod = enter_surface_data->game.penetrationmodifier;
		int      enter_material = enter_surface_data->game.material;

		if (data.trace_length > 3000.f || enter_pen_mod < 0.1f) {
			break;
		}

		//  this looks gay as fuck if we put it in to 1 long line of code

		C_BasePlayer* entity = (C_BasePlayer*)data.enter_trace.hit_entity;
		bool can_do_damage = (data.enter_trace.hitgroup != HITGROUP_GEAR && data.enter_trace.hitgroup != HITGROUP_GENERIC);
		bool      is_enemy = (local->m_iTeamNum() != entity->m_iTeamNum());

		//  check to see if we hit an entity, if so scale the damage
		if ((can_do_damage && is_enemy)) {

			//  scale our damage
			ScaleDamage((C_BasePlayer*)data.enter_trace.hit_entity, data.enter_trace.hitgroup, weapon_info, data.current_damage);

			return true;
		}

		//  if the bullet can't penetrate wall we don't want to continue looping
		if (!HandleBulletPen(weapon_info, data)) {
			break;
		}
	}

	return false;
}

float Autowall::GetDamage(Vector point) {
	//  define local
	auto   local = g_LocalPlayer;

	//  make sure local and weapon is not nil
	if (!local) {
		return -1.f;
	}

	// setup fire bullet data
	FireBulletData data = FireBulletData(point, local);

	data.src = local->GetEyePos();

	//  find the direction from source to point
	QAngle angles = Math::CalcAngle(data.src, point);
	Math::AngleVectors(angles, data.direction);
	data.direction.NormalizeInPlace();

	//  simulate a bullet being fired
	if (FireBullet(local->m_hActiveWeapon(), data)) {
		return data.current_damage;
	}

	return -1.f;
}

bool Autowall::CanHitFloatingPoint(const Vector& point, const Vector& source, C_BasePlayer* pEntity) // ez
{
	auto pLocalEnt = g_LocalPlayer;

	if (!pLocalEnt)
		return false;

	if (!pLocalEnt->IsAlive())
		return false;

	FireBulletData data = FireBulletData(source, pLocalEnt);

	QAngle angles = Math::CalcAngle(data.src, point);
	Math::AngleVectors(angles, data.direction);
	data.direction.Normalize();

	C_BaseCombatWeapon* pWeapon = pLocalEnt->m_hActiveWeapon();

	if (!pWeapon)
		return false;

	data.penetrate_count = 1;
	data.trace_length = 0.0f;

	CCSWeaponInfo* pWeaponData = pWeapon->GetCSWeaponData();

	if (!pWeaponData)
		return false;

	data.current_damage = (float)pWeaponData->iDamage;
	data.trace_length_remaining = pWeaponData->flRange - data.trace_length;
	Vector end = data.src + (data.direction * data.trace_length_remaining);
	TraceLine(data.src, end, MASK_SHOT | CONTENTS_HITBOX, pLocalEnt, &data.enter_trace);

	if (VectortoVectorVisible(data.src, point, pEntity) || HandleBulletPen(pWeaponData, data, true, point, pEntity))
		return true;

	return false;
}