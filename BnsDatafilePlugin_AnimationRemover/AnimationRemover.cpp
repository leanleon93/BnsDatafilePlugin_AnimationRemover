#include "DatafilePluginsdk.h"
#include "PluginConfig.h"
#include "SkillIdManager.h"
#ifdef _BNSLIVE
#include <LIVE/Generated/include/EU/BnsTableNames.h>
#include <LIVE/Generated/include/KR/BnsTableNames.h>
#include <LIVE/Generated/include/EU/skillshow3/AAA_skillshow3_RecordBase.h>
#include <LIVE/Generated/include/KR/skillshow3/AAA_skillshow3_RecordBase.h>
#include <LIVE/Generated/include/EU/phantomsword3/AAA_phantomsword3_RecordBase.h>
#include <LIVE/Generated/include/KR/phantomsword3/AAA_phantomsword3_RecordBase.h>
#include <LIVE/Generated/include/EU/job/AAA_job_RecordBase.h>
#include <LIVE/Generated/include/KR/job/AAA_job_RecordBase.h>
#include <LIVE/Generated/include/EU/stance/AAA_stance_RecordBase.h>
#include <LIVE/Generated/include/KR/stance/AAA_stance_RecordBase.h>
#else
#include <NEO/Generated/include/EU/BnsTableNames.h>
#include <NEO/Generated/include/KR/BnsTableNames.h>
#include <NEO/Generated/include/EU/skillshow3/AAA_skillshow3_RecordBase.h>
#include <NEO/Generated/include/KR/skillshow3/AAA_skillshow3_RecordBase.h>
#include <NEO/Generated/include/EU/phantomsword3/AAA_phantomsword3_RecordBase.h>
#include <NEO/Generated/include/KR/phantomsword3/AAA_phantomsword3_RecordBase.h>
#include <NEO/Generated/include/EU/job/AAA_job_RecordBase.h>
#include <NEO/Generated/include/KR/job/AAA_job_RecordBase.h>
#include <NEO/Generated/include/EU/stance/AAA_stance_RecordBase.h>
#include <NEO/Generated/include/KR/stance/AAA_stance_RecordBase.h>
#endif
#include <string>
#include "plugin_version.h"
#include "hooks.h"
#include "xorstr.hpp"

static int g_panelHandle = 0;
static int g_panelHandle2 = 0;
static RegisterImGuiPanelFn g_register = nullptr;
static UnregisterImGuiPanelFn g_unregister = nullptr;
static PluginImGuiAPI* g_imgui = nullptr;

static Data::DataManager* g_dataManager = nullptr;

#ifdef _BNSEU
static void RemoveAnimationsForRecord(BnsTables::EU::skillshow3_Record* record)
#elif _BNSKR
static void RemoveAnimationsForRecord(BnsTables::KR::skillshow3_Record* record)
#endif
{
	auto setToNull = [](wchar_t* member) {
		*member = L'\0';
		};

	// Iterate over the member variables of the record and set them to L'\0'
	for (wchar_t* member : {
		record->player_cast_show, record->player_exec_show_1, record->player_exec_show_2,
		record->player_exec_show_3, record->player_exec_show_4, record->player_exec_show_5,
		record->player_fire_show_1, record->player_fire_show_2, record->player_fire_show_3,
		record->player_fire_show_4, record->player_fire_show_5, record->player_stop_show,
		record->cast_show, record->decal_show_1, record->fire_show_1, record->fire_miss_show_1,
		record->exec_show_1, record->exec_hit_show_1, record->exec_immune_show_1,
		record->exec_dodge_show_1, record->exec_parry_show_1, record->exec_bounce_show_1,
		record->exec_bounce_caster_show_1, record->exec_counter_show_1, record->exec_critical_show_1,
		record->return_show_1, record->decal_show_2, record->fire_show_2, record->fire_miss_show_2,
		record->exec_show_2, record->exec_hit_show_2, record->exec_immune_show_2,
		record->exec_dodge_show_2, record->exec_parry_show_2, record->exec_bounce_show_2,
		record->exec_bounce_caster_show_2, record->exec_counter_show_2, record->exec_critical_show_2,
		record->return_show_2, record->decal_show_3, record->fire_show_3, record->fire_miss_show_3,
		record->exec_show_3, record->exec_hit_show_3, record->exec_immune_show_3,
		record->exec_dodge_show_3, record->exec_parry_show_3, record->exec_bounce_show_3,
		record->exec_bounce_caster_show_3, record->exec_counter_show_3, record->exec_critical_show_3,
		record->return_show_3, record->decal_show_4, record->fire_show_4, record->fire_miss_show_4,
		record->exec_show_4, record->exec_hit_show_4, record->exec_immune_show_4,
		record->exec_dodge_show_4, record->exec_parry_show_4, record->exec_bounce_show_4,
		record->exec_bounce_caster_show_4, record->exec_counter_show_4, record->exec_critical_show_4,
		record->return_show_4, record->decal_show_5, record->fire_show_5, record->fire_miss_show_5,
		record->exec_show_5, record->exec_hit_show_5, record->exec_immune_show_5,
		record->exec_dodge_show_5, record->exec_parry_show_5, record->exec_bounce_show_5,
		record->exec_bounce_caster_show_5, record->exec_counter_show_5, record->exec_critical_show_5,
		record->return_show_5, record->finish_show, record->bomb_show, record->cancel_show,
		record->stop_show, record->triggered_show, record->trigger_cast_show,
		record->triggered_enemy_show, record->trigger_show, record->trigger_hit_show,
		record->trigger_immune_show, record->trigger_dodge_show, record->trigger_parry_show,
		record->trigger_bounce_show, record->trigger_counter_show, record->trigger_critical_show
		}) {
		setToNull(member);
	}
	record->create_phantom_type = (signed char)0;
	record->lyn_phantom_count = 0;
}

