// Copyright (C) 2024 owoDra

#include "GameFeatureObserver_AddGameplayCuePath.h"

#include "GameFeature/GameFeatureAction_AddGameplayCuePath.h"
#include "GAEGameplayCueManager.h"

#include "GameplayCueSet.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"
#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureObserver_AddGameplayCuePath)


UGameFeatureObserver_AddGameplayCuePath::UGameFeatureObserver_AddGameplayCuePath(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UGameFeatureObserver_AddGameplayCuePath::OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UGameFeatureObserver_AddGameplayCuePath::OnGameFeatureRegistering);
	
	const auto PluginRootPath{ TEXT("/") + PluginName };

	for (const auto& Action : GameFeatureData->GetActions())
	{
		if (const auto* AddGameplayCue{ Cast<UGameFeatureAction_AddGameplayCuePath>(Action) })
		{
			const auto& DirsToAdd{ AddGameplayCue->GetDirectoryPathsToAdd() };
			
			if (auto* GCM{ UGAEGameplayCueManager::Get() })
			{
				auto* RuntimeGameplayCueSet{ GCM->GetRuntimeCueSet() };
				const auto PreInitializeNumCues{ RuntimeGameplayCueSet ? RuntimeGameplayCueSet->GameplayCueData.Num() : 0 };

				for (const auto& Directory : DirsToAdd)
				{
					auto MutablePath{ Directory.Path };
					UGameFeaturesSubsystem::FixPluginPackagePath(MutablePath, PluginRootPath, false);
					GCM->AddGameplayCueNotifyPath(MutablePath, /** bShouldRescanCueAssets = */ false);	
				}
				
				// Rebuild the runtime library with these new paths

				if (!DirsToAdd.IsEmpty())
				{
					GCM->InitializeRuntimeObjectLibrary();	
				}

				const auto PostInitializeNumCues{ RuntimeGameplayCueSet ? RuntimeGameplayCueSet->GameplayCueData.Num() : 0 };
				if (PreInitializeNumCues != PostInitializeNumCues)
				{
					GCM->RefreshGameplayCuePrimaryAsset();
				}
			}
		}
	}
}

void UGameFeatureObserver_AddGameplayCuePath::OnGameFeatureUnregistering(const UGameFeatureData* GameFeatureData, const FString& PluginName, const FString& PluginURL)
{
	const auto PluginRootPath{ TEXT("/") + PluginName };

	for (const auto& Action : GameFeatureData->GetActions())
	{
		if (const auto* AddGameplayCue{ Cast<UGameFeatureAction_AddGameplayCuePath>(GameFeatureData) })
		{
			const auto& DirsToAdd{ AddGameplayCue->GetDirectoryPathsToAdd() };
			
			if (auto* GCM{ UGAEGameplayCueManager::Get() })
			{
				auto NumRemoved{ 0 };

				for (const auto& Directory : DirsToAdd)
				{
					auto MutablePath{ Directory.Path };
					UGameFeaturesSubsystem::FixPluginPackagePath(MutablePath, PluginRootPath, false);
					NumRemoved += GCM->RemoveGameplayCueNotifyPath(MutablePath, /** bShouldRescanCueAssets = */ false);
				}

				ensure(NumRemoved == DirsToAdd.Num());
				
				// Rebuild the runtime library only if there is a need to

				if (NumRemoved > 0)
				{
					GCM->InitializeRuntimeObjectLibrary();	
				}			
			}
		}
	}
}
