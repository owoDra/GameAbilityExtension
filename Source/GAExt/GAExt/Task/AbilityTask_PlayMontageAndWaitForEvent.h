// Copyright (C) 2024 owoDra

#pragma once

#include "Abilities/Tasks/AbilityTask.h"

#include "GameplayTagContainer.h"

#include "AbilityTask_PlayMontageAndWaitForEvent.generated.h"


/** 
 * Delegate to notify GameplayEvent during Montage playback
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayMontageAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);


/**
 * AbilityTask to play AnimMontage on AvatarActor's mesh and track GameplayEvents received during playback and the end of playback.
 */
UCLASS()
class GAEXT_API UAbilityTask_PlayMontageAndWaitForEvent : public UAbilityTask
{
	GENERATED_BODY()
public:
	UAbilityTask_PlayMontageAndWaitForEvent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//////////////////////////////////////////////////////
	// Paramters
protected:
	//
	// Montage that is playing
	//
	UPROPERTY(Transient)
	UAnimMontage* MontageToPlay;

	// 
	// List of tags to match against gameplay events
	//
	UPROPERTY(Transient)
	FGameplayTagContainer EventTags;

	//
	// Playback rate
	//
	UPROPERTY(Transient)
	float Rate;

	//
	// Section to start montage from
	//
	UPROPERTY(Transient)
	FName StartSection;

	//
	// Modifies how root motion movement to apply
	//
	UPROPERTY(Transient)
	float AnimRootMotionTranslationScale;

	//
	// Rather montage should be aborted if ability ends
	//
	UPROPERTY(Transient)
	bool bStopWhenAbilityEnds;


	//////////////////////////////////////////////////////
	// Delegates
private:
	UPROPERTY(BlueprintAssignable, meta = (AllowPrivateAccess = true))
	FPlayMontageAndWaitForEventDelegate OnCompleted;

	UPROPERTY(BlueprintAssignable, meta = (AllowPrivateAccess = true))
	FPlayMontageAndWaitForEventDelegate OnBlendOut;

	UPROPERTY(BlueprintAssignable, meta = (AllowPrivateAccess = true))
	FPlayMontageAndWaitForEventDelegate OnInterrupted;

	UPROPERTY(BlueprintAssignable, meta = (AllowPrivateAccess = true))
	FPlayMontageAndWaitForEventDelegate OnCancelled;

	UPROPERTY(BlueprintAssignable, meta = (AllowPrivateAccess = true))
	FPlayMontageAndWaitForEventDelegate EventReceived;


	//////////////////////////////////////////////////////
	// Task Creation
public:
	/**
	 * AbilityTask to play AnimMontage on AvatarActor's mesh and track GameplayEvents received during playback and the end of playback.
	 *
	 * @param TaskInstanceName					Set to override the name of this task, for later querying
	 * @param MontageToPlay						The montage to play on the character
	 * @param EventTags							Any gameplay events matching this tag will activate the EventReceived callback. If empty, all events will trigger callback
	 * @param Rate								Change to play the montage faster or slower
	 * @param bStopWhenAbilityEnds				If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
	 * @param AnimRootMotionTranslationScale	Change to modify size of root motion or set to 0 to block it entirely
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilityTasks|PlayMontage", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "True"))
	static UAbilityTask_PlayMontageAndWaitForEvent* PlayMontageAndWaitForEvent(
		UGameplayAbility* OwningAbility
		, FName TaskInstanceName
		, UAnimMontage* InMontageToPlay
		, UPARAM(meta = (Categories = "Event")) FGameplayTagContainer InEventTags
		, float InRate = 1.0f
		, FName InStartSection = NAME_None
		, bool bInStopWhenAbilityEnds = true
		, float InAnimRootMotionTranslationScale = 1.0f);

	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual void OnDestroy(bool AbilityEnded) override;

	virtual FString GetDebugString() const override;


	//////////////////////////////////////////////////////
	// Cancellation
protected:
	FDelegateHandle CancelledHandle;

protected:
	void ListenAbilityCancel();
	void UnlistenAbilityCancel();

	UFUNCTION()
	void HandleAbilityCancelled();
	

	//////////////////////////////////////////////////////
	// Listen GameplayEvent
protected:
	FDelegateHandle EventHandle;

protected:
	void ListenGameplayEvent();
	void UnlistenGameplayEvent();

	void HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);


	//////////////////////////////////////////////////////
	// Play Montage
protected:
	FOnMontageBlendingOutStarted BlendingOutDelegate;

	FOnMontageEnded MontageEndedDelegate;

protected:
	bool PlayMontage();
	void StopPlayingMontage();

	void SetupRootMotionTranslationScale();
	void ResetRootMotionTranslationScale();

	UFUNCTION()
	void HandleMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);

};