#ifdef _BNSEU
static void SwapAnimationsForRecords(BnsTables::EU::skillshow3_Record* record1, BnsTables::EU::skillshow3_Record* record2)
#elif _BNSKR
static void SwapAnimationsForRecords(BnsTables::KR::skillshow3_Record* record1, BnsTables::KR::skillshow3_Record* record2)
#endif
{
	for (auto [member1, member2] : {
		//add all from above
		std::pair<wchar_t*&, wchar_t*&>(record1->player_cast_show, record2->player_cast_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_exec_show_1, record2->player_exec_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_exec_show_2, record2->player_exec_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_exec_show_3, record2->player_exec_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_exec_show_4, record2->player_exec_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_exec_show_5, record2->player_exec_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_fire_show_1, record2->player_fire_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_fire_show_2, record2->player_fire_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_fire_show_3, record2->player_fire_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_fire_show_4, record2->player_fire_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_fire_show_5, record2->player_fire_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->player_stop_show, record2->player_stop_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->cast_show, record2->cast_show),

		std::pair<wchar_t*&, wchar_t*&>(record1->decal_show_1, record2->decal_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->fire_show_1, record2->fire_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->fire_miss_show_1, record2->fire_miss_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_show_1, record2->exec_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_hit_show_1, record2->exec_hit_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_immune_show_1, record2->exec_immune_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_dodge_show_1, record2->exec_dodge_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_parry_show_1, record2->exec_parry_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_bounce_show_1, record2->exec_bounce_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_bounce_caster_show_1, record2->exec_bounce_caster_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_counter_show_1, record2->exec_counter_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_critical_show_1, record2->exec_critical_show_1),
		std::pair<wchar_t*&, wchar_t*&>(record1->return_show_1, record2->return_show_1),

		std::pair<wchar_t*&, wchar_t*&>(record1->decal_show_2, record2->decal_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->fire_show_2, record2->fire_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->fire_miss_show_2, record2->fire_miss_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_show_2, record2->exec_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_hit_show_2, record2->exec_hit_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_immune_show_2, record2->exec_immune_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_dodge_show_2, record2->exec_dodge_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_parry_show_2, record2->exec_parry_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_bounce_show_2, record2->exec_bounce_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_bounce_caster_show_2, record2->exec_bounce_caster_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_counter_show_2, record2->exec_counter_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_critical_show_2, record2->exec_critical_show_2),
		std::pair<wchar_t*&, wchar_t*&>(record1->return_show_2, record2->return_show_2),

		std::pair<wchar_t*&, wchar_t*&>(record1->decal_show_3, record2->decal_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->fire_show_3, record2->fire_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->fire_miss_show_3, record2->fire_miss_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_show_3, record2->exec_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_hit_show_3, record2->exec_hit_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_immune_show_3, record2->exec_immune_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_dodge_show_3, record2->exec_dodge_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_parry_show_3, record2->exec_parry_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_bounce_show_3, record2->exec_bounce_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_bounce_caster_show_3, record2->exec_bounce_caster_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_counter_show_3, record2->exec_counter_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_critical_show_3, record2->exec_critical_show_3),
		std::pair<wchar_t*&, wchar_t*&>(record1->return_show_3, record2->return_show_3),

		std::pair<wchar_t*&, wchar_t*&>(record1->decal_show_4, record2->decal_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->fire_show_4, record2->fire_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->fire_miss_show_4, record2->fire_miss_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_show_4, record2->exec_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_hit_show_4, record2->exec_hit_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_immune_show_4, record2->exec_immune_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_dodge_show_4, record2->exec_dodge_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_parry_show_4, record2->exec_parry_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_bounce_show_4, record2->exec_bounce_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_bounce_caster_show_4, record2->exec_bounce_caster_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_counter_show_4, record2->exec_counter_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_critical_show_4, record2->exec_critical_show_4),
		std::pair<wchar_t*&, wchar_t*&>(record1->return_show_4, record2->return_show_4),

		std::pair<wchar_t*&, wchar_t*&>(record1->decal_show_5, record2->decal_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->fire_show_5, record2->fire_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->fire_miss_show_5, record2->fire_miss_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_show_5, record2->exec_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_hit_show_5, record2->exec_hit_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_immune_show_5, record2->exec_immune_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_dodge_show_5, record2->exec_dodge_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_parry_show_5, record2->exec_parry_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_bounce_show_5, record2->exec_bounce_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_bounce_caster_show_5, record2->exec_bounce_caster_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_counter_show_5, record2->exec_counter_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->exec_critical_show_5, record2->exec_critical_show_5),
		std::pair<wchar_t*&, wchar_t*&>(record1->return_show_5, record2->return_show_5),

		std::pair<wchar_t*&, wchar_t*&>(record1->finish_show, record2->finish_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->bomb_show, record2->bomb_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->cancel_show, record2->cancel_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->stop_show, record2->stop_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->triggered_show, record2->triggered_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->trigger_cast_show, record2->trigger_cast_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->triggered_enemy_show, record2->triggered_enemy_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->trigger_show, record2->trigger_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->trigger_hit_show, record2->trigger_hit_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->trigger_immune_show, record2->trigger_immune_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->trigger_dodge_show, record2->trigger_dodge_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->trigger_parry_show, record2->trigger_parry_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->trigger_bounce_show, record2->trigger_bounce_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->trigger_counter_show, record2->trigger_counter_show),
		std::pair<wchar_t*&, wchar_t*&>(record1->trigger_critical_show, record2->trigger_critical_show)
		}) {
		member1 = member2;
	}
	record1->create_phantom_type = record2->create_phantom_type;
	record1->lyn_phantom_count = record2->lyn_phantom_count;
	record1->phantom_shoot_type = record2->phantom_shoot_type;
}

