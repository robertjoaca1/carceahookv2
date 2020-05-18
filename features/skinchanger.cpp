#include "SkinChanger.hpp"

int GetKnifeDefinitionIndex(int si)
{
	switch (si)
	{
	case 1:
		return WEAPON_KNIFE_BAYONET;
	case 2:
		return WEAPON_KNIFE_FLIP;
	case 3:
		return WEAPON_KNIFE_GUT;
	case 4:
		return WEAPON_KNIFE_KARAMBIT;
	case 5:
		return WEAPON_KNIFE_M9_BAYONET;
	case 6:
		return WEAPON_KNIFE_TACTICAL;
	case 7:
		return WEAPON_KNIFE_FALCHION;
	case 8:
		return WEAPON_KNIFE_SURVIVAL_BOWIE;
	case 9:
		return WEAPON_KNIFE_BUTTERFLY;
	case 10:
		return WEAPON_KNIFE_PUSH;
	case 11:
		return WEAPON_KNIFE_URSUS;
	case 12:
		return WEAPON_KNIFE_GYPSY_JACKKNIFE;
	case 13:
		return WEAPON_KNIFE_STILETTO;
	case 14:
		return WEAPON_KNIFE_WIDOWMAKER;
	case 15:
		return WEAPON_KNIFE_OUTDOOR;
	case 16:
		return WEAPON_KNIFE_SKELETON;
	case 17:
		return WEAPON_KNIFE_CANIS;
	case 18:
		return WEAPON_KNIFE_CORD;
	case 19:
		return WEAPON_KNIFE_CSS;
	}
}
bool ApplyKnife(C_BaseAttributableItem* pWeapon, const char* vMdl)
{
	auto local = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer());
	C_BaseViewModel* viewmodel = local->m_hViewModel().Get();
	if (!viewmodel)
		return false;
	// Get the weapon belonging to this view model.
	auto hViewModelWeapon = viewmodel->m_hWeapon();
	C_BaseAttributableItem* pViewModelWeapon = (C_BaseAttributableItem*)g_EntityList->GetClientEntityFromHandle(hViewModelWeapon);

	if (pViewModelWeapon != pWeapon)
		return false;

	// Check if an override exists for this view model.
	int nViewModelIndex = viewmodel->m_nModelIndex();

	// Set the replacement model.
	viewmodel->m_nModelIndex() = g_MdlInfo->GetModelIndex(vMdl);
	viewmodel->m_nViewModelIndex() = g_MdlInfo->GetModelIndex(vMdl);
	return true;
}
int SkinChanger::GetSkinId(int WeaponId, int Skin)
{
	if (Skin == 0)
		return 0;
	switch (WeaponId)
	{
	case WEAPON_KNIFE_BAYONET:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 558;
		case 2:
			return 568;
		case 3:
			return 573;
		case 4:
			return 563;
		case 5:
			return 580;
		case 6:
			return 578;
		case 7:
			return 856;
		case 8:
			return 409;
		case 9:
			return 415;
		case 10:
			return 620;
		case 11:
			return 857;
		case 12:
			return 203;
		case 13:
			return 38;
		case 14:
			return 59;
		case 15:
			return 44;
		case 16:
			return 12;
		case 17:
			return 42;
		case 18:
			return 40;
		case 19:
			return 72;
		case 20:
			return 5;
		case 21:
			return 43;
		case 22:
			return 175;
		case 23:
			return 77;
		case 24:
			return 143;
		}
		break;
	case WEAPON_KNIFE_FLIP:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 559;
		case 2:
			return 568;
		case 3:
			return 574;
		case 4:
			return 564;
		case 5:
			return 581;
		case 6:
			return 579;
		case 7:
			return 856;
		case 8:
			return 409;
		case 9:
			return 415;
		case 10:
			return 620;
		case 11:
			return 857;
		case 12:
			return 203;
		case 13:
			return 38;
		case 14:
			return 59;
		case 15:
			return 44;
		case 16:
			return 12;
		case 17:
			return 42;
		case 18:
			return 40;
		case 19:
			return 72;
		case 20:
			return 5;
		case 21:
			return 43;
		case 22:
			return 175;
		case 23:
			return 77;
		case 24:
			return 143;
		}
		break;
	case WEAPON_KNIFE_GUT:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 560;
		case 2:
			return 568;
		case 3:
			return 575;
		case 4:
			return 565;
		case 5:
			return 580;
		case 6:
			return 578;
		case 7:
			return 856;
		case 8:
			return 409;
		case 9:
			return 415;
		case 10:
			return 620;
		case 11:
			return 857;
		case 12:
			return 203;
		case 13:
			return 38;
		case 14:
			return 59;
		case 15:
			return 44;
		case 16:
			return 12;
		case 17:
			return 42;
		case 18:
			return 40;
		case 19:
			return 72;
		case 20:
			return 5;
		case 21:
			return 43;
		case 22:
			return 175;
		case 23:
			return 77;
		case 24:
			return 143;
		}
		break;
	case WEAPON_KNIFE_KARAMBIT:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 561;
		case 2:
			return 568;
		case 3:
			return 576;
		case 4:
			return 566;
		case 5:
			return 580;
		case 6:
			return 578;
		case 7:
			return 856;
		case 8:
			return 409;
		case 9:
			return 415;
		case 10:
			return 620;
		case 11:
			return 857;
		case 12:
			return 203;
		case 13:
			return 38;
		case 14:
			return 59;
		case 15:
			return 44;
		case 16:
			return 12;
		case 17:
			return 42;
		case 18:
			return 40;
		case 19:
			return 72;
		case 20:
			return 5;
		case 21:
			return 43;
		case 22:
			return 175;
		case 23:
			return 77;
		case 24:
			return 143;
		}
		break;
	case WEAPON_KNIFE_M9_BAYONET:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 562;
		case 2:
			return 568;
		case 3:
			return 577;
		case 4:
			return 567;
		case 5:
			return 580;
		case 6:
			return 578;
		case 7:
			return 856;
		case 8:
			return 409;
		case 9:
			return 415;
		case 10:
			return 620;
		case 11:
			return 857;
		case 12:
			return 203;
		case 13:
			return 38;
		case 14:
			return 59;
		case 15:
			return 44;
		case 16:
			return 12;
		case 17:
			return 42;
		case 18:
			return 40;
		case 19:
			return 72;
		case 20:
			return 5;
		case 21:
			return 43;
		case 22:
			return 175;
		case 23:
			return 77;
		case 24:
			return 143;
		}
		break;
	case WEAPON_KNIFE_TACTICAL:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_FALCHION:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_BUTTERFLY:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_PUSH:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_URSUS:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_STILETTO:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_WIDOWMAKER:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_OUTDOOR:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_SKELETON:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_CANIS:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_CORD:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_KNIFE_CSS:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 856;
		case 2:
			return 409;
		case 3:
			return 415;
		case 4:
			return 620;
		case 5:
			return 857;
		case 6:
			return 203;
		case 7:
			return 38;
		case 8:
			return 59;
		case 9:
			return 44;
		case 10:
			return 12;
		case 11:
			return 42;
		case 12:
			return 40;
		case 13:
			return 72;
		case 14:
			return 5;
		case 15:
			return 43;
		case 16:
			return 175;
		case 17:
			return 77;
		case 18:
			return 143;
		}
		break;
	case WEAPON_GLOCK:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 586;
		case 2:
			return 353;
		case 3:
			return 437;
		case 4:
			return 694;
		case 5:
			return 607;
		case 6:
			return 532;
		case 7:
			return 381;
		case 8:
			return 230;
		case 9:
			return 48;
		case 10:
			return 732;
		case 11:
			return 789;
		case 12:
			return 38;
		case 13:
			return 159;
		case 14:
			return 918;
		case 15:
			return 808;
		case 16:
			return 713;
		case 17:
			return 680;
		case 18:
			return 623;
		case 19:
			return 495;
		case 20:
			return 479;
		case 21:
			return 399;
		case 22:
			return 278;
		case 23:
			return 367;
		case 24:
			return 864;
		case 25:
			return 799;
		}
		break;
	case WEAPON_HKP2000:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 389;
		case 2:
			return 591;
		case 3:
			return 184;
		case 4:
			return 213;
		case 5:
			return 894;
		case 6:
			return 667;
		case 7:
			return 485;
		case 8:
			return 246;
		case 9:
			return 71;
		case 10:
			return 700;
		case 11:
			return 635;
		case 12:
			return 550;
		case 13:
			return 515;
		case 14:
			return 338;
		case 15:
			return 275;
		}
		break;
	case WEAPON_USP_SILENCER:
		switch (Skin)
		{
		case 0: 
			return 0;
		case 1:
			return 653;
		case 2:
			return 504;
		case 3:
			return 705;
		case 4:
			return 339;
		case 5:
			return 313;
		case 6:
			return 221;
		case 7:
			return 817;
		case 8:
			return 637;
		case 9:
			return 290;
		case 10:
			return 183;
		case 11:
			return 60;
		case 12:
			return 318;
		case 13:
			return 657;
		case 14:
			return 540;
		case 15:
			return 489;
		case 16:
			return 217;
		case 17:
			return 277;
		}
		break;
	case WEAPON_P250:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 678;
		case 2:
			return 551;
		case 3:
			return 404;
		case 4:
			return 388;
		case 5:
			return 271;
		case 6: 
			return 258;
		case 7:
			return 295;
		case 8:
			return 907;
		case 9:
			return 125;
		case 10:
			return 813;
		case 11:
			return 668;
		case 12:
			return 501;
		case 13:
			return 358;
		case 14:
			return 162;
		case 15:
			return 749;
		case 16:
			return 168;
		}
		break;
	case WEAPON_ELITE:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 658;
		case 2:
			return 747;
		case 3:
			return 625;
		case 4:
			return 396;
		case 5:
			return 261;
		case 6:
			return 220;
		case 7:
			return 447;
		case 8:
			return 249;
		case 9:
			return 153;
		case 10:
			return 895;
		case 11:
			return 903;
		case 12:
			return 710;
		case 13:
			return 544;
		case 14:
			return 528;
		case 15:
			return 491;
		case 16:
			return 307;
		}
		break;
	case WEAPON_CZ75A:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 270;
		case 2:
			return 643;
		case 3:
			return 476;
		case 4:
			return 269;
		case 5:
			return 709;
		case 6:
			return 687;
		case 7:
			return 543;
		case 8:
			return 435;
		case 9:
			return 350;
		case 10:
			return 268;
		case 11:
			return 325;
		case 12:
			return 622;
		case 13:
			return 602;
		case 14:
			return 218;
		case 15:
			return 334;
		case 16:
			return 315;
		case 17:
			return 12;
		case 18:
			return 859;
		case 19:
			return 196;
		}
		break;
	case WEAPON_TEC9:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 889;
		case 2:
			return 614;
		case 3:
			return 791;
		case 4:
			return 272;
		case 5:
			return 599;
		case 6:
			return 303;
		case 7:
			return 289;
		case 8:
			return 17;
		}
		break;
	case WEAPON_FIVESEVEN:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 660;
		case 2:
			return 427;
		case 3:
			return 530;
		case 4:
			return 693;
		case 5:
			return 377;
		}
		break;
	case WEAPON_DEAGLE:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 711;
		case 2:
			return 351;
		case 3:
			return 527;
		case 4:
			return 470;
		case 5:
			return 645;
		case 6:
			return 37;
		}
		break;
	case WEAPON_REVOLVER:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 38;
		case 2:
			return 322;
		case 3:
			return 232;
		case 4:
			return 701;
		case 5:
			return 523;
		case 6:
			return 683;
		}
		break;
	case WEAPON_MAC10:
		switch (Skin)
			{
			case 0:
				return 0;
			case 1:
				return 898;
			case 2:
				return 433;
			case 3:
				return 284;
			case 4:
				return 372;
			}
			break;
	case WEAPON_MP5_SD:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 810;
		case 2:
			return 915;
		case 3:
			return 872;
		case 4:
			return 781;
		}
		break;
	case WEAPON_MP7:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 893;
		case 2:
			return 696;
		case 3:
			return 481;
		case 4:
			return 423;
		}
		break;
	case WEAPON_MP9:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 910;
		case 2:
			return 734;
		case 3:
			return 262;
		case 4:
			return 448;
		}
		break;
	case WEAPON_BIZON:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 676;
		case 2:
			return 542;
		case 3:
			return 508;
		case 4:
			return 286;
		}
		break;
	case WEAPON_P90:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 593;
		case 2:
			return 636;
		case 3:
			return 283;
		case 4:
			return 759;
		}
		break;
	case WEAPON_UMP45:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 802;
		case 2:
			return 886;
		case 3:
			return 688;
		case 4:
			return 441;
		}
		break;
	case WEAPON_AK47:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 474;
		case 2:
			return 524;
		case 3:
			return 380;
		case 4:
			return 302;
		case 5:
			return 490;
		case 6:
			return 506;
		case 7:
			return 282;
		case 8:
			return 456;
		}
		break;
	case WEAPON_AUG:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 455;
		case 2:
			return 280;
		case 3:
			return 375;
		case 4:
			return 583;
		}
		break;
	case WEAPON_FAMAS:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 919;
		case 2:
			return 604;
		case 3:
			return 429;
		case 4:
			return 723;
		}
		break;
	case WEAPON_GALILAR:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 661;
		case 2:
			return 398;
		case 3:
			return 807;
		case 4:
			return 460;
		}
		break;
	case WEAPON_SG556:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 260;
		case 2:
			return 620;
		case 3:
			return 897;
		case 4:
			return 750;
		}
		break;
	case WEAPON_M4A1:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 309;
		case 2:
			return 844;
		case 3:
			return 632;
		case 4:
			return 155;
		}
		break;
	case WEAPON_M4A1_SILENCER:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 548;
		case 2:
			return 312;
		case 3:
			return 714;
		case 4:
			return 889;
		}
		break;
	case WEAPON_AWP:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 877;
		case 2:
			return 917;
		case 3:
			return 803;
		case 4:
			return 344;
		case 5:
			return 640;
		case 6:
			return 446;
		case 7:
			return 584;
		}
		break;
	case WEAPON_G3SG1:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 493;
		case 2:
			return 628;
		case 3:
			return 229;
		case 4:
			return 712;
		}
		break;
	case WEAPON_SCAR20:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 391;
		case 2:
			return 612;
		case 3:
			return 657;
		case 4:
			return 70;
		}
		break;
	case WEAPON_SSG08:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 624;
		case 2:
			return 222;
		case 3:
			return 554;
		case 4:
			return 304;
		}
		break;
	case WEAPON_MAG7:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 703;
		case 2:
			return 737;
		case 3:
			return 535;
		case 4:
			return 291;
		}
		break;
	case WEAPON_NOVA:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 306;
		case 2:
			return 62;
		case 3:
			return 746;
		case 4:
			return 590;
		}
		break;
	case WEAPON_SAWEDOFF:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 256;
		case 2:
			return 720;
		case 3:
			return 638;
		case 4:
			return 596;
		}
		break;
	case WEAPON_XM1014:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 850;
		case 2:
			return 393;
		case 3:
			return 654;
		case 4:
			return 760;
		}
		break;
	case WEAPON_M249:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 496;
		case 2:
			return 648;
		case 3:
			return 547;
		case 4:
			return 472;
		}
		break;
	case WEAPON_NEGEV:
		switch (Skin)
		{
		case 0:
			return 0;
		case 1:
			return 514;
		case 2:
			return 610;
		case 3:
			return 783;
		case 4:
			return 285;
		}
		break;
	}
	return 0;
}
void SkinChanger::SetSkins()
{
	//weapon skins

	SkinChangerInfo[WEAPON_DEAGLE].nFallbackPaintKit = GetSkinId(WEAPON_DEAGLE, Variables.DealgeSkin);
	SkinChangerInfo[WEAPON_DEAGLE].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_ELITE].nFallbackPaintKit = GetSkinId(WEAPON_ELITE, Variables.BeretasSkin);
	SkinChangerInfo[WEAPON_ELITE].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_FIVESEVEN].nFallbackPaintKit = GetSkinId(WEAPON_FIVESEVEN, Variables.FiveSevenSkin);
	SkinChangerInfo[WEAPON_FIVESEVEN].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_GLOCK].nFallbackPaintKit = GetSkinId(WEAPON_GLOCK, Variables.GlockSkin);
	SkinChangerInfo[WEAPON_GLOCK].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_AK47].nFallbackPaintKit = GetSkinId(WEAPON_AK47, Variables.Ak47Skin);
	SkinChangerInfo[WEAPON_AK47].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_AUG].nFallbackPaintKit = GetSkinId(WEAPON_AUG, Variables.AugSkin);
	SkinChangerInfo[WEAPON_AUG].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_AWP].nFallbackPaintKit = GetSkinId(WEAPON_AWP, Variables.AWPSkin);
	SkinChangerInfo[WEAPON_AWP].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_FAMAS].nFallbackPaintKit = GetSkinId(WEAPON_FAMAS, Variables.FamasSkin);
	SkinChangerInfo[WEAPON_FAMAS].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_G3SG1].nFallbackPaintKit = GetSkinId(WEAPON_G3SG1, Variables.G3SG1Skin);
	SkinChangerInfo[WEAPON_G3SG1].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_GALILAR].nFallbackPaintKit = GetSkinId(WEAPON_GALILAR, Variables.GalilSkin);
	SkinChangerInfo[WEAPON_GALILAR].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_M249].nFallbackPaintKit = GetSkinId(WEAPON_M249, Variables.M249Skin);
	SkinChangerInfo[WEAPON_M249].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_M4A1].nFallbackPaintKit = GetSkinId(WEAPON_M4A1, Variables.M4A4Skin);
	SkinChangerInfo[WEAPON_M4A1].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_MAC10].nFallbackPaintKit = GetSkinId(WEAPON_MAC10, Variables.Mac10Skin);
	SkinChangerInfo[WEAPON_MAC10].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_P90].nFallbackPaintKit = GetSkinId(WEAPON_P90, Variables.P90Skin);
	SkinChangerInfo[WEAPON_P90].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_MP5_SD].nFallbackPaintKit = GetSkinId(WEAPON_MP5_SD, Variables.MP5Skin);
	SkinChangerInfo[WEAPON_MP5_SD].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_UMP45].nFallbackPaintKit = GetSkinId(WEAPON_UMP45, Variables.UMPSkin);
	SkinChangerInfo[WEAPON_UMP45].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_XM1014].nFallbackPaintKit = GetSkinId(WEAPON_XM1014, Variables.XM1014Skin);
	SkinChangerInfo[WEAPON_XM1014].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_BIZON].nFallbackPaintKit = GetSkinId(WEAPON_BIZON, Variables.BizonSkin);
	SkinChangerInfo[WEAPON_BIZON].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_MAG7].nFallbackPaintKit = GetSkinId(WEAPON_MAG7, Variables.MAG7Skin);
	SkinChangerInfo[WEAPON_MAG7].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_NEGEV].nFallbackPaintKit = GetSkinId(WEAPON_NEGEV, Variables.NegevSkin);
	SkinChangerInfo[WEAPON_NEGEV].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_SAWEDOFF].nFallbackPaintKit = GetSkinId(WEAPON_SAWEDOFF, Variables.SawedOffSkin);
	SkinChangerInfo[WEAPON_SAWEDOFF].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_TEC9].nFallbackPaintKit = GetSkinId(WEAPON_TEC9, Variables.Tec9Skin);
	SkinChangerInfo[WEAPON_TEC9].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_HKP2000].nFallbackPaintKit = GetSkinId(WEAPON_HKP2000, Variables.P2000Skin);
	SkinChangerInfo[WEAPON_HKP2000].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_MP7].nFallbackPaintKit = GetSkinId(WEAPON_MP7, Variables.MP7Skin);
	SkinChangerInfo[WEAPON_MP7].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_MP9].nFallbackPaintKit = GetSkinId(WEAPON_MP9, Variables.MP9Skin);
	SkinChangerInfo[WEAPON_MP9].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_NOVA].nFallbackPaintKit = GetSkinId(WEAPON_NOVA, Variables.NovaSkin);
	SkinChangerInfo[WEAPON_NOVA].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_P250].nFallbackPaintKit = GetSkinId(WEAPON_P250, Variables.P250Skin);
	SkinChangerInfo[WEAPON_P250].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_SCAR20].nFallbackPaintKit = GetSkinId(WEAPON_SCAR20, Variables.Scar20Skin);
	SkinChangerInfo[WEAPON_SCAR20].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_SG556].nFallbackPaintKit = GetSkinId(WEAPON_SG556, Variables.SG556Skin);
	SkinChangerInfo[WEAPON_SG556].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_SSG08].nFallbackPaintKit = GetSkinId(WEAPON_SSG08, Variables.SSG08Skin);
	SkinChangerInfo[WEAPON_SSG08].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_M4A1_SILENCER].nFallbackPaintKit = GetSkinId(WEAPON_M4A1_SILENCER, Variables.M4A1Skin);
	SkinChangerInfo[WEAPON_M4A1_SILENCER].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_USP_SILENCER].nFallbackPaintKit = GetSkinId(WEAPON_USP_SILENCER, Variables.USPSkin);
	SkinChangerInfo[WEAPON_USP_SILENCER].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_CZ75A].nFallbackPaintKit = GetSkinId(WEAPON_CZ75A, Variables.CZ75ASkin);
	SkinChangerInfo[WEAPON_CZ75A].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_REVOLVER].nFallbackPaintKit = GetSkinId(WEAPON_REVOLVER, Variables.RevolverSkin);
	SkinChangerInfo[WEAPON_REVOLVER].flFallbackWear = 0.00000001f;

	// knife skins

	SkinChangerInfo[WEAPON_KNIFE_BAYONET].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_BAYONET, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_BAYONET].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_KNIFE_FLIP].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_FLIP, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_FLIP].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_KNIFE_GUT].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_GUT, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_GUT].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_KNIFE_KARAMBIT].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_KARAMBIT, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_KARAMBIT].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_KNIFE_M9_BAYONET].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_M9_BAYONET, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_M9_BAYONET].flFallbackWear = 0.00000001f;
	
	SkinChangerInfo[WEAPON_KNIFE_TACTICAL].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_TACTICAL, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_TACTICAL].flFallbackWear = 0.00000001f;
	
	SkinChangerInfo[WEAPON_KNIFE_FALCHION].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_FALCHION, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_FALCHION].flFallbackWear = 0.00000001f;
	
	SkinChangerInfo[WEAPON_KNIFE_SURVIVAL_BOWIE].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_SURVIVAL_BOWIE, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_SURVIVAL_BOWIE].flFallbackWear = 0.00000001f;
	
	SkinChangerInfo[WEAPON_KNIFE_BUTTERFLY].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_BUTTERFLY, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_BUTTERFLY].flFallbackWear = 0.00000001f;
	
	SkinChangerInfo[WEAPON_KNIFE_PUSH].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_PUSH, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_PUSH].flFallbackWear = 0.00000001f;
	
	SkinChangerInfo[WEAPON_KNIFE_URSUS].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_URSUS, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_URSUS].flFallbackWear = 0.00000001f;
	
	SkinChangerInfo[WEAPON_KNIFE_GYPSY_JACKKNIFE].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_GYPSY_JACKKNIFE, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_GYPSY_JACKKNIFE].flFallbackWear = 0.00000001f;
	
	SkinChangerInfo[WEAPON_KNIFE_STILETTO].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_STILETTO, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_STILETTO].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_KNIFE_WIDOWMAKER].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_WIDOWMAKER, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_WIDOWMAKER].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_KNIFE_OUTDOOR].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_OUTDOOR, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_OUTDOOR].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_KNIFE_SKELETON].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_SKELETON, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_SKELETON].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_KNIFE_CANIS].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_CANIS, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_CANIS].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_KNIFE_CORD].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_CORD, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_CORD].flFallbackWear = 0.00000001f;

	SkinChangerInfo[WEAPON_KNIFE_CSS].nFallbackPaintKit = GetSkinId(WEAPON_KNIFE_CSS, Variables.KnifeSkin);
	SkinChangerInfo[WEAPON_KNIFE_CSS].flFallbackWear = 0.00000001f;
}
void SkinChanger::DoSkinChanger() 
{
	C_BasePlayer* LocalPlayer = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
	if (!LocalPlayer ||
		!LocalPlayer->IsAlive() ||
		!Variables.SkinChangerEnabled)
		return;

	auto MyWeapons = LocalPlayer->m_hMyWeapons();
	if (!MyWeapons) 
		return;

	for (auto i = 0; MyWeapons[i]; i++)
	{
		C_BaseAttributableItem* Weapon = (C_BaseAttributableItem*)g_EntityList->GetClientEntityFromHandle(MyWeapons[i]);
		bool isknife = ((C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(MyWeapons[i]))->IsKnife();

		if (!Weapon)
			continue;

		if (Weapon->m_Item().m_iItemDefinitionIndex() > 5035 || Weapon->m_Item().m_iItemDefinitionIndex() < 1)
			continue;

		if (Weapon->m_Item().m_iItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_TASER)
			continue;

		if (LocalPlayer->GetPlayerInfo().xuid_low != Weapon->m_OriginalOwnerXuidLow())
			continue;

		if (LocalPlayer->GetPlayerInfo().xuid_high != Weapon->m_OriginalOwnerXuidHigh())
			continue;



#pragma region knivez
		static char* KnifeCT = "models/weapons/v_knife_ct.mdl";
		static char* KnifeT = "models/weapons/v_knife_t.mdl";
		static char* Bayonet = "models/weapons/v_knife_bayonet.mdl";
		static char* Butterfly = "models/weapons/v_knife_butterfly.mdl";
		static char* Flip = "models/weapons/v_knife_flip.mdl";
		static char* Gut = "models/weapons/v_knife_gut.mdl";
		static char* Karambit = "models/weapons/v_knife_karam.mdl";
		static char* M9Bayonet = "models/weapons/v_knife_m9_bay.mdl";
		static char* Huntsman = "models/weapons/v_knife_tactical.mdl";
		static char* Falchion = "models/weapons/v_knife_falchion_advanced.mdl";
		static char* Dagger = "models/weapons/v_knife_push.mdl";
		static char* Bowie = "models/weapons/v_knife_survival_bowie.mdl";
		static char* Ursus = "models/weapons/v_knife_ursus.mdl";
		static char* Navaja = "models/weapons/v_knife_gypsy_jackknife.mdl";
		static char* Stiletto = "models/weapons/v_knife_stiletto.mdl";
		static char* Talon = "models/weapons/v_knife_widowmaker.mdl";
		static char* Nomad = "models/weapons/v_knife_outdoor.mdl";
		static char* Skeleton = "models/weapons/v_knife_skeleton.mdl";
		static char* Survival = "models/weapons/v_knife_canis.mdl";
		static char* Paracord = "models/weapons/v_knife_cord.mdl";
		static char* Classic = "models/weapons/v_knife_css.mdl";
		if (isknife)
		{
			switch (Variables.KnifeModel)
			{
			case 0:
				if (LocalPlayer->m_iTeamNum() == 0) Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(KnifeT);
				if (LocalPlayer->m_iTeamNum() == 1) Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(KnifeCT);
				break;
			case 1:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Bayonet);
				break;
			case 2:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Flip);
				break;
			case 3:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Gut);
				break;
			case 4:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Karambit);
				break;
			case 5:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(M9Bayonet);
				break;
			case 6:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Huntsman);
				break;
			case 7:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Falchion);
				break;
			case 8:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Bowie);
				break;
			case 9:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Butterfly);
				break;
			case 10:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Dagger);
				break;
			case 11:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Ursus);
				break;
			case 12:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Navaja);
				break;
			case 13:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Stiletto);
				break;
			case 14:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Talon);
				break;
			case 15:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Nomad);
				break;
			case 16:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Skeleton);
				break;
			case 17:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Survival);
				break;
			case 18:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Paracord);
				break;
			case 19:
				Weapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Classic);
				break;
			}
			// changine knife model
			switch (Variables.KnifeModel)
			{
			case 0:
				if (LocalPlayer->m_iTeamNum() == 0) ApplyKnife(Weapon, KnifeT);
				if (LocalPlayer->m_iTeamNum() == 1) ApplyKnife(Weapon, KnifeCT);
				break;
			case 1:
				ApplyKnife(Weapon, Bayonet);
				break;
			case 2:
				ApplyKnife(Weapon, Flip);
				break;
			case 3:
				ApplyKnife(Weapon, Gut);
				break;
			case 4:
				ApplyKnife(Weapon, Karambit);
				break;
			case 5:
				ApplyKnife(Weapon, M9Bayonet);
				break;
			case 6:
				ApplyKnife(Weapon, Huntsman);
				break;
			case 7:
				ApplyKnife(Weapon, Falchion);
				break;
			case 8:
				ApplyKnife(Weapon, Bowie);
				break;
			case 9:
				ApplyKnife(Weapon, Butterfly);
				break;
			case 10:
				ApplyKnife(Weapon, Dagger);
				break;
			case 11:
				ApplyKnife(Weapon, Ursus);
				break;
			case 12:
				ApplyKnife(Weapon, Navaja);
				break;
			case 13:
				ApplyKnife(Weapon, Stiletto);
				break;
			case 14:
				ApplyKnife(Weapon, Talon);
				break;
			case 15:
				ApplyKnife(Weapon, Nomad);
				break;
			case 16:
				ApplyKnife(Weapon, Skeleton);
				break;
			case 17:
				ApplyKnife(Weapon, Survival);
				break;
			case 18:
				ApplyKnife(Weapon, Paracord);
				break;
			case 19:
				ApplyKnife(Weapon, Classic);
				break;
			}
			if (Variables.KnifeModel > 0) {
				if (Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_KNIFE || Weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_KNIFE_T)
					Weapon->m_Item().m_iItemDefinitionIndex() = GetKnifeDefinitionIndex(Variables.KnifeModel);
			}
			else
			{
				if (LocalPlayer->m_iTeamNum() == 0) Weapon->m_Item().m_iItemDefinitionIndex() = WEAPON_KNIFE_T;
				if (LocalPlayer->m_iTeamNum() == 1) Weapon->m_Item().m_iItemDefinitionIndex() = WEAPON_KNIFE;
			}
		}
