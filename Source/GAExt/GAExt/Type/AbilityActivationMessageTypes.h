// Copyright (C) 2024 owoDra

#pragma once

#include "GameplayTagContainer.h"

#include "AbilityActivationMessageTypes.generated.h"

class AActor;
class UGAEGameplayAbility;


/**
 * Data for messages that notify the ability activation
 */
USTRUCT(BlueprintType)
struct FAbilityActivationMessage
{
	GENERATED_BODY()
public:
	FAbilityActivationMessage() {}

public:
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<const UGAEGameplayAbility> Ability{ nullptr };

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> OwnerActor{ nullptr };

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> AvatarActor{ nullptr };

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> SourceObject{ nullptr };

};
