// Copyright (C) 2024 owoDra

#pragma once

#include "Abilities/GameplayAbility.h"

#include "GAEGameplayAbility.generated.h"

class UGAEAbilitySystemComponent;
class UAbilityCost;
class APlayerController;
class AController;
class ACharacter;
class APawn;
class AActor;
class APlayerState;

/**
 * Types of method that activate or deactivated abilities
 */
UENUM(BlueprintType)
enum class EAbilityActivationMethod : uint8
{
	// Activated when input is received
	OnInputTriggered,	

	// Activated when abilities are registered and created
	OnSpawn,			

	// Explicitly activated by special events
	Custom				
};


/**
 * Policy that can activate abilities
 */
UENUM(BlueprintType)
enum class EAbilityActivationPolicy : uint8
{
	// Can be activated only when both cooldown and costs are ready
	Default,			

	// Can be activated ignoring cooldown if both costs are ready
	// 
	// Tips:
	//	Can be used for abilities where stock is charged by cooldown
	//
	CostOverCooldown,	

	// Can be activated ignoring cost if cooldown is both ready.
	//
	// Tips:
	//	Can be used for abilities that can be used at the cost of something else when the cost is not enough
	//
	CooldownOverCost	
};


/**
 * GameplayAbility with enhanced availability activation and other features
 */
UCLASS(Abstract, HideCategories = "Input")
class GAEXT_API UGAEGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

	friend class UGAEAbilitySystemComponent;
	friend class UAbilityCost;

public:
	UGAEGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** 
	 * Called when this ability is granted to the ability system component. 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (DisplayName = "OnGiveAbility"))
	void BP_OnGiveAbility();

	/** 
	 * Called when this ability is removed from the ability system component. 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (DisplayName = "OnRemoveAbility"))
	void BP_OnRemoveAbility();

	/** 
	 * Called when the ability system is initialized with a avatar. 
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability", meta = (DisplayName = "OnAvatarSet"))
	void BP_OnAvatarSet();


	///////////////////////////////////////////////////////////////////////////////////
	// Ability Activation
#pragma region Ability Activation
protected:
	//
	// Determine under what situation an ability will be activated or deactivated
	//
	UPROPERTY(EditDefaultsOnly, Category = "Ability Activation")
	EAbilityActivationMethod ActivationMethod{ EAbilityActivationMethod::OnInputTriggered };

	//
	// Determine policy that can activate abilities
	//
	UPROPERTY(EditDefaultsOnly, Category = "Ability Activation")
	EAbilityActivationPolicy ActivationPolicy{ EAbilityActivationPolicy::Default };

	//
	// Tag to be used when sending activation messages
	// 
	// Tips:
	//	If not set, no message will be sent
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ability Activation|Message", meta = (Categories = "Message.Ability.Activation"))
	FGameplayTag ActivationMessageTag;

	//
	// Whether to send Activation messages locally only
	// 
	// Tips:
	//	This is useful when you want to notify only locally, for example, when informing the UI, etc. of an execution.
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ability Activation|Message")
	bool bActivationMessageLocallyOnly{ true };

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;

	/**
	 * Runs when this ability is created and try to activate the ability
	 */
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

	/**
	 * Broadcast the ability activation to the GameplayMessageSubsystem
	 */
	void BroadcastActivationMassage() const;

#pragma endregion


	///////////////////////////////////////////////////////////////////////////////////
	// Cooldowns
#pragma region Cooldowns
protected:
	//
	// Whether to use cooldown or not
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Cooldowns")
	bool bUseCooldown{ false };

	//
	// Override the effect time of CooldownGameplayEffect.
	// 
	// Note:
	//	If the class is not the default GameplayEffect class, it may not be available in some settings.
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Cooldowns", meta = (ClampMin = 0.00, EditCondition = "bUseCooldown"))
	float CooltimeOverride{ 0.0f };

	//
	// Tag to be used when sending cooldown messages
	// 
	// Tips:
	//	If not set, no message will be sent
	//
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Cooldowns", meta = (Categories = "Message.Ability.Cooldown", EditCondition = "bUseCooldown"))
	FGameplayTag CooldownMessageTag;

protected:
	UPROPERTY(Transient)
	FActiveGameplayEffectHandle CooldownGEHandle;

	UPROPERTY(Transient)
	bool bCoolingdown{ false };

