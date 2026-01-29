#include "PluginConfig.h"
#include "pugixml/pugixml.hpp"
#include <Windows.h>
#include "SkillIdManager.h"
#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG
#include <fstream>
#include <string>
#include "AnimFilterConfig.h"
#include <algorithm>
#include <filesystem>
#include <cwctype>
#include <system_error>
#include <cstdlib>
#include <malloc.h>
#include "Data.h"

namespace fs = std::filesystem;
std::unique_ptr<PluginConfig> g_PluginConfig;

PluginConfig::PluginConfig()
{
	Initialize();
}

void PluginConfig::Initialize()
{
	AnimFilterConfig = {};
	std::filesystem::path configDir = std::filesystem::path(GetConfigDirectory());
	if (configDir.empty()) {
		MessageBoxW(nullptr, L"Could not determine a valid configuration directory. The plugin will not save profiles.", L"AnimFilter Fatal Error", MB_OK | MB_ICONWARNING);
		ConfigPath.clear();
		return;
	}
#ifdef _BNSLIVE
	ConfigPath = configDir / L"BnS" / L"animfilter_config_live.xml";
#else
	ConfigPath = configDir / L"BnS" / L"animfilter_config.xml";
#endif
	if (!fs::exists(ConfigPath.parent_path())) {
		fs::create_directories(ConfigPath.parent_path());
	}
}

void PluginConfig::CreateDefaultConfigFile()
{
	AnimFilterConfig = {};
	AnimFilterConfig.Enabled = true;
	AnimFilterConfig.AutoJobDetection = true;
	AnimFilterConfig.DefaultProfileId = 1;
	const auto& jobNameMap = g_SkillIdManager->jobNameFallbackMap;
	std::vector<std::pair<std::wstring, char>> sortedJobNames(jobNameMap.begin(), jobNameMap.end());
	std::sort(sortedJobNames.begin(), sortedJobNames.end(),
		[](const auto& a, const auto& b) { return a.second < b.second; });

	int index = 1;
	for (const auto& [jobName, jobId] : sortedJobNames)
	{
#ifdef _BNSLIVE
		if (jobId == 13) { //skip live spearmaster
			continue;
		}
#else
		if (g_SkillIdManager->neoJobAvailability.at(jobId) == false)
			continue;
#endif // !_BNSLIVE
		AnimFilterConfig::AnimFilterProfile profile;
		profile.Name = std::to_string(index);
		profile.Text = L"Only " + g_SkillIdManager->customJobAbbreviations.at(jobId);
		profile.HideTree = false;
		profile.HideProjectileResists = false;
		profile.HideTimeDistortion = false;
		profile.HideTaxi = false;
		profile.HideGlobalItemSkills = false;
		profile.HideSoulCores = false;
		profile.HideOthersSoulCores = false;
		profile.HideAllOtherPlayerSkills = true;
		profile.HideGrabs = false;
		profile.SkillFilters = {};
		for (const auto& [jobNameInner, jobIdInner] : jobNameMap)
		{
			bool hide = (jobIdInner != jobId);
			AnimFilterConfig::AnimFilterProfile::SkillOption skillOption;
			skillOption.Name = jobNameInner;
			skillOption.Job = jobIdInner;
			skillOption.HideSpec1 = hide;
			skillOption.HideSpec2 = hide;
			skillOption.HideSpec3 = hide;
			profile.SkillFilters.push_back(skillOption);
		}
		AnimFilterConfig.Profiles[index] = profile;
		index++;
	}
	AddHideShowAllDefaultProfile(true, index, L"Hide All");
	AddHideShowAllDefaultProfile(false, ++index, L"Show All");
	SaveToDisk();
}

bool PluginConfig::FileExists(const std::string& path)
{
	std::ifstream file(path);
	return file.good();
}

