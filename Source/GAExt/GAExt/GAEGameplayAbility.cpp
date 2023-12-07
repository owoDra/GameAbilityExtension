// Copyright (C) 2023 owoDra

#include "GAEGameplayAbility.h"

#include "GAEAbilitySystemComponent.h"
#include "AbilityCost.h"
#include "AbilityTags.h"
#include "GAExtLogs.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAEGameplayAbility)


UGAEGameplayAbility::UGAEGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}


bool UGAEGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UGAEGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UGAEGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

bool UGAEGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}

	// Verify we can afford any additional costs

	for (const auto& Cost : AdditionalCosts)
	{
		if (Cost)
		{
			if (!Cost->CheckCost(this, Handle, ActorInfo, /*InOut*/ OptionalRelevantTags))
			{
				return false;
			}
		}
	}

	return true;
}

void UGAEGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	for (const auto& Cost : AdditionalCosts)
	{
		if (Cost)
		{
			Cost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
		}
	}
}

FGameplayEffectContextHandle UGAEGameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	return Super::MakeEffectContext(Handle, ActorInfo);
}

void UGAEGameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);
}

bool UGAEGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// Specialized version to handle death exclusion and AbilityTags expansion via ASC

	auto bBlocked{ false };
	auto bMissing{ false };

	auto& AbilitySystemGlobals{ UAbilitySystemGlobals::Get() };
	const auto& BlockedTag{ AbilitySystemGlobals.ActivateFailTagsBlockedTag };
	const auto& MissingTag{ AbilitySystemGlobals.ActivateFailTagsMissingTag };

	// Check if any of this ability's tags are currently blocked

	if (AbilitySystemComponent.AreAbilityTagsBlocked(AbilityTags))
	{
		bBlocked = true;
	}

	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// Expand our ability tags to add additional required/blocked tags

	if (const auto* GAEASC{ Cast<UGAEAbilitySystemComponent>(&AbilitySystemComponent) })
	{
		GAEASC->GetAdditionalActivationTagRequirements(AbilityTags, AllRequiredTags, AllBlockedTags);
	}

	// Check to see the required/blocked tags for this ability

	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;

		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}
		return false;
	}

	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}
		return false;
	}

	return true;
}


void UGAEGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
	const auto bIsPredicting{ Spec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting };

	// Try to activate if activation policy is on spawn.

	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && (ActivationMethod == EAbilityActivationMethod::OnSpawn))
	{
		auto* ASC{ ActorInfo->AbilitySystemComponent.Get() };
		const auto* AvatarActor{ ActorInfo->AvatarActor.Get() };

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.

		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const auto bIsLocalExecution{ (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly) };
			const auto bIsServerExecution{ (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated) };

			const auto bClientShouldActivate{ ActorInfo->IsLocallyControlled() && bIsLocalExecution };
			const auto bServerShouldActivate{ ActorInfo->IsNetAuthority() && bIsServerExecution };

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}


UAbilitySystemComponent* UGAEGameplayAbility::GetAbilitySystemComponent(TSubclassOf<UAbilitySystemComponent> Class) const
{
	return CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
}

APlayerController* UGAEGameplayAbility::GetPlayerController(TSubclassOf<APlayerController> Class) const
{
	return CurrentActorInfo ? CurrentActorInfo->PlayerController.Get() : nullptr;
}

AController* UGAEGameplayAbility::GetController(TSubclassOf<AController> Class) const
{
	if (CurrentActorInfo)
	{
		if (auto * PC{ CurrentActorInfo->PlayerController.Get() })
		{
			return PC;
		}

		auto* TestActor{ CurrentActorInfo->OwnerActor.Get() };
		while (TestActor)
		{
			if (auto* C{ Cast<AController>(TestActor) })
			{
				return C;
			}

			if (auto* Pawn{ Cast<APawn>(TestActor) })
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

ACharacter* UGAEGameplayAbility::GetCharacter(TSubclassOf<ACharacter> Class) const
{
	return CurrentActorInfo ? Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}

APawn* UGAEGameplayAbility::GetPawn(TSubclassOf<APawn> Class) const
{
	return CurrentActorInfo ? Cast<APawn>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}

AActor* UGAEGameplayAbility::GetActor(TSubclassOf<AActor> Class) const
{
	return CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;
}
