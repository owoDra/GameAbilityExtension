// Copyright (C) 2024 owoDra

#pragma once

#include "Abilities/Tasks/AbilityTask.h"

#include "AbilityTask_ListenInputPress.generated.h"


/**
 *	Delegate for notify input pressed
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FListenInputPressDelegate);


/**
 * Listen the input is pressed from activating an ability. 
 */
UCLASS()
class GAEXT_API UAbilityTask_ListenInputPress : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_ListenInputPress(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UPROPERTY(BlueprintAssignable)
	FListenInputPressDelegate OnPress;

	/** 
	 * Listen the input is pressed from activating an ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_ListenInputPress* ListenInputPress(UGameplayAbility* OwningAbility);


protected:
	FDelegateHandle DelegateHandle;

public:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

protected:
	UFUNCTION()
	void OnPressCallback();

};
