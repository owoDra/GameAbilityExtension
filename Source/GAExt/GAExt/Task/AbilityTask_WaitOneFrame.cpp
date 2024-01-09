// Copyright (C) 2024 owoDra

#include "AbilityTask_WaitOneFrame.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_WaitOneFrame)


UAbilityTask_WaitOneFrame::UAbilityTask_WaitOneFrame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UAbilityTask_WaitOneFrame* UAbilityTask_WaitOneFrame::CreateWaitOneFrame(UGameplayAbility* OwningAbility)
{
	auto* MyObj = NewAbilityTask<UAbilityTask_WaitOneFrame>(OwningAbility);
	return MyObj;
}

void UAbilityTask_WaitOneFrame::Activate()
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::OnDelayFinish);
}

void UAbilityTask_WaitOneFrame::OnDelayFinish()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinish.Broadcast();
	}

	EndTask();
}