void PluginConfig::AddHideShowAllDefaultProfile(bool hide, int index, std::wstring text)
{
	AnimFilterConfig::AnimFilterProfile profile;
	profile.Name = std::to_string(index);
	profile.Text = text;
	profile.HideTree = hide;
	profile.HideProjectileResists = hide;
	profile.HideTimeDistortion = hide;
	profile.HideTaxi = hide;
	profile.HideGlobalItemSkills = hide;
	profile.HideSoulCores = hide;
	profile.HideOthersSoulCores = hide;
	profile.HideAllOtherPlayerSkills = hide;
	profile.HideGrabs = hide;
	const auto& jobNameMap = g_SkillIdManager->jobNameFallbackMap;
	for (const auto& [jobNameInner, jobIdInner] : jobNameMap)
	{
		AnimFilterConfig::AnimFilterProfile::SkillOption skillOption;
		skillOption.Name = jobNameInner;
		skillOption.Job = jobIdInner;
		skillOption.HideSpec1 = hide;
		skillOption.HideSpec2 = hide;
		skillOption.HideSpec3 = hide;
		profile.SkillFilters.push_back(skillOption);
	}

	AnimFilterConfig.Profiles[index] = profile;
}

void PluginConfig::DeleteProfile(int profileId)
{
	AnimFilterConfig.Profiles.erase(profileId);
}

void PluginConfig::SetDefaultProfile(int profileId)
{
	if (auto it = AnimFilterConfig.Profiles.find(profileId); it != AnimFilterConfig.Profiles.end()) {
		AnimFilterConfig.DefaultProfileId = profileId;
	}
	else {
		//default to first in list if list not empty
		if (!AnimFilterConfig.Profiles.empty()) {
			AnimFilterConfig.DefaultProfileId = AnimFilterConfig.Profiles.begin()->first;
		}
		else {
			AnimFilterConfig.DefaultProfileId = 1;
		}
	}
}

std::wstring& PluginConfig::GetDefaultProfileName()
{
	if (auto it = AnimFilterConfig.Profiles.find(AnimFilterConfig.DefaultProfileId); it != AnimFilterConfig.Profiles.end()) {
		return it->second.Text;
	}
	else {
		static std::wstring emptyString = L"";
		return emptyString;
	}
}

int PluginConfig::GetActiveProfileId()
{
	for (const auto& [profileId, profile] : AnimFilterConfig.Profiles) {
		if (profile.Name == AnimFilterConfig.ActiveProfile.Name) {
			return profileId;
		}
	}
	return -1;
}

void PluginConfig::AddCustomSkillId(int profileId, int id, std::string text)
{
	if (auto it = AnimFilterConfig.Profiles.find(profileId); it != AnimFilterConfig.Profiles.end()) {
		it->second.CustomSkillIdFilters[id] = text;
	}
}

void PluginConfig::RemoveCustomSkillId(int profileId, int id)
{
	if (auto it = AnimFilterConfig.Profiles.find(profileId); it != AnimFilterConfig.Profiles.end()) {
		it->second.CustomSkillIdFilters.erase(id);
	}
}

