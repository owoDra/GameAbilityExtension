// Copyright (C) 2023 owoDra

#pragma once

#include "GameplayAbilitySpec.h"

#include "AbilityCost.generated.h"

class UGAEGameplayAbility;


/**
 * Base class that defines the cost consumed when executing an ability
 */
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class GAEXT_API UAbilityCost : public UObject
{
	GENERATED_BODY()
public:
	UAbilityCost(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	/**
	 * Checks if we can afford this cost.
	 *
	 * Tips:
	 *	A failure reason tag can be added to OptionalRelevantTags (if non-null), which can be queried
	 *	elsewhere to determine how to provide user feedback (e.g., a clicking noise if a weapon is out of ammo)
	 * 
	 *	Ability and ActorInfo are guaranteed to be non-null on entry, but OptionalRelevantTags can be nullptr.
	 */
	virtual bool CheckCost(
		const UGAEGameplayAbility* Ability,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags) const PURE_VIRTUAL(, return false;);

	/**
	 * Applies the ability's cost to the target
	 *
	 * Note:
 	 *  Ability and ActorInfo are guaranteed to be non-null on entry.
	 */
	virtual void ApplyCost(
		const UGAEGameplayAbility* Ability,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) PURE_VIRTUAL(, );

};
