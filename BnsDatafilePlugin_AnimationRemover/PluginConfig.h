#pragma once
#include "AnimFilterConfig.h"
#include <filesystem>
#include "DatafilePluginsdk.h"

class PluginConfig {
public:
	PluginConfig();
	const AnimFilterConfig::AnimFilterProfile& GetActiveProfile();
	const AnimFilterConfig& GetAnimFilterConfig() const;
	AnimFilterConfig& GetAnimFilterConfigEditable();
	bool AnimFilterEnabled() const;
	void ReloadFromConfig();
	void SaveToDisk();
	void SetActiveFilter(int profileId);
	bool IsLoaded() const;
	bool HasActiveProfile() const;
	void SetEnabled(bool enabled);
	bool GetAutoJobDetection() const;
	void SetAutoJobDetection(bool enabled);
	void AddHideShowAllDefaultProfile(bool hide, int index, std::wstring text);
	void DeleteProfile(int profileId);
	void SetDefaultProfile(int profileId);
	std::wstring& GetDefaultProfileName();
	int GetActiveProfileId();
	void AddCustomSkillId(int profileId, int id, std::string text);
	void RemoveCustomSkillId(int profileId, int id);
	BnsClient_GetWorldFunc getWorld = nullptr;
	UnregisterDetoursFunc unregisterDetours = nullptr;
	void SetAutoJobProfile();
	const AnimFilterConfig::AnimFilterProfile& GetAutoJobProfile();
	void ResetAutoJobProfile();
private:
	std::filesystem::path ConfigPath;
	bool Loaded;
	AnimFilterConfig AnimFilterConfig;
	static std::filesystem::path GetConfigDirectory();
	void Initialize();
	void CreateDefaultConfigFile();
	bool FileExists(const std::string& path);
};

extern std::unique_ptr<PluginConfig> g_PluginConfig;
