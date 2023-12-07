// Copyright (C) 2023 owoDra

#pragma once

#include "Abilities/GameplayAbility.h"

#include "GAEGameplayAbility.generated.h"

class UAbilityCost;
class APlayerController;
class AController;
class ACharacter;
class APawn;
class AActor;

/**
 * Types of method that activate or deactivated abilities
 */
UENUM(BlueprintType)
enum class EAbilityActivationMethod : uint8
{
	OnInputTriggered,	// Activated when input is received

	WhileInputActive,	// Activated when input is received and deactivated when input is lost

	OnSpawn,			// Activated when abilities are registered and created

	Custom				// Explicitly activated by special events
};


/**
 * GameplayAbility with enhanced availability activation and other features
 */
UCLASS(Abstract, HideCategories = "Input")
class GAEXT_API UGAEGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

	friend class UGAEAbilitySystemComponent;

public:
	UGAEGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//
	// Determine under what situation an ability will be activated or deactivated
	//
	UPROPERTY(EditDefaultsOnly, Category = "Ability Activation")
	EAbilityActivationMethod ActivationMethod{ EAbilityActivationMethod::OnInputTriggered };

	//
	// Additional costs that must be paid to activate this ability
	//
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Costs")
	TArray<TObjectPtr<UAbilityCost>> AdditionalCosts;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	/** 
	 * Called when this ability is granted to the ability system component. 
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnAbilityAdded();
	virtual void OnAbilityAdded_Implementation() {}

	/** 
	 * Called when this ability is removed from the ability system component. 
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnAbilityRemoved();
	virtual void OnAbilityRemoved_Implementation() {}

	/** 
	 * Called when the ability system is initialized with a avatar. 
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnAvatarSet();
	virtual void OnAvatarSet_Implementation() {}


protected:
	//
	// Map of failure tags to simple error messages
	//
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

	//
	// Map of failure tags to anim montages that should be played with them
	//
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

public:
	/**
	 * Runs when this ability is created and try to activate the ability
	 */
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

protected:
	/**
	 * Called when the ability fails to activate
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;
	virtual void OnAbilityFailedToActivate_Implementation(const FGameplayTagContainer& FailedReason) const {}


public:
	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (DeterminesOutputType = "Class"))
	UAbilitySystemComponent* GetAbilitySystemComponent(TSubclassOf<UAbilitySystemComponent> Class) const;

	template<typename T = UGAEAbilitySystemComponent>
	T* GetAbilitySystemComponent() const
	{
		return Cast<T>(GetAbilitySystemComponent(T::StaticClass()));
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

};
