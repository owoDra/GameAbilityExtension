// Copyright (C) 2024 owoDra

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"

#include "GameplayTagContainer.h"

#include "AnimNotify_GameplayEvent.generated.h"

class UAnimMontage;


/**
 * AnimNotify class that sends a GameplayEvent to the Actor that owns the Mesh in which the Animation is playing.
 */
UCLASS(BlueprintType, meta = (DisplayName = "AN Send Gameplay Event"))
class UAnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAnimNotify_GameplayEvent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gameplay Event", meta = (Categories = "Event"))
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gameplay Event")
	FGameplayEventData Payload;

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp, 
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

};