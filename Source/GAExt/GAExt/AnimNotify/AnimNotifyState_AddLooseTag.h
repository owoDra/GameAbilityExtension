// Copyright (C) 2024 owoDra

#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"

#include "GameplayTagContainer.h"

#include "AnimNotifyState_AddLooseTag.generated.h"


/**
 * AnimNotifyState class to temporarily add a LooseTag to the Actor that owns the Mesh in which the Animation is playing.
 */
UCLASS(BlueprintType, meta = (DisplayName = "ANS Add Loose Tag"))
class UAnimNotifyState_AddLooseTag : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	UAnimNotifyState_AddLooseTag(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AddLooseTag")
	FGameplayTag Tag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AddLooseTag")
	bool bShouldReplicate{ false };

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration, 
		const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

};