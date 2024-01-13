// Copyright (C) 2024 owoDra

#include "AnimNotify_GameplayEvent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotify_GameplayEvent)


UAnimNotify_GameplayEvent::UAnimNotify_GameplayEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


FString UAnimNotify_GameplayEvent::GetNotifyName_Implementation() const
{
	return EventTag.IsValid() ? EventTag.GetTagName().ToString() : TEXT("None");
}

void UAnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (auto* ASC{ UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MeshComp->GetOwner()) })
	{
		auto NewScopedWindow{ FScopedPredictionWindow(ASC, true) };

		ASC->HandleGameplayEvent(EventTag, &Payload);
	}
}