#pragma endregion

		Weapon->m_nFallbackPaintKit() = SkinChangerInfo[Weapon->m_Item().m_iItemDefinitionIndex()].nFallbackPaintKit;
		Weapon->m_nFallbackSeed() = 420;
		Weapon->m_nFallbackStatTrak() = -1;
		Weapon->m_flFallbackWear() = SkinChangerInfo[Weapon->m_Item().m_iItemDefinitionIndex()].flFallbackWear;
		Weapon->m_Item().m_iItemIDHigh() = -1;
		Weapon->m_Item().m_iAccountID() = LocalPlayer->GetPlayerInfo().xuid_low;
		auto debug1 = Weapon->m_Item().m_iItemDefinitionIndex();
		auto debug2 = SkinChangerInfo[Weapon->m_Item().m_iItemDefinitionIndex()].nFallbackPaintKit;
	}
}
#define SEQUENCE_DEFAULT_DRAW 0
#define SEQUENCE_DEFAULT_IDLE1 1
#define SEQUENCE_DEFAULT_IDLE2 2
#define SEQUENCE_DEFAULT_LIGHT_MISS1 3
#define SEQUENCE_DEFAULT_LIGHT_MISS2 4
#define SEQUENCE_DEFAULT_HEAVY_MISS1 9
#define SEQUENCE_DEFAULT_HEAVY_HIT1 10
#define SEQUENCE_DEFAULT_HEAVY_BACKSTAB 11
#define SEQUENCE_DEFAULT_LOOKAT01 12