fs::path PluginConfig::GetConfigDirectory() {
	// Helper lambda to get an environment variable as a wide string path
	auto get_env_path = [](const wchar_t* var) -> fs::path {
		wchar_t* value = nullptr;
		size_t len = 0;
		fs::path result;
		if (_wdupenv_s(&value, &len, var) == 0 && value != nullptr) {
			result = fs::path(value);
			free(value);
		}
		return result;
		};

	// Helper lambda to fallback to LOCALAPPDATA
	auto fallback_localappdata = [&]() -> fs::path {
		fs::path localAppDataBnS = get_env_path(L"LOCALAPPDATA");
		if (localAppDataBnS.parent_path().empty()) {
			//MessageBoxW(nullptr, L"Could not find LOCALAPPDATA directory.", L"AnimFilter Fatal Error", MB_OK | MB_ICONERROR);
			return {};
		}
		return localAppDataBnS;
		};

	fs::path documentsDir = get_env_path(L"USERPROFILE") / L"Documents";
	if (documentsDir.empty()) {
		//MessageBoxW(nullptr, L"Could not find Documents directory. Falling back to LOCALAPPDATA.", L"AnimFilter Error", MB_OK | MB_ICONERROR);
		return fallback_localappdata();
	}

	// Check for OneDrive in the path (case-insensitive)
	std::wstring docStr = documentsDir.native();
	std::wstring onedriveStr = L"onedrive";
	auto it = std::search(
		docStr.begin(), docStr.end(),
		onedriveStr.begin(), onedriveStr.end(),
		[](wchar_t c1, wchar_t c2) { return std::towlower(c1) == std::towlower(c2); }
	);

	// Fallback if OneDrive detected
	if (it != docStr.end()) {
		return fallback_localappdata();
	}

	// Check if Documents is writeable by attempting to create a temp file
	fs::path testFile = documentsDir / "BnS" / L".animfilter_write_test";
	{
		std::error_code ec;
		if (!fs::exists(testFile.parent_path())) {
			fs::create_directories(testFile.parent_path(), ec);
			if (ec) {
				//MessageBoxW(nullptr, L"Could not create the BnS config directory in Documents. Falling back to LOCALAPPDATA.", L"AnimFilter Error", MB_OK | MB_ICONERROR);
				return fallback_localappdata();
			}
		}
		std::wofstream ofs(testFile);
		if (!ofs) {
			//MessageBoxW(nullptr, L"Documents directory is not writeable. Falling back to LOCALAPPDATA.", L"AnimFilter Error", MB_OK | MB_ICONERROR);
			return fallback_localappdata();
		}
	}
	fs::remove(testFile);
	return documentsDir;
}

static std::wstring PugiCharPtrToWString(const pugi::char_t* charPtr) {
	if (charPtr == nullptr) {
		return L"";
	}
	// Convert pugi::char_t* to char* first
	const char* utf8Str = charPtr;
	// Convert char* to std::wstring
	return std::wstring(utf8Str, utf8Str + strlen(utf8Str));
}

