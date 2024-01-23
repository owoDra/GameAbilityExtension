// Copyright (C) 2024 owoDra

#include "GameFeatureAction_AddAbilities.h"

#include "GAEAbilitySystemComponent.h"

#include "Components/GameFrameworkComponentManager.h"
#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddAbilities)


#define LOCTEXT_NAMESPACE "GameFeatures"

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddAbilities::IsDataValid(TArray<FText>& ValidationErrors)
{
	auto Result{ CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid) };

	auto EntryIndex{ 0 };
	for (const auto& Entry : AbilitiesToAdd)
	{
		if (Entry.ActorClass.IsNull())
		{
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
			ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNullActor", "Null ActorClass at index {0} in AbilitiesList"), FText::AsNumber(EntryIndex)));
		}

		auto AttributeSetIndex{ 0 };
		for (const auto& AttributeSetPtr : Entry.GrantedAbilitySets)
		{
			if (AttributeSetPtr.IsNull())
			{
				Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
				ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNullAttributeSet", "Null AbilitySet at index {0} in AbilitiesList[{1}].GrantedAbilitySets"), FText::AsNumber(AttributeSetIndex), FText::AsNumber(EntryIndex)));
			}
			++AttributeSetIndex;
		}
		++EntryIndex;
	}

	return Result;
}
#endif


void UGameFeatureAction_AddAbilities::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	auto& ActiveData{ ContextData.FindOrAdd(Context) };

	if (!ensureAlways(ActiveData.ActiveExtensions.IsEmpty()) || !ensureAlways(ActiveData.ComponentRequests.IsEmpty()))
	{
		Reset(ActiveData);
	}

	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	auto* ActiveData{ ContextData.Find(Context) };

	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}


void UGameFeatureAction_AddAbilities::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	auto* World{ WorldContext.World() };
	auto GameInstance{ WorldContext.OwningGameInstance };
	auto& ActiveData{ ContextData.FindOrAdd(ChangeContext) };

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		if (auto* Manager{ UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance) })
		{			
			auto EntryIndex{ 0 };
			for (const auto& Entry : AbilitiesToAdd)
			{
				if (!Entry.ActorClass.IsNull())
				{
					auto AddAbilitiesDelegate{ UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &UGameFeatureAction_AddAbilities::HandleActorExtension, EntryIndex, ChangeContext) };
					auto ExtensionRequestHandle{ Manager->AddExtensionHandler(Entry.ActorClass, AddAbilitiesDelegate) };

					ActiveData.ComponentRequests.Add(ExtensionRequestHandle);

					EntryIndex++;
				}
			}
		}
	}
}


void UGameFeatureAction_AddAbilities::Reset(FPerContextData& ActiveData)
{
	while (!ActiveData.ActiveExtensions.IsEmpty())
	{
		auto ExtensionIt{ ActiveData.ActiveExtensions.CreateIterator() };
		RemoveActorAbilities(ExtensionIt->Key, ActiveData);
	}

	ActiveData.ComponentRequests.Empty();
}

void UGameFeatureAction_AddAbilities::HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext)
{
	auto* ActiveData{ ContextData.Find(ChangeContext) };

	if (AbilitiesToAdd.IsValidIndex(EntryIndex) && ActiveData)
	{
		const auto& Entry{ AbilitiesToAdd[EntryIndex] };

		if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
		{
			RemoveActorAbilities(Actor, *ActiveData);
		}
		else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == UGAEAbilitySystemComponent::NAME_AbilityReady))
		{
			AddActorAbilities(Actor, Entry, *ActiveData);
		}
	}
}

void UGameFeatureAction_AddAbilities::AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData)
{
	check(Actor);

	if (!Actor->HasAuthority())
	{
		return;
	}

	// early out if Actor already has ability extensions applied

	if (ActiveData.ActiveExtensions.Find(Actor) != nullptr)
	{
		return;	
	}

	if (auto* AbilitySystemComponent{ UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor) })
	{
		FActorExtensions AddedExtensions;
		AddedExtensions.AbilitySetHandles.Reserve(AbilitiesEntry.GrantedAbilitySets.Num());

		for (const auto& AbilitySetSoftObj : AbilitiesEntry.GrantedAbilitySets)
		{
			const auto* AbilitySet{ AbilitySetSoftObj.IsValid() ? AbilitySetSoftObj.Get() : AbilitySetSoftObj.LoadSynchronous() };

			if (AbilitySet)
			{
				AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, &AddedExtensions.AbilitySetHandles.AddDefaulted_GetRef());
			}
		}

		ActiveData.ActiveExtensions.Add(Actor, AddedExtensions);
	}
	else
	{
		UE_LOG(LogGameFeatures, Error, TEXT("Failed to find/add an ability component to '%s'. Abilities will not be granted."), *Actor->GetPathName());
	}
}

void UGameFeatureAction_AddAbilities::RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData)
{
	if (auto* ActorExtensions{ ActiveData.ActiveExtensions.Find(Actor) })
	{
		if (auto* AbilitySystemComponent{ UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor) })
		{
			for (auto& Handles : ActorExtensions->AbilitySetHandles)
			{
				Handles.TakeFromAbilitySystem(AbilitySystemComponent);
			}
		}

		ActiveData.ActiveExtensions.Remove(Actor);
	}
}

#undef LOCTEXT_NAMESPACE
