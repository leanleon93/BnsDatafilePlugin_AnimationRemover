#include "hooks.h"
#include "PluginConfig.h"
#include "SkillIdManager.h"
#include "Data.h"

using namespace BnsTables::Dynamic;
// For performance measurement in profiling builds
#ifdef _PROFILING
#include <chrono>
#define HKSKILLUSE_LOG(fmt, ...) \
    do { char buf[160]; snprintf(buf, sizeof(buf), fmt, __VA_ARGS__); OutputDebugStringA(buf); } while(0)
#else
#define HKSKILLUSE_LOG(fmt, ...)
#endif
int64_t(__fastcall* oSkillUse)(void* thisPtr, skill3_Record** rdx, uint8_t  flagByte, uint64_t origin_id);
int64_t __fastcall hkSkillUse(void* thisPtr, skill3_Record** rdx, uint8_t  flagByte, uint64_t origin_id)
{
#ifdef _PROFILING
	auto start = std::chrono::high_resolution_clock::now();
#endif
	// Most likely early-outs first
	auto originType = GetGameObjectType(origin_id);
	if (originType != GameObject::TYPE::GO_PC) {
#ifdef _PROFILING
		auto oSkillStart = std::chrono::high_resolution_clock::now();
#endif
		auto oResult = oSkillUse(thisPtr, rdx, flagByte, origin_id);
#ifdef _PROFILING
		auto oSkillEnd = std::chrono::high_resolution_clock::now();
		auto end = std::chrono::high_resolution_clock::now();
		auto oSkillDuration = std::chrono::duration_cast<std::chrono::microseconds>(oSkillEnd - oSkillStart).count();
		auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		HKSKILLUSE_LOG("[AnimRemover] hkSkillUse early return: not PC in %lld us (oSkillUse: %lld us, overhead: %lld us)\n", totalDuration, oSkillDuration, totalDuration - oSkillDuration);
#endif
		return oResult;
	}

	if (!g_PluginConfig->AnimFilterEnabled() || !g_PluginConfig->IsLoaded() || !g_PluginConfig->HasActiveProfile() || !g_SkillIdManager->IsSetupComplete() || g_PluginConfig->getWorld == nullptr) {
#ifdef _PROFILING
		auto oSkillStart = std::chrono::high_resolution_clock::now();
#endif
		auto oResult = oSkillUse(thisPtr, rdx, flagByte, origin_id);
#ifdef _PROFILING
		auto oSkillEnd = std::chrono::high_resolution_clock::now();
		auto end = std::chrono::high_resolution_clock::now();
		auto oSkillDuration = std::chrono::duration_cast<std::chrono::microseconds>(oSkillEnd - oSkillStart).count();
		auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		HKSKILLUSE_LOG("[AnimRemover] hkSkillUse early return: not enabled/loaded/profile/setup in %lld us (oSkillUse: %lld us, overhead: %lld us)\n", totalDuration, oSkillDuration, totalDuration - oSkillDuration);
#endif
		return oResult;
	}

	auto& activeProfile = g_PluginConfig->GetActiveProfile();
	if (!activeProfile.HideOthersSoulCores && !activeProfile.HideAllOtherPlayerSkills) {
#ifdef _PROFILING
		auto oSkillStart = std::chrono::high_resolution_clock::now();
#endif
		auto oResult = oSkillUse(thisPtr, rdx, flagByte, origin_id);
#ifdef _PROFILING
		auto oSkillEnd = std::chrono::high_resolution_clock::now();
		auto end = std::chrono::high_resolution_clock::now();
		auto oSkillDuration = std::chrono::duration_cast<std::chrono::microseconds>(oSkillEnd - oSkillStart).count();
		auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		HKSKILLUSE_LOG("[AnimRemover] hkSkillUse early return: not hiding in %lld us (oSkillUse: %lld us, overhead: %lld us)\n", totalDuration, oSkillDuration, totalDuration - oSkillDuration);
#endif
		return oResult;
	}
#ifdef _BNSLIVE
	if (!activeProfile.HideAllOtherPlayerSkills) {
		// Soulcore hiding only supported on NEO
		return oSkillUse(thisPtr, rdx, flagByte, origin_id);
	}
#endif
	auto& ids = activeProfile.HideAllOtherPlayerSkills ? g_SkillIdManager->GetAllIdsToFilterFromOtherPlayers() : g_SkillIdManager->GetSoulcoreSkillIds();
	// Cache world pointer
	auto* world = g_PluginConfig->getWorld();
	if (ids.empty() || g_PluginConfig == nullptr || world == nullptr) {
#ifdef _PROFILING
		auto oSkillStart = std::chrono::high_resolution_clock::now();
#endif
		auto oResult = oSkillUse(thisPtr, rdx, flagByte, origin_id);
#ifdef _PROFILING
		auto oSkillEnd = std::chrono::high_resolution_clock::now();
		auto end = std::chrono::high_resolution_clock::now();
		auto oSkillDuration = std::chrono::duration_cast<std::chrono::microseconds>(oSkillEnd - oSkillStart).count();
		auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		HKSKILLUSE_LOG("[AnimRemover] hkSkillUse early return: ids empty/world null in %lld us (oSkillUse: %lld us, overhead: %lld us)\n", totalDuration, oSkillDuration, totalDuration - oSkillDuration);
#endif
		return oResult;
	}
	auto* player = world->_player;

	if (player != nullptr && origin_id != player->id && rdx && *rdx) {
		auto skillRecord = *rdx;
		if (ids.contains(skillRecord->key.id)) {
			static skill3_Record nullSkill = [] {
				skill3_Record s{};
				s.key.key = 0;
				s.key.id = 100;
				s.key.variation_id = 1;
				return s;
				}();
			skill3_Record* rdxNullSkill = &nullSkill;
#ifdef _PROFILING
			auto oSkillStart = std::chrono::high_resolution_clock::now();
#endif
			auto oResult = oSkillUse(thisPtr, &rdxNullSkill, flagByte, origin_id);
#ifdef _PROFILING
			auto oSkillEnd = std::chrono::high_resolution_clock::now();
			auto end = std::chrono::high_resolution_clock::now();
			auto oSkillDuration = std::chrono::duration_cast<std::chrono::microseconds>(oSkillEnd - oSkillStart).count();
			auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			HKSKILLUSE_LOG("[AnimRemover] hkSkillUse filtered skill in %lld us (oSkillUse: %lld us, overhead: %lld us)\n", totalDuration, oSkillDuration, totalDuration - oSkillDuration);
#endif
			return oResult;
		}
	}
#ifdef _PROFILING
	auto oSkillStart = std::chrono::high_resolution_clock::now();
#endif
	auto oResult = oSkillUse(thisPtr, rdx, flagByte, origin_id);
#ifdef _PROFILING
	auto oSkillEnd = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	auto oSkillDuration = std::chrono::duration_cast<std::chrono::microseconds>(oSkillEnd - oSkillStart).count();
	auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	HKSKILLUSE_LOG("[AnimRemover] hkSkillUse default return in %lld us (oSkillUse: %lld us, overhead: %lld us)\n", totalDuration, oSkillDuration, totalDuration - oSkillDuration);
#endif
	return oResult;
}
