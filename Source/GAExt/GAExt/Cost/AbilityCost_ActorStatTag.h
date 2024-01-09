// Copyright (C) 2024 owoDra

#pragma once

#include "Cost/AbilityCost.h"

#include "AbilityCost_ActorStatTag.generated.h"


/**
 * Used to select OwningActor or AvatarActor
 */
UENUM(BlueprintType)
enum class EConsumeStatTarget : uint8
{
	OwningActor,
	AvatarActor,
};


/**
 * AbilityCost class of the type that consumes the OwningActor or AvatarActor's StatTag
 */
UCLASS(DefaultToInstanced, EditInlineNew, meta = (DisplayName = "Cost Actor Stat Tag"))
class GAEXT_API UAbilityCost_ActorStatTag : public UAbilityCost
{
	GENERATED_BODY()
public:
	UAbilityCost_ActorStatTag(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//
	// Whether the StatTag refers to the OwningActor or the AvatarActor
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	EConsumeStatTarget Target{ EConsumeStatTarget::AvatarActor };

	//
	// How much of the tag to spend (keyed on ability level)
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs")
	FScalableFloat Cost{ 1.0f };

	//
	// Which tag to spend some of
	//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Costs", meta = (Categories = "Stat"))
	FGameplayTag ConsumeTag;

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

};
