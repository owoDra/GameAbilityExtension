// Copyright (C) 2023 owoDra

#include "AbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTagRelationshipMapping)


UAbilityTagRelationshipMapping::UAbilityTagRelationshipMapping(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel(
	const FGameplayTagContainer& AbilityTags, 
	FGameplayTagContainer* OutTagsToBlock,
	FGameplayTagContainer* OutTagsToCancel) const
{
	for (const auto& Each : AbilityTagRelationships)
	{
		if (AbilityTags.HasTag(Each.AbilityTag))
		{
			if (OutTagsToBlock)
			{
				OutTagsToBlock->AppendTags(Each.AbilityTagsToBlock);
			}
			if (OutTagsToCancel)
			{
				OutTagsToCancel->AppendTags(Each.AbilityTagsToCancel);
			}
		}
	}
}

void UAbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTags(
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer* OutActivationRequired,
	FGameplayTagContainer* OutActivationBlocked) const
{
	for (const auto& Each : AbilityTagRelationships)
	{
		if (AbilityTags.HasTag(Each.AbilityTag))
		{
			if (OutActivationRequired)
			{
				OutActivationRequired->AppendTags(Each.ActivationRequiredTags);
			}
			if (OutActivationBlocked)
			{
				OutActivationBlocked->AppendTags(Each.ActivationBlockedTags);
			}
		}
	}
}

bool UAbilityTagRelationshipMapping::IsAbilityCancelledByTag(
	const FGameplayTagContainer& AbilityTags,
	const FGameplayTag& ActionTag) const
{
	for (const auto& Each : AbilityTagRelationships)
	{
		if (Each.AbilityTag == ActionTag && Each.AbilityTagsToCancel.HasAny(AbilityTags))
		{
			return true;
		}
	}

	return false;
}