static void SetProfiles(pugi::xml_document const& doc, AnimFilterConfig* animFilterConfig) {
	for (pugi::xml_node profileNode : doc.child("config").child("profiles").children("profile")) {
		auto profileId = profileNode.attribute("name").as_int();
		AnimFilterConfig::AnimFilterProfile profile;
		profile.Name = profileNode.attribute("name").as_string();
		if (pugi::xml_node extraoptionsNode = profileNode.child("extra_options"); extraoptionsNode) {
			if (extraoptionsNode.attribute("hideTree") != nullptr) {
				profile.HideTree = extraoptionsNode.attribute("hideTree").as_bool();
			}
			if (extraoptionsNode.attribute("hideProjectileResists") != nullptr) {
				profile.HideProjectileResists = extraoptionsNode.attribute("hideProjectileResists").as_bool();
			}
			if (extraoptionsNode.attribute("hideTD") != nullptr) {
				profile.HideTimeDistortion = extraoptionsNode.attribute("hideTD").as_bool();
			}
			if (extraoptionsNode.attribute("hideTaxi") != nullptr) {
				profile.HideTaxi = extraoptionsNode.attribute("hideTaxi").as_bool();
			}
			if (extraoptionsNode.attribute("hideItemSkills") != nullptr) {
				profile.HideGlobalItemSkills = extraoptionsNode.attribute("hideItemSkills").as_bool();
			}
			if (extraoptionsNode.attribute("hideSoulCores") != nullptr) {
				profile.HideSoulCores = extraoptionsNode.attribute("hideSoulCores").as_bool();
			}
			if (extraoptionsNode.attribute("hideOthersSoulCores") != nullptr) {
				profile.HideOthersSoulCores = extraoptionsNode.attribute("hideOthersSoulCores").as_bool();
			}
			if (extraoptionsNode.attribute("hideAllOtherPlayerSkills") != nullptr) {
				profile.HideAllOtherPlayerSkills = extraoptionsNode.attribute("hideAllOtherPlayerSkills").as_bool();
			}
			else {
				profile.HideAllOtherPlayerSkills = true;
			}
			if (extraoptionsNode.attribute("hideGrabs") != nullptr) {
				profile.HideGrabs = extraoptionsNode.attribute("hideGrabs").as_bool();
			}
		}
		if (pugi::xml_node textNode = profileNode.child("text"); textNode) {
			auto text = textNode.text();
			profile.Text = PugiCharPtrToWString(text.as_string());
		}
		for (pugi::xml_node effectOptionNode : profileNode.child("effect_options").children("option")) {
			AnimFilterConfig::AnimFilterProfile::EffectOption effectOption;

			auto isAlias = effectOptionNode.attribute("isAlias").as_bool();
			effectOption.IsAlias = isAlias;
			if (isAlias) {
				auto alias = effectOptionNode.attribute("alias").as_string();
				effectOption.Alias = std::string(alias);
			}
			else {
				auto key = effectOptionNode.attribute("key").as_ullong();
				effectOption.Key = key;
			}
			profile.EffectFilters.push_back(effectOption);
		}
		for (pugi::xml_node animSwapGroupNode : profileNode.child("swap_options").children("group")) {
			AnimFilterConfig::AnimFilterProfile::SwapGroup swapGroup{};
			swapGroup.Name = animSwapGroupNode.attribute("name").as_string();
			for (pugi::xml_node animSwapOptionNode : animSwapGroupNode.children("option")) {
				AnimFilterConfig::AnimFilterProfile::SwapOption swapOption{};
				auto targetSkillId = animSwapOptionNode.attribute("targetSkillId").as_int();
				swapOption.TargetSkillId = targetSkillId;
				auto animationSkillId = animSwapOptionNode.attribute("animationSkillId").as_int();
				swapOption.AnimationSkillId = animationSkillId;
				auto animationSkillVariationAttr = animSwapOptionNode.attribute("animationSkillVariation");
				int animationSkillVariation = animationSkillVariationAttr ? animationSkillVariationAttr.as_int() : 1;
				swapOption.AnimationSkillVariation = animationSkillVariation;
				swapGroup.Swaps.push_back(swapOption);
				profile.AnimationSwaps[targetSkillId] = swapOption;
			}
			profile.AnimationSwapGroups.push_back(swapGroup);
		}

		for (pugi::xml_node animSwapOptionNode : profileNode.child("effect_swap_options").children("option")) {
			AnimFilterConfig::AnimFilterProfile::EffectSwapOption effectSwapOption{};
			auto targetEffectId = animSwapOptionNode.attribute("targetEffectId").as_int();
			effectSwapOption.TargetEffectId = targetEffectId;
			auto animationEffectId = animSwapOptionNode.attribute("animationEffectId").as_int();
			effectSwapOption.AnimationEffectId = animationEffectId;
			profile.EffectSwaps[targetEffectId] = effectSwapOption;
		}
		for (pugi::xml_node skillOptionNode : profileNode.child("skill_options").children("option")) {
			AnimFilterConfig::AnimFilterProfile::SkillOption skillOption;
			auto name = skillOptionNode.attribute("name").as_string();
			skillOption.Name = PugiCharPtrToWString(name);
			skillOption.Job = g_SkillIdManager->GetJobIdForEnName(skillOption.Name);
			skillOption.HideSpec1 = skillOptionNode.attribute("hideSpec1").as_bool();
			skillOption.HideSpec2 = skillOptionNode.attribute("hideSpec2").as_bool();
			skillOption.HideSpec3 = skillOptionNode.attribute("hideSpec3").as_bool();
			profile.SkillFilters.push_back(skillOption);
		}
		//CustomSkillIdFilters
		profile.CustomSkillIdFilters.clear();
		for (pugi::xml_node customSkillOptionNode : profileNode.child("custom_skill_options").children("option")) {
			auto skillId = customSkillOptionNode.attribute("id").as_int();
			auto text = customSkillOptionNode.attribute("text").as_string();
			profile.CustomSkillIdFilters[skillId] = text;
		}
		animFilterConfig->Profiles[profileId] = profile;
	}
}

