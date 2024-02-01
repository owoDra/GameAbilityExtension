// Copyright (C) 2024 owoDra

#include "AbilityTask_ListenInputPress.h"

#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_ListenInputPress)


UAbilityTask_ListenInputPress::UAbilityTask_ListenInputPress(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UAbilityTask_ListenInputPress* UAbilityTask_ListenInputPress::ListenInputPress(class UGameplayAbility* OwningAbility)
{
	return NewAbilityTask<UAbilityTask_ListenInputPress>(OwningAbility);
}


void UAbilityTask_ListenInputPress::Activate()
{
	auto* ASC{ AbilitySystemComponent.Get() };
	if (ASC && Ability)
	{
		DelegateHandle = ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UAbilityTask_ListenInputPress::OnPressCallback);
		if (IsForRemoteClient())
		{
			if (!ASC->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()))
			{
				SetWaitingOnRemotePlayerData();
			}
		}
	}
}

void UAbilityTask_ListenInputPress::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);

	auto* ASC{ AbilitySystemComponent.Get() };
	if (!Ability || !ASC)
	{
		return;
	}

	ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey()).Remove(DelegateHandle);
}

void UAbilityTask_ListenInputPress::OnPressCallback()
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

		ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
	}
	else
	{
		ASC->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, GetAbilitySpecHandle(), GetActivationPredictionKey());
	}

	// We are done. Kill us so we don't keep getting broadcast messages

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnPress.Broadcast();
	}
}
