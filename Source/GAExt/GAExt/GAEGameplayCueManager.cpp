// Copyright (C) 2023 owoDra

#include "GAEGameplayCueManager.h"

#include "AbilityDeveloperSettings.h"
#include "GAExtLogs.h"

#include "AbilitySystemGlobals.h"
#include "GameplayTagsManager.h"
#include "GameplayCueSet.h"
#include "HAL/IConsoleManager.h"
#include "UObject/UObjectThreadContext.h"
#include "Async/Async.h"
#include "Algo/Transform.h"
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GAEGameplayCueManager)


//////////////////////////////////////////////////////////////////////
// GAEGameplayCueManagerCvars

#pragma region GAEGameplayCueManagerCvars

namespace GAEGameplayCueManagerCvars
{
	static FAutoConsoleCommand CVarDumpGameplayCues(
		TEXT("DumpGameplayCues"),
		TEXT("Shows all assets that were loaded via GAEGameplayCueManager and are currently in memory."),
		FConsoleCommandWithArgsDelegate::CreateStatic(UGAEGameplayCueManager::DumpGameplayCues));
}

#pragma endregion


//////////////////////////////////////////////////////////////////////
// FGameplayCueTagThreadSynchronizeGraphTask

#pragma region FGameplayCueTagThreadSynchronizeGraphTask

struct FGameplayCueTagThreadSynchronizeGraphTask : public FAsyncGraphTaskBase
{
	TFunction<void()> TheTask;

	ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }

	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) 
	{ 
		TheTask(); 
	}

	FGameplayCueTagThreadSynchronizeGraphTask(TFunction<void()>&& Task) 
		: TheTask(MoveTemp(Task)) 
	{ }
};

#pragma endregion


//////////////////////////////////////////////////////////////////////
// UFortAssetManager

#pragma region UFortAssetManager

namespace UFortAssetManager
{
	static const auto GameplayCueRefsType{ FPrimaryAssetType(TEXT("GameplayCueRefs")) };
	static const auto GameplayCueRefsName{ FName(TEXT("GameplayCueReferences")) };
	static const auto LoadStateClient{ FName(TEXT("Client")) };
}

#pragma endregion


//////////////////////////////////////////////////////////////////////
// UGAEGameplayCueManager

#pragma region UGAEGameplayCueManager

UGAEGameplayCueManager::UGAEGameplayCueManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UGAEGameplayCueManager::OnCreated()
{
	Super::OnCreated();

	UpdateDelayLoadDelegateListeners();
}


bool UGAEGameplayCueManager::ShouldAsyncLoadRuntimeObjectLibraries() const
{
	const auto* DevSettings{ GetDefault<UAbilityDeveloperSettings>() };

	switch (DevSettings->GameplayCueLoadMode)
	{
	case EGameplayCueEditorLoadMode::LoadUpfront:
		return true;

	case EGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return false;
		}
#endif
		break;

	case EGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	return !ShouldDelayLoadGameplayCues();
}

bool UGAEGameplayCueManager::ShouldSyncLoadMissingGameplayCues() const
{
	return false;
}

bool UGAEGameplayCueManager::ShouldAsyncLoadMissingGameplayCues() const
{
	return true;
}


void UGAEGameplayCueManager::LoadAlwaysLoadedCues()
{
	if (ShouldDelayLoadGameplayCues())
	{
		static const auto NAME_GameplayCueTag{ FName(TEXT("GameplayCue")) };

		auto& TagManager{ UGameplayTagsManager::Get() };

		const auto OriginalGameplayCueTag{ TagManager.RequestGameplayTag(NAME_GameplayCueTag, /*ErrorIfNotFound=*/ true) };
		const auto AdditionalAlwaysLoadedCueTags{ TagManager.RequestGameplayTagChildren(OriginalGameplayCueTag) };

		for (const auto& CueTag : AdditionalAlwaysLoadedCueTags)
		{
			if (CueTag.IsValid())
			{
				ProcessTagToPreload(CueTag, nullptr);
			}
			else
			{
				UE_LOG(LogGAE, Warning, TEXT("UGAEGameplayCueManager::AdditionalAlwaysLoadedCueTags contains invalid tag %s"), *CueTag.GetTagName().ToString());
			}
		}
	}
}

