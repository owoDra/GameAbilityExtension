// Copyright (C) 2024 owoDra

#include "AnimNotifyState_AddLooseTag.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotifyState_AddLooseTag)


UAnimNotifyState_AddLooseTag::UAnimNotifyState_AddLooseTag(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


FString UAnimNotifyState_AddLooseTag::GetNotifyName_Implementation() const
{
	return Tag.IsValid() ? Tag.GetTagName().ToString() : TEXT("None");
}

void UAnimNotifyState_AddLooseTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (auto* ASC{ UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MeshComp->GetOwner()) })
	{
		ASC->AddLooseGameplayTag(Tag);

		if (bShouldReplicate)
		{
			ASC->AddReplicatedLooseGameplayTag(Tag);
		}
	}
}

void UAnimNotifyState_AddLooseTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (auto* ASC{ UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MeshComp->GetOwner()) })
	{
		ASC->RemoveLooseGameplayTag(Tag);

		if (bShouldReplicate)
		{
			ASC->RemoveReplicatedLooseGameplayTag(Tag);
		}
	}
}
