// Copyright (C) 2024 owoDra

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

	//////////////////////////////////////////////////////////////
	// Enssential functions
public:
	/**
	 * Checks if we can afford this cost.
	 *
	 * Tips:
	 *	A failure reason tag can be added to OptionalRelevantTags (if non-null), which can be queried
	 *	elsewhere to determine how to provide user feedback (e.g., a clicking noise if a weapon is out of ammo)
	 * 
	 * Note:
	 *	Ability and ActorInfo are guaranteed to be non-null on entry, but OptionalRelevantTags can be nullptr.
	 */
	virtual bool CheckCost(
		const UGAEGameplayAbility* Ability
		, const FGameplayAbilitySpecHandle Handle
		, const FGameplayAbilityActorInfo* ActorInfo
		, FGameplayTagContainer* OptionalRelevantTags) const PURE_VIRTUAL(, return false;);

	/**
	 * Applies the ability's cost to the target
	 *
	 * Note:
 	 *  Ability and ActorInfo are guaranteed to be non-null on entry.
	 */
	virtual void ApplyCost(
		const UGAEGameplayAbility* Ability
		, const FGameplayAbilitySpecHandle Handle
		, const FGameplayAbilityActorInfo* ActorInfo
		, const FGameplayAbilityActivationInfo ActivationInfo) PURE_VIRTUAL(, );


	///////////////////////////////////////////////////////////////
	// Optional functions
	//
	//  The following functions are not necessarily required for operation, 
	//  but their use can be used to easily implement simple cost-related processes. (e.g, cooltime recharge, kill recharge)
	//  
	//  However, for complex processes, it is recommended that they be implemented within the respective GameplayAbility. (e.g, reloading).
	//
public:
	/**
	 * Executed when ability is added.
	 *
	 * Tips:
	 *	Override this function if necessary to initialize costs, set up listen, etc.
	 * 
	 * Note:
 	 *  Ability is guaranteed to be non-null on entry.
	 */
	virtual void OnGiveAbility(
		const UGAEGameplayAbility* Ability
		, const FGameplayAbilityActorInfo* ActorInfo
		, const FGameplayAbilitySpec& Spec) {}

	/**
	 * Executed when ability is removed.
	 *
	 * Tips:
	 *	Override this function if necessary to stop listen, etc.
	 *
	 * Note:
	 *  Ability is guaranteed to be non-null on entry.
	 */
	virtual void OnRemoveAbility(
		const UGAEGameplayAbility* Ability
		, const FGameplayAbilityActorInfo* ActorInfo
		, const FGameplayAbilitySpec& Spec) {}

	/**
	 * Executed when avatar has set.
	 *
	 * Tips:
	 *	Override this function if necessary to initialize costs, set up listen, etc.
	 *
	 * Note:
	 *  Ability and ActorInfo is guaranteed to be non-null on entry.
	 */
	virtual void OnAvatarSet(
		const UGAEGameplayAbility* Ability
		, const FGameplayAbilityActorInfo* ActorInfo
		, const FGameplayAbilitySpec& Spec) {}

	/**
	 * Executed when ability's cooldown has done.
	 *
	 * Tips:
	 *	Override this function if necessary, e.g. to charge costs
	 *
	 * Note:
	 *  Ability and ActorInfo is guaranteed to be non-null on entry.
	 */
	virtual void OnCooldownEnd(
		const UGAEGameplayAbility* Ability
		, const FGameplayAbilitySpecHandle Handle
		, const FGameplayAbilityActorInfo* ActorInfo
		, const FGameplayAbilityActivationInfo ActivationInfo) {}

};
