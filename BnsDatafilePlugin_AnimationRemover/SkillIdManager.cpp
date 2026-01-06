#include "SkillIdManager.h"
#include <algorithm>
#include <thread>
#include "EU/job/AAA_job_RecordBase.h"
#include "EU/text/AAA_text_RecordBase.h"
#include "EU/skill_inheritance/AAA_skill_inheritance_RecordBase.h"
#include "EU/skillbookcatalogueitem/AAA_skillbookcatalogueitem_RecordBase.h"
#include "EU/skill_train_by_item/AAA_skill_train_by_item_RecordBase.h"
#include "EU/skill3/AAA_skill3_RecordBase.h"
#include "EU/skill3/skill3_active_skill_Record.h"
#include "EU/itemskill/AAA_itemskill_RecordBase.h"
#include "EU/effect_group/AAA_effect_group_RecordBase.h"
#include "EU/item/item_gem_Record.h"
#include "EU/BnsTableNames.h"

#include "KR/job/AAA_job_RecordBase.h"
#include "KR/text/AAA_text_RecordBase.h"
#include "KR/skill_inheritance/AAA_skill_inheritance_RecordBase.h"
#include "KR/skillbookcatalogueitem/AAA_skillbookcatalogueitem_RecordBase.h"
#include "KR/skill_train_by_item/AAA_skill_train_by_item_RecordBase.h"
#include "KR/skill3/AAA_skill3_RecordBase.h"
#include "KR/skill3/skill3_active_skill_Record.h"
#include "KR/itemskill/AAA_itemskill_RecordBase.h"
#include "KR/effect_group/AAA_effect_group_RecordBase.h"
#include "KR/item/item_gem_Record.h"
#include "KR/BnsTableNames.h"

#include "PluginConfig.h"
#include <Windows.h>
#ifdef _DEBUG
#include <iostream>
#include <utility>
#endif // _DEBUG
#include "SkillTraitJobstyleHelper.cpp"
#include "DatafilePluginsdk.h"

using namespace BnsTables;

std::unique_ptr<SkillIdManager> g_SkillIdManager;

__int16 SkillIdManager::GetSkillshowTableId() const {
	return skillshowTableId;
}

char SkillIdManager::GetJobIdForEnName(std::wstring const& enName) {
	if (auto it = jobNameMap.find(enName); it != jobNameMap.end()) {
		return it->second;
	}
	if (auto it = jobNameFallbackMap.find(enName); it != jobNameFallbackMap.end()) {
		return it->second;
	}
	return 0;
}

bool SkillIdManager::SetupSkillShowTableId() {
	if (this->dataManager == nullptr) {
		return false;
	}
	const auto skillshow3Table = GetTable(this->dataManager, L"skillshow3");
	const auto tableId = skillshow3Table->_tabledef->type;
	if (tableId == 0) {
		return false;
	}
	skillshowTableId = tableId;
	return true;
}

std::unordered_set<int> SkillIdManager::GetInheritedIds(int id) {
	if (!versionCheckSuccess.contains(L"skill-inheritance") || !versionCheckSuccess[L"skill-inheritance"]) {
		return {};
	}
	const auto table = GetTable(this->dataManager, L"skill-inheritance");
	if (table == nullptr) return {};
	auto innerIter = table->__vftable->createInnerIter(table);
	std::unordered_set<int> inheritedIds;
	do {
		if (!innerIter->_vtptr->IsValid(innerIter)) continue;
#ifdef _BNSEU
		auto record = (EU::skill_inheritance_Record*)innerIter->_vtptr->Ptr(innerIter);

#elif _BNSKR
		auto record = (KR::skill_inheritance_Record*)innerIter->_vtptr->Ptr(innerIter);
#endif
		if (record == nullptr) continue;
		if (record->parent_skill_id != id) continue;
		if (std::ranges::find(idExclusionList.begin(), idExclusionList.end(), record->key.skill_id) != idExclusionList.end()) continue;
		inheritedIds.insert(record->key.skill_id);
		auto nestedIds = GetInheritedIds(record->key.skill_id);
		inheritedIds.insert(nestedIds.begin(), nestedIds.end());
	} while (innerIter->_vtptr->Next(innerIter));
	//table->__vftable->removeInnerIter(table, innerIter);
	return inheritedIds;
}

std::unordered_set<int> SkillIdManager::GetNeoChildSkillIds2(int id) {
	if (!versionCheckSuccess.contains(L"skill-train-by-item") || !versionCheckSuccess[L"skill-train-by-item"]) {
		return {};
	}
	const auto table = GetTable(this->dataManager, L"skill-train-by-item");
	if (table == nullptr) return {};
	auto innerIter = table->__vftable->createInnerIter(table);
	std::unordered_set<int> childIds;
	if (innerIter == nullptr) return childIds; //On live server its empty
	do {
		if (!innerIter->_vtptr->IsValid(innerIter)) continue;
#ifdef _BNSEU
		auto record = (EU::skill_train_by_item_Record*)innerIter->_vtptr->Ptr(innerIter);

#elif _BNSKR
		auto record = (KR::skill_train_by_item_Record*)innerIter->_vtptr->Ptr(innerIter);
#endif
		if (record == nullptr) continue;
		if (auto skill3Key = Skill3KeyHelper::BuildKey(id, 1); record->main_origin_skill.Key != skill3Key) continue;
		if (std::ranges::find(idExclusionList.begin(), idExclusionList.end(), id) != idExclusionList.end()) continue;
		if (record->main_change_skill.Key != 0 && std::ranges::find(idExclusionList.begin(), idExclusionList.end(), Skill3KeyHelper::ExtractId(record->main_change_skill.Key)) == idExclusionList.end()) {
			childIds.insert(Skill3KeyHelper::ExtractId(record->main_change_skill.Key));
			auto nestedIds = GetNeoChildSkillIds(Skill3KeyHelper::ExtractId(record->main_change_skill.Key));
			childIds.insert(nestedIds.begin(), nestedIds.end());
		}
		for (auto changeSkillRef : record->sub_change_skill) {
			if (changeSkillRef.Key == 0) continue;
			if (std::ranges::find(idExclusionList.begin(), idExclusionList.end(), Skill3KeyHelper::ExtractId(changeSkillRef.Key)) != idExclusionList.end()) continue;
			childIds.insert(Skill3KeyHelper::ExtractId(changeSkillRef.Key));
			auto nestedIds = GetNeoChildSkillIds(Skill3KeyHelper::ExtractId(changeSkillRef.Key));
			childIds.insert(nestedIds.begin(), nestedIds.end());
		}
		for (auto changeSkillRef : record->sub_origin_skill) {
			if (changeSkillRef.Key == 0) continue;
			if (std::ranges::find(idExclusionList.begin(), idExclusionList.end(), Skill3KeyHelper::ExtractId(changeSkillRef.Key)) != idExclusionList.end()) continue;
			childIds.insert(Skill3KeyHelper::ExtractId(changeSkillRef.Key));
			auto nestedIds = GetNeoChildSkillIds(Skill3KeyHelper::ExtractId(changeSkillRef.Key));
			childIds.insert(nestedIds.begin(), nestedIds.end());
		}
	} while (innerIter->_vtptr->Next(innerIter));
	//table->__vftable->removeInnerIter(table, innerIter);
	return childIds;
}

