// Copyright (C) 2024 owoDra

#pragma once

#include "GameplayTagContainer.h"

#include "AbilityCooldownMessageTypes.generated.h"

class AActor;
class UGAEGameplayAbility;


/**
 * Data for messages that signal the beginning or end of an ability's cooldown
 */
USTRUCT(BlueprintType)
struct FAbilityCooldownMessage
{
	GENERATED_BODY()
public:
	FAbilityCooldownMessage() {}

public:
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<const UGAEGameplayAbility> Ability{ nullptr };

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> OwnerActor{ nullptr };

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> AvatarActor{ nullptr };

	UPROPERTY(BlueprintReadWrite)
	float Duration{ 0.0f };

};
