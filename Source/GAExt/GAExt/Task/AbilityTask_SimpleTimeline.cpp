// Copyright (C) 2024 owoDra

#include "AbilityTask_SimpleTimeline.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_SimpleTimeline)


UAbilityTask_SimpleTimeline::UAbilityTask_SimpleTimeline(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}


UAbilityTask_SimpleTimeline* UAbilityTask_SimpleTimeline::TaskTimeline_Curve(UGameplayAbility* OwningAbility, const UCurveFloat* Curve)
{
	auto* NewTask{ NewAbilityTask<UAbilityTask_SimpleTimeline>(OwningAbility) };
	NewTask->Curve = Curve->FloatCurve;

	return NewTask;
}

UAbilityTask_SimpleTimeline* UAbilityTask_SimpleTimeline::TaskTimeline_RuntimeCurve(UGameplayAbility* OwningAbility, const FRuntimeFloatCurve& RuntimeCurve)
{
	auto* NewTask{ NewAbilityTask<UAbilityTask_SimpleTimeline>(OwningAbility) };
	NewTask->Curve = *RuntimeCurve.GetRichCurveConst();

	return NewTask;
}


void UAbilityTask_SimpleTimeline::Activate()
{
	TimeElapsed = 0.0f;

	// Invalidate if curve is empty

	if (Curve.IsEmpty())
	{
		InvalidateTimeline();
		return;
	}

	float MinValue{ 0.0f };
	Curve.GetValueRange(MinValue, Duration);
	
	// Invalidate if duration is less equals 0

	if (Duration <= 0.0)
	{
		InvalidateTimeline();
		return;
	}

	UpdateTimeline();
}

void UAbilityTask_SimpleTimeline::TickTask(float DeltaTime)
{
	TimeElapsed += DeltaTime;

	UpdateTimeline();
}


void UAbilityTask_SimpleTimeline::InvalidateTimeline()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnUpdate.Broadcast(0.0f);
		OnFinished.Broadcast(0.0f);
	}

	EndTask();
}

void UAbilityTask_SimpleTimeline::UpdateTimeline()
{
	const auto CurrentValue{ Curve.Eval(TimeElapsed) };

	OnUpdate.Broadcast(CurrentValue);

	if (TimeElapsed >= Duration)
	{
		OnFinished.Broadcast(CurrentValue);

		EndTask();
	}
}
