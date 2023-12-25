// Copyright (C) 2023 owoDra

#pragma once

#include "GameFeatureStateChangeObserver.h"

#include "GameFeatureObserver_AddGameplayCuePath.generated.h"

class UGameFeatureData;


/**
 * Observe the addition and deletion of GameplayCuePath by registering and unregistering GmeFeatures
 */
UCLASS()
class UGameFeatureObserver_AddGameplayCuePath : public UObject, public IGameFeatureStateChangeObserver
{
	GENERATED_BODY()
public:
	UGameFeatureObserver_AddGameplayCuePath(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;
	virtual void OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL) override;

};
