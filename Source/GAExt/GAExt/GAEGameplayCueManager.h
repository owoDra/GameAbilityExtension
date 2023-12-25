// Copyright (C) 2023 owoDra

#pragma once

#include "GameplayCueManager.h"

#include "GameplayTagContainer.h"

#include "GAEGameplayCueManager.generated.h"


/**
 * Game-specific manager for gameplay cues
 */
UCLASS()
class UGAEGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()
public:
	UGAEGameplayCueManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:
	struct FLoadedGameplayTagToProcessData
	{
		FGameplayTag Tag;
		TWeakObjectPtr<UObject> WeakOwner;

		FLoadedGameplayTagToProcessData() {}

		FLoadedGameplayTagToProcessData(const FGameplayTag& InTag, const TWeakObjectPtr<UObject>& InWeakOwner) 
			: Tag(InTag)
			, WeakOwner(InWeakOwner) 
		{}
	};

private:
	//
	// Cues that were preloaded on the client due to being referenced by content
	//
	UPROPERTY(Transient)
	TSet<UClass*> PreloadedCues;
	TMap<FObjectKey, TSet<FObjectKey>> PreloadedCueReferencers;

	//
	// Cues that were preloaded on the client and will always be loaded (code referenced or explicitly always loaded)
	//
	UPROPERTY(Transient)
	TSet<UClass*> AlwaysLoadedCues;

	TArray<FLoadedGameplayTagToProcessData> LoadedGameplayTagsToProcess;
	FCriticalSection LoadedGameplayTagsToProcessCS;

	bool bProcessLoadedTagsAfterGC{ false };

public:
	virtual void OnCreated() override;
	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override;
	virtual bool ShouldSyncLoadMissingGameplayCues() const override;
	virtual bool ShouldAsyncLoadMissingGameplayCues() const override;

	/**
	 * When delay loading cues, this will load the cues that must be always loaded anyway
	 */
	void LoadAlwaysLoadedCues();

	/**
	 * Updates the bundles for the singular gameplay cue primary asset
	 */
	void RefreshGameplayCuePrimaryAsset();

private:
	void OnGameplayTagLoaded(const FGameplayTag& Tag);
	void HandlePostGarbageCollect();
	void ProcessLoadedTags();
	void ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject);
	void OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject, bool bAlwaysLoadedCue);
	void RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject);
	void HandlePostLoadMap(UWorld* NewWorld);
	void UpdateDelayLoadDelegateListeners();
	bool ShouldDelayLoadGameplayCues() const;


public:
	static void DumpGameplayCues(const TArray<FString>& Args);

	static UGAEGameplayCueManager* Get();

};
