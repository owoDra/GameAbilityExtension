// Copyright (C) 2024 owoDra

#include "AbilityTask_PlayMontageAndWaitForEvent.h"

#include "GAExtLogs.h"

#include "Character/CharacterMeshAccessorInterface.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"


UAbilityTask_PlayMontageAndWaitForEvent::UAbilityTask_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UAbilityTask_PlayMontageAndWaitForEvent* UAbilityTask_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(
	UGameplayAbility* OwningAbility
	, FName TaskInstanceName
	, UAnimMontage* InMontageToPlay
	, FGameplayTagContainer InEventTags
	, float InRate
	, FName InStartSection
	, bool bInStopWhenAbilityEnds
	, float InAnimRootMotionTranslationScale)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(InRate);

	auto* NewTask{ NewAbilityTask<UAbilityTask_PlayMontageAndWaitForEvent>(OwningAbility, TaskInstanceName) };
	NewTask->MontageToPlay = InMontageToPlay;
	NewTask->EventTags = InEventTags;
	NewTask->Rate = InRate;
	NewTask->StartSection = InStartSection;
	NewTask->AnimRootMotionTranslationScale = InAnimRootMotionTranslationScale;
	NewTask->bStopWhenAbilityEnds = bInStopWhenAbilityEnds;

	return NewTask;
}


void UAbilityTask_PlayMontageAndWaitForEvent::Activate()
{
	ListenAbilityCancel();

	if (PlayMontage())
	{
		ListenGameplayEvent();

		SetupRootMotionTranslationScale();
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	SetWaitingOnAvatar();
}

void UAbilityTask_PlayMontageAndWaitForEvent::ExternalCancel()
{
	check(AbilitySystemComponent.IsValid());

	HandleAbilityCancelled();

	Super::ExternalCancel();
}

void UAbilityTask_PlayMontageAndWaitForEvent::OnDestroy(bool AbilityEnded)
{
	UnlistenAbilityCancel();
	UnlistenGameplayEvent();

	if (AbilityEnded && bStopWhenAbilityEnds)
	{
		StopPlayingMontage();
	}
	
	Super::OnDestroy(AbilityEnded);
}


FString UAbilityTask_PlayMontageAndWaitForEvent::GetDebugString() const
{
	const auto* ActorInfo{ Ability ? Ability->GetCurrentActorInfo() : nullptr };
	auto* AnimInstance{ ActorInfo ? ActorInfo->GetAnimInstance() : nullptr };

	auto* PlayingMontage{ AnimInstance ? (AnimInstance->Montage_IsActive(MontageToPlay) ? MontageToPlay : AnimInstance->GetCurrentActiveMontage()) : nullptr};

	return FString::Printf(TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}


void UAbilityTask_PlayMontageAndWaitForEvent::ListenAbilityCancel()
{
	check(Ability);

	CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &ThisClass::HandleAbilityCancelled);
}

void UAbilityTask_PlayMontageAndWaitForEvent::UnlistenAbilityCancel()
{
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
	}
}

void UAbilityTask_PlayMontageAndWaitForEvent::HandleAbilityCancelled()
{
	StopPlayingMontage();

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
	}
}


void UAbilityTask_PlayMontageAndWaitForEvent::ListenGameplayEvent()
{
	check(AbilitySystemComponent.IsValid());

	EventHandle = AbilitySystemComponent->AddGameplayEventTagContainerDelegate(
		EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::HandleGameplayEvent));
}

void UAbilityTask_PlayMontageAndWaitForEvent::UnlistenGameplayEvent()
{
	if (AbilitySystemComponent.IsValid())
	{
		AbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
	}
}

void UAbilityTask_PlayMontageAndWaitForEvent::HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FGameplayEventData TempData = *Payload;
		TempData.EventTag = EventTag;

		EventReceived.Broadcast(EventTag, TempData);
	}
}