std::unordered_set<int> SkillIdManager::GetNeoChildSkillIds(int id) {
	if (!versionCheckSuccess.contains(L"skillbookcatalogueitem") || !versionCheckSuccess[L"skillbookcatalogueitem"]) {
		return {};
	}
	const auto table = GetTable(this->dataManager, L"skillbookcatalogueitem");
	if (table == nullptr) return {};
	auto innerIter = table->__vftable->createInnerIter(table);
	std::unordered_set<int> childIds;
	if (innerIter == nullptr) return childIds; //On live server its empty
	do {
		if (!innerIter->_vtptr->IsValid(innerIter)) continue;
#ifdef _BNSEU
		auto record = (EU::skillbookcatalogueitem_Record*)innerIter->_vtptr->Ptr(innerIter);

#elif _BNSKR
		auto record = (KR::skillbookcatalogueitem_Record*)innerIter->_vtptr->Ptr(innerIter);
#endif
		if (record == nullptr) continue;
		if (auto skill3Key = Skill3KeyHelper::BuildKey(id, 1); record->parent_skill.Key != skill3Key) continue;
		if (std::ranges::find(idExclusionList.begin(), idExclusionList.end(), id) != idExclusionList.end()) continue;
		if (record->base_skill.Key != 0 && std::ranges::find(idExclusionList.begin(), idExclusionList.end(), Skill3KeyHelper::ExtractId(record->base_skill.Key)) == idExclusionList.end()) {
			childIds.insert(Skill3KeyHelper::ExtractId(record->base_skill.Key));
			auto nestedIds = GetNeoChildSkillIds(Skill3KeyHelper::ExtractId(record->base_skill.Key));
			childIds.insert(nestedIds.begin(), nestedIds.end());
		}
		for (auto changeSkillRef : record->change_skill) {
			if (changeSkillRef.Key == 0) continue;
			if (std::ranges::find(idExclusionList.begin(), idExclusionList.end(), Skill3KeyHelper::ExtractId(changeSkillRef.Key)) != idExclusionList.end()) continue;
			childIds.insert(Skill3KeyHelper::ExtractId(changeSkillRef.Key));
			auto nestedIds = GetNeoChildSkillIds(Skill3KeyHelper::ExtractId(changeSkillRef.Key));
			childIds.insert(nestedIds.begin(), nestedIds.end());
		}
	} while (innerIter->_vtptr->Next(innerIter));
	//table->__vftable->removeInnerIter(table, innerIter);
	return childIds;
}