void PluginConfig::ReloadFromConfig()
{
	if (!FileExists(ConfigPath.string())) {
		CreateDefaultConfigFile();
	}
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(ConfigPath.string().c_str());
	if (!result) {
#ifdef _DEBUG
		std::cerr << "Failed to load AnimFilter config file: " << result.description() << std::endl;
#endif // _DEBUG
		// rename the bad config for user inspection
		try {
			fs::path badPath = ConfigPath;
			badPath += L".bad";
			fs::rename(ConfigPath, badPath);
		}
		catch (const std::exception&) {
			// If renaming fails, ignore and proceed to overwrite
		}
		MessageBox(nullptr, L"Config file is missing, corrupted, or has invalid format. A new default config will be created.", L"AnimFilter Error", MB_OK | MB_ICONERROR);
		CreateDefaultConfigFile();
		result = doc.load_file(ConfigPath.string().c_str());
	}
	if (!result) {
		Loaded = false;
		return;
	}
	if (pugi::xml_node enabledNode = doc.child("config").child("enabled"); enabledNode) {
		AnimFilterConfig.Enabled = enabledNode.attribute("value").as_bool();
	}
	if (pugi::xml_node experimentalNode = doc.child("config").child("auto-job-detection"); experimentalNode) {
		AnimFilterConfig.AutoJobDetection = experimentalNode.attribute("value").as_bool();
	}
	else {
		AnimFilterConfig.AutoJobDetection = true; //migrate old configs
	}
	if (pugi::xml_node defaultProfileNode = doc.child("config").child("default-profile"); defaultProfileNode) {
		AnimFilterConfig.DefaultProfileId = defaultProfileNode.attribute("id").as_int();
	}
	else {
		AnimFilterConfig.DefaultProfileId = 1;
	}
	AnimFilterConfig.Profiles.clear();
	SetProfiles(doc, &AnimFilterConfig);
	SetActiveFilter(AnimFilterConfig.DefaultProfileId);
	Loaded = true;
}