//static constexpr const wchar_t* fullParticleRef = L"00027869.UI_Shooter_gauge";
//
//static PluginReturnData __fastcall StanceDetour(PluginExecuteParams* params) {
//	if (!g_PluginConfig->AnimFilterEnabled() || !g_PluginConfig->IsLoaded() || !g_PluginConfig->HasActiveProfile() || !g_SkillIdManager->IsSetupComplete()) {
//		return {};
//	}
//#ifdef _BNSEU
//	PLUGIN_DETOUR_GUARD(params, BnsTables::EU::TableNames::GetTableVersion);
//	auto stance = (BnsTables::EU::stance_Record*)params->oFind(params->table, params->key);
//#elif _BNSKR
//	PLUGIN_DETOUR_GUARD(params, BnsTables::KR::TableNames::GetTableVersion);
//	auto stance = (BnsTables::KR::stance_Record*)params->oFind(params->table, params->key);
//#endif
//	stance->sp_gauge_style = (signed char)BnsTables::EU::stance_Record::sp_gauge_style::bar_1;
//	stance->sp_gauge_numeric_visible = true;
//
//	stance->sp_gauge_full_particle_ref = const_cast<wchar_t*>(fullParticleRef);
//	stance->sp_gauge_charge_particle_ref = const_cast<wchar_t*>(fullParticleRef);
//	stance->sp_gauge_consume_particle_ref = const_cast<wchar_t*>(fullParticleRef);
//	stance->sp_gauge_charge_observer_particle_ref = const_cast<wchar_t*>(fullParticleRef);
//	stance->sp_gauge_consume_observer_particle_ref = const_cast<wchar_t*>(fullParticleRef);
//
//	stance->sp_gauge_full_particle_scale = 0.8f;
//	stance->sp_gauge_charge_particle_scale = 0.8f;
//	stance->sp_gauge_consume_particle_scale = 0.8f;
//	stance->sp_gauge_charge_observer_particle_scale = 0.8f;
//	stance->sp_gauge_consume_observer_particle_scale = 0.8f;
//	return {};
//}

static PluginReturnData __fastcall JobDetour(PluginExecuteParams* params) {
	if (!g_PluginConfig->AnimFilterEnabled() || !g_PluginConfig->IsLoaded() || !g_PluginConfig->HasActiveProfile() || !g_SkillIdManager->IsSetupComplete()) {
		return {};
	}
	static std::unordered_map<uint64_t, signed char> originalPhantomWeaponActiveTypes = {};
	static std::unordered_map<uint64_t, short> originalPhantomWeaponActive1Flags = {};

#ifdef _BNSEU
	PLUGIN_DETOUR_GUARD(params, BnsTables::EU::TableNames::GetTableVersion);
	auto job = (BnsTables::EU::job_Record*)params->oFind(params->table, params->key);
#elif _BNSKR
	PLUGIN_DETOUR_GUARD(params, BnsTables::KR::TableNames::GetTableVersion);
	auto job = (BnsTables::KR::job_Record*)params->oFind(params->table, params->key);
#endif
	if (job == nullptr) return {};
	const auto& profile = g_PluginConfig->GetActiveProfile();
	auto skillOptions = profile.GetJobSkillOption(job->key.job);
	bool bmProjectilResistCase = false;
	if (job->key.job == 1 && !profile.HideProjectileResists) {
		bmProjectilResistCase = true;
	}
	if (skillOptions.IsHideAll()) {
		if (bmProjectilResistCase) {
			if (originalPhantomWeaponActive1Flags.find(job->key.key) == originalPhantomWeaponActive1Flags.end()) {
				originalPhantomWeaponActive1Flags[job->key.key] = job->phantom_weapon_active_effect_flag_1st;
			}
			job->phantom_weapon_active_effect_flag_1st = (short)BnsTables::EU::job_Record::phantom_weapon_active_effect_flag_1st::smokescreen;
			job->phantom_weapon_count = 2;
		}
		else {
			//store original value if not already stored
			if (originalPhantomWeaponActiveTypes.find(job->key.key) == originalPhantomWeaponActiveTypes.end()) {
				originalPhantomWeaponActiveTypes[job->key.key] = job->phantom_weapon_active_type;
			}
			job->phantom_weapon_active_type = (signed char)0;
		}
	}
	else {
		//restore original value if it was stored
		if (originalPhantomWeaponActiveTypes.find(job->key.key) != originalPhantomWeaponActiveTypes.end()) {
			job->phantom_weapon_active_type = originalPhantomWeaponActiveTypes[job->key.key];
			originalPhantomWeaponActiveTypes.erase(job->key.key);
		}
		if (originalPhantomWeaponActive1Flags.find(job->key.key) != originalPhantomWeaponActive1Flags.end()) {
			job->phantom_weapon_active_effect_flag_1st = originalPhantomWeaponActive1Flags[job->key.key];
			originalPhantomWeaponActive1Flags.erase(job->key.key);
			job->phantom_weapon_count = 6;
		}
	}
	return {};
}