void UGAEGameplayCueManager::RefreshGameplayCuePrimaryAsset()
{
	TArray<FSoftObjectPath> CuePaths;

	if (auto* RuntimeGameplayCueSet{ GetRuntimeCueSet() })
	{
		RuntimeGameplayCueSet->GetSoftObjectPaths(CuePaths);
	}

	FAssetBundleData BundleData;
	BundleData.AddBundleAssetsTruncated(UFortAssetManager::LoadStateClient, CuePaths);

	auto PrimaryAssetId{ FPrimaryAssetId(UFortAssetManager::GameplayCueRefsType, UFortAssetManager::GameplayCueRefsName) };

	UAssetManager::Get().AddDynamicAsset(PrimaryAssetId, FSoftObjectPath(), BundleData);
}


void UGAEGameplayCueManager::OnGameplayTagLoaded(const FGameplayTag& Tag)
{
	auto ScopeLock{ FScopeLock(&LoadedGameplayTagsToProcessCS) };
	auto bStartTask{ LoadedGameplayTagsToProcess.Num() == 0 };
	auto* LoadContext{ FUObjectThreadContext::Get().GetSerializeContext() };
	auto* OwningObject{ LoadContext ? LoadContext->SerializedObject : nullptr };

	LoadedGameplayTagsToProcess.Emplace(Tag, OwningObject);

	if (bStartTask)
	{
		TGraphTask<FGameplayCueTagThreadSynchronizeGraphTask>::CreateTask().ConstructAndDispatchWhenReady(
			[]()
			{
				if (GIsRunning)
				{
					if (auto* StrongThis{ Get() })
					{
						// If we are garbage collecting we cannot call StaticFindObject (or a few other static uobject functions), so we'll just wait until the GC is over and process the tags then
						
						if (IsGarbageCollecting())
						{
							StrongThis->bProcessLoadedTagsAfterGC = true;
						}
						else
						{
							StrongThis->ProcessLoadedTags();
						}
					}
				}
			}
		);
	}
}

void UGAEGameplayCueManager::HandlePostGarbageCollect()
{
	if (bProcessLoadedTagsAfterGC)
	{
		ProcessLoadedTags();
	}

	bProcessLoadedTagsAfterGC = false;
}

void UGAEGameplayCueManager::ProcessLoadedTags()
{
	// Lock LoadedGameplayTagsToProcess just long enough to make a copy and clear

	auto TaskLoadedGameplayTagsToProcess{ TArray<FLoadedGameplayTagToProcessData>() };
	auto TaskScopeLock{ FScopeLock(&LoadedGameplayTagsToProcessCS) };
	TaskLoadedGameplayTagsToProcess = LoadedGameplayTagsToProcess;
	LoadedGameplayTagsToProcess.Empty();

	// This might return during shutdown, and we don't want to proceed if that is the case

	if (GIsRunning)
	{
		if (RuntimeGameplayCueObjectLibrary.CueSet)
		{
			for (const auto& LoadedTagData : TaskLoadedGameplayTagsToProcess)
			{
				if (RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Contains(LoadedTagData.Tag))
				{
					if (!LoadedTagData.WeakOwner.IsStale())
					{
						ProcessTagToPreload(LoadedTagData.Tag, LoadedTagData.WeakOwner.Get());
					}
				}
			}
		}
		else
		{
			UE_LOG(LogGAE, Warning, TEXT("UGAEGameplayCueManager::OnGameplayTagLoaded processed loaded tag(s) but RuntimeGameplayCueObjectLibrary.CueSet was null. Skipping processing."));
		}
	}
}