void PluginConfig::SaveToDisk()
{
	using namespace pugi;

	xml_document doc;

	// Root node
	xml_node configNode = doc.append_child("config");

	// Enabled
	xml_node enabledNode = configNode.append_child("enabled");
	enabledNode.append_attribute("value") = AnimFilterConfig.Enabled;

	//Auto Job Detection
	xml_node experimentalNode = configNode.append_child("auto-job-detection");
	experimentalNode.append_attribute("value") = AnimFilterConfig.AutoJobDetection;


	//Default Profile Id
	xml_node defaultProfileNode = configNode.append_child("default-profile");
	defaultProfileNode.append_attribute("id") = AnimFilterConfig.DefaultProfileId;

	// Profiles
	xml_node profilesNode = configNode.append_child("profiles");
	for (const auto& profilePair : AnimFilterConfig.Profiles) {
		const auto& profile = profilePair.second;
		xml_node profileNode = profilesNode.append_child("profile");
		profileNode.append_attribute("name") = profile.Name.c_str();

		// Extra options
		xml_node extraOptionsNode = profileNode.append_child("extra_options");
		extraOptionsNode.append_attribute("hideTree") = profile.HideTree;
		extraOptionsNode.append_attribute("hideProjectileResists") = profile.HideProjectileResists;
		extraOptionsNode.append_attribute("hideTD") = profile.HideTimeDistortion;
		extraOptionsNode.append_attribute("hideTaxi") = profile.HideTaxi;
		extraOptionsNode.append_attribute("hideItemSkills") = profile.HideGlobalItemSkills;
		extraOptionsNode.append_attribute("hideSoulCores") = profile.HideSoulCores;
		extraOptionsNode.append_attribute("hideOthersSoulCores") = profile.HideOthersSoulCores;
		extraOptionsNode.append_attribute("hideAllOtherPlayerSkills") = profile.HideAllOtherPlayerSkills;
		extraOptionsNode.append_attribute("hideGrabs") = profile.HideGrabs;

		// Text
		if (!profile.Text.empty()) {
			xml_node textNode = profileNode.append_child("text");
			// Convert wstring to UTF-8
			std::string utf8Text(profile.Text.begin(), profile.Text.end());
			textNode.text().set(utf8Text.c_str());
		}

		// Effect options
		xml_node effectOptionsNode = profileNode.append_child("effect_options");
		for (const auto& effectOption : profile.EffectFilters) {
			xml_node optionNode = effectOptionsNode.append_child("option");
			optionNode.append_attribute("isAlias") = effectOption.IsAlias;
			if (effectOption.IsAlias) {
				optionNode.append_attribute("alias") = effectOption.Alias.c_str();
			}
			else {
				optionNode.append_attribute("key") = effectOption.Key;
			}
		}

		// Swap options
		xml_node swapOptionsNode = profileNode.append_child("swap_options");
		for (const auto& swapGroup : profile.AnimationSwapGroups) {
			xml_node groupNode = swapOptionsNode.append_child("group");
			groupNode.append_attribute("name") = swapGroup.Name.c_str();
			for (const auto& swapOption : swapGroup.Swaps) {
				xml_node optionNode = groupNode.append_child("option");
				optionNode.append_attribute("targetSkillId") = swapOption.TargetSkillId;
				optionNode.append_attribute("animationSkillId") = swapOption.AnimationSkillId;
				optionNode.append_attribute("animationSkillVariation") = swapOption.AnimationSkillVariation;
			}
		}

		// Effect swap options
		xml_node effectSwapOptionsNode = profileNode.append_child("effect_swap_options");
		for (const auto& effectSwapPair : profile.EffectSwaps) {
			const auto& effectSwapOption = effectSwapPair.second;
			xml_node optionNode = effectSwapOptionsNode.append_child("option");
			optionNode.append_attribute("targetEffectId") = effectSwapOption.TargetEffectId;
			optionNode.append_attribute("animationEffectId") = effectSwapOption.AnimationEffectId;
		}

		// Skill options
		xml_node skillOptionsNode = profileNode.append_child("skill_options");
		for (const auto& skillOption : profile.SkillFilters) {
			xml_node optionNode = skillOptionsNode.append_child("option");
			// Convert wstring to UTF-8
			std::string utf8Name(skillOption.Name.begin(), skillOption.Name.end());
			optionNode.append_attribute("name") = utf8Name.c_str();
			optionNode.append_attribute("hideSpec1") = skillOption.HideSpec1;
			optionNode.append_attribute("hideSpec2") = skillOption.HideSpec2;
			optionNode.append_attribute("hideSpec3") = skillOption.HideSpec3;
		}
		// Custom Skill ID Filters
		xml_node customSkillOptionsNode = profileNode.append_child("custom_skill_options");
		for (const auto& [skillId, text] : profile.CustomSkillIdFilters) {
			xml_node optionNode = customSkillOptionsNode.append_child("option");
			optionNode.append_attribute("id") = skillId;
			optionNode.append_attribute("text") = text.c_str();
		}
	}
	pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "UTF-8";
	// Save to file
	if (ConfigPath.empty()) {
		//dont try saving
		return;
	}
	if (!fs::exists(ConfigPath.parent_path())) {
		fs::create_directories(ConfigPath.parent_path());
	}
	std::error_code ec;
	if (!fs::exists(ConfigPath.parent_path(), ec)) {
		std::wstring errorMsg = L"Config directory does not exist: " + ConfigPath.parent_path().wstring() +
			L"\nError code: " + std::to_wstring(ec.value()) + L"\nMessage: " + std::wstring(ec.message().begin(), ec.message().end());
		MessageBoxW(nullptr, errorMsg.c_str(), L"AnimFilter Fatal Error", MB_OK | MB_ICONERROR);
	}
	else if (!doc.save_file(ConfigPath.string().c_str(), PUGIXML_TEXT("  "))) {
		std::wstring errorMsg = L"Could not save config file:\n" + ConfigPath.wstring() +
			L"\nPlease check file permissions or disk space.";
		MessageBoxW(nullptr, errorMsg.c_str(), L"AnimFilter Fatal Error", MB_OK | MB_ICONERROR);
	}
}