static PluginReturnData __fastcall Phantomsword3Detour(PluginExecuteParams* params) {
#ifdef _BNSEU
	PLUGIN_DETOUR_GUARD(params, BnsTables::EU::TableNames::GetTableVersion);
	static BnsTables::EU::phantomsword3_Record emptyRecord = {};
#elif _BNSKR
	PLUGIN_DETOUR_GUARD(params, BnsTables::KR::TableNames::GetTableVersion);
	static BnsTables::KR::phantomsword3_Record emptyRecord = {};
#endif
	if (!g_PluginConfig->AnimFilterEnabled() || !g_PluginConfig->IsLoaded() || !g_PluginConfig->HasActiveProfile() || !g_SkillIdManager->IsSetupComplete()) {
		return {};
	}
	const auto& ids = g_SkillIdManager->GetIdsToFilter();

	auto key = BnsTables::EU::phantomsword3_Record::Key{};
	key.key = params->key;
	const auto skillId = key.id;
	if (!ids.contains(skillId)) return {};
	return { (DrEl*)&emptyRecord };
}

static bool reloadRequired = false;
static PluginReturnData __fastcall Skillshow3Detour(PluginExecuteParams* params) {
#ifdef _BNSEU
	PLUGIN_DETOUR_GUARD(params, BnsTables::EU::TableNames::GetTableVersion);
#elif _BNSKR
	PLUGIN_DETOUR_GUARD(params, BnsTables::KR::TableNames::GetTableVersion);
#endif
	if (!g_PluginConfig->AnimFilterEnabled() || !g_PluginConfig->IsLoaded() || !g_PluginConfig->HasActiveProfile() || !g_SkillIdManager->IsSetupComplete()) {
		return {};
	}
	const auto& ids = g_SkillIdManager->GetIdsToFilter();
	const auto skillId = static_cast<int32_t>(params->key);

#ifdef _SKILLSWAP
	const auto& profile = g_PluginConfig->GetActiveProfile();
	if (const auto& swap = profile.AnimationSwaps.find(skillId); swap != profile.AnimationSwaps.end()) {
		auto animationSwapKey = SkillIdManager::SkillShow3KeyHelper::BuildKey(swap->second.AnimationSkillId, (short)swap->second.AnimationSkillVariation, 0);
		auto animSwapRecordBase = params->oFind(params->table, animationSwapKey);
		if (animSwapRecordBase == nullptr) return {};
#ifdef _BNSEU
		auto animSwapRecord = (BnsTables::EU::skillshow3_Record*)animSwapRecordBase;
#elif _BNSKR
		auto animSwapRecord = (BnsTables::KR::skillshow3_Record*)animSwapRecordBase;
#endif
		auto recordBase = params->oFind(params->table, params->key);
		if (recordBase == nullptr) return {};
#ifdef _BNSEU
		auto record = (BnsTables::EU::skillshow3_Record*)recordBase;
#elif _BNSKR
		auto record = (BnsTables::KR::skillshow3_Record*)recordBase;
#endif
		SwapAnimationsForRecords(record, animSwapRecord);
		reloadRequired = true;
		return { recordBase };
	}
#endif

	if (!ids.contains(skillId)) return {};
	if (const auto& taxiIds = g_SkillIdManager->GetTaxiSkillIds(); taxiIds.contains(skillId)) {
		const auto& taxiIdVariations = g_SkillIdManager->GetTaxiExclusionIdVariations();
		const auto skillshowKey = SkillIdManager::SkillShow3KeyHelper::ExtractKey(params->key);
		if (taxiIdVariations.at(skillId) == skillshowKey.variation_id) return {};
	}
	auto recordBase = params->oFind(params->table, params->key);
	if (recordBase == nullptr) return {};
#ifdef _BNSEU
	auto record = (BnsTables::EU::skillshow3_Record*)recordBase;
#elif _BNSKR
	auto record = (BnsTables::KR::skillshow3_Record*)recordBase;
#endif
	RemoveAnimationsForRecord(record);
	reloadRequired = true;
	return { recordBase };
}

