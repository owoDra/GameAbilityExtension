// Copyright (C) 2024 owoDra

#pragma once

#include "Abilities/Tasks/AbilityTask.h"

#include "AbilityTask_WaitOneFrame.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitOneFrameDelegate);


/**
 * Ability task to wait only one frame
 */
UCLASS()
class GAEXT_API UAbilityTask_WaitOneFrame : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_WaitOneFrame(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UPROPERTY(BlueprintAssignable)
	FWaitOneFrameDelegate OnFinish;

public:
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly, DisplayName = "Wait One Frame"))
	static UAbilityTask_WaitOneFrame* CreateWaitOneFrame(UGameplayAbility* OwningAbility);

	virtual void Activate() override;

private:
	void OnDelayFinish();

};
