// Copyright (C) 2023 owoDra

#include "GAEAbilitySystemComponent.h"

#include "AbilityTagRelationshipMapping.h"
#include "GameplayTag/GAETags_Ability.h"
#include "GameplayTag/GAETags_Flag.h"
#include "GlobalAbilitySubsystem.h"
#include "GAExtLogs.h"

#include "Actor/GFCPlayerController.h"
#include "InitState/InitStateTags.h"
#include "InitState/InitStateComponent.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpec.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAEAbilitySystemComponent)


const FName UGAEAbilitySystemComponent::NAME_ActorFeatureName("AbilitySystem");

const FName UGAEAbilitySystemComponent::NAME_AbilityReady("AbilityReady");

UGAEAbilitySystemComponent::UGAEAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGAEAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.Condition = COND_None;

	DOREPLIFETIME_WITH_PARAMS_FAST(UGAEAbilitySystemComponent, TagRelationshipMapping, Params);
}


void UGAEAbilitySystemComponent::OnRegister()
{
	Super::OnRegister();

	// No more than two of these components should be added to a Pawn.

	TArray<UActorComponent*> Components;
	GetOwner()->GetComponents(StaticClass(), Components);
	ensureAlwaysMsgf((Components.Num() == 1), TEXT("Only one [%s] should exist on [%s]."), *GetNameSafe(StaticClass()), *GetNameSafe(GetOwner()));

	// Register this component in the GameFrameworkComponentManager.

	RegisterInitStateFeature();
}

void UGAEAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// Start listening for changes in the initialization state of all features 
	// related to the Pawn that owns this component.

	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Change the initialization state of this component to [Spawned]

	ensureMsgf(TryToChangeInitState(TAG_InitState_Spawned), TEXT("[%s] on [%s]."), *GetNameSafe(this), *GetNameSafe(GetOwner()));

	// Check if initialization process can continue

	CheckDefaultInitialization();
}

void UGAEAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	if (auto* GlobalAbilitySubsystem{ UWorld::GetSubsystem<UGlobalAbilitySubsystem>(GetWorld()) })
	{
		GlobalAbilitySubsystem->UnregisterASC(this);
	}

	Super::EndPlay(EndPlayReason);
}


void UGAEAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	check(InOwnerActor);
	check(InAvatarActor);

	auto* ActorInfo{ AbilityActorInfo.Get() };
	check(ActorInfo);
	
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (InAvatarActor && (InAvatarActor != ActorInfo->AvatarActor))
	{
		// Register with the global system once we actually have a pawn avatar. 
		// We wait until this time since some globally-applied effects may require an avatar.

		if (auto* GlobalAbilitySystem{ UWorld::GetSubsystem<UGlobalAbilitySubsystem>(GetWorld()) })
		{
			GlobalAbilitySystem->RegisterASC(this);
		}

		TryActivateAbilitiesOnSpawn();
	}

	CheckDefaultInitialization();
}


bool UGAEAbilitySystemComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	auto* Owner{ GetOwner() };

	/**
	 * [None] -> [Spawned]
	 */
	if (!CurrentState.IsValid() && DesiredState == TAG_InitState_Spawned)
	{
		// Check Owner

		if (Owner)
		{
			return true;
		}
	}

	/**
	 * [Spawned] -> [DataAvailable]
	 */
	else if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		if (!Manager->HasFeatureReachedInitState(GetOwner(), UInitStateComponent::NAME_ActorFeatureName, TAG_InitState_DataAvailable))
		{
			return false;
		}

		// Check Avatar/Owner Actor

		if (GetAvatarActor() && GetOwnerActor())
		{
			return true;
		}
	}

	/**
	 * [DataAvailable] -> [DataInitialized]
	 */
	else if (CurrentState == TAG_InitState_DataAvailable && DesiredState == TAG_InitState_DataInitialized)
	{
		return true;
	}

	/**
	 * [DataInitialized] -> [GameplayReady]
	 */
	else if (CurrentState == TAG_InitState_DataInitialized && DesiredState == TAG_InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UGAEAbilitySystemComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	UE_LOG(LogGAE, Log, TEXT("[%s] Ability System Component InitState Reached: %s"),
		GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"), *DesiredState.GetTagName().ToString());

	/**
	 * [Spawned] -> [DataAvailable]
	 */
	if (CurrentState == TAG_InitState_Spawned && DesiredState == TAG_InitState_DataAvailable)
	{
		UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(GetOwnerActor(), NAME_AbilityReady);
	}
}

void UGAEAbilitySystemComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UInitStateComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == TAG_InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void UGAEAbilitySystemComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain
	{
		TAG_InitState_Spawned,
		TAG_InitState_DataAvailable,
		TAG_InitState_DataInitialized,
		TAG_InitState_GameplayReady
	};

	ContinueInitStateChain(StateChain);
}


void UGAEAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (auto* PawnAvatar{ Cast<APawn>(GetAvatarActor()) })
	{
		if (!PawnAvatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

void UGAEAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();

	for (const auto& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const auto* GAEAbilityCDO{ Cast<UGAEGameplayAbility>(AbilitySpec.Ability) })
		{
			GAEAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
		}
	}
}

void UGAEAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}

void UGAEAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	if (const auto* GAEAbility{ Cast<const UGAEGameplayAbility>(Ability) })
	{
		GAEAbility->OnAbilityFailedToActivate(FailureReason);
	}
}

void UGAEAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();

	for (const auto& AbilitySpec : ActivatableAbilities.Items)
	{
		// Skip if not active.

		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		const auto& AbilityCDO{ AbilitySpec.Ability };

		// Cancel all the spawned instances, not the CDO.

		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			auto Instances{ AbilitySpec.GetAbilityInstances() };

			for (const auto& AbilityInstance : Instances)
			{
				if (ShouldCancelFunc(AbilityInstance, AbilitySpec.Handle))
				{
					if (AbilityInstance->CanBeCanceled())
					{
						AbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), AbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
					}
					else
					{
						UE_LOG(LogGAE, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *GetNameSafe(AbilityInstance));
					}
				}
			}
		}

		// Cancel the non-instanced ability CDO.

		else if (ShouldCancelFunc(AbilityCDO, AbilitySpec.Handle))
		{
			if (AbilityCDO->CanBeCanceled())
			{
				AbilityCDO->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), bReplicateCancelAbility);
			}
			else
			{
				UE_LOG(LogGAE, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *GetNameSafe(AbilityCDO));
			}
		}
	}
}


void UGAEAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	auto ModifiedBlockTags{ BlockTags };
	auto ModifiedCancelTags{ CancelTags };

	// Use the mapping to expand the ability tags into block and cancel tag

	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);
}

void UGAEAbilitySystemComponent::SetTagRelationshipMapping(const UAbilityTagRelationshipMapping* NewMapping)
{
	if (GetOwner()->HasAuthority())
	{
		if (TagRelationshipMapping != NewMapping)
		{
			TagRelationshipMapping = NewMapping;

			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TagRelationshipMapping, this);
		}
	}
}

void UGAEAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}


void UGAEAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.

	if (Spec.IsActive())
	{
		// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.

		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
	}
}

void UGAEAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.

	if (Spec.IsActive())
	{
		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.

		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
	}
}

void UGAEAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const auto& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UGAEAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const auto& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UGAEAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(TAG_Flag_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();


	// Process all abilities that activate when the input is held.

	for (const auto& SpecHandle : InputHeldSpecHandles)
	{
		if (const auto* AbilitySpec{ FindAbilitySpecFromHandle(SpecHandle) })
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				if (const auto* GAEAbilityCDO{ Cast<UGAEGameplayAbility>(AbilitySpec->Ability) })
				{
					if (GAEAbilityCDO->ActivationMethod == EAbilityActivationMethod::WhileInputActive)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}


	// Process all abilities that had their input pressed this frame.

	for (const auto& SpecHandle : InputPressedSpecHandles)
	{
		if (auto* AbilitySpec{ FindAbilitySpecFromHandle(SpecHandle) })
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.

					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					if (const auto* GAEAbilityCDO{ Cast<UGAEGameplayAbility>(AbilitySpec->Ability) })
					{
						if (GAEAbilityCDO->ActivationMethod == EAbilityActivationMethod::OnInputTriggered)
						{
							AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
						}
					}
				}
			}
		}
	}


	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send a input event to the ability because of the press.

	for (const auto& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}


	// Process all abilities that had their input released this frame.

	for (const auto& SpecHandle : InputReleasedSpecHandles)
	{
		if (auto* AbilitySpec{ FindAbilitySpecFromHandle(SpecHandle) })
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.

					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}


	// Clear the cached ability handles.

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UGAEAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UGAEAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	TShouldCancelAbilityFunc ShouldCancelFunc = [this](const UGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
	{
		if (auto* GAEAbility{ Cast<UGAEGameplayAbility>(Ability) })
		{
			const auto ActivationMethod{ GAEAbility->ActivationMethod };

			return ((ActivationMethod == EAbilityActivationMethod::OnInputTriggered) || (ActivationMethod == EAbilityActivationMethod::WhileInputActive));
		}

		return false;
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}