std::unordered_set<int> SkillIdManager::GetChildSkillIds(int id) {
	if (!versionCheckSuccess.contains(L"skill3") || !versionCheckSuccess[L"skill3"]) {
		return {};
	}
	const auto table = GetTable(this->dataManager, L"skill3");
	if (table == nullptr) return {};
	auto innerIter = table->__vftable->createInnerIter(table);
	std::unordered_set<int> childIds;
	do {
		if (!innerIter->_vtptr->IsValid(innerIter)) continue;
		auto record = innerIter->_vtptr->Ptr(innerIter);
		if (record == nullptr) continue;
#ifdef _BNSEU
		if (record->subtype != (__int16)std::to_underlying(EU::skill3_RecordSubType::skill3_record_sub_active_skill)) continue;
		auto activeSkillRecord = (EU::skill3_active_skill_Record*)record;
#elif _BNSKR
		if (record->subtype != (__int16)std::to_underlying(KR::skill3_RecordSubType::skill3_record_sub_active_skill)) continue;
		auto activeSkillRecord = (KR::skill3_active_skill_Record*)record;
#endif

		if (activeSkillRecord->key.id != id) continue;
		if (std::ranges::find(idExclusionList.begin(), idExclusionList.end(), activeSkillRecord->key.id) != idExclusionList.end()) continue;
		for (auto key : activeSkillRecord->children_skill) {
			if (key.Key == 0) continue;
			auto idPtr = reinterpret_cast<__int32*>(&key);
			auto childSkillId = *idPtr;
			if (std::ranges::find(idExclusionList.begin(), idExclusionList.end(), childSkillId) != idExclusionList.end()) continue;
			childIds.insert(childSkillId);
			auto nestedIds = GetChildSkillIds(childSkillId);
			childIds.insert(nestedIds.begin(), nestedIds.end());
		}
	} while (innerIter->_vtptr->Next(innerIter));
	//table->__vftable->removeInnerIter(table, innerIter);
	return childIds;
}
#ifdef _BNSEU
void SkillIdManager::AddIds(EU::skill_trait_Record const* record, int const* ids, int size, SkillIdsForJob& skillIdsForJobEntry) {
#elif _BNSKR
void SkillIdManager::AddIds(KR::skill_trait_Record const* record, int const* ids, int size, SkillIdsForJob & skillIdsForJobEntry) {
#endif
	auto jobStyleIndex = Data::SkillTraitJobStyleHelper::GetSpecIndexForJobStyleType(record->key.job_style);
	if (jobStyleIndex <= 0) return;
	if (!skillIdsForJobEntry.SkillIdsForSpec.contains(jobStyleIndex)) {
		skillIdsForJobEntry.SkillIdsForSpec[jobStyleIndex] = {};
	}

	for (int i = 0; i < size; i++) {
		if (ids[i] == 0) continue;
		if (std::ranges::find(idExclusionList.begin(), idExclusionList.end(), ids[i]) != idExclusionList.end()) continue;
		skillIdsForJobEntry.SkillIdsForSpec[jobStyleIndex].insert(ids[i]);
		auto inheritedIds = GetInheritedIds(ids[i]);
		skillIdsForJobEntry.SkillIdsForSpec[jobStyleIndex].insert(inheritedIds.begin(), inheritedIds.end());
	}
}
#ifdef _BNSEU
void  SkillIdManager::AddFixedIds(EU::skill_trait_Record const* record, SkillIdsForJob & skillIdsForJobEntry) {
#elif _BNSKR
void  SkillIdManager::AddFixedIds(KR::skill_trait_Record const* record, SkillIdsForJob & skillIdsForJobEntry) {
#endif
	int size = sizeof(record->fixed_skill3_id) / sizeof(record->fixed_skill3_id[0]);
	AddIds(record, record->fixed_skill3_id, size, skillIdsForJobEntry);
}

#ifdef _BNSEU
void  SkillIdManager::AddVariableIds(EU::skill_trait_Record const* record, SkillIdsForJob & skillIdsForJobEntry) {
#elif _BNSKR
void  SkillIdManager::AddVariableIds(KR::skill_trait_Record const* record, SkillIdsForJob & skillIdsForJobEntry) {
#endif
	int size = sizeof(record->variable_skill3_id) / sizeof(record->variable_skill3_id[0]);
	AddIds(record, record->variable_skill3_id, size, skillIdsForJobEntry);
}

void SkillIdManager::AddChildrenSkillIds(SkillIdsForJob & skillIdsForJobEntry) {
	for (auto const& [specIndex, skillIds] : skillIdsForJobEntry.SkillIdsForSpec) {
		for (auto id : skillIds) {
			auto childIds = GetChildSkillIds(id);
			skillIdsForJobEntry.SkillIdsForSpec[specIndex].insert(childIds.begin(), childIds.end());
		}
	}
	for (auto const& [specIndex, skillIds] : skillIdsForJobEntry.SkillIdsForSpec) {
		for (auto id : skillIds) {
			auto childIds = GetNeoChildSkillIds(id);
			skillIdsForJobEntry.SkillIdsForSpec[specIndex].insert(childIds.begin(), childIds.end());
		}
	}
	for (auto const& [specIndex, skillIds] : skillIdsForJobEntry.SkillIdsForSpec) {
		for (auto id : skillIds) {
			auto childIds = GetNeoChildSkillIds2(id);
			skillIdsForJobEntry.SkillIdsForSpec[specIndex].insert(childIds.begin(), childIds.end());
		}
	}
	for (auto const& [specIndex, skillIds] : skillIdsForJobEntry.SkillIdsForSpec) {
		for (auto id : skillIds) {
			auto inheritedIds = GetInheritedIds(id);
			skillIdsForJobEntry.SkillIdsForSpec[specIndex].insert(inheritedIds.begin(), inheritedIds.end());
		}
	}
	for (auto const& [specIndex, skillIds] : skillIdsForJobEntry.SkillIdsForSpec) {
		for (auto id : skillIds) {
			auto childIds = GetChildSkillIds(id);
			skillIdsForJobEntry.SkillIdsForSpec[specIndex].insert(childIds.begin(), childIds.end());
		}
	}
}

static bool containsSubstring(const wchar_t* str, const wchar_t* substr) {
	std::wstring s(str);
	std::wstring sub(substr);

	// Convert both strings to lowercase
	std::ranges::transform(s.begin(), s.end(), s.begin(), ::towlower);
	std::ranges::transform(sub.begin(), sub.end(), sub.begin(), ::towlower);

	return s.find(sub) != std::wstring::npos;
}

void SkillIdManager::AddSoulCoreChildren() {
	std::unordered_set<int> newIds;
	for (auto const& id : soulCoreSkills) {
		auto childIds = GetChildSkillIds(id);
		newIds.insert(childIds.begin(), childIds.end());
		auto childIds2 = GetNeoChildSkillIds(id);
		newIds.insert(childIds2.begin(), childIds2.end());
		auto childIds3 = GetNeoChildSkillIds2(id);
		newIds.insert(childIds3.begin(), childIds3.end());
		auto inheritedIds = GetInheritedIds(id);
		newIds.insert(inheritedIds.begin(), inheritedIds.end());
	}
	soulCoreSkills.insert(newIds.begin(), newIds.end());
}

bool SkillIdManager::SetupSoulCoreSkills() {
	if (!versionCheckSuccess.contains(L"item") || !versionCheckSuccess[L"item"]) {
		return false;
	}
	const auto table = GetTable(this->dataManager, L"item");
	if (table == nullptr) return false;
	auto innerIter = table->__vftable->createInnerIter(table);
	std::unordered_set<int> itemSkills;
	do {
		if (!innerIter->_vtptr->IsValid(innerIter)) continue;
		auto baseRecord = innerIter->_vtptr->Ptr(innerIter);
		if (baseRecord == nullptr) continue;
#ifdef _BNSEU
		if (baseRecord->subtype != EU::item_gem_Record::SubType()) continue;
		auto record = (EU::item_gem_Record*)baseRecord;
#elif _BNSKR
		if (baseRecord->subtype != KR::item_gem_Record::SubType()) continue;
		auto record = (KR::item_gem_Record*)baseRecord;
#endif
		if (record == nullptr || record->gem_type != 8 || record->skill3.Key == 0) continue;
		soulCoreSkills.insert(Skill3KeyHelper::ExtractId(record->skill3.Key));
	} while (innerIter->_vtptr->Next(innerIter));
	//table->__vftable->removeInnerIter(table, innerIter);
	AddSoulCoreChildren();
	return true;
}

std::unordered_set<int> SkillIdManager::GetItemSkills(int id) {
	if (!versionCheckSuccess.contains(L"itemskill") || !versionCheckSuccess[L"itemskill"]) {
		return {};
	}
	const auto table = GetTable(this->dataManager, L"itemskill");
	if (table == nullptr) return {};
	auto innerIter = table->__vftable->createInnerIter(table);
	std::unordered_set<int> itemSkills;
	do {
		if (!innerIter->_vtptr->IsValid(innerIter)) continue;
#ifdef _BNSEU
		auto record = (EU::itemskill_Record*)innerIter->_vtptr->Ptr(innerIter);
#elif _BNSKR
		auto record = (KR::itemskill_Record*)innerIter->_vtptr->Ptr(innerIter);
#endif
		if (record == nullptr) continue;
		if (record->skill_id != id) continue;
		auto itemSimSkillId = static_cast<__int32>(record->item_sim_skill.Key);
		if (itemSimSkillId == 0) continue;
		itemSkills.insert(itemSimSkillId);
	} while (innerIter->_vtptr->Next(innerIter));
	//table->__vftable->removeInnerIter(table, innerIter);
	return itemSkills;
}

void SkillIdManager::AddItemSkills(SkillIdsForJob & skillIdsForJobEntry) {
	for (auto const& [specIndex, skillIds] : skillIdsForJobEntry.SkillIdsForSpec) {
		for (auto id : skillIds) {
			auto itemSkills = GetItemSkills(id);
			globalItemSkillIds.insert(itemSkills.begin(), itemSkills.end());
			skillIdsForJobEntry.SkillIdsForSpec[specIndex].insert(itemSkills.begin(), itemSkills.end());
		}
	}
}

bool SkillIdManager::SetupSkillIdsForJob(char jobId) {
	if (!versionCheckSuccess.contains(L"skill-trait") || !versionCheckSuccess[L"skill-trait"]) {
		return false;
	}
	const auto table = GetTable(this->dataManager, L"skill-trait");
	if (table == nullptr) return false;
	auto skillIdsForJobEntry = SkillIdsForJob();
	skillIdsForJobEntry.JobId = jobId;

	auto innerIter = table->__vftable->createInnerIter(table);
	//loop
#ifdef _BNSEU
	std::vector<EU::skill_trait_Record> filteredTraitRecords;
#elif _BNSKR
	std::vector<KR::skill_trait_Record> filteredTraitRecords;
#endif
	do {
		if (!innerIter->_vtptr->IsValid(innerIter)) continue;
#ifdef _BNSEU
		auto record = (EU::skill_trait_Record*)innerIter->_vtptr->Ptr(innerIter);
#elif _BNSKR
		auto record = (KR::skill_trait_Record*)innerIter->_vtptr->Ptr(innerIter);
#endif
		if (record == nullptr) continue;
		if (record->key.job != jobId) continue;
		AddFixedIds(record, skillIdsForJobEntry);
		AddVariableIds(record, skillIdsForJobEntry);
	} while (innerIter->_vtptr->Next(innerIter));
	AddChildrenSkillIds(skillIdsForJobEntry);
	AddItemSkills(skillIdsForJobEntry);
	skillIdsForJobMap[jobId] = skillIdsForJobEntry;
	//table->__vftable->removeInnerIter(table, innerIter);
	return true;
}

std::unordered_set<unsigned __int64> SkillIdManager::GetEffectIdsForEffectGroup(unsigned __int64 effectGroupId) {
	if (!versionCheckSuccess.contains(L"effect-group") || !versionCheckSuccess[L"effect-group"]) {
		return {};
	}
	const auto table = GetTable(this->dataManager, L"effect-group");
	if (table == nullptr) return  { };
	auto record = table->__vftable->Find(table, effectGroupId);
	if (record == nullptr) return  { };
#ifdef _BNSEU
	auto effectGroupRecord = (EU::effect_group_Record*)record;
#elif _BNSKR
	auto effectGroupRecord = (KR::effect_group_Record*)record;
#endif
	std::unordered_set<unsigned __int64> effectIds;
	for (auto effectId : effectGroupRecord->effect) {
		if (effectId.Key == 0) continue;
		effectIds.insert(effectId.Key);
	}
	return effectIds;
}

void SkillIdManager::AddEffectsForSkillIds(const std::unordered_set<int>&skillIds, DrMultiKeyTable* const table, std::unordered_set<unsigned long long>&effectSet)
{
	for (auto id : skillIds) {
		auto innerIter = table->__vftable->createInnerIter(table);
		do {
			if (!innerIter->_vtptr->IsValid(innerIter)) continue;
#ifdef _BNSEU
			auto record = (EU::skill3_Record*)innerIter->_vtptr->Ptr(innerIter);
#elif _BNSKR
			auto record = (KR::skill3_Record*)innerIter->_vtptr->Ptr(innerIter);
#endif
			if (record == nullptr) continue;
			if (record->key.id != id) continue;
#ifdef _BNSEU
			if (record->subtype != (__int16)EU::skill3_RecordSubType::skill3_record_sub_active_skill) continue;
#elif _BNSKR
			if (record->subtype != (__int16)KR::skill3_RecordSubType::skill3_record_sub_active_skill) continue;
#endif
			bool breakOuter = false;
			for (auto const& systematization : record->systematization) {
				if (systematization.Key == 0) continue;
				if (systematization.Key == 30 || systematization.Key == 31 || systematization.Key == 32) { //skip party buff and protect skills
					breakOuter = true;
					break;
				}
			}
			if (breakOuter) continue;
#ifdef _BNSEU
			auto activeSkillRecord = (EU::skill3_active_skill_Record*)record;
#elif _BNSKR
			auto activeSkillRecord = (KR::skill3_active_skill_Record*)record;
#endif
			for (auto passiveEffect : activeSkillRecord->passive_effect) {
				if (passiveEffect.Key == 0) continue;
				effectSet.insert(passiveEffect.Key);
			}
			for (auto castEffect : activeSkillRecord->cast_effect) {
				if (castEffect.Key == 0) continue;
				effectSet.insert(castEffect.Key);
			}

			for (auto swing_caster_effect : activeSkillRecord->swing_caster_effect_1) {
				if (swing_caster_effect.Key == 0) continue;
				effectSet.insert(swing_caster_effect.Key);
			}
			for (auto swing_caster_effect : activeSkillRecord->swing_caster_effect_2) {
				if (swing_caster_effect.Key == 0) continue;
				effectSet.insert(swing_caster_effect.Key);
			}
			for (auto swing_caster_effect : activeSkillRecord->swing_caster_effect_3) {
				if (swing_caster_effect.Key == 0) continue;
				effectSet.insert(swing_caster_effect.Key);
			}
			for (auto swing_caster_effect : activeSkillRecord->swing_caster_effect_4) {
				if (swing_caster_effect.Key == 0) continue;
				effectSet.insert(swing_caster_effect.Key);
			}
			for (auto swing_caster_effect : activeSkillRecord->swing_caster_effect_5) {
				if (swing_caster_effect.Key == 0) continue;
				effectSet.insert(swing_caster_effect.Key);
			}

			for (auto execCasterEffect : activeSkillRecord->exec_caster_effect_1) {
				if (execCasterEffect.Key == 0) continue;
				effectSet.insert(execCasterEffect.Key);
			}
			for (auto execCasterEffect : activeSkillRecord->exec_caster_effect_2) {
				if (execCasterEffect.Key == 0) continue;
				effectSet.insert(execCasterEffect.Key);
			}
			for (auto execCasterEffect : activeSkillRecord->exec_caster_effect_3) {
				if (execCasterEffect.Key == 0) continue;
				effectSet.insert(execCasterEffect.Key);
			}
			for (auto execCasterEffect : activeSkillRecord->exec_caster_effect_4) {
				if (execCasterEffect.Key == 0) continue;
				effectSet.insert(execCasterEffect.Key);
			}
			for (auto execCasterEffect : activeSkillRecord->exec_caster_effect_5) {
				if (execCasterEffect.Key == 0) continue;
				effectSet.insert(execCasterEffect.Key);
			}
			for (auto execEffectGroupId : activeSkillRecord->exec_effect_1) {
				if (execEffectGroupId.Key == 0) continue;
				auto effectIds = GetEffectIdsForEffectGroup(execEffectGroupId.Key);
				effectSet.insert(effectIds.begin(), effectIds.end());
			}
			for (auto execEffectGroupId : activeSkillRecord->exec_effect_2) {
				if (execEffectGroupId.Key == 0) continue;
				auto effectIds = GetEffectIdsForEffectGroup(execEffectGroupId.Key);
				effectSet.insert(effectIds.begin(), effectIds.end());
			}
			for (auto execEffectGroupId : activeSkillRecord->exec_effect_3) {
				if (execEffectGroupId.Key == 0) continue;
				auto effectIds = GetEffectIdsForEffectGroup(execEffectGroupId.Key);
				effectSet.insert(effectIds.begin(), effectIds.end());
			}
			for (auto execEffectGroupId : activeSkillRecord->exec_effect_4) {
				if (execEffectGroupId.Key == 0) continue;
				auto effectIds = GetEffectIdsForEffectGroup(execEffectGroupId.Key);
				effectSet.insert(effectIds.begin(), effectIds.end());
			}
			for (auto execEffectGroupId : activeSkillRecord->exec_effect_5) {
				if (execEffectGroupId.Key == 0) continue;
				auto effectIds = GetEffectIdsForEffectGroup(execEffectGroupId.Key);
				effectSet.insert(effectIds.begin(), effectIds.end());
			}
		} while (innerIter->_vtptr->Next(innerIter));
		//table->__vftable->removeInnerIter(table, innerIter);
	}
}

bool SkillIdManager::SetupEffectIdsForJob(char jobId) {
	if (!versionCheckSuccess.contains(L"skill3") || !versionCheckSuccess[L"skill3"]) {
		return false;
	}
	const auto table = GetTable(this->dataManager, L"skill3");
	if (table == nullptr) return false;
	auto effectIdsForJobEntry = EffectIdsForJob();
	effectIdsForJobEntry.JobId = jobId;

	auto skillIdsForJobEntry = skillIdsForJobMap.find(jobId);
	if (skillIdsForJobEntry == skillIdsForJobMap.end()) return false;
	auto const& skillIdsForJob = skillIdsForJobEntry->second;

	auto const& fixedTargetEffectIdsForJob = fixedTargetEffectIds.find(jobId);

	for (auto const& skillIdsForSpec = skillIdsForJob.SkillIdsForSpec; auto const& [specIndex, skillIds] : skillIdsForSpec) {
		if (fixedTargetEffectIdsForJob != fixedTargetEffectIds.end()) {
			auto const& fixedEffectIdsForSpec = fixedTargetEffectIdsForJob->second.EffectIdsForSpec.find(specIndex);
			if (fixedEffectIdsForSpec != fixedTargetEffectIdsForJob->second.EffectIdsForSpec.end()) {
				effectIdsForJobEntry.EffectIdsForSpec[specIndex].insert(fixedEffectIdsForSpec->second.begin(), fixedEffectIdsForSpec->second.end());
			}
		}
		AddEffectsForSkillIds(skillIds, table, effectIdsForJobEntry.EffectIdsForSpec[specIndex]);
	}
	effectIdsForJobMap[jobId] = effectIdsForJobEntry;
	return true;
}

bool SkillIdManager::SetupAllSkillIds() {
	skillIdsForJobMap.clear();
	globalItemSkillIds.clear();
	soulCoreSkills.clear();
	if (this->dataManager == nullptr) {
		return false;
	}
	auto table = GetTable(this->dataManager, L"zonetriggereventstage");
	if (table == nullptr) {
		return false;
	}
	for (auto const& jobId : jobIds) {
		if (SetupSkillIdsForJob(jobId)) {
			if (!SetupEffectIdsForJob(jobId)) {
				return false;
			}
		}
		else {
			return false;
		}
	}

	if (versionCheckSuccess.contains(L"skill3") && versionCheckSuccess[L"skill3"]) {
		const auto skillTable = GetTable(this->dataManager, L"skill3");
		if (skillTable != nullptr) {
			for (auto& skillId : GetAllFixedExclusionSkillIds()) {
				AddEffectsForSkillIds({ skillId }, skillTable, exclusionEffectIds[skillId]);
			}
		}
	}

	if (!SetupSoulCoreSkills()) {
		return false;
	}
	return true;
}

bool SkillIdManager::SetupJobNameMap() {
	if (this->dataManager == nullptr) {
		return false;
	}
	jobNameMap.clear();
	jobIds.clear();
	if (!versionCheckSuccess.contains(L"job") || !versionCheckSuccess[L"job"]) {
		jobIds = jobIdsFallback;
		return true;
	}
	const auto table = GetTable(this->dataManager, L"job");
	const auto textTable = GetTable(this->dataManager, L"text");
	if (table == nullptr || textTable == nullptr) {
		return false;
	}
	auto innerIter = table->__vftable->createInnerIter(table);
	do {
		if (!innerIter->_vtptr->IsValid(innerIter)) continue;
#ifdef _BNSEU
		auto record = (EU::job_Record*)innerIter->_vtptr->Ptr(innerIter);
#elif _BNSKR
		auto record = (KR::job_Record*)innerIter->_vtptr->Ptr(innerIter);
#endif
		if (record == nullptr) continue;
		jobIds.insert(record->key.job);
		if (record->name2.Key == 0) continue;
#ifdef _BNSEU
		auto enJobName = (EU::text_Record*)textTable->__vftable->Find(textTable, record->name2.Key);
#elif _BNSKR
		auto enJobName = (KR::text_Record*)textTable->__vftable->Find(textTable, record->name2.Key);
#endif
		jobNameMap[enJobName->text.ReadableText] = record->key.job;
#ifdef _DEBUG
		std::wcout << "{ L\"" << enJobName->text.ReadableText << "\", " << +record->key.job << " }," << std::endl;
#endif // _DEBUG
	} while (innerIter->_vtptr->Next(innerIter));
	//table->__vftable->removeInnerIter(table, innerIter);
	return true;
}

bool SkillIdManager::AllVersionsSuccess() const {
	auto allSuccess = std::ranges::all_of(versionCheckSuccess, [](auto const& kvp) { return kvp.second; });
	return allSuccess;
}

bool SkillIdManager::CompatabilityCheck() {
	if (this->dataManager == nullptr) {
		return false;
	}
	for (auto const& tableName : usedTables) {
		auto table = GetTable(this->dataManager, tableName.c_str());
		if (table == nullptr) {
			return false;
		}
#ifdef _DEBUG
		std::wcout << "Table " << tableName << " found." << std::endl;
#endif // _DEBUG
		auto tableDef = table->_tabledef;
		if (tableDef == nullptr) {
			continue;
		}
#ifdef _DEBUG
		std::wcout << "\tID: " << tableDef->type << std::endl;
		std::wcout << "\tVersion union: " << tableDef->version.ver << std::endl;
		std::wcout << "\tMajor Version: " << tableDef->version.major_ver << std::endl;
		std::wcout << "\tMinor Version: " << tableDef->version.minor_ver << std::endl;
		printf("\tAddress of %s is %p\n", "type", &tableDef->type);
		std::cout << std::endl;
#endif // _DEBUG
#ifdef _BNSEU
		auto confirmedVersion = EU::TableNames::GetTableVersion(EU::TableNames::GetTableId(tableName));
#elif _BNSKR
		auto confirmedVersion = KR::TableNames::GetTableVersion(KR::TableNames::GetTableId(tableName));
#endif
#ifdef _DEBUG
		std::wcout << "Confirmed version for " << tableName << " is " << confirmedVersion.Version.VersionKey << std::endl;
		std::wcout << "Confirmed major version for " << tableName << " is " << confirmedVersion.Version.MajorVersion << std::endl;
		std::wcout << "Confirmed minor version for " << tableName << " is " << confirmedVersion.Version.MinorVersion << std::endl;
		std::cout << std::endl;
#endif // _DEBUG

		if (tableDef->version.ver == confirmedVersion.Version.VersionKey) {
			versionCheckSuccess[tableName] = true;
		}
	}
	return true;
}

bool SkillIdManager::Setup() {
	try {
		if (auto successVersionCheck = CompatabilityCheck(); !successVersionCheck) return false;
		if (!AllVersionsSuccess()) {
			MessageBox(nullptr, L"AnimFilter version is not 100% compatible with the game version.\nSome Animations might not be removed but your game will not break.\nPlease update the plugin if available.", L"AnimFilter Version Mismatch", MB_OK | MB_ICONWARNING);
		}
		auto success1 = SetupJobNameMap();
		if (!success1) return false;
		auto success2 = SetupAllSkillIds();
		if (!success2) return false;
		auto success3 = SetupSkillShowTableId();
		if (success1 && success2 && success3) {
			SetupComplete.store(true);
			ResetIdsToFilter();
			ReapplyEffectFilters();
		}
		return success1 && success2 && success3;
	}
	catch (const std::exception& ex) {
		// Log the exception message if needed
#ifdef _DEBUG
		std::cerr << "Exception caught in Setup: " << ex.what() << std::endl;
#endif
		return false;
	}
	catch (...) {
		// Catch any other types of exceptions
#ifdef _DEBUG
		std::cerr << "Unknown exception caught in Setup." << std::endl;
#endif
		return false;
	}
}

bool SkillIdManager::SetupWithRetry() {
	const int maxRetries = 10;
	const std::chrono::milliseconds retryDelay(1000); // 1 second delay between retries
	for (int attempt = 1; attempt <= maxRetries; ++attempt) {
		if (Setup()) {
			return true; // Setup succeeded
		}
		// If not the last attempt, wait before retrying
		if (attempt < maxRetries) {
			std::this_thread::sleep_for(retryDelay);
		}
	}
	return false; // All attempts failed
}

void SkillIdManager::SetupAsync() {
	std::thread([this]() { this->SetupWithRetry(); }).detach();
}

Data::DataManager* SkillIdManager::GetDataManager() {
	return this->dataManager;
}

static void addIdsForSpec(std::unordered_set<int>&idsToFilter, std::unordered_map<int, std::unordered_set<int>>&skillIdsForSpec, int specIndex) {
	auto skillIdsForSpecRecord = skillIdsForSpec.find(specIndex);
	if (skillIdsForSpecRecord != skillIdsForSpec.end()) {
		for (auto id : skillIdsForSpecRecord->second) {
			idsToFilter.insert(id);
		}
	}
}

static void removeIdsForSpec(std::unordered_set<int>&idsToFilter, std::unordered_map<int, std::unordered_set<int>>&skillIdsForSpec, int specIndex) {
	auto skillIdsForSpecRecord = skillIdsForSpec.find(specIndex);
	if (skillIdsForSpecRecord != skillIdsForSpec.end()) {
		for (auto id : skillIdsForSpecRecord->second) {
			idsToFilter.erase(id);
		}
	}
}

void SkillIdManager::ResetIdsToFilter() {
	idsToFilter.clear();
	if (!g_PluginConfig->IsLoaded() || !g_PluginConfig->HasActiveProfile())
		return;
	auto& activeProfile = g_PluginConfig->GetActiveProfile();

	//iterate over filters
	for (auto const& jobOption : activeProfile.SkillFilters) {
		if (jobOption.Name.empty() || !jobOption.IsHideAny()) continue;
		auto jobId = GetJobIdForEnName(jobOption.Name);
		if (jobId == 0) continue;
		auto skillIdsForJobRecord = skillIdsForJobMap.find(jobId);
		if (skillIdsForJobRecord == skillIdsForJobMap.end()) continue;
		auto& skillIdsForJob = skillIdsForJobRecord->second;
		//add ids to filter if hide
		if (jobOption.HideSpec1) {
			addIdsForSpec(idsToFilter, skillIdsForJob.SkillIdsForSpec, 1);
		}
		if (jobOption.HideSpec2) {
			addIdsForSpec(idsToFilter, skillIdsForJob.SkillIdsForSpec, 2);
		}
		if (jobOption.HideSpec3) {
			addIdsForSpec(idsToFilter, skillIdsForJob.SkillIdsForSpec, 3);
		}
		//remove the ids from the filter if not hide to not remove shared ids
		if (!jobOption.HideSpec1)
		{
			removeIdsForSpec(idsToFilter, skillIdsForJob.SkillIdsForSpec, 1);
		}
		if (!jobOption.HideSpec2)
		{
			removeIdsForSpec(idsToFilter, skillIdsForJob.SkillIdsForSpec, 2);
		}
		if (!jobOption.HideSpec3)
		{
			removeIdsForSpec(idsToFilter, skillIdsForJob.SkillIdsForSpec, 3);
		}
	}

	//remove all itemskills if set to hide global
	if (activeProfile.HideGlobalItemSkills) {
		idsToFilter.insert(globalItemSkillIds.begin(), globalItemSkillIds.end());
	}

	//remove all soulcores 
	if (activeProfile.HideSoulCores) {
		idsToFilter.insert(soulCoreSkills.begin(), soulCoreSkills.end());
	}

	RemoveSpecialExclusionIds(idsToFilter, activeProfile);

	if (activeProfile.HideTaxi) {
		taxiExclusionIds = {};
	}
	else {
		taxiExclusionIds = defaultTaxiExclusionIds;
	}
	for (const auto& [skillId, text] : activeProfile.CustomSkillIdFilters) {
		idsToFilter.insert(skillId);
	}
	ResetEffectIdsToFilter();
}

void SkillIdManager::RemoveSpecialExclusionEffectIds(std::unordered_set<unsigned __int64>&effectIdSet, const AnimFilterConfig::AnimFilterProfile & activeProfile) {
	//remove bardTreeExclusionIds from idsToFilter if not hidetree
	if (!activeProfile.HideTree) {
		for (auto id : bardTreeExclusionIds) {
			for (auto effectId : exclusionEffectIds[id]) {
				effectIdSet.erase(effectId);
			}
		}
	}
	//remove projectileResistExclusionIds from idsToFilter if not hideProjectileResists
	if (!activeProfile.HideProjectileResists) {
		for (auto id : projectileResistExclusionIds) {
			for (auto effectId : exclusionEffectIds[id]) {
				effectIdSet.erase(effectId);
			}
		}
	}

	//remove grabExclusionIds from idsToFilter if not HideGrabs
	if (!activeProfile.HideGrabs) {
		for (auto id : grabExclusionIds) {
			for (auto effectId : exclusionEffectIds[id]) {
				effectIdSet.erase(effectId);
			}
		}
	}

	//remove wlTDExclusionIds from idsToFilter if not HideTimeDistortion
	if (!activeProfile.HideTimeDistortion) {
		for (auto id : wlTDExclusionIds) {
			for (auto effectId : exclusionEffectIds[id]) {
				effectIdSet.erase(effectId);
			}
		}
	}
}

void SkillIdManager::RemoveSpecialExclusionIds(std::unordered_set<int> &idSet, const AnimFilterConfig::AnimFilterProfile & activeProfile)
{
	//remove bardTreeExclusionIds from idsToFilter if not hidetree
	if (!activeProfile.HideTree) {
		for (auto id : bardTreeExclusionIds) {
			idSet.erase(id);
		}
	}
	//remove projectileResistExclusionIds from idsToFilter if not hideProjectileResists
	if (!activeProfile.HideProjectileResists) {
		for (auto id : projectileResistExclusionIds) {
			idSet.erase(id);
		}
	}

	//remove grabExclusionIds from idsToFilter if not HideGrabs
	if (!activeProfile.HideGrabs) {
		for (auto id : grabExclusionIds) {
			idSet.erase(id);
		}
	}

	//remove wlTDExclusionIds from idsToFilter if not HideTimeDistortion
	if (!activeProfile.HideTimeDistortion) {
		for (auto id : wlTDExclusionIds) {
			idSet.erase(id);
		}
	}
}

static void addEffectIdsForSpec(std::unordered_set<unsigned __int64>&idsToFilter, std::unordered_map<int, std::unordered_set<unsigned __int64>>&effectIdsForSpec, int specIndex) {
	auto effectIdsForSpecRecord = effectIdsForSpec.find(specIndex);
	if (effectIdsForSpecRecord != effectIdsForSpec.end()) {
		for (auto id : effectIdsForSpecRecord->second) {
			idsToFilter.insert(id);
		}
	}
}

static void removeEffectIdsForSpec(std::unordered_set<unsigned __int64>&idsToFilter, std::unordered_map<int, std::unordered_set<unsigned __int64>>&effectIdsForSpec, int specIndex) {
	auto effectIdsForSpecRecord = effectIdsForSpec.find(specIndex);
	if (effectIdsForSpecRecord != effectIdsForSpec.end()) {
		for (auto id : effectIdsForSpecRecord->second) {
			idsToFilter.erase(id);
		}
	}
}

void SkillIdManager::ResetEffectIdsToFilter() {
	effectIdsToFilter.clear();
	if (!g_PluginConfig->IsLoaded() || !g_PluginConfig->HasActiveProfile())
		return;
	auto& activeProfile = g_PluginConfig->GetActiveProfile();

	//iterate over filters
	for (auto const& jobOption : activeProfile.SkillFilters) {
		if (jobOption.Name.empty() || !jobOption.IsHideAny()) continue;
		auto jobId = GetJobIdForEnName(jobOption.Name);
		if (jobId == 0) continue;
		auto effectIdsForJobRecord = effectIdsForJobMap.find(jobId);
		if (effectIdsForJobRecord == effectIdsForJobMap.end()) continue;
		auto& effectIdsForJob = effectIdsForJobRecord->second;

		if (jobOption.HideSpec1) {
			addEffectIdsForSpec(effectIdsToFilter, effectIdsForJob.EffectIdsForSpec, 1);
		}
		if (jobOption.HideSpec2) {
			addEffectIdsForSpec(effectIdsToFilter, effectIdsForJob.EffectIdsForSpec, 2);
		}
		if (jobOption.HideSpec3) {
			addEffectIdsForSpec(effectIdsToFilter, effectIdsForJob.EffectIdsForSpec, 3);
		}

		if (!jobOption.HideSpec1)
		{
			removeEffectIdsForSpec(effectIdsToFilter, effectIdsForJob.EffectIdsForSpec, 1);
		}
		if (!jobOption.HideSpec2)
		{
			removeEffectIdsForSpec(effectIdsToFilter, effectIdsForJob.EffectIdsForSpec, 2);
		}
		if (!jobOption.HideSpec3)
		{
			removeEffectIdsForSpec(effectIdsToFilter, effectIdsForJob.EffectIdsForSpec, 3);
		}
	}
	RemoveSpecialExclusionEffectIds(effectIdsToFilter, activeProfile);
	if (versionCheckSuccess.contains(L"skill3") && versionCheckSuccess[L"skill3"] && !activeProfile.CustomSkillIdFilters.empty()) {
		const auto table = GetTable(this->dataManager, L"skill3");
		if (table != nullptr) {
			std::unordered_set<int> skillIdSet = { };
			for (auto const& [id, text] : activeProfile.CustomSkillIdFilters) {
				skillIdSet.insert(id);
			}
			AddEffectsForSkillIds(skillIdSet, table, effectIdsToFilter);
		}
	}
}

__int64 SkillIdManager::SkillShow3KeyHelper::BuildKey(__int32 id_, __int16 variation_id_, __int16 skillskin_id_) {
#ifdef _BNSEU
	EU::skillshow3_Record::Key key{};
#elif _BNSKR
	KR::skillshow3_Record::Key key{};
#endif
	key.id = id_;
	key.variation_id = variation_id_;
	key.skillskin_id = skillskin_id_;
	return std::bit_cast<__int64>(key);
}

#ifdef _BNSEU
BnsTables::EU::skillshow3_Record::Key SkillIdManager::SkillShow3KeyHelper::ExtractKey(__int64 key)
{
	EU::skillshow3_Record::Key parts{};
	parts.key = key;
	return parts;
}
#endif

#ifdef _BNSKR
BnsTables::KR::skillshow3_Record::Key SkillIdManager::SkillShow3KeyHelper::ExtractKey(__int64 key)
{
	KR::skillshow3_Record::Key parts{};
	parts.key = key;
	return parts;
}
#endif

__int32 SkillIdManager::SkillShow3KeyHelper::ExtractId(__int64 key) {
	return static_cast<__int32>(key);
}

void SkillIdManager::SetDataManager(Data::DataManager * ptr) {
	this->dataManager = ptr;
}

bool SkillIdManager::IsSetupComplete() const {
	return SetupComplete.load();
}

const std::unordered_set<int>& SkillIdManager::GetIdsToFilter() const {
	return idsToFilter;
}

void SkillIdManager::RestoreEffects() {
	if (this->dataManager == nullptr) {
		return;
	}
	if (!versionCheckSuccess.contains(L"effect") || !versionCheckSuccess[L"effect"]) {
		return;
	}
	const auto table = GetTable(this->dataManager, L"effect");
	if (table == nullptr) return;
	for (auto& [key, kvp] : effectRestoreList) {
		auto record = table->__vftable->Find(table, key);
		if (record == nullptr) continue;
#ifdef _BNSEU
		auto effectRecord = (EU::effect_Record*)record;
#elif _BNSKR
		auto effectRecord = (KR::effect_Record*)record;
#endif
		std::string attribute = "normal_component";
		if (kvp.contains(attribute)) effectRecord->normal_component[0] = kvp[attribute];
		attribute = "critical_component";
		if (kvp.contains(attribute)) effectRecord->critical_component[0] = kvp[attribute];
		attribute = "back_normal_component";
		if (kvp.contains(attribute)) effectRecord->back_normal_component[0] = kvp[attribute];
		attribute = "back_critical_component";
		if (kvp.contains(attribute)) effectRecord->back_critical_component[0] = kvp[attribute];
		attribute = "buff_continuance_component";
		if (kvp.contains(attribute)) effectRecord->buff_continuance_component[0] = kvp[attribute];
		attribute = "immune_buff_component";
		if (kvp.contains(attribute)) effectRecord->immune_buff_component[0] = kvp[attribute];
		attribute = "detach_show";
		if (kvp.contains(attribute)) effectRecord->detach_show[0] = kvp[attribute];
		attribute = "dispel_show";
		if (kvp.contains(attribute)) effectRecord->dispel_show[0] = kvp[attribute];
	}
	for (auto& [key, kvp] : effectSwapRestoreList) {
		auto record = table->__vftable->Find(table, key);
		if (record == nullptr) continue;
#ifdef _BNSEU
		auto effectRecord = (EU::effect_Record*)record;
#elif _BNSKR
		auto effectRecord = (KR::effect_Record*)record;
#endif
		std::string attribute = "normal_component";
		if (kvp.contains(attribute)) effectRecord->normal_component = kvp[attribute];
		attribute = "critical_component";
		if (kvp.contains(attribute)) effectRecord->critical_component = kvp[attribute];
		attribute = "back_normal_component";
		if (kvp.contains(attribute)) effectRecord->back_normal_component = kvp[attribute];
		attribute = "back_critical_component";
		if (kvp.contains(attribute)) effectRecord->back_critical_component = kvp[attribute];
		attribute = "buff_continuance_component";
		if (kvp.contains(attribute)) effectRecord->buff_continuance_component = kvp[attribute];
		attribute = "immune_buff_component";
		if (kvp.contains(attribute)) effectRecord->immune_buff_component = kvp[attribute];
		attribute = "detach_show";
		if (kvp.contains(attribute)) effectRecord->detach_show = kvp[attribute];
		attribute = "dispel_show";
		if (kvp.contains(attribute)) effectRecord->dispel_show = kvp[attribute];
	}

	//Clear
	for (auto& [key, kvp] : effectRestoreList) {
		kvp.clear();
	}
	for (auto& [key, kvp] : effectSwapRestoreList) {
		kvp.clear();
	}
	effectRestoreList.clear();
	effectSwapRestoreList.clear();
}

std::unordered_set<int> SkillIdManager::GetAllFixedExclusionSkillIds()
{
	std::unordered_set<int> allFixedExclusionSkillIds;
	allFixedExclusionSkillIds.insert(bardTreeExclusionIds.begin(), bardTreeExclusionIds.end());
	allFixedExclusionSkillIds.insert(projectileResistExclusionIds.begin(), projectileResistExclusionIds.end());
	allFixedExclusionSkillIds.insert(grabExclusionIds.begin(), grabExclusionIds.end());
	allFixedExclusionSkillIds.insert(wlTDExclusionIds.begin(), wlTDExclusionIds.end());
	return allFixedExclusionSkillIds;
}

#ifdef _BNSEU
void SkillIdManager::RemoveAnimationsForEffect(EU::effect_Record * effectRecord) {
#elif _BNSKR
void SkillIdManager::RemoveAnimationsForEffect(KR::effect_Record * effectRecord) {
#endif
	if (effectRestoreList.contains(effectRecord->key.key)) return;
	if (effectSwapRestoreList.contains(effectRecord->key.key)) return;
	std::string attribute = "normal_component";
	if (effectRecord->normal_component != nullptr && effectRecord->normal_component[0] != L'\0') {
		effectRestoreList[effectRecord->key.key][attribute] = effectRecord->normal_component[0];
		*effectRecord->normal_component = L'\0';
	}
	if (effectRecord->critical_component != nullptr && effectRecord->critical_component[0] != L'\0') {
		attribute = "critical_component";
		effectRestoreList[effectRecord->key.key][attribute] = effectRecord->critical_component[0];
		*effectRecord->critical_component = L'\0';
	}
	if (effectRecord->back_normal_component != nullptr && effectRecord->back_normal_component[0] != L'\0') {
		attribute = "back_normal_component";
		effectRestoreList[effectRecord->key.key][attribute] = effectRecord->back_normal_component[0];
		*effectRecord->back_normal_component = L'\0';
	}
	if (effectRecord->back_critical_component != nullptr && effectRecord->back_critical_component[0] != L'\0') {
		attribute = "back_critical_component";
		effectRestoreList[effectRecord->key.key][attribute] = effectRecord->back_critical_component[0];
		*effectRecord->back_critical_component = L'\0';
	}
	if (effectRecord->buff_continuance_component != nullptr && effectRecord->buff_continuance_component[0] != L'\0') {
		attribute = "buff_continuance_component";
		effectRestoreList[effectRecord->key.key][attribute] = effectRecord->buff_continuance_component[0];
		*effectRecord->buff_continuance_component = L'\0';
	}
	if (effectRecord->immune_buff_component != nullptr && effectRecord->immune_buff_component[0] != L'\0') {
		attribute = "immune_buff_component";
		effectRestoreList[effectRecord->key.key][attribute] = effectRecord->immune_buff_component[0];
		*effectRecord->immune_buff_component = L'\0';
	}
	if (effectRecord->detach_show != nullptr && effectRecord->detach_show[0] != L'\0') {
		attribute = "detach_show";
		effectRestoreList[effectRecord->key.key][attribute] = effectRecord->detach_show[0];
		*effectRecord->detach_show = L'\0';
	}
	if (effectRecord->dispel_show != nullptr && effectRecord->dispel_show[0] != L'\0') {
		attribute = "dispel_show";
		effectRestoreList[effectRecord->key.key][attribute] = effectRecord->dispel_show[0];
		*effectRecord->dispel_show = L'\0';
	}
}
#ifdef _BNSEU
void SkillIdManager::SwapAnimationsForEffect(EU::effect_Record * target, EU::effect_Record * animation) {
#elif _BNSKR
void SkillIdManager::SwapAnimationsForEffect(KR::effect_Record * target, KR::effect_Record * animation) {
#endif
	if (effectSwapRestoreList.contains(target->key.key)) return;
	for (auto [attribute, member1, member2] : {
		std::tuple<std::string, wchar_t*&, wchar_t*&>("normal_component", target->normal_component, animation->normal_component),
		std::tuple<std::string, wchar_t*&, wchar_t*&>("critical_component", target->critical_component, animation->critical_component),
		std::tuple<std::string, wchar_t*&, wchar_t*&>("back_normal_component", target->back_normal_component, animation->back_normal_component),
		std::tuple<std::string, wchar_t*&, wchar_t*&>("back_critical_component", target->back_critical_component, animation->back_critical_component),
		std::tuple<std::string, wchar_t*&, wchar_t*&>("buff_continuance_component", target->buff_continuance_component, animation->buff_continuance_component),
		std::tuple<std::string, wchar_t*&, wchar_t*&>("immune_buff_component", target->immune_buff_component, animation->immune_buff_component),
		std::tuple<std::string, wchar_t*&, wchar_t*&>("detach_show", target->detach_show, animation->detach_show),
		std::tuple<std::string, wchar_t*&, wchar_t*&>("dispel_show", target->dispel_show, animation->dispel_show),
		}) {
		effectSwapRestoreList[target->key.key][attribute] = member1;
		member1 = member2;
	}
}

void SkillIdManager::SwapEffects() {
	if (this->dataManager == nullptr) {
		return;
	}
	if (!g_PluginConfig->IsLoaded() || !g_PluginConfig->HasActiveProfile()) return;
	if (!versionCheckSuccess.contains(L"effect") || !versionCheckSuccess[L"effect"]) {
		return;
	}
	const auto table = GetTable(this->dataManager, L"effect");
	if (table == nullptr) return;
	for (auto& [effectKey, effectSwap] : g_PluginConfig->GetActiveProfile().EffectSwaps) {
		auto animationKey = effectSwap.AnimationEffectId;
#ifdef _BNSEU
		auto effectRecord = (EU::effect_Record*)table->__vftable->Find(table, effectKey);
		if (effectRecord == nullptr) continue;
		auto animationRecord = (EU::effect_Record*)table->__vftable->Find(table, animationKey);
#elif _BNSKR
		auto effectRecord = (KR::effect_Record*)table->__vftable->Find(table, effectKey);
		if (effectRecord == nullptr) continue;
		auto animationRecord = (KR::effect_Record*)table->__vftable->Find(table, animationKey);
#endif
		if (animationRecord == nullptr) continue;
		SwapAnimationsForEffect(effectRecord, animationRecord);
	}
}

void SkillIdManager::RemoveEffects() {
	if (this->dataManager == nullptr) {
		return;
	}
	if (!g_PluginConfig->IsLoaded() || !g_PluginConfig->HasActiveProfile()) return;
	if (!versionCheckSuccess.contains(L"effect") || !versionCheckSuccess[L"effect"]) {
		return;
	}
	const auto table = GetTable(this->dataManager, L"effect");
	if (table == nullptr) return;
	for (auto& effectFilters = g_PluginConfig->GetActiveProfile().EffectFilters; auto const& filter : effectFilters) {
		if (filter.IsAlias) continue; //TODO: add alias support
		auto record = table->__vftable->Find(table, filter.Key);
		if (record == nullptr) continue;
#ifdef _BNSEU
		auto effectRecord = (EU::effect_Record*)record;
#elif _BNSKR
		auto effectRecord = (KR::effect_Record*)record;
#endif
		RemoveAnimationsForEffect(effectRecord);
	}
	for (auto const effectId : effectIdsToFilter) {
		auto record = table->__vftable->Find(table, effectId);
		if (record == nullptr) continue;
#ifdef _BNSEU
		auto effectRecord = (EU::effect_Record*)record;
#elif _BNSKR
		auto effectRecord = (KR::effect_Record*)record;
#endif
		RemoveAnimationsForEffect(effectRecord);
	}
}

std::unordered_set<int> SkillIdManager::GetAllSkillIdsFromJobMap() {
	std::unordered_set<int> allSkillIds;
	for (auto const& [enName, skillIdsForJob] : skillIdsForJobMap) {
		for (auto const& [specIndex, skillIds] : skillIdsForJob.SkillIdsForSpec) {
			allSkillIds.insert_range(skillIds);
		}
	}
	return allSkillIds;
}

void SkillIdManager::ReapplyEffectFilters() {
	RestoreEffects();
	SwapEffects();
	RemoveEffects();
}

bool SkillIdManager::IsCriticalFail() const {
	return CriticalFail;
}

const std::unordered_map<__int32, __int16>& SkillIdManager::GetTaxiExclusionIdVariations() const {
	return taxiExclusionIdVariations;
}

const std::unordered_set<int>& SkillIdManager::GetTaxiSkillIds() const {
	return taxiExclusionIds;
}

__int64 SkillIdManager::Skill3KeyHelper::BuildKey(__int32 skillId, signed char variation_id)
{
#ifdef _BNSEU
	EU::skill3_Record::Key key{};
#elif _BNSKR
	KR::skill3_Record::Key key{};
#endif
	key.id = skillId;
	key.variation_id = variation_id;
	return std::bit_cast<__int64>(key);
}

__int32 SkillIdManager::Skill3KeyHelper::ExtractId(__int64 key)
{
	return static_cast<__int32>(key);
}
