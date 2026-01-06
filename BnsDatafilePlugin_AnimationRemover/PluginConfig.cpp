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

namespace fs = std::filesystem;
std::unique_ptr<PluginConfig> g_PluginConfig;

PluginConfig::PluginConfig()
{
	Initialize();
}

void PluginConfig::Initialize()
{
	AnimFilterConfig = {};
	ConfigPath = PluginConfig::GetDocumentsDirectory() + "\\BnS\\animfilter_config.xml";
}

void PluginConfig::CreateDefaultConfigFile()
{
	AnimFilterConfig = {};
	AnimFilterConfig.Enabled = true;
	AnimFilterConfig.ExperimentalMemoryLoading = false;
	AnimFilterConfig.DefaultProfileId = 1;
	const auto& jobNameMap = g_SkillIdManager->jobNameFallbackMap;
	std::vector<std::pair<std::wstring, char>> sortedJobNames(jobNameMap.begin(), jobNameMap.end());
	std::sort(sortedJobNames.begin(), sortedJobNames.end(),
		[](const auto& a, const auto& b) { return a.second < b.second; });

	int index = 1;
	for (const auto& [jobName, jobId] : sortedJobNames)
	{
		if (g_SkillIdManager->neoJobAvailability.at(jobId) == false)
			continue;
		AnimFilterConfig::AnimFilterProfile profile;
		profile.Name = std::to_string(index);
		profile.Text = L"Only " + g_SkillIdManager->customJobAbbreviations.at(jobId);
		profile.HideTree = false;
		profile.HideProjectileResists = false;
		profile.HideTimeDistortion = false;
		profile.HideTaxi = false;
		profile.HideGlobalItemSkills = false;
		profile.HideSoulCores = false;
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

std::string PluginConfig::GetDocumentsDirectory() {
	char* userProfile = nullptr;
	size_t len = 0;
	if (_dupenv_s(&userProfile, &len, "USERPROFILE") == 0 && userProfile != nullptr) {
		std::string documentsDir = std::string(userProfile) + "\\Documents";
		free(userProfile);
		return documentsDir;
	}
	MessageBox(nullptr, L"Could not find Documents directory.", L"AnimFilter Fatal Error", MB_OK | MB_ICONERROR);
	return "";
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
	if (pugi::xml_parse_result result = doc.load_file(ConfigPath.string().c_str()); !result) {
#ifdef _DEBUG
		std::cerr << "Failed to load AnimFilter config file: " << result.description() << std::endl;
#endif // _DEBUG
		MessageBox(nullptr, L"Could not load config file.", L"AnimFilter Fatal Error", MB_OK | MB_ICONERROR);
		Loaded = false;
		return;
	}
	if (pugi::xml_node enabledNode = doc.child("config").child("enabled"); enabledNode) {
		AnimFilterConfig.Enabled = enabledNode.attribute("value").as_bool();
	}
	if (pugi::xml_node experimentalNode = doc.child("config").child("experimental-feature"); experimentalNode) {
		AnimFilterConfig.ExperimentalMemoryLoading = experimentalNode.attribute("value").as_bool();
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
	if (!doc.save_file(ConfigPath.string().c_str(), PUGIXML_TEXT("  "))) {
		MessageBox(nullptr, L"Could not save config file.", L"AnimFilter Fatal Error", MB_OK | MB_ICONERROR);
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

const AnimFilterConfig::AnimFilterProfile& PluginConfig::GetActiveProfile() const
{
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
	return !AnimFilterConfig.ActiveProfile.Name.empty();
}

void PluginConfig::SetEnabled(bool enabled)
{
	AnimFilterConfig.Enabled = enabled;
}
