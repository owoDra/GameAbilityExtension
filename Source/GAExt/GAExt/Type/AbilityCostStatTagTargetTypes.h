// Copyright (C) 2024 owoDra

#pragma once

#include "AbilityCostStatTagTargetTypes.generated.h"


/**
 * Enumeration to determine targets related to ability cost
 */
UENUM(BlueprintType)
enum class EStatTagCostTarget : uint8
{
	// SourceObject of Abilities holding AbilityCost
	SourceObject,

	// AvatarActor of Abilities holding AbilityCost
	Avatar,

	// OwnerActor of Abilities holding AbilityCost
	Owner,

	// Controller in ActorInfo of the ability that holds the AbilityCost
	Controller,

	// PlayerState in ActorInfo of the ability that holds the AbilityCost
	PlayerState
};
