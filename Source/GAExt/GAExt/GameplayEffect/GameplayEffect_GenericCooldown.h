// Copyright (C) 2023 owoDra

#pragma once

#include "GameplayEffect.h"

#include "GameplayEffect_GenericCooldown.generated.h"


/**
 * GameplayEffect class used to represent generic ability cooldowns
 */
UCLASS(Blueprintable)
class UGameplayEffect_GenericCooldown : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGameplayEffect_GenericCooldown(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static const FName NAME_SetByCaller_Cooldown;
};
