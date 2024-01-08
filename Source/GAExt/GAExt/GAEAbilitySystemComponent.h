// Copyright (C) 2023 owoDra

#pragma once

#include "AbilitySystemComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"

#include "GAEGameplayAbility.h"

#include "GAEAbilitySystemComponent.generated.h"

class UAbilityTagRelationshipMapping;


/**
 * AbilitySystemComponent with additional functionality to extend the ability management 
 * and to enable implementation of processing by player input.
 * 
 * Note:
 *	When using this AbilitySystemCompoenent, it is recommended to create abilities that inherit from GAEGameplayAbility instead of the GameplayAbility.
 *  Derivation from GameplayAbility will not allow you to take full advantage of its features.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAEXT_API UGAEAbilitySystemComponent : public UAbilitySystemComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
public:
	UGAEAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//
	// Function name used to add this component
	//
	static const FName NAME_ActorFeatureName;

	static const FName NAME_AbilityReady;

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

public:
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;


protected:
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;

	void TryActivateAbilitiesOnSpawn();

	/** Notify client that an ability failed to activate */
	UFUNCTION(Client, Unreliable)
	void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);
	void ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	typedef TFunctionRef<bool(const UGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);


protected:
	//
	// Mapping data for relationships by ability tag 
	// that will be applied to all abilities added to this AbilitySystemComponent
	// 
	// Tips:
	//	If nullptr, only the tags set in the abilities are referenced.
	//
	UPROPERTY(Replicated, Transient)
	TObjectPtr<const UAbilityTagRelationshipMapping> TagRelationshipMapping;

protected:
	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;

public:
	/** 
	 * Sets the current tag relationship mapping, if null it will clear it out 
	 */
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Tag Relationship")
	void SetTagRelationshipMapping(const UAbilityTagRelationshipMapping* NewMapping);

	/** 
	 * Looks at ability tags and gathers additional required and blocking tags 
	 */
	void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;


protected:
	//
	// Handles to abilities that had their input pressed this frame.
	//
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	//
	// Handles to abilities that had their input released this frame.
	//
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	//
	// Handles to abilities that have their input held.
	//
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

protected:
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

public:
	/**
	 * Mark the press and hold start of abilities according to the tag.
	 */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/**
	 * Mark the release and hold end of abilities according to the tag.
	 */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/**
	 * Clear marked ability inputs.
	 */
	void ClearAbilityInput();

	/**
	 * Execute the input process for the marked abilities.
	 */
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	/**
	 * Cancel abilities activated by input
	 */
	void CancelInputActivatedAbilities(bool bReplicateCancelAbility);


public:
	template <class T>
	T* GetPawn() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' template parameter to GetPawn must be derived from APawn");
		return Cast<T>(GetOwner());
	}

	template <class T>
	T* GetPlayerState() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APlayerState>::Value, "'T' template parameter to GetPlayerState must be derived from APlayerState");

		const auto* Pawn{ GetPawn<APawn>() };

		return Pawn ? Pawn->GetPlayerState<APlayerState>() : Cast<T>(GetOwner());
	}

	template <class T>
	T* GetController() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AController>::Value, "'T' template parameter to GetController must be derived from AController");

		const auto* Pawn{ GetPawn<APawn>() };

		return Pawn ? Pawn->GetController<AController>() : Cast<T>(GetOwner());
	}

};
