#pragma once
#include "framework.h"

struct SimulationSettings
{
    // Basic Settings
    CString fightStyle;
    int duration;
    int iterations;
    int targetCount;
    BOOL useFlask;
    BOOL useFood;
    BOOL useRune;
    BOOL useAugment;
    BOOL useBloodlust;
    BOOL usePotion;

    // Advanced Settings
    BOOL calculateScaleFactors;
    CString scaleOnly;
    BOOL reportDetails;
    int threads;
    BOOL singleActorBatch;
    BOOL optimizeExpressions;
    double targetError;

    // Midnight Expansion
    CString temporaryEnchant;
    BOOL useCrucibleViolence;
    BOOL useCrucibleSustenance;
    BOOL useCruciblePredation;

    // Buff Overrides
    BOOL useArcaneIntellect;
    BOOL usePowerWordFortitude;
    BOOL useBattleShout;
    BOOL useMysticTouch;
    BOOL useChaosBrand;
    BOOL useSkyfury;
    BOOL useMarkOfTheWild;
    BOOL useHuntersMark;
    BOOL useBleeding;

    SimulationSettings()
        : fightStyle(_T("patchwerk"))
        , duration(300)
        , iterations(10000)
        , targetCount(1)
        , useFlask(TRUE)
        , useFood(TRUE)
        , useRune(TRUE)
        , useAugment(TRUE)
        , useBloodlust(TRUE)
        , usePotion(TRUE)
        , calculateScaleFactors(FALSE)
        , scaleOnly(_T("strength,intellect,agility,crit,mastery,vers,haste,weapon_dps,weapon_offhand_dps"))
        , reportDetails(FALSE)
        , threads(0)
        , singleActorBatch(TRUE)
        , optimizeExpressions(TRUE)
        , targetError(0.0)
        , temporaryEnchant(_T("None"))
        , useCrucibleViolence(FALSE)
        , useCrucibleSustenance(FALSE)
        , useCruciblePredation(FALSE)
        , useArcaneIntellect(TRUE)
        , usePowerWordFortitude(TRUE)
        , useBattleShout(TRUE)
        , useMysticTouch(TRUE)
        , useChaosBrand(TRUE)
        , useSkyfury(TRUE)
        , useMarkOfTheWild(TRUE)
        , useHuntersMark(FALSE)
        , useBleeding(FALSE)
    {}
};
