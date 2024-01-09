// Copyright (C) 2024 owoDra

#pragma once

#include "GameFeatureAction.h"

#include "GameFeatureAction_AddGameplayCuePath.generated.h"


/**
 * GameFeatureAction responsible for adding gameplay cue paths to the gameplay cue manager.
 */
UCLASS(meta = (DisplayName = "Add Gameplay Cue Path"))
class UGameFeatureAction_AddGameplayCuePath final : public UGameFeatureAction
{
	GENERATED_BODY()
public:
	UGameFeatureAction_AddGameplayCuePath();

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif // WITH_EDITOR


protected:
	//
	// List of paths to register to the gameplay cue manager. These are relative tot he game content directory
	//
	UPROPERTY(EditAnywhere, Category = "Gameplay Cues", meta = (RelativeToGameContentDir, LongPackageName))
	TArray<FDirectoryPath> DirectoryPathsToAdd;

public:
	const TArray<FDirectoryPath>& GetDirectoryPathsToAdd() const { return DirectoryPathsToAdd; }

};
