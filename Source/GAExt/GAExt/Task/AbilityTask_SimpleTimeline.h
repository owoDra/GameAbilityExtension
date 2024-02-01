// Copyright (C) 2024 owoDra

#pragma once

#include "Abilities/Tasks/AbilityTask.h"

#include "AbilityTask_SimpleTimeline.generated.h"

class UCurveFloat;
struct FRuntimeFloatCurve;


/**
 * Delegate for notify tiline updates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSimpleTimelineDelegate, float, Value);


/**
 * Ability task that executes the simple timeline process
 */
UCLASS()
class GAEXT_API UAbilityTask_SimpleTimeline : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_SimpleTimeline(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
public:
	UPROPERTY(BlueprintAssignable)
	FSimpleTimelineDelegate	OnUpdate;

	UPROPERTY(BlueprintAssignable)
	FSimpleTimelineDelegate	OnFinished;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", DisplayName = "Task Timeline (Curve)"))
	static UAbilityTask_SimpleTimeline* TaskTimeline_Curve(UGameplayAbility* OwningAbility, const UCurveFloat* Curve);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", DisplayName = "Task Timeline (Runtime Curve)"))
	static UAbilityTask_SimpleTimeline* TaskTimeline_RuntimeCurve(UGameplayAbility* OwningAbility, const FRuntimeFloatCurve& RuntimeCurve);

protected:
	FRichCurve Curve;

	float Duration{ 0.0f };
	float TimeElapsed{ 0.0f };

public:
	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

protected:
	void InvalidateTimeline();
	void UpdateTimeline();

};
