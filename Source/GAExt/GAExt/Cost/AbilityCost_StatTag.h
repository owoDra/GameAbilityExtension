// Copyright (C) 2024 owoDra

#pragma once

#include "Cost/AbilityCost.h"

#include "Type/AbilityCostStatTagTargetTypes.h"

#include "AbilityCost_StatTag.generated.h"


/**
 * Entry data to define StatTag cost's
 */
USTRUCT(BlueprintType)
struct GAEXT_API FStatTagCostDefinition
{
	GENERATED_BODY()
public:
	FStatTagCostDefinition() {}

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (Categories = "Stat"))
	FGameplayTag StatTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EStatTagCostTarget Target{ EStatTagCostTarget::SourceObject };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FScalableFloat Cost{ 1.0f };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bShouldInitStatTag{ true };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bShouldInitStatTag", EditConditionHides))
	FScalableFloat MaxValue{ 1.0f };

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "bShouldInitStatTag", EditConditionHides))
	FScalableFloat DefaultValue{ 1.0f };

};


/**
 * AbilityCost class of the type that consumes the OwningActor or AvatarActor's StatTag
 */
UCLASS(DefaultToInstanced, EditInlineNew, meta = (DisplayName = "Cost Actor Stat Tag"))
class GAEXT_API UAbilityCost_StatTag : public UAbilityCost
{
	GENERATED_BODY()
public:
	UAbilityCost_StatTag(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	//
	// Whether the StatTag refers to the OwningActor or the AvatarActor
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs", meta = (TitleProperty = "{Target} {StatTag}", ShowOnlyInnerProperties))
	TArray<FStatTagCostDefinition> StatTagCosts;

public:
	virtual bool CheckCost(
		const UGAEGameplayAbility* Ability
		, const FGameplayAbilitySpecHandle Handle
		, const FGameplayAbilityActorInfo* ActorInfo
		, FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ApplyCost(
		const UGAEGameplayAbility* Ability
		, const FGameplayAbilitySpecHandle Handle
		, const FGameplayAbilityActorInfo* ActorInfo
		, const FGameplayAbilityActivationInfo ActivationInfo) override;

public:
	virtual void OnAvatarSet(
		const UGAEGameplayAbility* Ability
		, const FGameplayAbilityActorInfo* ActorInfo
		, const FGameplayAbilitySpec& Spec) override;


	//////////////////////////////////////////////////////////////
	// Utilities
protected:
	UObject* GetStatTagCostTarget(EStatTagCostTarget Type) const;

	template<typename T>
	T* GetCostTarget(EStatTagCostTarget Type) const
	{
		return Cast<T>(GetStatTagCostTarget(Type));
	}

};
