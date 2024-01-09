// Copyright (C) 2024 owoDra

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "GlobalAbilitySubsystem.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;
struct FActiveGameplayEffectHandle;
struct FGameplayAbilitySpecHandle;


/**
 * List of currently applied global abilities
 */
USTRUCT()
struct FGlobalAppliedAbilityList
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TMap<UAbilitySystemComponent*, FGameplayAbilitySpecHandle> Handles;

public:
	void AddToASC(TSubclassOf<UGameplayAbility> Ability, UAbilitySystemComponent* ASC);
	void RemoveFromASC(UAbilitySystemComponent* ASC);
	void RemoveFromAll();
};


/**
 * List of currently applied global effects
 */
USTRUCT()
struct FGlobalAppliedEffectList
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TMap<UAbilitySystemComponent*, FActiveGameplayEffectHandle> Handles;

public:
	void AddToASC(TSubclassOf<UGameplayEffect> Effect, UAbilitySystemComponent* ASC);
	void RemoveFromASC(UAbilitySystemComponent* ASC);
	void RemoveFromAll();
};


/**
 * A subsystem that applies and manages abilities and effects common to all registered AbilitySystemComponents in the world.
 */
UCLASS()
class UGlobalAbilitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UGlobalAbilitySubsystem() {}

protected:
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList> AppliedAbilities;

	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList> AppliedEffects;

	UPROPERTY()
	TArray<TObjectPtr<UAbilitySystemComponent>> RegisteredASCs;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GlobalAbility")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="GlobalAbility")
	void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GlobalAbility")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GlobalAbility")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	/** 
	 * Register an ASC with global system and apply any active global effects/abilities. 
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GlobalAbility")
	void RegisterASC(UAbilitySystemComponent* ASC);

	/** 
	 * Removes an ASC from the global system, along with any active global effects/abilities. 
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GlobalAbility")
	void UnregisterASC(UAbilitySystemComponent* ASC);

};
