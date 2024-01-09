// Copyright (C) 2024 owoDra

#pragma once

#include "GameFeature/GameFeatureAction_WorldActionBase.h"

#include "AbilitySet.h"

#include "GameFeatureAction_AddAbilities.generated.h"


/**
 * Entry data of AbilitySet to be added by GameFeatureAction_AddAbilities
 */
USTRUCT()
struct FGameFeatureAbilitiesEntry
{
	GENERATED_BODY()
public:
	FGameFeatureAbilitiesEntry() {}

public:
	//
	// The base actor class to add to
	//
	UPROPERTY(EditAnywhere, Category="Abilities")
	TSoftClassPtr<AActor> ActorClass;

	//
	// List of ability sets to grant to actors of the specified class
	//
	UPROPERTY(EditAnywhere, Category="Attributes", meta=(AssetBundles="Client,Server"))
	TArray<TSoftObjectPtr<const UAbilitySet>> GrantedAbilitySets;
};


/**
 * GameFeatureAction responsible for granting abilities (and attributes) to actors of a specified type.
 */
UCLASS(meta = (DisplayName = "Add Abilities"))
class UGameFeatureAction_AddAbilities final : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()
public:
	UGameFeatureAction_AddAbilities() {}

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif // WITH_EDITOR


private:
	struct FActorExtensions
	{
		TArray<FAbilitySet_GrantedHandles> AbilitySetHandles;
	};

	struct FPerContextData
	{
		TMap<AActor*, FActorExtensions> ActiveExtensions;
		TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
	};

	TMap<FGameFeatureStateChangeContext, FPerContextData> ContextData;

protected:
	UPROPERTY(EditAnywhere, Category = "Abilities", meta = (TitleProperty = "ActorClass", ShowOnlyInnerProperties))
	TArray<FGameFeatureAbilitiesEntry> AbilitiesToAdd;

public:
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

	virtual void AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext) override;

private:
	void Reset(FPerContextData& ActiveData);
	void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext);
	void AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData);
	void RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData);

};
