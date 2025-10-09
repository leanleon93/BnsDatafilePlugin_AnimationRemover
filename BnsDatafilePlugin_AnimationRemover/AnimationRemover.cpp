#include "DatafilePluginsdk.h"
#include "PluginConfig.h"
#include "SkillIdManager.h"
#include <EU/BnsTableNames.h>
#include <EU/skillshow3/AAA_skillshow3_RecordBase.h>
#include <string>
#include "plugin_version.h"

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
}
static bool reloadRequired = false;
static PluginReturnData __fastcall Skillshow3Detour(PluginExecuteParams* params) {
	PLUGIN_DETOUR_GUARD(params, BnsTables::EU::TableNames::GetTableVersion);
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
	auto profileTextCtrs = ToCStringVector(profileTexts);
	if (g_imgui->Combo("Select Profile", &currentIndex, profileTextCtrs.data(), profileTextCtrs.size(), profileTextCtrs.size())) {
		selectedProfileId = profileIds[currentIndex];
	}


	// If no profile is selected, return
	if (selectedProfileId == -1 || profiles.find(selectedProfileId) == profiles.end())
		return;

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
	g_imgui->Checkbox("Hide Bard Tree", &profile.HideTree);
	g_imgui->SameLine(0.0f, 10.0f);
	g_imgui->Checkbox("Hide Time Distortion", &profile.HideTimeDistortion);
	g_imgui->SameLine(0.0f, 10.0f);
	g_imgui->Checkbox("Hide Taxi", &profile.HideTaxi);

	g_imgui->Checkbox("Hide Global Item Skills", &profile.HideGlobalItemSkills);
	g_imgui->SameLine(0.0f, 10.0f);
	g_imgui->Checkbox("Hide Soul Cores", &profile.HideSoulCores);
	g_imgui->Spacing();

	//Skill options
	if (g_imgui->CollapsingHeader("Class Skill Animations")) {
		g_imgui->Spacing();
		int idx = 0;
		for (auto& skill : profile.SkillFilters) {
			g_imgui->PushIdInt(idx);
			g_imgui->Indent(10.0f);
			bool showAll = !skill.HideSpec1 && !skill.HideSpec2 && !skill.HideSpec3;
			if (g_imgui->Checkbox(WStringToString(skill.Name).c_str(), &showAll)) {
				skill.HideSpec1 = !showAll;
				skill.HideSpec2 = !showAll;
				skill.HideSpec3 = !showAll;
			}
			g_imgui->Unindent(10.0f);
			g_imgui->PopId();
			++idx;
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
		g_imgui->TextColored(1.0f, 0.0f, 0.0f, 1.0f, "Critical error in SkillIdManager! Plugin may not work correctly.");
		g_imgui->Spacing();
		g_imgui->Separator();
	}
	if (!g_SkillIdManager->IsSetupComplete()) {
		g_imgui->TextColored(1.0f, 1.0f, 0.0f, 1.0f, "SkillIdManager setup in progress, please wait...");
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

	// Section: Current Profile
	g_imgui->TextColored(0.7f, 0.7f, 1.0f, 1.0f, "Current Profile:");
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
	g_imgui->Separator();

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
	if (g_imgui->CustomButton("Reload Config", 120, 0)) {
		ReloadConfig();
	}

	// Profile Editor Window
	if (window_open) {
		g_imgui->Begin("Animation Filter Profile Editor", &window_open, 0);
		ProfileEditorUiPanel(nullptr);
		g_imgui->End();
	}
}



static void __fastcall Init(PluginInitParams* params) {
	if (params && params->registerImGuiPanel && params->unregisterImGuiPanel && params->imgui)
	{
		g_imgui = params->imgui;
		g_register = params->registerImGuiPanel;
		g_unregister = params->unregisterImGuiPanel;
		ImGuiPanelDesc desc = { "Animation Filter", UiPanel, nullptr };
		g_panelHandle = g_register(&desc, false);
	}
	g_PluginConfig = std::make_unique<PluginConfig>();
	g_PluginConfig->ReloadFromConfig();
	if (params && params->dataManager) {
		g_dataManager = params->dataManager;
		g_SkillIdManager = std::make_unique<SkillIdManager>(params->dataManager);
		g_SkillIdManager->SetupAsync();
	}
}

static void __fastcall Unregister() {
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
	{ L"skillshow3", &Skillshow3Detour }
};

DEFINE_PLUGIN_API_VERSION()
DEFINE_PLUGIN_IDENTIFIER("AnimationFilter")
DEFINE_PLUGIN_VERSION(PLUGIN_VERSION)
DEFINE_PLUGIN_INIT(Init, Unregister)
DEFINE_PLUGIN_TABLE_HANDLERS(handlers)