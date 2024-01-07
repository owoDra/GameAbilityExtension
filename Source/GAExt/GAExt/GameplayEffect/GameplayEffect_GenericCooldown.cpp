// Copyright (C) 2023 owoDra

#include "GameplayEffect_GenericCooldown.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayEffect_GenericCooldown)


const FName UGameplayEffect_GenericCooldown::NAME_SetByCaller_Cooldown("Cooldown");

UGameplayEffect_GenericCooldown::UGameplayEffect_GenericCooldown(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat NewSetByCallerFloat;
	NewSetByCallerFloat.DataName = UGameplayEffect_GenericCooldown::NAME_SetByCaller_Cooldown;

	DurationMagnitude = FGameplayEffectModifierMagnitude(NewSetByCallerFloat);
}
