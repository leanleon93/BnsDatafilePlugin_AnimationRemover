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
private:
	std::filesystem::path ConfigPath;
	bool Loaded;
	AnimFilterConfig AnimFilterConfig;
	static std::string GetDocumentsDirectory();
	void Initialize();
};

extern std::unique_ptr<PluginConfig> g_PluginConfig;
