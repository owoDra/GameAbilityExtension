// Copyright (C) 2023 owoDra

#include "GAEGameplayAbility.h"

#include "GAEAbilitySystemComponent.h"
#include "Cost/AbilityCost.h"
#include "GameplayEffect/GameplayEffect_GenericCooldown.h"
#include "GameplayTag/GAETags_Ability.h"
#include "GameplayTag/GAETags_Message.h"
#include "Type/AbilityFailureMessageTypes.h"
#include "Type/AbilityCooldownMessageTypes.h"
#include "GAExtLogs.h"
#include "GAExtStatGroup.h"

#include "Message/GameplayMessageSubsystem.h"

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

	CooldownGameplayEffectClass = UGameplayEffect_GenericCooldown::StaticClass();

#if WITH_EDITOR
	StaticClass()->FindPropertyByName(FName{ TEXTVIEW("CooldownGameplayEffectClass") })->SetPropertyFlags(CPF_AdvancedDisplay);
#endif
}


bool UGAEGameplayAbility::CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	const auto bResultCooldown{ CommitAbilityCooldown(Handle, ActorInfo, ActivationInfo, false, OptionalRelevantTags) };
	const auto bResultCosts{ CommitAbilityCost(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags) };

	return bResultCooldown || bResultCosts;
}

void UGAEGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	ListenToCooldownEnd(ActorInfo);

	for (const auto& Cost : AdditionalCosts)
	{
		if (Cost)
		{
			Cost->OnGiveAbility(this, ActorInfo, Spec);
		}
	}

	BP_OnGiveAbility();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UGAEGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	UnlistenToCooldownEnd(ActorInfo);

	for (const auto& Cost : AdditionalCosts)
	{
		if (Cost)
		{
			Cost->OnRemoveAbility(this, ActorInfo, Spec);
		}
	}

	BP_OnRemoveAbility();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UGAEGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	for (const auto& Cost : AdditionalCosts)
	{
		if (Cost)
		{
			Cost->OnAvatarSet(this, ActorInfo, Spec);
		}
	}

	BP_OnAvatarSet();

	Super::OnAvatarSet(ActorInfo, Spec);
}


#pragma region Ability Activation

bool UGAEGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	/**
	 * This is copy from UGameplayAbility::CanActivateAbility()
	 *
	 * Note:
	 *  If an update changes the code of the parent class, please update this code
	 */

	  ////////////////////////////////////////////////////////////////
	 // Don't set the actor info, CanActivate is called on the CDO
	////////////////////////////////////////////////////////////////

	// A valid AvatarActor is required. Simulated proxy check means only authority or autonomous proxies should be executing abilities.

	auto* const AvatarActor{ ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr };
	if (AvatarActor == nullptr || !ShouldActivateAbility(AvatarActor->GetLocalRole()))
	{
		return false;
	}

	// Make into a reference for simplicity

	static FGameplayTagContainer DummyContainer;
	DummyContainer.Reset();

	auto& OutTags{ OptionalRelevantTags ? *OptionalRelevantTags : DummyContainer };

	// Make sure the ability system component is valid, if not bail out.

	auto* const AbilitySystemComponent{ ActorInfo->AbilitySystemComponent.Get() };
	if (!AbilitySystemComponent)
	{
		return false;
	}

	if (AbilitySystemComponent->GetUserAbilityActivationInhibited())
	{
		/**
		 *	Input is inhibited (UI is pulled up, another ability may be blocking all other input, etc).
		 *	When we get into triggered abilities, we may need to better differentiate between CanActivate and CanUserActivate or something.
		 *	E.g., we would want LMB/RMB to be inhibited while the user is in the menu UI, but we wouldn't want to prevent a 'buff when I am low health'
		 *	ability to not trigger.
		 *
		 *	Basically: CanActivateAbility is only used by user activated abilities now. If triggered abilities need to check costs/cooldowns, then we may
		 *	want to split this function up and change the calling API to distinguish between 'can I initiate an ability activation' and 'can this ability be activated'.
		 */
		return false;
	}

	///
	/// Changes from parent code
	/// ----------------------->

	auto& AbilitySystemGlobals{ UAbilitySystemGlobals::Get() };

	const auto bCooldownReady{ AbilitySystemGlobals.ShouldIgnoreCooldowns() || CheckCooldown(Handle, ActorInfo, OptionalRelevantTags) };
	const auto bCostsReady{ AbilitySystemGlobals.ShouldIgnoreCosts() || CheckCost(Handle, ActorInfo, OptionalRelevantTags) };

	if (ActivationPolicy == EAbilityActivationPolicy::Default)
	{
		if (!bCooldownReady || !bCostsReady)
		{
			return false;
		}
	}
	else if (ActivationPolicy == EAbilityActivationPolicy::CostOverCooldown)
	{
		if (!bCostsReady)
		{
			return false;
		}

	}
	else if (ActivationPolicy == EAbilityActivationPolicy::CooldownOverCost)
	{
		if (!bCooldownReady)
		{
			return false;
		}
	}

	/// <-----------------------
	/// Changes from parent code
	/// 

	// If the ability's tags are blocked, or if it has a "Blocking" tag or is missing a "Required" tag, then it can't activate.

	if (!DoesAbilitySatisfyTagRequirements(*AbilitySystemComponent, SourceTags, TargetTags, OptionalRelevantTags))
	{	
		/*if (FScopedCanActivateAbilityLogEnabler::IsLoggingEnabled())
		{
			ABILITY_VLOG(ActorInfo->OwnerActor.Get(), Verbose, TEXT("Ability could not be activated due to Blocking Tags or Missing Required Tags: %s"), *GetName());
		}*/
		return false;
	}

	auto* Spec{ AbilitySystemComponent->FindAbilitySpecFromHandle(Handle) };
	if (!Spec)
	{
		//ABILITY_LOG(Warning, TEXT("CanActivateAbility %s failed, called with invalid Handle"), *GetName());
		return false;
	}

	// Check if this ability's input binding is currently blocked

	if (AbilitySystemComponent->IsAbilityInputBlocked(Spec->InputID))
	{
		/*if (FScopedCanActivateAbilityLogEnabler::IsLoggingEnabled())
		{
			ABILITY_VLOG(ActorInfo->OwnerActor.Get(), Verbose, TEXT("Ability could not be activated due to blocked input ID %i: %s"), Spec->InputID, *GetName());
		}*/
		return false;
	}

	if (bHasBlueprintCanUse)
	{
		if (K2_CanActivateAbility(*ActorInfo, Handle, OutTags) == false)
		{
			//ABILITY_LOG(Log, TEXT("CanActivateAbility %s failed, blueprint refused"), *GetName());
			return false;
		}
	}

	return true;
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