void UGAEGameplayCueManager::ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject)
{
	const auto* DevSettings{ GetDefault<UAbilityDeveloperSettings>() };

	switch (DevSettings->GameplayCueLoadMode)
	{
	case EGameplayCueEditorLoadMode::LoadUpfront:
		return;

	case EGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return;
		}
#endif
		break;

	case EGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	check(RuntimeGameplayCueObjectLibrary.CueSet);

	auto* DataIdx{ RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Find(Tag) };
	if (DataIdx && RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData.IsValidIndex(*DataIdx))
	{
		const auto& CueData{ RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData[*DataIdx] };

		if (auto* LoadedGameplayCueClass{ FindObject<UClass>(nullptr, *CueData.GameplayCueNotifyObj.ToString()) })
		{
			RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject);
		}
		else
		{
			auto bAlwaysLoadedCue{ OwningObject == nullptr };
			auto WeakOwner{ TWeakObjectPtr<UObject>(OwningObject) };
			StreamableManager.RequestAsyncLoad(
				CueData.GameplayCueNotifyObj,
				FStreamableDelegate::CreateUObject(
					this,
					&ThisClass::OnPreloadCueComplete,
					CueData.GameplayCueNotifyObj,
					WeakOwner,
					bAlwaysLoadedCue),
				FStreamableManager::DefaultAsyncLoadPriority,
				false,
				false,
				TEXT("GameplayCueManager"));
		}
	}
}

void UGAEGameplayCueManager::OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject, bool bAlwaysLoadedCue)
{
	if (bAlwaysLoadedCue || OwningObject.IsValid())
	{
		if (auto* LoadedGameplayCueClass{ Cast<UClass>(Path.ResolveObject()) })
		{
			RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject.Get());
		}
	}
}

void UGAEGameplayCueManager::RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject)
{
	check(LoadedGameplayCueClass);

	const auto bAlwaysLoadedCue{ OwningObject == nullptr };

	if (bAlwaysLoadedCue)
	{
		AlwaysLoadedCues.Add(LoadedGameplayCueClass);
		PreloadedCues.Remove(LoadedGameplayCueClass);
		PreloadedCueReferencers.Remove(LoadedGameplayCueClass);
	}
	else if ((OwningObject != LoadedGameplayCueClass) && (OwningObject != LoadedGameplayCueClass->GetDefaultObject()) && !AlwaysLoadedCues.Contains(LoadedGameplayCueClass))
	{
		PreloadedCues.Add(LoadedGameplayCueClass);

		auto& ReferencerSet{ PreloadedCueReferencers.FindOrAdd(LoadedGameplayCueClass) };
		ReferencerSet.Add(OwningObject);
	}
}

void UGAEGameplayCueManager::HandlePostLoadMap(UWorld* NewWorld)
{
	if (RuntimeGameplayCueObjectLibrary.CueSet)
	{
		for (const auto& CueClass : AlwaysLoadedCues)
		{
			RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
		}

		for (const auto& CueClass : PreloadedCues)
		{
			RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
		}
	}

	for (auto CueIt{ PreloadedCues.CreateIterator() }; CueIt; ++CueIt)
	{
		auto& ReferencerSet{ PreloadedCueReferencers.FindChecked(*CueIt) };

		for (auto RefIt{ ReferencerSet.CreateIterator() }; RefIt; ++RefIt)
		{
			if (!RefIt->ResolveObjectPtr())
			{
				RefIt.RemoveCurrent();
			}
		}

		if (ReferencerSet.Num() == 0)
		{
			PreloadedCueReferencers.Remove(*CueIt);
			CueIt.RemoveCurrent();
		}
	}
}