void PluginConfig::SetActiveFilter(int profileId)
{
	if (auto it = AnimFilterConfig.Profiles.find(profileId); it != AnimFilterConfig.Profiles.end()) {
		AnimFilterConfig.ActiveProfile = it->second;
	}
	else {
		//default to first in list if list not empty
		if (!AnimFilterConfig.Profiles.empty()) {
			AnimFilterConfig.ActiveProfile = AnimFilterConfig.Profiles.begin()->second;
		}
	}
}

void PluginConfig::ResetAutoJobProfile() {
	AnimFilterConfig.AutoJobProfile.DetectedJobId = 0;
}

void PluginConfig::SetAutoJobProfile()
{
	if (g_PluginConfig->getWorld != nullptr) {
		auto* world = g_PluginConfig->getWorld();
		if (world != nullptr && world->_player != nullptr) {
			auto player = g_PluginConfig->getWorld()->_player;
			auto playerCreature = (Creature*)player;
			if (playerCreature->job != 0 && playerCreature->job != AnimFilterConfig.AutoJobProfile.DetectedJobId) {
				auto jobId = playerCreature->job;
				const auto& jobNameMap = g_SkillIdManager->jobNameFallbackMap;
				AnimFilterConfig::AnimFilterProfile profile;
				profile.Name = std::to_string(999);
				profile.Text = L"Auto";
				profile.HideTree = false;
				profile.HideProjectileResists = false;
				profile.HideTimeDistortion = false;
				profile.HideTaxi = false;
				profile.HideGlobalItemSkills = false;
				profile.HideSoulCores = false;
				profile.HideOthersSoulCores = false;
				profile.HideAllOtherPlayerSkills = true;
				profile.HideGrabs = false;
				profile.SkillFilters = {};
				for (const auto& [jobNameInner, jobIdInner] : jobNameMap)
				{
					bool hide = (jobIdInner != jobId);
					AnimFilterConfig::AnimFilterProfile::SkillOption skillOption;
					skillOption.Name = jobNameInner;
					skillOption.Job = jobIdInner;
					skillOption.HideSpec1 = hide;
					skillOption.HideSpec2 = hide;
					skillOption.HideSpec3 = hide;
					profile.SkillFilters.push_back(skillOption);
				}
				profile.DetectedJobId = jobId;
				AnimFilterConfig.AutoJobProfile = profile;
				g_SkillIdManager->ResetIdsToFilter();
				g_SkillIdManager->ReapplyEffectFilters();
				g_SkillIdManager->ReloadSkillShow3();
			}
		}
	}
}

const AnimFilterConfig::AnimFilterProfile& PluginConfig::GetAutoJobProfile()
{
	SetAutoJobProfile();
	return this->AnimFilterConfig.AutoJobProfile;
}

const AnimFilterConfig::AnimFilterProfile& PluginConfig::GetActiveProfile()
{
	if (this->AnimFilterConfig.AutoJobDetection) {
		return GetAutoJobProfile();
	}
	return this->AnimFilterConfig.ActiveProfile;
}

const AnimFilterConfig& PluginConfig::GetAnimFilterConfig() const
{
	return this->AnimFilterConfig;
}

AnimFilterConfig& PluginConfig::GetAnimFilterConfigEditable() {
	return AnimFilterConfig;
}

bool PluginConfig::AnimFilterEnabled() const
{
	return this->AnimFilterConfig.Enabled;
}

bool PluginConfig::IsLoaded() const
{
	return Loaded;
}

bool PluginConfig::HasActiveProfile() const
{
	return AnimFilterConfig.AutoJobDetection || !AnimFilterConfig.ActiveProfile.Name.empty();
}

void PluginConfig::SetEnabled(bool enabled)
{
	AnimFilterConfig.Enabled = enabled;
}

bool PluginConfig::GetAutoJobDetection() const {
	return AnimFilterConfig.AutoJobDetection;
}

void PluginConfig::SetAutoJobDetection(bool enabled) {
	AnimFilterConfig.AutoJobDetection = enabled;
}