#pragma endregion


#pragma region Cooldowns

bool UGAEGameplayAbility::CommitAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const bool ForceCooldown, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	if (IsCooldownAvailable())
	{
		return Super::CommitAbilityCooldown(Handle, ActorInfo, ActivationInfo, ForceCooldown, OptionalRelevantTags);
	}

	return false;
}

bool UGAEGameplayAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (IsCooldownAvailable())
	{
		return Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags);
	}

	return true;
}

void UGAEGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (IsCooldownAvailable())
	{
		if (ActorInfo->IsNetAuthority())
		{
			auto SpecHandle
			{
				MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, CooldownGameplayEffectClass, GetAbilityLevel(Handle, ActorInfo))
			};

			// Override Gameplay Effect values

			if (auto* Spec{ SpecHandle.Data.Get() })
			{
				// Override Spec values

				Spec->SetSetByCallerMagnitude(UGameplayEffect_GenericCooldown::NAME_SetByCaller_Cooldown, CooltimeOverride);
				Spec->DynamicGrantedTags.AddTag(CooldownTag);
			}

			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}

		// Broadcast

		BroadcastCooldownMassage(CooltimeOverride);
	}
}


const FGameplayTagContainer* UGAEGameplayAbility::GetCooldownTags() const
{
	// If CooldownGETags is valid, return it.

	if (CooldownGETags.IsValid())
	{
		return &CooldownGETags;
	}

	// If CooldownGETags is invalid, cache Tags to CooldownGETags and return CooldownGETags

	if (auto* OriginalTags{ Super::GetCooldownTags() })
	{
		CooldownGETags = *OriginalTags;

		if (CooldownTag.IsValid())
		{
			CooldownGETags.AddTag(CooldownTag);
		}

		return &CooldownGETags;
	}
	
	return nullptr;
}

bool UGAEGameplayAbility::IsCooldownAvailable() const
{
	// If CooldownTag not set, return false

	if (!CooldownTag.IsValid())
	{
		return false;
	}

	// If cooldown class not set, return false

	if (!CooldownGameplayEffectClass)
	{
		return false;
	}

	// If bShouldOverrideCooltime is true, but CooltimeOverride is 0, return false

	if (CooltimeOverride <= 0.0f)
	{
		return false;
	}

	return true;
}