#define SEQUENCE_BUTTERFLY_DRAW 0
#define SEQUENCE_BUTTERFLY_DRAW2 1
#define SEQUENCE_BUTTERFLY_LOOKAT01 13
#define SEQUENCE_BUTTERFLY_LOOKAT03 15

#define SEQUENCE_FALCHION_IDLE1 1
#define SEQUENCE_FALCHION_HEAVY_MISS1 8
#define SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP 9
#define SEQUENCE_FALCHION_LOOKAT01 12
#define SEQUENCE_FALCHION_LOOKAT02 13

#define SEQUENCE_DAGGERS_IDLE1 1
#define SEQUENCE_DAGGERS_LIGHT_MISS1 2
#define SEQUENCE_DAGGERS_LIGHT_MISS5 6
#define SEQUENCE_DAGGERS_HEAVY_MISS2 11
#define SEQUENCE_DAGGERS_HEAVY_MISS1 12

#define SEQUENCE_BOWIE_IDLE1 1
#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);

typedef void(*RecvVarProxyFn)(const CRecvProxyData* pData, void* pStruct, void* pOut);
RecvVarProxyFn fnSequenceProxyFn = nullptr;
RecvVarProxyFn oRecvnModelIndex;

void Hooked_RecvProxy_Viewmodel(CRecvProxyData* pData, void* pStruct, void* pOut)
{
	static int default_t = g_MdlInfo->GetModelIndex("models/weapons/v_knife_default_t.mdl");
	static int default_ct = g_MdlInfo->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
	static int iBayonet = g_MdlInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
	static int iButterfly = g_MdlInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
	static int iFlip = g_MdlInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
	static int iGut = g_MdlInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
	static int iKarambit = g_MdlInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
	static int iM9Bayonet = g_MdlInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
	static int iHuntsman = g_MdlInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
	static int iFalchion = g_MdlInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
	static int iDagger = g_MdlInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
	static int iBowie = g_MdlInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
	static int iUrsus = g_MdlInfo->GetModelIndex("models/weapons/v_knife_ursus.mdl");
	static int iNavaja = g_MdlInfo->GetModelIndex("models/weapons/v_knife_gypsy_jackknife.mdl");
	static int iStiletto = g_MdlInfo->GetModelIndex("models/weapons/v_knife_stiletto.mdl");
	static int iTalon = g_MdlInfo->GetModelIndex("models/weapons/v_knife_widowmaker.mdl");
	static int iNomad = g_MdlInfo->GetModelIndex("models/weapons/v_knife_outdoor.mdl");
	static int iSkeleton = g_MdlInfo->GetModelIndex("models/weapons/v_knife_skeleton.mdl");
	static int iSurvival = g_MdlInfo->GetModelIndex("models/weapons/v_knife_canis.mdl");
	static int iParacord = g_MdlInfo->GetModelIndex("models/weapons/v_knife_cord.mdl");
	static int iClassic = g_MdlInfo->GetModelIndex("models/weapons/v_knife_css.mdl");

	if (g_LocalPlayer)
	{
		if (g_LocalPlayer->IsAlive() && (pData->m_Value.m_Int == default_ct
			|| pData->m_Value.m_Int == default_t
			|| pData->m_Value.m_Int == iGut
			|| pData->m_Value.m_Int == iKarambit
			|| pData->m_Value.m_Int == iM9Bayonet
			|| pData->m_Value.m_Int == iHuntsman
			|| pData->m_Value.m_Int == iFalchion
			|| pData->m_Value.m_Int == iDagger
			|| pData->m_Value.m_Int == iBowie
			|| pData->m_Value.m_Int == iButterfly
			|| pData->m_Value.m_Int == iFlip
			|| pData->m_Value.m_Int == iBayonet
			|| pData->m_Value.m_Int == iUrsus
			|| pData->m_Value.m_Int == iNavaja
			|| pData->m_Value.m_Int == iStiletto
			|| pData->m_Value.m_Int == iTalon
			|| pData->m_Value.m_Int == iNomad
			|| pData->m_Value.m_Int == iSkeleton
			|| pData->m_Value.m_Int == iSurvival
			|| pData->m_Value.m_Int == iParacord
			|| pData->m_Value.m_Int == iClassic))
		{
			if (Variables.KnifeModel == 1)
				pData->m_Value.m_Int = iBayonet;
			else if (Variables.KnifeModel == 2)
				pData->m_Value.m_Int = iFlip;
			else if (Variables.KnifeModel == 3)
				pData->m_Value.m_Int = iGut;
			else if (Variables.KnifeModel == 4)
				pData->m_Value.m_Int = iKarambit;
			else if (Variables.KnifeModel == 5)
				pData->m_Value.m_Int = iM9Bayonet;
			else if (Variables.KnifeModel == 6)
				pData->m_Value.m_Int = iHuntsman;
			else if (Variables.KnifeModel == 7)
				pData->m_Value.m_Int = iFalchion;
			else if (Variables.KnifeModel == 8)
				pData->m_Value.m_Int = iBowie;
			else if (Variables.KnifeModel == 9)
				pData->m_Value.m_Int = iButterfly;
			else if (Variables.KnifeModel == 10)
				pData->m_Value.m_Int = iDagger;
			else if (Variables.KnifeModel == 11)
				pData->m_Value.m_Int = iUrsus;
			else if (Variables.KnifeModel == 12)
				pData->m_Value.m_Int = iNavaja;
			else if (Variables.KnifeModel == 13)
				pData->m_Value.m_Int = iStiletto;
			else if (Variables.KnifeModel == 14)
				pData->m_Value.m_Int = iTalon;
			else if (Variables.KnifeModel == 15)
				pData->m_Value.m_Int = iNomad;
			else if (Variables.KnifeModel == 16)
				pData->m_Value.m_Int = iSkeleton;
			else if (Variables.KnifeModel == 17)
				pData->m_Value.m_Int = iSurvival;
			else if (Variables.KnifeModel == 18)
				pData->m_Value.m_Int = iParacord;
			else if (Variables.KnifeModel == 19)
				pData->m_Value.m_Int = iClassic;


		}
	}


	oRecvnModelIndex(pData, pStruct, pOut);
}


