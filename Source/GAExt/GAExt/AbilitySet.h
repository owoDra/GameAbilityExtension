// Copyright (C) 2024 owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "GameplayTagContainer.h"

#include "AbilitySet.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAttributeSet;
class UAbilitySystemComponent;
struct FGameplayAbilitySpecHandle;
struct FActiveGameplayEffectHandle;


/**
 * Data used by the ability set to grant gameplay ability
 */
USTRUCT(BlueprintType)
struct GAEXT_API FAbilitySet_GameplayAbility
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> Ability{ nullptr };

	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel{ 1 };

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "Input"))
	FGameplayTag InputTag;

};


/**
 * Data used by the ability set to grant gameplay effect
 */
USTRUCT(BlueprintType)
struct GAEXT_API FAbilitySet_GameplayEffect
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect{ nullptr };

	UPROPERTY(EditDefaultsOnly)
	float EffectLevel{ 1.0f };

};


/**
 * Data used by the ability set to grant attribute sets.
 */
USTRUCT(BlueprintType)
struct GAEXT_API FAbilitySet_AttributeSet
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet{ nullptr };

};


/**
 * Data used to store handles to what has been granted by the ability set.
 */
USTRUCT(BlueprintType)
struct GAEXT_API FAbilitySet_GrantedHandles
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;

public:
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	void AddAbilities(UAbilitySystemComponent* ASC, const TArray<FAbilitySet_GameplayAbility>& Abilities, UObject* SourceObject = nullptr);
	void AddGameplayEffects(UAbilitySystemComponent* ASC, const TArray<FAbilitySet_GameplayEffect>& Effects, UObject* SourceObject = nullptr);
	void AddAttributeSets(UAbilitySystemComponent* ASC, const TArray<FAbilitySet_AttributeSet>& Sets, UObject* SourceObject = nullptr);

	void TakeFromAbilitySystem(UAbilitySystemComponent* ASC);

};


/**
 * Non-mutable data asset used to grant gameplay abilities and gameplay effects.
 */
UCLASS(BlueprintType, Const)
class GAEXT_API UAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//
	// Gameplay abilities to grant when this ability set is granted.
	//
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySet", meta = (TitleProperty = Ability))
	TArray<FAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	//
	// Gameplay effects to grant when this ability set is granted.
	//
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySet", meta = (TitleProperty = GameplayEffect))
	TArray<FAbilitySet_GameplayEffect> GrantedGameplayEffects;

	//
	// Attribute sets to grant when this ability set is granted.
	//
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySet", meta = (TitleProperty = AttributeSet))
	TArray<FAbilitySet_AttributeSet> GrantedAttributes;

public:
	/**
	 * Grants the ability set to the specified ability system component.
	 * The returned handles can be used later to take away anything that was granted.
	 */
	void GiveToAbilitySystem(UAbilitySystemComponent* ASC, FAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

};
