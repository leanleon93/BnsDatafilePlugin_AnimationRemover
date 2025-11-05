#pragma once
#include <unordered_set>
#include <unordered_map>
#include <string>
#include "EU/skillshow3/AAA_skillshow3_RecordBase.h"
#include "KR/skillshow3/AAA_skillshow3_RecordBase.h"
#include "Data.h"
#include "EU/skill_trait/AAA_skill_trait_RecordBase.h"
#include "KR/skill_trait/AAA_skill_trait_RecordBase.h"
#include "EU/effect/AAA_effect_RecordBase.h"
#include "KR/effect/AAA_effect_RecordBase.h"
#include <atomic>

class SkillIdManager {
	struct SkillIdsForJob {
		char JobId = -1;
		std::unordered_map<int, std::unordered_set<int>> SkillIdsForSpec;
	};
	struct EffectIdsForJob {
		char JobId = -1;
		std::unordered_map<int, std::unordered_set<unsigned __int64>> EffectIdsForSpec;
	};
public:
	SkillIdManager(Data::DataManager* ptr)
		: skillshowTableId(0),
		dataManager(ptr),
		SetupComplete(false),
		CriticalFail(false)
	{
		// User has to call SetupAsync
	}
	__int16 GetSkillshowTableId() const;
	bool Setup();
	bool SetupWithRetry();
	void SetupAsync();
	bool IsSetupComplete() const;
	bool IsCriticalFail() const;
	void ResetIdsToFilter();
	const std::unordered_set<int>& GetIdsToFilter() const;
	const std::unordered_set<int>& GetTaxiSkillIds() const;
	const std::unordered_map<__int32, __int16>& GetTaxiExclusionIdVariations() const;
	Data::DataManager* GetDataManager();
	struct Skill3KeyHelper {
		static __int64 BuildKey(__int32 skillId, signed char variation_id);
		static __int32 ExtractId(__int64 key);
	};
	struct SkillShow3KeyHelper {
		static __int64 BuildKey(__int32 id_, __int16 variation_id_, __int16 skillskin_id_);
#if _BNSEU
		static BnsTables::EU::skillshow3_Record::Key ExtractKey(__int64 key);
#elif _BNSKR
		static BnsTables::KR::skillshow3_Record::Key ExtractKey(__int64 key);
#endif
		static __int32 ExtractId(__int64 key);
	};
	void SetDataManager(Data::DataManager* ptr);
	void ReapplyEffectFilters();
	char GetJobIdForEnName(std::wstring const& enName);
private:
	__int16 skillshowTableId;
	Data::DataManager* dataManager;
	std::atomic<bool> SetupComplete;
	bool CriticalFail;
	const std::vector<int> idExclusionList = {
		66104,
		66105,
		66106,
		66022,
		66020,
		66021,
		66023,
		66024,
		66050
	};

	const std::unordered_set<int> bardTreeExclusionIds = {
		242500,
		242501,
		242502,
		242503,
		242504,
		242505,
		242506,
		242507
	};

	const std::unordered_set<int> wlTDExclusionIds = {
		171200,
		173000,
		171168,
		171169
	};

	const std::unordered_set<int> defaultTaxiExclusionIds = {
		141190,
		142190,
		143290,
		142270,
		142271,
		142272,
		142280,
		142281,
		142282
	};

	const std::unordered_map<__int32, __int16> taxiExclusionIdVariations = {
		{ 141190, 9 },
		{ 142190, 9 },
		{ 143290, 5 },
		{ 142270, 1 },
		{ 142271, 1 },
		{ 142272, 1 },
		{ 142280, 1 },
		{ 142281, 1 },
		{ 142282, 1 }
	};

