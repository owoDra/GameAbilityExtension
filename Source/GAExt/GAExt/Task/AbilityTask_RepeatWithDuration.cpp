// Copyright (C) 2024 owoDra

#include "AbilityTask_RepeatWithDuration.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_RepeatWithDuration)


UAbilityTask_RepeatWithDuration::UAbilityTask_RepeatWithDuration(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UAbilityTask_RepeatWithDuration* UAbilityTask_RepeatWithDuration::CreateRepeatWithDuration(UGameplayAbility* OwningAbility, float Duration)
{
	auto* MyObj = NewAbilityTask<UAbilityTask_RepeatWithDuration>(OwningAbility);

	MyObj->DesiredDuration = Duration;

	return MyObj;
}

void UAbilityTask_RepeatWithDuration::Activate()
{
	StartTime = GetWorld()->GetTimeSeconds();
	LastActionTime = StartTime;

	if (DesiredDuration > 0)
	{
		PerformAction();
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnFinished.Broadcast(0);
		}

		EndTask();
	}
}

void UAbilityTask_RepeatWithDuration::PerformAction()
{
	CurrentTime = GetWorld()->GetTimeSeconds();
	LastTimeDelta = CurrentTime - LastActionTime;
	LastActionTime = CurrentTime;

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnPerformAction.Broadcast(LastTimeDelta);
	}
	else
	{
		EndTask();
		return;
	}

	if (CurrentTime - StartTime >= DesiredDuration)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnFinished.Broadcast(LastTimeDelta);
		}
		EndTask();
	}
	else
	{
		TimerRepeatAction = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::PerformAction);
	}
}

FString UAbilityTask_RepeatWithDuration::GetDebugString() const
{
	return FString::Printf(TEXT("RepeatAction. DeltaSeconds: %.2f"), LastTimeDelta);
}

void UAbilityTask_RepeatWithDuration::OnDestroy(bool AbilityIsEnding)
{
	if (auto* World{ GetWorld() })
	{
		World->GetTimerManager().ClearTimer(TimerRepeatAction);
	}

	Super::OnDestroy(AbilityIsEnding);
}
