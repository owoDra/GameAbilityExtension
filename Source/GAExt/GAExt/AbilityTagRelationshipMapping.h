// Copyright (C) 2023 owoDra

#pragma once

#include "Engine/DataAsset.h"

#include "GameplayTagContainer.h"

#include "AbilityTagRelationshipMapping.generated.h"


/** 
 * Struct that defines the relationship between different ability tags 
 */
USTRUCT(BlueprintType)
struct FAbilityTagRelationship
{
	GENERATED_BODY()
public:
	FAbilityTagRelationship() {}

public:
	//
	// The tag that this container relationship is about. Single tag, but abilities can have multiple of these
	//
	UPROPERTY(EditAnywhere, meta = (Categories = "Ability.Type"))
	FGameplayTag AbilityTag;

	//
	// The other ability tags that will be blocked by any ability using this tag
	//
	UPROPERTY(EditAnywhere, meta = (Categories = "Ability.Type"))
	FGameplayTagContainer AbilityTagsToBlock;

	//
	// The other ability tags that will be canceled by any ability using this tag
	//
	UPROPERTY(EditAnywhere, meta = (Categories = "Ability.Type"))
	FGameplayTagContainer AbilityTagsToCancel;

	//
	// If an ability has the tag, this is implicitly added to the activation required tags of the ability
	//
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer ActivationRequiredTags;

	//
	// If an ability has the tag, this is implicitly added to the activation blocked tags of the ability
	//
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer ActivationBlockedTags;
};


/** 
 * Mapping of how ability tags block or cancel other abilities 
 */
UCLASS(BlueprintType, Const)
class UAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()
public:
	UAbilityTagRelationshipMapping(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//
	// The list of relationships between different gameplay tags
	//
	UPROPERTY(EditDefaultsOnly, Category = "Ability", meta = (TitleProperty = "AbilityTag"))
	TArray<FAbilityTagRelationship> AbilityTagRelationships;

public:
	/** 
	 * Given a set of ability tags, parse the tag relationship and fill out tags to block and cancel 
	 */
	void GetAbilityTagsToBlockAndCancel(
		const FGameplayTagContainer& AbilityTags,
		FGameplayTagContainer* OutTagsToBlock,
		FGameplayTagContainer* OutTagsToCancel) const;

	/** 
	 * Given a set of ability tags, add additional required and blocking tags 
	 */
	void GetRequiredAndBlockedActivationTags(
		const FGameplayTagContainer& AbilityTags,
		FGameplayTagContainer* OutActivationRequired,
		FGameplayTagContainer* OutActivationBlocked) const;

	/** 
	 * Returns true if the specified ability tags are canceled by the passed in action tag 
	 */
	bool IsAbilityCancelledByTag(
		const FGameplayTagContainer& AbilityTags,
		const FGameplayTag& ActionTag) const;

};
