#pragma once

#include <vector>
#include <cstdint>

struct CTrinketOption
{
    int entryId = 0;
    CString displayName;
    CString simcToken;
    CString sourceType;
    CString sourceName;
    uint32_t itemId = 0;
    int defaultItemLevel = 0;
    CString defaultBonusId;
    CString defaultContext;
    bool uniqueEquipped = true;
    bool onUse = false;
};

const std::vector<CTrinketOption>& GetMidnightTrinketOptions();
const CTrinketOption* FindMidnightTrinketOption(int entryId);
