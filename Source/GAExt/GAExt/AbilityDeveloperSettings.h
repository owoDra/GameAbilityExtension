// Copyright (C) 2023 owoDra

#pragma once

#include "Engine/DeveloperSettings.h"

#include "AbilityDeveloperSettings.generated.h"

/**
 * When the Editor loads GameplauCue
 */
UENUM(BlueprintType)
enum class EGameplayCueEditorLoadMode : uint8
{
	// Loads all cues upfront; longer loading speed in the editor but short PIE times and effects never fail to play
	LoadUpfront,

	// Outside of editor: Async loads as cue tag are registered
	// In editor: Async loads when cues are invoked
	//   Note: This can cause some 'why didn't I see the effect for X' issues in PIE and is good for iteration speed but otherwise bad for designers
	PreloadAsCuesAreReferenced_GameOnly,

	// Async loads as cue tag are registered
	PreloadAsCuesAreReferenced
};


/**
 * Settings for a Game framework.
 */
UCLASS(Config = "Game", Defaultconfig, meta = (DisplayName = "Game Ability Extension"))
class UAbilityDeveloperSettings : public UDeveloperSettings
{
public:
	GENERATED_BODY()
public:
	UAbilityDeveloperSettings();

	///////////////////////////////////////////////
	// Gameplay Cue
public:
	//
	// When the Editor loads GameplauCue
	//
	UPROPERTY(Config, EditAnywhere, Category = "Gameplay Cue")
	EGameplayCueEditorLoadMode GameplayCueLoadMode{ EGameplayCueEditorLoadMode::LoadUpfront };

};

