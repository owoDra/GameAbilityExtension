// Copyright (C) 2024 owoDra

#pragma once

#include "Abilities/Tasks/AbilityTask.h"

#include "AbilityTask_RepeatWithDuration.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRepeatedActionDelegate, float, Delta);


/**
 * Ability task that executes the same process every tick for a specified time
 */
UCLASS()
class GAEXT_API UAbilityTask_RepeatWithDuration : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_RepeatWithDuration(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UPROPERTY(BlueprintAssignable)
	FRepeatedActionDelegate	OnPerformAction;

	UPROPERTY(BlueprintAssignable)
	FRepeatedActionDelegate	OnFinished;

protected:
	float StartTime;
	float LastActionTime;
	float LastTimeDelta;
	float CurrentTime;

	float DesiredDuration;

	FTimerHandle TimerRepeatAction;

public:
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", DisplayName = "Repeat With Duration"))
	static UAbilityTask_RepeatWithDuration* CreateRepeatWithDuration(UGameplayAbility* OwningAbility, float Duration);

	virtual void Activate() override;

	void PerformAction();

	virtual FString GetDebugString() const override;

protected:
	virtual void OnDestroy(bool AbilityIsEnding) override;
	
};
