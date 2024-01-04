// Copyright (C) 2023 owoDra

#pragma once

#include "AbilitySystemGlobals.h"

#include "GAEAbilitySystemGlobals.generated.h"


UCLASS(Config = "Game")
class UGAEAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
public:
	UGAEAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

};
