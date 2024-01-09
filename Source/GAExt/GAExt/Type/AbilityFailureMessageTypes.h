// Copyright (C) 2024 owoDra

#pragma once

#include "GameplayTagContainer.h"

#include "AbilityFailureMessageTypes.generated.h"


/**
 * Data for messages when abilities fail, used in GameplayMessage
 */
USTRUCT(BlueprintType)
struct FAbilityFailureMessage
{
	GENERATED_BODY()
public:
	FAbilityFailureMessage() {}

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController{ nullptr };

	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	FText UserFacingReason;

};


/**
 * Data for the message in the montage that is played when an ability fails, which is used in GameplayMessage.
 */
USTRUCT(BlueprintType)
struct FAbilityFailureMontageMessage
{
	GENERATED_BODY()
public:
	FAbilityFailureMontageMessage() {}

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController{ nullptr };

	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage{ nullptr };

};