bool UAbilityTask_PlayMontageAndWaitForEvent::PlayMontage()
{
	check(AbilitySystemComponent.IsValid());
	check(Ability);

	const auto Duration{ AbilitySystemComponent->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection) };

	if (Duration > 0.0f)
	{
		auto* AnimInstance{ Ability->GetCurrentActorInfo() ? Ability->GetCurrentActorInfo()->GetAnimInstance() : nullptr };
		if (!AnimInstance)
		{
			auto* Actor{ Ability->GetCurrentActorInfo()->AvatarActor.Get() };
			if (Actor && Actor->Implements<UCharacterMeshAccessorInterface>())
			{
				if (auto* MainMesh{ ICharacterMeshAccessorInterface::Execute_GetMainMesh(Actor) })
				{
					AnimInstance = MainMesh->GetAnimInstance();
				}
			}
		}

		if (AnimInstance && ShouldBroadcastAbilityTaskDelegates())
		{
			BlendingOutDelegate.BindUObject(this, &ThisClass::HandleMontageBlendingOut);
			AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

			MontageEndedDelegate.BindUObject(this, &ThisClass::HandleMontageEnded);
			AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);
		}
		else
		{
			UE_LOG(LogGameExt_Ability, Error, TEXT("Cannot bind montage events(AnimIns: %s, Should: %s)"),
				*GetNameSafe(AnimInstance), ShouldBroadcastAbilityTaskDelegates() ? TEXT("T"): TEXT("F"));
		}

		return true;
	}

	return false;
}

void UAbilityTask_PlayMontageAndWaitForEvent::StopPlayingMontage()
{
	if (AbilitySystemComponent.IsValid())
	{
		if (AbilitySystemComponent->GetAnimatingAbility() == Ability && AbilitySystemComponent->GetCurrentMontage() == MontageToPlay)
		{
			const auto* ActorInfo{ Ability ? Ability->GetCurrentActorInfo() : nullptr };
			auto* AnimInstance{ ActorInfo ? ActorInfo->GetAnimInstance() : nullptr };

			if (auto* MontageInstance{ AnimInstance ? AnimInstance->GetActiveInstanceForMontage(MontageToPlay) : nullptr })
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			AbilitySystemComponent->CurrentMontageStop();
		}
	}
}


void UAbilityTask_PlayMontageAndWaitForEvent::SetupRootMotionTranslationScale()
{
	check(Ability);

	if (auto* Character{ Cast<ACharacter>(GetAvatarActor()) })
	{
		const auto bHasAuthority{ Character->HasAuthority() };
		const auto bIsAutonomousProxy{ Character->GetLocalRole() == ROLE_AutonomousProxy };
		const auto bAbilityLocalPredicted{ Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted };

		if (bHasAuthority || (bIsAutonomousProxy && bAbilityLocalPredicted))
		{
			Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
		}
	}
}

void UAbilityTask_PlayMontageAndWaitForEvent::ResetRootMotionTranslationScale()
{
	check(Ability);

	if (auto* Character{ Cast<ACharacter>(GetAvatarActor()) })
	{
		const auto bHasAuthority{ Character->HasAuthority() };
		const auto bIsAutonomousProxy{ Character->GetLocalRole() == ROLE_AutonomousProxy };
		const auto bAbilityLocalPredicted{ Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted };

		if (bHasAuthority || (bIsAutonomousProxy && bAbilityLocalPredicted))
		{
			Character->SetAnimRootMotionTranslationScale(1.0f);
		}
	}
}


void UAbilityTask_PlayMontageAndWaitForEvent::HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (AbilitySystemComponent.IsValid())
	{
		if (Ability && (Ability->GetCurrentMontage() == MontageToPlay))
		{
			if (Montage == MontageToPlay)
			{
				AbilitySystemComponent->ClearAnimatingAbility(Ability);

				ResetRootMotionTranslationScale();
			}
		}
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (bInterrupted)
		{
			OnInterrupted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
		else
		{
			OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UAbilityTask_PlayMontageAndWaitForEvent::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
	}

	EndTask();
}
