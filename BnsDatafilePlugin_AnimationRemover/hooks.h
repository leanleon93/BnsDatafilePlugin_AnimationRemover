#pragma once
#include <cstdint>
#ifdef _BNSLIVE
#include <LIVE/Generated/include/Dynamic/skill3/AAA_skill3_RecordBase.h>
#else
#include <NEO/Generated/include/Dynamic/skill3/AAA_skill3_RecordBase.h>
#endif

extern int64_t(__fastcall* oSkillUse)(void* thisPtr, BnsTables::Dynamic::skill3_Record** rdx, uint8_t  flagByte, uint64_t origin_id);
int64_t __fastcall hkSkillUse(void* thisPtr, BnsTables::Dynamic::skill3_Record** rdx, uint8_t  flagByte, uint64_t origin_id);