public:
	virtual bool CommitAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const bool ForceCooldown, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	/**
	 * Returns whether Cooldown is available in the current configuration
	 */
	virtual bool IsCooldownAvailable() const;

	/**
	 * Broadcast the start and end of a Cooldown through the GameplayMessageSubsystem
	 * 
	 * Note:
	 *	This broadcast is basically only performed by the local proxy
	 */
	void BroadcastCooldownMassage(float Duration) const;

protected:
	void ListenToCooldown(const FGameplayAbilityActorInfo* ActorInfo);
	void UnlistenToCooldown(const FGameplayAbilityActorInfo* ActorInfo);

	/**
	 * Returns whether GameplayEffectSpec is related for this ability
	 */
	virtual bool IsCDGameplayEffectForThis(const FGameplayEffectSpec& Spec) const;

private:
	void HandleAnyGameplayEffectAdded(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);
	void HandleCDGameplayEffectRemoved(const FGameplayEffectRemovalInfo& Info);

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Cooldowns")
	void OnCooldownStart(float Duration);
	virtual void OnCooldownStart_Implementation(float Duration);

	UFUNCTION(BlueprintNativeEvent, Category = "Cooldowns")
	void OnCooldownEnd();
	virtual void OnCooldownEnd_Implementation();

#pragma endregion


	///////////////////////////////////////////////////////////////////////////////////
	// Costs
#pragma region Costs
protected:
	//
	// Additional costs that must be paid to activate this ability
	//
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Costs")
	TArray<TObjectPtr<UAbilityCost>> AdditionalCosts;

protected:
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

#pragma endregion


	///////////////////////////////////////////////////////////////////////////////////
	// Activation Failure
#pragma region Activation Failure
protected:
	//
	// Map of failure tags to simple error messages
	//
	UPROPERTY(EditDefaultsOnly, Category = "Activation Failure")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

	//
	// Map of failure tags to anim montages that should be played with them
	//
	UPROPERTY(EditDefaultsOnly, Category = "Activation Failure")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

protected:
	/**
	 * Called when the ability fails to activate
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;
	virtual void OnAbilityFailedToActivate_Implementation(const FGameplayTagContainer& FailedReason) const;

#pragma endregion


	///////////////////////////////////////////////////////////////////////////////////
	// Utilities
#pragma region Utilities
public:
	template<typename T>
	T* GetTypedSourceObject() const
	{
		return Cast<T>(GetCurrentSourceObject());
	}

	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	UAbilitySystemComponent* GetAbilitySystemComponent(TSubclassOf<UAbilitySystemComponent> Class) const;

	template<typename T = UGAEAbilitySystemComponent>
	T* GetAbilitySystemComponent() const
	{
		return Cast<T>(GetAbilitySystemComponent(T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	UMovementComponent* GetMovementComponent(TSubclassOf<UMovementComponent> Class) const;

	template<typename T = UMovementComponent>
	T* GetMovementComponent() const
	{
		return Cast<T>(GetMovementComponent(T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	USkeletalMeshComponent* GetSkeletalMeshComponent(TSubclassOf<USkeletalMeshComponent> Class) const;

	template<typename T = USkeletalMeshComponent>
	T* GetSkeletalMeshComponent() const
	{
		return Cast<T>(GetSkeletalMeshComponent(T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	UAnimInstance* GetAnimInstance(TSubclassOf<UAnimInstance> Class) const;

	template<typename T = UAnimInstance>
	T* GetAnimInstance() const
	{
		return Cast<T>(GetAnimInstance(T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	APlayerController* GetPlayerController(TSubclassOf<APlayerController> Class) const;

	template<typename T = APlayerController>
	T* GetPlayerController() const
	{
		return Cast<T>(GetPlayerController(T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	AController* GetController(TSubclassOf<AController> Class) const;

	template<typename T = AController>
	T* GetController() const
	{
		return Cast<T>(GetController(T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	ACharacter* GetCharacter(TSubclassOf<ACharacter> Class) const;

	template<typename T = ACharacter>
	T* GetCharacter() const
	{
		return Cast<T>(GetCharacter(T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	APawn* GetPawn(TSubclassOf<APawn> Class) const;

	template<typename T = APawn>
	T* GetPawn() const
	{
		return Cast<T>(GetPawn(T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	AActor* GetActor(TSubclassOf<AActor> Class) const;

	template<typename T = AActor>
	T* GetActor() const
	{
		return Cast<T>(GetActor(T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	APlayerState* GetPlayerState(TSubclassOf<APlayerState> Class) const;

	template<typename T = APlayerState>
	T* GetPlayerState() const
	{
		return Cast<T>(GetPlayerState(T::StaticClass()));
	}

#pragma endregion

};