static void ReloadSkillShow3() {
	if (reloadRequired) {
		auto table = GetTable(g_dataManager, L"skillshow3");
		if (table != nullptr) {
			//if (g_PluginConfig->GetAnimFilterConfig().ExperimentalMemoryLoading) {
			//	table->__vftable->SetUseLowMemory(table, false);
			//}
			auto it = table->__vftable->createInnerIter(table);
			//cycle the cache
			do {
				if (!it->_vtptr->IsValid(it)) continue;
#ifdef _BNSEU
				if (auto record = (BnsTables::EU::skillshow3_Record*)it->_vtptr->Ptr(it); record == nullptr) continue;
#elif _BNSKR
				if (auto record = (BnsTables::KR::skillshow3_Record*)it->_vtptr->Ptr(it); record == nullptr) continue;
#endif
			} while (it->_vtptr->Next(it));
			reloadRequired = false;
		}
	}
}

static void ReloadConfig() {
	g_PluginConfig->ReloadFromConfig();
	g_SkillIdManager->ResetIdsToFilter();
	g_SkillIdManager->ReapplyEffectFilters();
	ReloadSkillShow3();
	//maybe print text chat message
}

static void SetProfile(int profileId) {
	g_PluginConfig->SetActiveFilter(profileId);
	g_SkillIdManager->ResetIdsToFilter();
	g_SkillIdManager->ReapplyEffectFilters();
	ReloadSkillShow3();
	if (!g_PluginConfig->AnimFilterEnabled()) return;
	//maybe print text chat message
}

#include <windows.h>

// Converts a std::wstring to a std::string (UTF-8)
inline static std::string WStringToString(const std::wstring& wstr) {
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
	return strTo;
}

