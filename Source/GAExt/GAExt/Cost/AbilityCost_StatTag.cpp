// Copyright (C) 2024 owoDra

#include "AbilityCost_StatTag.h"

#include "GAEGameplayAbility.h"

#include "GameplayTag/GameplayTagStackInterface.h"

#include "GameFramework/PlayerState.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityCost_StatTag)


UAbilityCost_StatTag::UAbilityCost_StatTag(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


bool UAbilityCost_StatTag::CheckCost(const UGAEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	auto Result{ true };

	const auto AbilityLevel{ Ability->GetAbilityLevel(Handle, ActorInfo) };

	for (const auto& Cost : StatTagCosts)
	{
		if (auto* Interface{ GetCostTarget<IGameplayTagStackInterface>(Cost.Target) })
		{
			const auto CostReal{ Cost.Cost.GetValueAtLevel(AbilityLevel) };
			const auto CostValue{ FMath::TruncToInt(CostReal) };

			Result &= (Interface->GetStatTagStackCount(Cost.StatTag) >= CostValue);
		}
	}
	
	return Result;
}

void UAbilityCost_StatTag::ApplyCost(const UGAEGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// Must have authority

	if (!ActorInfo->IsNetAuthority())
	{
		return;
	}

	const auto AbilityLevel{ Ability->GetAbilityLevel(Handle, ActorInfo) };

	for (const auto& Cost : StatTagCosts)
	{
		if (auto* Interface{ GetCostTarget<IGameplayTagStackInterface>(Cost.Target) })
		{
			const auto CostReal{ Cost.Cost.GetValueAtLevel(AbilityLevel) };
			const auto CostValue{ FMath::TruncToInt(CostReal) };

			Interface->RemoveStatTagStack(Cost.StatTag, CostValue);
		}
	}
}

void UAbilityCost_StatTag::OnAvatarSet(const UGAEGameplayAbility* Ability, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	// Must have authority

	if (!ActorInfo->IsNetAuthority())
	{
		return;
	}

	const auto AbilityLevel{ Spec.Level };

	for (const auto& Cost : StatTagCosts)
	{
		if (Cost.bShouldInitStatTag)
		{
			if (auto* Interface{ GetCostTarget<IGameplayTagStackInterface>(Cost.Target) })
			{
				const auto MaxReal{ Cost.MaxValue.GetValueAtLevel(AbilityLevel) };
				const auto MaxValue{ FMath::TruncToInt(MaxReal) };

				Interface->SetMaxStatTagStack(Cost.StatTag, MaxValue);

				if (!Cost.bDoNotInitDefaultValue)
				{
					const auto DefaultReal{ Cost.DefaultValue.GetValueAtLevel(AbilityLevel) };
					const auto DefaultValue{ FMath::TruncToInt(DefaultReal) };

					Interface->SetStatTagStack(Cost.StatTag, DefaultValue);
				}
			}
		}
	}
}


UObject* UAbilityCost_StatTag::GetStatTagCostTarget(EStatTagCostTarget Type) const
{
	auto* GAEAbility{ GetOwnerAbility() };

	switch (Type)
	{
	case EStatTagCostTarget::SourceObject:
		return GAEAbility->GetCurrentSourceObject();
		break;

	case EStatTagCostTarget::Avatar:
		return GAEAbility->GetAvatarActorFromActorInfo();
		break;

	case EStatTagCostTarget::Owner:
		return GAEAbility->GetOwningActorFromActorInfo();
		break;

	case EStatTagCostTarget::Controller:
		return GAEAbility->GetController();
		break;

	case EStatTagCostTarget::PlayerState:
		return GAEAbility->GetPlayerState();
		break;

	default:
		return nullptr;
		break;
	}
}
