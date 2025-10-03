#include "PluginConfig.h"
#include "pugixml/pugixml.hpp"
#include <Windows.h>
#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG

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
		for (pugi::xml_node animSwapOptionNode : profileNode.child("swap_options").children("option")) {
			AnimFilterConfig::AnimFilterProfile::SwapOption swapOption{};
			auto targetSkillId = animSwapOptionNode.attribute("targetSkillId").as_int();
			swapOption.TargetSkillId = targetSkillId;
			auto animationSkillId = animSwapOptionNode.attribute("animationSkillId").as_int();
			swapOption.AnimationSkillId = animationSkillId;
			auto animationSkillVariation = animSwapOptionNode.attribute("animationSkillVariation").as_int();
			swapOption.AnimationSkillVariation = animationSkillVariation;
			profile.AnimationSwaps[targetSkillId] = swapOption;
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
			skillOption.HideSpec1 = skillOptionNode.attribute("hideSpec1").as_bool();
			skillOption.HideSpec2 = skillOptionNode.attribute("hideSpec2").as_bool();
			skillOption.HideSpec3 = skillOptionNode.attribute("hideSpec3").as_bool();
			profile.SkillFilters.push_back(skillOption);
		}
		animFilterConfig->Profiles[profileId] = profile;
	}
}

void PluginConfig::ReloadFromConfig()
{
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
	SetProfiles(doc, &AnimFilterConfig);
	SetActiveFilter(1);
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

	// Profiles
	xml_node profilesNode = configNode.append_child("profiles");
	for (const auto& profilePair : AnimFilterConfig.Profiles) {
		const auto& profile = profilePair.second;
		xml_node profileNode = profilesNode.append_child("profile");
		profileNode.append_attribute("name") = profile.Name.c_str();

		// Extra options
		xml_node extraOptionsNode = profileNode.append_child("extra_options");
		extraOptionsNode.append_attribute("hideTree") = profile.HideTree;
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
		for (const auto& swapPair : profile.AnimationSwaps) {
			const auto& swapOption = swapPair.second;
			xml_node optionNode = swapOptionsNode.append_child("option");
			optionNode.append_attribute("targetSkillId") = swapOption.TargetSkillId;
			optionNode.append_attribute("animationSkillId") = swapOption.AnimationSkillId;
			optionNode.append_attribute("animationSkillVariation") = swapOption.AnimationSkillVariation;
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
