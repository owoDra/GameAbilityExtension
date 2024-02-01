// Copyright (C) 2024 owoDra

#pragma once

#include "Abilities/Tasks/AbilityTask.h"

#include "AbilityTask_ListenInputRelease.generated.h"


/**
 *	Delegate for notify input released
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FListenInputReleaseDelegate);


/**
 * Listen the input is released from activating an ability. 
 */
UCLASS()
class GAEXT_API UAbilityTask_ListenInputRelease : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_ListenInputRelease(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UPROPERTY(BlueprintAssignable)
	FListenInputReleaseDelegate	OnRelease;

	/** 
	 * Listen the input is released from activating an ability. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_ListenInputRelease* ListenInputRelease(UGameplayAbility* OwningAbility);


protected:
	FDelegateHandle DelegateHandle;

public:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

protected:
	UFUNCTION()
	void OnReleaseCallback();

};