//std::string(UTF-8) to std::wstring
inline static std::wstring StringToWString(const std::string& str) {
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

inline static std::vector<const char*> ToCStringVector(const std::vector<std::string>& strings) {
	std::vector<const char*> cstrs;
	cstrs.reserve(strings.size());
	for (const auto& s : strings) {
		cstrs.push_back(s.c_str());
	}
	return cstrs;
}

static void ProfileEditorUiPanel(void* userData) {
	static bool profileListDirty = true;
	// Get the active profile (edit in-place)
	auto& config = g_PluginConfig->GetAnimFilterConfigEditable();
	auto& profiles = config.Profiles;

	static int selectedProfileId = -1;
	static std::vector<int> profileIds;
	static std::vector<std::string> profileTexts;

	// Build profile id/name lists if changed
	if (profileListDirty || profileIds.size() != profiles.size()) {
		profileIds.clear();
		profileTexts.clear();
		for (const auto& kv : profiles) {
			profileIds.push_back(kv.first);
			profileTexts.push_back(WStringToString(kv.second.Text));
		}
		if (!profileIds.empty() && (selectedProfileId == -1 || profiles.find(selectedProfileId) == profiles.end())) {
			selectedProfileId = profileIds[0];
		}
		profileListDirty = false;
	}

	// Combo box for profile selection
	int currentIndex = 0;
	for (size_t i = 0; i < profileIds.size(); ++i) {
		if (profileIds[i] == selectedProfileId) {
			currentIndex = static_cast<int>(i);
			break;
		}
	}


	if (g_imgui->Button("Add New Profile")) {
		//get the highest profile id
		int newProfileId = 1;
		for (const auto& kv : profiles) {
			if (kv.first >= newProfileId) {
				newProfileId = kv.first + 1;
			}
		}
		g_PluginConfig->AddHideShowAllDefaultProfile(true, newProfileId, std::wstring(L"New Profile ") + std::to_wstring(newProfileId));
		profileListDirty = true;
	}
	g_imgui->Spacing();
	g_imgui->Separator();
	g_imgui->Spacing();
	auto profileTextCtrs = ToCStringVector(profileTexts);
	if (g_imgui->Combo("Select Profile", &currentIndex, profileTextCtrs.data(), profileTextCtrs.size(), profileTextCtrs.size())) {
		selectedProfileId = profileIds[currentIndex];
	}


	// If no profile is selected, return
	if (selectedProfileId == -1 || profiles.find(selectedProfileId) == profiles.end())
		return;

	if (g_imgui->Button("Set as default Profile")) {
		g_PluginConfig->SetDefaultProfile(selectedProfileId);
		g_PluginConfig->SaveToDisk();
	}

	g_imgui->Spacing();
	g_imgui->Separator();
	g_imgui->Spacing();

	auto& profile = profiles[selectedProfileId];

	// Text (std::wstring)
	std::string textUtf8 = WStringToString(profile.Text);
	char textBuf[512] = {};
	strncpy_s(textBuf, textUtf8.c_str(), sizeof(textBuf) - 1);
	if (g_imgui->InputText("Profile Name", textBuf, sizeof(textBuf))) {
		profile.Text = StringToWString(textBuf);
		profileListDirty = true;
	}

	g_imgui->Spacing();

	// Booleans
	g_imgui->Columns(2, nullptr, false); // 2 columns for better alignment

#ifndef _BNSLIVE
	g_imgui->Checkbox("Hide Grab Skills", &profile.HideGrabs);
	g_imgui->NextColumn();
#endif

	g_imgui->Checkbox("Hide Time Distortion", &profile.HideTimeDistortion);
	g_imgui->NextColumn();

	g_imgui->Checkbox("Hide Taxi", &profile.HideTaxi);
	g_imgui->NextColumn();

	g_imgui->Checkbox("Hide Global Item Skills", &profile.HideGlobalItemSkills);
	g_imgui->NextColumn();

#ifdef _BNSLIVE
	g_imgui->Checkbox("Hide Bard Tree", &profile.HideTree);
	g_imgui->NextColumn();
#else
	g_imgui->Checkbox("Hide all Soul Cores", &profile.HideSoulCores);
	g_imgui->NextColumn();
	g_imgui->Checkbox("Hide others Soul Cores", &profile.HideOthersSoulCores);
	g_imgui->NextColumn();
#endif

	g_imgui->Checkbox("Hide all Skills from other Players (beta)", &profile.HideAllOtherPlayerSkills);
	g_imgui->NextColumn();

#ifndef _BNSLIVE
	g_imgui->Checkbox("Hide Projectile Resists", &profile.HideProjectileResists);
	g_imgui->NextColumn();
#endif

	g_imgui->Columns(1, nullptr, false); // Reset to single column
	g_imgui->Spacing();

	//Skill options
	if (g_imgui->CollapsingHeader("Show skill animations for ")) {
		g_imgui->Spacing();
		int idx = 0;
#ifdef _BNSLIVE
		// Set up 4 columns: Job, 1st Spec, 2nd Spec, 3rd Spec
		g_imgui->Columns(4, nullptr, false);
		g_imgui->Text("Job"); g_imgui->NextColumn();
		g_imgui->Text("1st Spec"); g_imgui->NextColumn();
		g_imgui->Text("2nd Spec"); g_imgui->NextColumn();
		g_imgui->Text("3rd Spec"); g_imgui->NextColumn();

#endif
		for (auto& skill : profile.SkillFilters) {
#ifdef _BNSLIVE
			if (skill.Job == 13) { //skip live spearmaster
				continue;
			}
#else
			if (g_SkillIdManager->neoJobAvailability.at(skill.Job) == false) {
				continue;
			}
#endif
			g_imgui->PushIdInt(idx);
#ifndef _BNSLIVE
			g_imgui->Indent(10.0f);
#endif
#ifdef _BNSLIVE
			// Job name
			g_imgui->Text("%s", WStringToString(skill.Name).c_str());
			g_imgui->NextColumn();

			// 1st Spec
			bool showSpec1 = !skill.HideSpec1;
			if (g_imgui->Checkbox("##Spec1", &showSpec1)) {
				skill.HideSpec1 = !showSpec1;
			}
			g_imgui->NextColumn();

			// 2nd Spec
			bool showSpec2 = !skill.HideSpec2;
			if (g_imgui->Checkbox("##Spec2", &showSpec2)) {
				skill.HideSpec2 = !showSpec2;
			}
			g_imgui->NextColumn();

			// 3rd Spec
			bool showSpec3 = !skill.HideSpec3;
			if (g_imgui->Checkbox("##Spec3", &showSpec3)) {
				skill.HideSpec3 = !showSpec3;
			}
			g_imgui->NextColumn();
#else
			bool showAll = !skill.HideSpec1 && !skill.HideSpec2 && !skill.HideSpec3;
			if (g_imgui->Checkbox(WStringToString(skill.Name).c_str(), &showAll)) {
				skill.HideSpec1 = !showAll;
				skill.HideSpec2 = !showAll;
				skill.HideSpec3 = !showAll;
			}
#endif
#ifndef _BNSLIVE
			g_imgui->Unindent(10.0f);
#endif
			g_imgui->PopId();
			++idx;
		}
#ifdef _BNSLIVE
		g_imgui->Columns(1, nullptr, false); // Reset to single column
#endif
	}

	g_imgui->Spacing();
	g_imgui->Separator();
	g_imgui->Spacing();

	if (g_imgui->CollapsingHeader("Hide specific skills by id")) {
		g_imgui->TextColored(0.6f, 0.6f, 0.6f, 1.0f, "These overrule all other settings.");
		g_imgui->Spacing();
		int idx = 0;
		for (auto& [skillId, text] : profile.CustomSkillIdFilters) {
			g_imgui->PushIdInt(idx);
			g_imgui->Indent(10.0f);
			std::string skillText = text.empty()
				? ""
				: ("(" + text + ")");
			g_imgui->Text("Skill: %d %s", skillId, skillText.c_str());
			g_imgui->SameLineDefault();
			if (g_imgui->SmallButton("Remove")) {
				g_PluginConfig->RemoveCustomSkillId(selectedProfileId, skillId);
				g_imgui->Unindent(10.0f);
				g_imgui->PopId();
				break;
			}
			g_imgui->Unindent(10.0f);
			g_imgui->PopId();
			++idx;
		}
		g_imgui->Spacing();
		static char newSkillIdBuf[16] = {};
		static char newSkillTextBuf[128] = {};
		g_imgui->InputText("Skill Id", newSkillIdBuf, sizeof(newSkillIdBuf));
		g_imgui->InputText("Text (optional)", newSkillTextBuf, sizeof(newSkillTextBuf));
		if (g_imgui->Button("Add")) {
			int newSkillId = atoi(newSkillIdBuf);
			if (newSkillId != 0) {
				g_PluginConfig->AddCustomSkillId(selectedProfileId, newSkillId, std::string(newSkillTextBuf));
				newSkillIdBuf[0] = '\0';
				newSkillTextBuf[0] = '\0';
			}
		}
	}
	g_imgui->Spacing();
	g_imgui->Separator();
	g_imgui->Spacing();
	static bool saveFeedback = false;
	static int saveFeedbackFrames = 0;
	const int FEEDBACK_DISPLAY_FRAMES = 120; // ~2 seconds at 60 FPS

	if (g_imgui->Button("Save Profile")) {
		g_PluginConfig->SaveToDisk();
		g_PluginConfig->SetActiveFilter(selectedProfileId);
		g_SkillIdManager->ResetIdsToFilter();
		g_SkillIdManager->ReapplyEffectFilters();
		ReloadSkillShow3();
		saveFeedback = true;
		saveFeedbackFrames = FEEDBACK_DISPLAY_FRAMES;
	}
	g_imgui->SameLineDefault();
	if (g_imgui->Button("Delete Profile")) {
		g_PluginConfig->DeleteProfile(selectedProfileId);
		g_PluginConfig->SaveToDisk();
		int profile0 = profileIds.empty() ? -1 : profileIds[0];
		if (profile0 == selectedProfileId) {
			profile0 = profileIds.size() > 1 ? profileIds[1] : -1;
		}
		g_PluginConfig->SetActiveFilter(profile0);
		g_SkillIdManager->ResetIdsToFilter();
		g_SkillIdManager->ReapplyEffectFilters();
		ReloadSkillShow3();
		selectedProfileId = -1;
		profileListDirty = true;
	}

	if (saveFeedback && saveFeedbackFrames > 0) {
		g_imgui->TextColored(0.0f, 1.0f, 0.0f, 1.0f, "Profile saved!");
		--saveFeedbackFrames;
		if (saveFeedbackFrames == 0) {
			saveFeedback = false;
		}
	}
}

static void UiPanel(void* userData) {
	if (g_SkillIdManager->IsCriticalFail()) {
		g_imgui->TextColored(1.0f, 0.0f, 0.0f, 1.0f, "Critical error in Animation Filter! Plugin may not work correctly.");
		g_imgui->Spacing();
		g_imgui->Separator();
	}
	if (!g_SkillIdManager->IsSetupComplete()) {
		g_imgui->TextColored(1.0f, 1.0f, 0.0f, 1.0f, "Animation Filter setup in progress, please wait...");
		return;
	}

	// Section: Enable
	bool enabled = g_PluginConfig->AnimFilterEnabled();
	if (g_imgui->Checkbox("Enable", &enabled)) {
		g_PluginConfig->SetEnabled(enabled);
		g_SkillIdManager->ResetIdsToFilter();
		g_SkillIdManager->ReapplyEffectFilters();
		ReloadSkillShow3();
	}
	g_imgui->Spacing();
	g_imgui->Separator();
	g_imgui->Spacing();
	// Section: Char Info
	//if (g_PluginConfig->getWorld != nullptr) {
	//	auto* world = g_PluginConfig->getWorld();
	//	if (world != nullptr && world->_player != nullptr) {
	//		auto player = g_PluginConfig->getWorld()->_player;
	//		auto playerCreature = (Creature*)player;
	//		auto playerName = playerCreature->name.str;
	//		g_imgui->TextColored(0.7f, 0.7f, 1.0f, 1.0f, "Character:");
	//		g_imgui->SameLineDefault();
	//		g_imgui->TextColored(0.0f, 1.0f, 0.0f, 1.0f, WStringToString(playerName).c_str());
	//		auto playerJob = playerCreature->job;
	//		g_imgui->TextColored(0.7f, 0.7f, 1.0f, 1.0f, "Job:");
	//		g_imgui->SameLineDefault();
	//		g_imgui->TextColored(0.0f, 1.0f, 0.0f, 1.0f, WStringToString(g_SkillIdManager->customJobAbbreviations.at(playerJob)).c_str());
	//		g_imgui->Spacing();
	//		g_imgui->Separator();
	//		g_imgui->Spacing();
	//	}
	//}

	// Section: Active Profile
	g_imgui->TextColored(0.7f, 0.7f, 1.0f, 1.0f, "Active Profile:");
	if (g_PluginConfig->HasActiveProfile()) {
		const auto& profile = g_PluginConfig->GetActiveProfile();
		g_imgui->SameLineDefault();
		g_imgui->TextColored(0.0f, 1.0f, 0.0f, 1.0f, WStringToString(profile.Text).c_str());
	}
	else {
		g_imgui->SameLineDefault();
		g_imgui->TextColored(1.0f, 0.0f, 0.0f, 1.0f, "None");
	}
	g_imgui->Spacing();
	// Section: Default Profile
	g_imgui->TextColored(0.7f, 0.7f, 1.0f, 1.0f, "Default Profile:");
	const auto& defaultProfileName = g_PluginConfig->GetDefaultProfileName();
	if (!defaultProfileName.empty()) {

		g_imgui->SameLineDefault();
		g_imgui->TextColored(1.0f, 0.65f, 0.0f, 1.0f, WStringToString(defaultProfileName).c_str());
	}
	else {
		g_imgui->SameLineDefault();
		g_imgui->TextColored(1.0f, 0.0f, 0.0f, 1.0f, "None");
	}
	g_imgui->TextColored(0.6f, 0.6f, 0.6f, 1.0f, "This is used on game start.");
	g_imgui->Spacing();

	if (g_PluginConfig->HasActiveProfile()) {
		const auto profileId = g_PluginConfig->GetActiveProfileId();
		if (g_imgui->SmallButton("Set active profile as default profile")) {
			g_PluginConfig->SetDefaultProfile(profileId);
			g_PluginConfig->SaveToDisk();
		}
	}

	g_imgui->Spacing();
	g_imgui->Separator();
	g_imgui->Spacing();
	// Section: Select Profile
	g_imgui->TextColored(0.7f, 0.7f, 1.0f, 1.0f, "Select Profile:");
	g_imgui->Spacing();

	// Use columns for profile buttons
	int numProfiles = static_cast<int>(g_PluginConfig->GetAnimFilterConfig().Profiles.size());
	int columns = numProfiles > 4 ? 3 : numProfiles; // 3 columns if many profiles
	g_imgui->Columns(columns, nullptr, false);
	for (const auto& profile : g_PluginConfig->GetAnimFilterConfig().Profiles) {
		if (g_imgui->CustomButton(WStringToString(profile.second.Text).c_str(), -FLT_MIN, 0)) {
			SetProfile(profile.first);
		}
		g_imgui->NextColumn();
	}
	g_imgui->Columns(1, nullptr, false);
	g_imgui->Spacing();

	// Section: Actions
	g_imgui->Separator();
	g_imgui->Spacing();

	static bool window_open = false;
	if (g_imgui->CustomButton("Open Profile Editor", 180, 0)) {
		window_open = true;
	}
	g_imgui->SameLineDefault();
	if (g_imgui->CustomButton("Reload Config from file", 180, 0)) {
		ReloadConfig();
	}

	// Profile Editor Window
	if (window_open) {
		g_imgui->Begin("Animation Filter Profile Editor", &window_open, 0);
		ProfileEditorUiPanel(nullptr);
		g_imgui->End();
	}
}

HookFunctionParams hooks[] = {
	{ xorstr_("48 8B 12 48 8B 01 48 8B 52 08 FF 90 ?? 00 00 00 48 8B D8 48 89 47 24"), -0x33, (void**)&oSkillUse, (void*)hkSkillUse, "oSkillUse" }
};

static void __fastcall Init(PluginInitParams* params) {
	if (params && params->registerImGuiPanel && params->unregisterImGuiPanel && params->imgui)
	{
		g_imgui = params->imgui;
		g_register = params->registerImGuiPanel;
		g_unregister = params->unregisterImGuiPanel;
		ImGuiPanelDesc desc = { "Animation Filter", UiPanel, nullptr };
		g_panelHandle = g_register(&desc, false);
	}
	if (params && params->dataManager) {
		g_dataManager = params->dataManager;
		g_SkillIdManager = std::make_unique<SkillIdManager>(params->dataManager);
		g_PluginConfig = std::make_unique<PluginConfig>();
		g_PluginConfig->ReloadFromConfig();
		g_SkillIdManager->SetupAsync();
	}
	if (params && params->getWorld && g_PluginConfig != nullptr) {
		g_PluginConfig->getWorld = params->getWorld;
	}
	if (params && params->registerDetours && params->unregisterDetours) {
		g_PluginConfig->unregisterDetours = params->unregisterDetours;
		params->registerDetours(hooks, sizeof(hooks) / sizeof(hooks[0]));
	}
}

static void __fastcall Unregister() {
	//disable and restore all effects before unload
	if (g_PluginConfig && g_SkillIdManager) {
		g_PluginConfig->SetEnabled(false);
		g_SkillIdManager->ResetIdsToFilter();
		g_SkillIdManager->ReapplyEffectFilters();
		ReloadSkillShow3();
	}
	if (g_PluginConfig->unregisterDetours) {
		g_PluginConfig->unregisterDetours(hooks, sizeof(hooks) / sizeof(hooks[0]));
		g_PluginConfig->unregisterDetours = nullptr;
	}
	if (g_unregister && g_panelHandle != 0) {
		g_unregister(g_panelHandle);
		g_panelHandle = 0;
	}
	if (g_unregister && g_panelHandle2 != 0) {
		g_unregister(g_panelHandle2);
		g_panelHandle2 = 0;
	}
}

PluginTableHandler handlers[] = {
	{ L"skillshow3", &Skillshow3Detour },
	{ L"phantomsword3", &Phantomsword3Detour },
	{ L"job", &JobDetour }
	//{ L"stance", &StanceDetour }
};

DEFINE_PLUGIN_API_VERSION()
DEFINE_PLUGIN_IDENTIFIER("AnimationFilter")
DEFINE_PLUGIN_VERSION(PLUGIN_VERSION)
DEFINE_PLUGIN_INIT(Init, Unregister)
DEFINE_PLUGIN_TABLE_HANDLERS(handlers)