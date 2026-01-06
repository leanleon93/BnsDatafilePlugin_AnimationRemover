#pragma once
#include "AnimFilterConfig.h"
#include <filesystem>

class PluginConfig {
public:
	PluginConfig();
	const AnimFilterConfig::AnimFilterProfile& GetActiveProfile() const;
	const AnimFilterConfig& GetAnimFilterConfig() const;
	AnimFilterConfig& GetAnimFilterConfigEditable();
	bool AnimFilterEnabled() const;
	void ReloadFromConfig();
	void SaveToDisk();
	void SetActiveFilter(int profileId);
	bool IsLoaded() const;
	bool HasActiveProfile() const;
	void SetEnabled(bool enabled);
	void AddHideShowAllDefaultProfile(bool hide, int index, std::wstring text);
	void DeleteProfile(int profileId);
	void SetDefaultProfile(int profileId);
	std::wstring& GetDefaultProfileName();
	int GetActiveProfileId();
	void AddCustomSkillId(int profileId, int id, std::string text);
	void RemoveCustomSkillId(int profileId, int id);
private:
	std::filesystem::path ConfigPath;
	bool Loaded;
	AnimFilterConfig AnimFilterConfig;
	static std::string GetDocumentsDirectory();
	void Initialize();
	void CreateDefaultConfigFile();
	bool FileExists(const std::string& path);
};

extern std::unique_ptr<PluginConfig> g_PluginConfig;