void UGAEGameplayAbility::BroadcastCooldownMassage(float Duration) const
{
	if (IsLocallyControlled() && CooldownMessageTag.IsValid())
	{
		FAbilityCooldownMessage Message;
		Message.Ability = this;
		Message.OwnerActor = GetOwningActorFromActorInfo();
		Message.AvatarActor = GetAvatarActorFromActorInfo();
		Message.Duration = Duration;
	
		auto& MessageSubsystem{ UGameplayMessageSubsystem::Get(GetWorld()) };
		MessageSubsystem.BroadcastMessage(CooldownMessageTag, Message);
	}
}


void UGAEGameplayAbility::ListenToCooldownEnd(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (bShouldListenToCooldownEnd)
	{
		auto ASC{ ActorInfo->AbilitySystemComponent };
		if (ASC.IsValid())
		{
			ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &ThisClass::HandleAnyGameplayEffectRemoved);
		}
	}
}

void UGAEGameplayAbility::UnlistenToCooldownEnd(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (bShouldListenToCooldownEnd)
	{
		auto ASC{ ActorInfo->AbilitySystemComponent };
		if (ASC.IsValid())
		{
			ASC->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);
		}
	}
}

void UGAEGameplayAbility::HandleAnyGameplayEffectRemoved(const FActiveGameplayEffect& ActiveGameplayEffect)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("UGAEGameplayAbility::HandleAnyGameplayEffectRemoved()"), STAT_UGAEGameplayAbility_HandleAnyGameplayEffectRemoved, STATGROUP_Ability);

	const auto* CooldownTags{ GetCooldownTags() };

	if (CooldownTags && CooldownTags->Num() > 0)
	{
		const auto Query{ FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags) };

		if (Query.Matches(ActiveGameplayEffect))
		{
			OnCooldownEnd();
		}
	}
}

void UGAEGameplayAbility::OnCooldownEnd_Implementation()
{
	for (const auto& Cost : AdditionalCosts)
	{
		if (Cost)
		{
			Cost->OnCooldownEnd(this, GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());
		}
	}

	BroadcastCooldownMassage(0.0f);
}

#pragma endregion


#pragma region Costs

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

#pragma endregion


#pragma region Activation Failure

void UGAEGameplayAbility::OnAbilityFailedToActivate_Implementation(const FGameplayTagContainer& FailedReason) const
{
	auto bFailureMessageFound{ false };
	auto bFailureMontageFound{ false };

	for (const auto& Reason : FailedReason)
	{
		if (!bFailureMessageFound)
		{
			if (const auto* FoundMessage{ FailureTagToUserFacingMessages.Find(Reason) })
			{
				FAbilityFailureMessage Message;
				Message.PlayerController = GetActorInfo().PlayerController.Get();
				Message.FailureTags = FailedReason;
				Message.UserFacingReason = *FoundMessage;

				auto& MessageSubsystem{ UGameplayMessageSubsystem::Get(GetWorld()) };
				MessageSubsystem.BroadcastMessage(TAG_Message_Ability_ActivateFail_UserFacingMessage, Message);

				bFailureMessageFound = true;
			}
		}

		if (!bFailureMontageFound)
		{
			if (auto FoundMontage{ FailureTagToAnimMontage.FindRef(Reason) })
			{
				FAbilityFailureMontageMessage Message;
				Message.PlayerController = GetActorInfo().PlayerController.Get();
				Message.FailureTags = FailedReason;
				Message.FailureMontage = FoundMontage;

				auto& MessageSubsystem{ UGameplayMessageSubsystem::Get(GetWorld()) };
				MessageSubsystem.BroadcastMessage(TAG_Message_Ability_ActivateFail_PlayMontage, Message);

				bFailureMontageFound = true;
			}
		}

		if (bFailureMessageFound && bFailureMontageFound)
		{
			return;
		}
	}
}

#pragma endregion


#pragma region Utilities

UAbilitySystemComponent* UGAEGameplayAbility::GetAbilitySystemComponent(TSubclassOf<UAbilitySystemComponent> Class) const
{
	return CurrentActorInfo ? CurrentActorInfo->AbilitySystemComponent.Get() : nullptr;
}

UMovementComponent* UGAEGameplayAbility::GetMovementComponent(TSubclassOf<UMovementComponent> Class) const
{
	return CurrentActorInfo ? CurrentActorInfo->MovementComponent.Get() : nullptr;
}

USkeletalMeshComponent* UGAEGameplayAbility::GetSkeletalMeshComponent(TSubclassOf<USkeletalMeshComponent> Class) const
{
	return CurrentActorInfo ? CurrentActorInfo->SkeletalMeshComponent.Get() : nullptr;
}

UAnimInstance* UGAEGameplayAbility::GetAnimInstance(TSubclassOf<UAnimInstance> Class) const
{
	return CurrentActorInfo ? CurrentActorInfo->AnimInstance.Get() : nullptr;
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

#pragma endregion
