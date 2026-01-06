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
#include "AnimFilterConfig.h"

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
	void RemoveSpecialExclusionIds(std::unordered_set<int>& idSet, const AnimFilterConfig::AnimFilterProfile& activeProfile);
	void RemoveSpecialExclusionEffectIds(std::unordered_set<unsigned __int64>& effectIdSet, const AnimFilterConfig::AnimFilterProfile& activeProfile);
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
	{ L"Spear Master", 13 },
	{ L"Astromancer", 14 },
	{ L"Dual Blade", 15 },
	{ L"Musician", 16 }
	};
	const std::unordered_map<char, std::wstring> customJobAbbreviations = {
		{ 1, L"BM" },
		{ 2, L"KFM" },
		{ 3, L"FM" },
		{ 4, L"GS" },
		{ 5, L"DES" },
		{ 6, L"SUM" },
		{ 7, L"SIN" },
		{ 8, L"BD" },
		{ 9, L"WL" },
		{ 10, L"SF" },
		{ 11, L"WD" },
		{ 12, L"Archer" },
		{ 13, L"SM" },
		{ 14, L"Astro" },
		{ 15, L"DB" },
		{ 16, L"Bard" }
	};
	const std::unordered_map<char, bool> neoJobAvailability = {
		{ 1, true },
		{ 2, true },
		{ 3, true },
		{ 4, false },
		{ 5, true },
		{ 6, true },
		{ 7, true },
		{ 8, true },
		{ 9, true },
		{ 10, false },
		{ 11, false },
		{ 12, false },
		{ 13, true },
		{ 14, false },
		{ 15, false },
		{ 16, false}
	};
	void RestoreEffects();
private:
	__int16 skillshowTableId;
	Data::DataManager* dataManager;
	std::atomic<bool> SetupComplete;
	bool CriticalFail;
	std::unordered_set<int> GetAllFixedExclusionSkillIds();
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

	const std::unordered_set<int> grabExclusionIds = {
		//kfm grapple
		114395,
		114396,

		//fm phantom grip
		135139,
		135145,
		135151,
		135160,
		135167,

		//destro grab (wtf)
		126000,
		126001,
		126002,
		126003,
		126004,
		126005,
		126006,
		126007,
		126008,
		126009,
		126010,
		126011,
		126012,
		126013,
		126014,
		126015,
		126016,
		126017,
		126018,
		126019,
		126020,
		126021,
		126022,
		126023,
		126024,
		126025,
		126026,
		126027,
		126028,
		126029,
		126030,
		126031,
		126032,
		126033,
		126034,
		126035,
		126036,
		126037,
		126038,
		126039,

		//sin webbing
		140710,
		140711,
		140715,
		140716,

		//sum grapple
		155519,
		155520,
		155521,
		155522,
		155523,
		155524,
		155525,
		155526,
		155527,
		155528,

		//bd phantom grip (jesus)
		165290,
		165291,
		165292,
		165293,
		165294,
		165295,
		165296,
		165297,
		165298,
		165299,
		165300,
		165301,
		165302,
		165303,
		165304,
		165305,
		165306,
		165307,
		165314,
		165315,
		165316,
		165317,
		165318,
		165319,
		165320,
		165321,
		165322,
		165323,
		165324,
		165325,
		165332,
		165333,
		165334,
		165335,
		165336,
		165337,
		165338,
		165339,
		165340,
		165341,
		165342,
		165343,

		//spearmaster fixate
		250060,
		250061,
		250062,
		250063,
		250064
	};

	const std::unordered_set<int> projectileResistExclusionIds = {
		//do not hide party projectile resists
		//sum petal storm
		155571,
		155572,
		155573,
		155574,

		//sin smoke
		141860,
		141870,
		141880,

		//bm warding spirit
		105041,

		//fm divine veil
		135269,
		135270,
		135271,
		135272
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
	std::unordered_map<int, std::unordered_set<unsigned __int64>> exclusionEffectIds;
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
	void AddEffectsForSkillIds(const std::unordered_set<int>& skillIds, DrMultiKeyTable* const table, std::unordered_set<unsigned long long>& effectSet);
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
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
	};
	bool AllVersionsSuccess() const;
};

extern std::unique_ptr<SkillIdManager> g_SkillIdManager;