void UGAEGameplayCueManager::UpdateDelayLoadDelegateListeners()
{
	UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.RemoveAll(this);
	FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	const auto* DevSettings{ GetDefault<UAbilityDeveloperSettings>() };

	switch (DevSettings->GameplayCueLoadMode)
	{
	case EGameplayCueEditorLoadMode::LoadUpfront:
		return;

	case EGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return;
		}
#endif
		break;

	case EGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.AddUObject(this, &ThisClass::OnGameplayTagLoaded);
	FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this, &ThisClass::HandlePostGarbageCollect);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
}

bool UGAEGameplayCueManager::ShouldDelayLoadGameplayCues() const
{
	const auto bClientDelayLoadGameplayCues{ true };

	return !IsRunningDedicatedServer() && bClientDelayLoadGameplayCues;
}


void UGAEGameplayCueManager::DumpGameplayCues(const TArray<FString>& Args)
{
	auto* GCM{ UGAEGameplayCueManager::Get() };

	if (!GCM)
	{
		UE_LOG(LogGAE, Error, TEXT("DumpGameplayCues failed. No UGAEGameplayCueManager found."));
		return;
	}

	const auto bIncludeRefs{ Args.Contains(TEXT("Refs")) };

	UE_LOG(LogGAE, Log, TEXT("=========== Dumping Always Loaded Gameplay Cue Notifies ==========="));
	for (const auto& CueClass : GCM->AlwaysLoadedCues)
	{
		UE_LOG(LogGAE, Log, TEXT("  %s"), *GetPathNameSafe(CueClass));
	}

	UE_LOG(LogGAE, Log, TEXT("=========== Dumping Preloaded Gameplay Cue Notifies ==========="));
	for (const auto& CueClass : GCM->PreloadedCues)
	{
		auto* ReferencerSet{ GCM->PreloadedCueReferencers.Find(CueClass) };
		auto NumRefs{ ReferencerSet ? ReferencerSet->Num() : 0 };

		UE_LOG(LogGAE, Log, TEXT("  %s (%d refs)"), *GetPathNameSafe(CueClass), NumRefs);

		if (bIncludeRefs && ReferencerSet)
		{
			for (const auto& Ref : *ReferencerSet)
			{
				auto* RefObject{ Ref.ResolveObjectPtr() };

				UE_LOG(LogGAE, Log, TEXT("    ^- %s"), *GetPathNameSafe(RefObject));
			}
		}
	}

	UE_LOG(LogGAE, Log, TEXT("=========== Dumping Gameplay Cue Notifies loaded on demand ==========="));
	auto NumMissingCuesLoaded{ 0 };
	if (GCM->RuntimeGameplayCueObjectLibrary.CueSet)
	{
		for (const auto& CueData : GCM->RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData)
		{
			if (CueData.LoadedGameplayCueClass && !GCM->AlwaysLoadedCues.Contains(CueData.LoadedGameplayCueClass) && !GCM->PreloadedCues.Contains(CueData.LoadedGameplayCueClass))
			{
				NumMissingCuesLoaded++;
				UE_LOG(LogGAE, Log, TEXT("  %s"), *CueData.LoadedGameplayCueClass->GetPathName());
			}
		}
	}

	UE_LOG(LogGAE, Log, TEXT("=========== Gameplay Cue Notify summary ==========="));
	UE_LOG(LogGAE, Log, TEXT("  ... %d cues in always loaded list"), GCM->AlwaysLoadedCues.Num());
	UE_LOG(LogGAE, Log, TEXT("  ... %d cues in preloaded list"), GCM->PreloadedCues.Num());
	UE_LOG(LogGAE, Log, TEXT("  ... %d cues loaded on demand"), NumMissingCuesLoaded);
	UE_LOG(LogGAE, Log, TEXT("  ... %d cues in total"), GCM->AlwaysLoadedCues.Num() + GCM->PreloadedCues.Num() + NumMissingCuesLoaded);
}

UGAEGameplayCueManager* UGAEGameplayCueManager::Get()
{
	return Cast<UGAEGameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
}

#pragma endregion