void SetViewModelSequence2(const CRecvProxyData* pDataConst, void* pStruct, void* pOut)
{
	CRecvProxyData* pData = const_cast<CRecvProxyData*>(pDataConst);

	// Confirm that we are replacing our view model and not someone elses.
	C_BaseViewModel* pViewModel = (C_BaseViewModel*)pStruct;

	if (pViewModel) {
		IClientEntity* pOwner = g_EntityList->GetClientEntityFromHandle(pViewModel->m_hOwner());

		// Compare the owner entity of this view model to the local player entity.
		if (pOwner && pOwner->EntIndex() == g_EngineClient->GetLocalPlayer()) {
			// Get the filename of the current view model.
			const model_t* pModel = g_MdlInfo->GetModel(pViewModel->m_nModelIndex());
			const char* szModel = g_MdlInfo->GetModelName(pModel);

			// Store the current sequence.
			int m_nSequence = pData->m_Value.m_Int;

			if (!strcmp(szModel, "models/weapons/v_knife_butterfly.mdl")) {
				// Fix animations for the Butterfly Knife.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_DRAW:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2); break;
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03); break;
				default:
					m_nSequence++;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_falchion_advanced.mdl")) {
				// Fix animations for the Falchion Knife.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_FALCHION_IDLE1; break;
				case SEQUENCE_DEFAULT_HEAVY_MISS1:
					m_nSequence = RandomInt(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP); break;
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02); break;
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				default:
					m_nSequence--;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_push.mdl")) {
				// Fix animations for the Shadow Daggers.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_DAGGERS_IDLE1; break;
				case SEQUENCE_DEFAULT_LIGHT_MISS1:
				case SEQUENCE_DEFAULT_LIGHT_MISS2:
					m_nSequence = RandomInt(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5); break;
				case SEQUENCE_DEFAULT_HEAVY_MISS1:
					m_nSequence = RandomInt(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1); break;
				case SEQUENCE_DEFAULT_HEAVY_HIT1:
				case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence += 3; break;
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				default:
					m_nSequence += 2;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_survival_bowie.mdl")) {
				// Fix animations for the Bowie Knife.
				switch (m_nSequence) {
				case SEQUENCE_DEFAULT_DRAW:
				case SEQUENCE_DEFAULT_IDLE1:
					break;
				case SEQUENCE_DEFAULT_IDLE2:
					m_nSequence = SEQUENCE_BOWIE_IDLE1; break;
				default:
					m_nSequence--;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_ursus.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_DRAW:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
					break;
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
					break;
				default:
					m_nSequence++;
					break;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_stiletto.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(12, 13);
					break;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_widowmaker.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(14, 15);
					break;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_css.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = 15;
					break;
				}
			}
			else if (!strcmp(szModel, "models/weapons/v_knife_cord.mdl") ||
				!strcmp(szModel, "models/weapons/v_knife_canis.mdl") ||
				!strcmp(szModel, "models/weapons/v_knife_outdoor.mdl") ||
				!strcmp(szModel, "models/weapons/v_knife_skeleton.mdl"))
			{
				switch (m_nSequence)
				{
				case SEQUENCE_DEFAULT_DRAW:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
					break;
				case SEQUENCE_DEFAULT_LOOKAT01:
					m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
					break;
				default:
					m_nSequence++;
				}
			}
			// Set the fixed sequence.
			pData->m_Value.m_Int = m_nSequence;
		}
	}

	// Call original function with the modified data.
	fnSequenceProxyFn(pData, pStruct, pOut);

}

