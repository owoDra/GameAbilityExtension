// Copyright (C) 2024 owoDra

#include "AbilityTask_ListenInputRelease.h"

#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_ListenInputRelease)


UAbilityTask_ListenInputRelease::UAbilityTask_ListenInputRelease(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}



UAbilityTask_ListenInputRelease* UAbilityTask_ListenInputRelease::ListenInputRelease(class UGameplayAbility* OwningAbility)
{
	return NewAbilityTask<UAbilityTask_ListenInputRelease>(OwningAbility);
}


void UAbilityTask_ListenInputRelease::Activate()
{
	auto* ASC{ AbilitySystemComponent.Get() };
	if (ASC && Ability)
	{
		DelegateHandle = ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputReleased, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UAbilityTask_ListenInputRelease::OnReleaseCallback);
		if (IsForRemoteClient())
		{
			if (!ASC->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::InputReleased, GetAbilitySpecHandle(), GetActivationPredictionKey()))
			{
				SetWaitingOnRemotePlayerData();
			}
		}
	}
}

void UAbilityTask_ListenInputRelease::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);

	auto* ASC{ AbilitySystemComponent.Get() };
	if (!Ability || !ASC)
	{
		return;
	}

	ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputReleased, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);
}

void UAbilityTask_ListenInputRelease::OnReleaseCallback()
{
	auto* ASC{ AbilitySystemComponent.Get() };
	if (!Ability || !ASC)
	{
		return;
	}

	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	if (IsPredictingClient())
	{
		// Tell the server about this

		ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
	}
	else
	{
		ASC->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, GetAbilitySpecHandle(), GetActivationPredictionKey());
	}

	// We are done. Kill us so we don't keep getting broadcast messages

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnRelease.Broadcast();
	}
}