	const std::unordered_map<char, EffectIdsForJob> fixedTargetEffectIds = {
		{ 6, { 6, {
			{ 1, { 26130068} },
			{ 2, { 26130069} }
		}}},
		{ 9, { 9, {
			{ 2, { 55170400, 28090040, 55170390, 55170151, 28090001}}
		}}},
		{ 14, { 14, {
			{ 2, { 34500000}}
		}}},
		{ 12, { 12, {
			{ 3, { 33800095, 33800048}}
		}}},
	};
	std::unordered_set<char> jobIds;
	std::unordered_map<std::wstring, char> jobNameMap;
	std::unordered_set<int> globalItemSkillIds;
	std::unordered_set<int> soulCoreSkills;
	std::unordered_map<char, SkillIdsForJob> skillIdsForJobMap;
	std::unordered_map<char, EffectIdsForJob> effectIdsForJobMap;
	std::unordered_set<int> GetAllSkillIdsFromJobMap();
	std::unordered_set<int> idsToFilter;
	std::unordered_set<unsigned __int64> effectIdsToFilter;
	std::unordered_set<int> taxiExclusionIds;
	void ResetEffectIdsToFilter();
	std::unordered_map<unsigned __int64, std::unordered_map<std::string, wchar_t>> effectRestoreList;
	std::unordered_map<unsigned __int64, std::unordered_map<std::string, wchar_t*>> effectSwapRestoreList;
	bool SetupJobNameMap();
	bool SetupAllSkillIds();
	bool SetupSkillShowTableId();
	bool SetupSkillIdsForJob(char jobId);
	bool SetupEffectIdsForJob(char jobId);
#if _BNSEU
	void AddIds(BnsTables::EU::skill_trait_Record const* record, int const* ids, int size, SkillIdsForJob& skillIdsForJobEntry);
	void AddFixedIds(BnsTables::EU::skill_trait_Record const* record, SkillIdsForJob& skillIdsForJobEntry);
	void AddVariableIds(BnsTables::EU::skill_trait_Record const* record, SkillIdsForJob& skillIdsForJobEntry);
	void SwapAnimationsForEffect(BnsTables::EU::effect_Record* target, BnsTables::EU::effect_Record* animation);
	void RemoveAnimationsForEffect(BnsTables::EU::effect_Record* effectRecord);
#elif _BNSKR
	void AddIds(BnsTables::KR::skill_trait_Record const* record, int const* ids, int size, SkillIdsForJob& skillIdsForJobEntry);
	void AddFixedIds(BnsTables::KR::skill_trait_Record const* record, SkillIdsForJob& skillIdsForJobEntry);
	void AddVariableIds(BnsTables::KR::skill_trait_Record const* record, SkillIdsForJob& skillIdsForJobEntry);
	void SwapAnimationsForEffect(BnsTables::KR::effect_Record* target, BnsTables::KR::effect_Record* animation);
	void RemoveAnimationsForEffect(BnsTables::KR::effect_Record* effectRecord);
#endif
	bool SetupSoulCoreSkills();
	void AddSoulCoreChildren();
	std::unordered_set<int> GetInheritedIds(int id);
	void AddChildrenSkillIds(SkillIdsForJob& skillIdsForJobEntry);
	std::unordered_set<int> GetNeoChildSkillIds(int id);
	std::unordered_set<int> GetNeoChildSkillIds2(int id);
	std::unordered_set<int> GetChildSkillIds(int id);
	void FilterBracelet(SkillIdsForJob& skillIdsForJobEntry);
	bool IsBraceletId(int id);
	void AddItemSkills(SkillIdsForJob& skillIdsForJobEntry);
	std::unordered_set<int> GetItemSkills(int id);
	void RestoreEffects();
	void RemoveEffects();
	void SwapEffects();

	std::unordered_set<unsigned __int64> GetEffectIdsForEffectGroup(unsigned __int64 effectGroupId);
	bool CompatabilityCheck();
	const std::unordered_set<std::wstring> usedTables = {
		L"skillshow3",
		L"effect",
		L"job",
		L"skill3",
		L"skill-trait",
		L"itemskill",
		L"skill-inheritance",
		L"effect-group",
		L"skillbookcatalogueitem",
		L"skill-train-by-item",
		L"item"
	};

	std::unordered_map<std::wstring, bool> versionCheckSuccess = {
		{ L"skillshow3", false },
		{ L"effect", false },
		{ L"job", false },
		{ L"skill3", false },
		{ L"skill-trait", false },
		{ L"itemskill", false },
		{ L"skill-inheritance", false },
		{ L"effect-group", false },
		{ L"skillbookcatalogueitem", false },
		{ L"skill-train-by-item", false },
		{ L"item", false}
	};

	const std::unordered_set<char> jobIdsFallback = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 16
	};
	const std::unordered_map<std::wstring, char> jobNameFallbackMap = {
		{ L"Blade Master", 1 },
		{ L"Kung Fu Master", 2 },
		{ L"Force Master", 3 },
		{ L"Gunslinger", 4 },
		{ L"Destroyer", 5 },
		{ L"Summoner", 6 },
		{ L"Assassin", 7 },
		{ L"Blade Dancer", 8 },
		{ L"Warlock", 9 },
		{ L"Soul Fighter", 10 },
		{ L"Warden", 11 },
		{ L"Zen Archer", 12 },
		{ L"Astromancer", 14 },
		{ L"Dual Blade", 15 },
		{ L"Musician", 16 }
	};
	bool AllVersionsSuccess() const;
};

extern std::unique_ptr<SkillIdManager> g_SkillIdManager;