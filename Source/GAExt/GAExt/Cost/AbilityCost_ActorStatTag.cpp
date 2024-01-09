// Copyright (C) 2024 owoDra

#include "AbilityCost_ActorStatTag.h"

#include "GAEGameplayAbility.h"

#include "GameplayTag/GameplayTagStackInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityCost_ActorStatTag)


UAbilityCost_ActorStatTag::UAbilityCost_ActorStatTag(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


bool UAbilityCost_ActorStatTag::CheckCost(const UGAEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	auto* TargetActor{ (Target == EConsumeStatTarget::OwningActor) ? ActorInfo->OwnerActor.Get() : ActorInfo->AvatarActor.Get() };

	if (auto* Interface{ Cast<IGameplayTagStackInterface>(TargetActor) })
	{
		const auto AbilityLevel{ Ability->GetAbilityLevel(Handle, ActorInfo) };
		const auto NumStacksReal{ Cost.GetValueAtLevel(AbilityLevel) };
		const auto NumStacks{ FMath::TruncToInt(NumStacksReal) };

		return (Interface->GetStatTagStackCount(ConsumeTag) >= NumStacks);
	}
	
	return false;
}

void UAbilityCost_ActorStatTag::ApplyCost(const UGAEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// Must have authority

	if (!ActorInfo->IsNetAuthority())
	{
		return;
	}

	auto* TargetActor{ (Target == EConsumeStatTarget::OwningActor) ? ActorInfo->OwnerActor.Get() : ActorInfo->AvatarActor.Get() };

	if (auto* Interface{ Cast<IGameplayTagStackInterface>(TargetActor) })
	{
		const auto AbilityLevel{ Ability->GetAbilityLevel(Handle, ActorInfo) };
		const auto NumStacksReal{ Cost.GetValueAtLevel(AbilityLevel) };
		const auto NumStacks{ FMath::TruncToInt(NumStacksReal) };

		Interface->RemoveStatTagStack(ConsumeTag, NumStacks);
	}
}