void SkinChanger::AnimationFixHook()
{
	for (ClientClass* pClass = g_CHLClient->GetAllClasses(); pClass; pClass = pClass->m_pNext) {
		if (!strcmp(pClass->m_pNetworkName, "CBaseViewModel")) {
			// Search for the 'm_nModelIndex' property.
			RecvTable* pClassTable = pClass->m_pRecvTable;

			for (int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++) {
				RecvProp* pProp = &pClassTable->m_pProps[nIndex];

				if (!pProp || strcmp(pProp->m_pVarName, "m_nSequence"))
					continue;

				// Store the original proxy function.
				fnSequenceProxyFn = static_cast<RecvVarProxyFn>(pProp->m_ProxyFn);

				// Replace the proxy function with our sequence changer.
				pProp->m_ProxyFn = static_cast<RecvVarProxyFn>(SetViewModelSequence2);

				break;
			}

			break;
		}
	}
}

void SkinChanger::AnimationFixUnhook()
{
	for (ClientClass* pClass = g_CHLClient->GetAllClasses(); pClass; pClass = pClass->m_pNext) {
		if (!strcmp(pClass->m_pNetworkName, "CBaseViewModel")) {
			// Search for the 'm_nModelIndex' property.
			RecvTable* pClassTable = pClass->m_pRecvTable;

			for (int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++) {
				RecvProp* pProp = &pClassTable->m_pProps[nIndex];

				if (!pProp || strcmp(pProp->m_pVarName, "m_nSequence"))
					continue;

				// Replace the proxy function with our sequence changer.
				pProp->m_ProxyFn = fnSequenceProxyFn;

				break;
			}

			break;
		}
	}
}