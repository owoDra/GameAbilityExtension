// Copyright (C) 2023 owoDra

#pragma once

#include "NativeGameplayTags.h"


///////////////////////////////////////////////////////
// Ability.ActivateFail

GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_IsDead);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_Cooldown);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_TagsBlocked);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_TagsMissing);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_Networking);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_ActivationGroup);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_ActivateFail_Cost);

///////////////////////////////////////////////////////
// Ability.Type

GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Movement);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Action);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Passive);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Skill);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Weapon);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_StatusChange);
GACORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Type_Misc);

///////////////////////////////////////////////////////
// Ability.Group

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Group);
