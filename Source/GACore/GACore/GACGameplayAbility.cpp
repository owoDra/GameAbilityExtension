// Copyright (C) 2023 owoDra

#include "GACGameplayAbility.h"

#include "GACAbilitySystemComponent.h"
#include "AbilityTags.h"
#include "GACoreLogs.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GACGameplayAbility)


UGACGameplayAbility::UGACGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
}


bool UGACGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (auto* GACASC{ GetAbilitySystemComponent<UGACAbilitySystemComponent>() })
	{
		if (false)
		{
			if (OptionalRelevantTags)
			{
				OptionalRelevantTags->AddTag(TAG_Ability_ActivateFail_ActivationGroup);
			}
			return false;
		}
	}

	return true;
}

void UGACGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	if (!bCanBeCanceled && (ActivationPolicy == EAbilityActivationPolicy::Replaceable))
	{
		UE_LOG(LogGAC, Error, TEXT("SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable."), *GetNameSafe(this));
		return;
	}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

void UGACGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UGACGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

bool UGACGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
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

void UGACGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
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

FGameplayEffectContextHandle UGACGameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	return Super::MakeEffectContext(Handle, ActorInfo);
}

void UGACGameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);
}

bool UGACGameplayAbility::DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
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

	const auto* GACASC = Cast<UGACAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	// Expand our ability tags to add additional required/blocked tags

	if (GACASC)
	{
		GACASC->GetAdditionalActivationTagRequirements(AbilityTags, AllRequiredTags, AllBlockedTags);
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


bool UGACGameplayAbility::CanChangeActivationGroup(EAbilityActivationPolicy NewPolicy) const
{
	if (!IsInstantiated() || !IsActive())
	{
		return false;
	}

	if (ActivationPolicy == NewPolicy)
	{
		return true;
	}

	// This ability can't become replaceable if it can't be canceled.

	if ((NewPolicy == EAbilityActivationPolicy::Replaceable) && !CanBeCanceled())
	{
		return false;
	}

	return true;
}

bool UGACGameplayAbility::ChangeActivationGroup(EAbilityActivationPolicy NewPolicy)
{
	if (!CanChangeActivationGroup(NewPolicy))
	{
		return false;
	}

	if (ActivationPolicy != NewPolicy)
	{
		if(auto* GACASC{ GetAbilitySystemComponent() })
		{

		}

		ActivationPolicy = NewPolicy;
	}

	return true;
}


void UGACGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
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


UAbilitySystemComponent* UGACGameplayAbility::GetAbilitySystemComponent(TSubclassOf<UAbilitySystemComponent> Class) const
{
	return CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
}

APlayerController* UGACGameplayAbility::GetPlayerController(TSubclassOf<APlayerController> Class) const
{
	return CurrentActorInfo ? CurrentActorInfo->PlayerController.Get() : nullptr;
}

AController* UGACGameplayAbility::GetController(TSubclassOf<AController> Class) const
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

ACharacter* UGACGameplayAbility::GetCharacter(TSubclassOf<ACharacter> Class) const
{
	return CurrentActorInfo ? Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}

APawn* UGACGameplayAbility::GetPawn(TSubclassOf<APawn> Class) const
{
	return CurrentActorInfo ? Cast<APawn>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
}

AActor* UGACGameplayAbility::GetActor(TSubclassOf<AActor> Class) const
{
	return CurrentActorInfo ? CurrentActorInfo->AvatarActor.Get() : nullptr;
}
