// Copyright (C) 2024 owoDra

#include "GameFeatureAction_AddGameplayCuePath.h"

#include "GameplayCueManager.h"
#include "AbilitySystemGlobals.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddGameplayCuePath)


#define LOCTEXT_NAMESPACE "GameFeatures"

UGameFeatureAction_AddGameplayCuePath::UGameFeatureAction_AddGameplayCuePath()
{
	// Add a default path that is commonly used

	DirectoryPathsToAdd.Add(FDirectoryPath{ TEXT("/GameplayCues") });
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddGameplayCuePath::IsDataValid(TArray<FText>& ValidationErrors)
{
	auto Result{ Super::IsDataValid(ValidationErrors) };
	auto ErrorReason{ FText::GetEmpty() };

	for (const auto& Directory : DirectoryPathsToAdd)
	{
		if (Directory.Path.IsEmpty())
		{
			const auto InvalidCuePathError{ FText::Format(LOCTEXT("InvalidCuePathError", "'{0}' is not a valid path!"), FText::FromString(Directory.Path)) };
			
			ValidationErrors.Emplace(InvalidCuePathError);
			ValidationErrors.Emplace(ErrorReason);
			
			Result = CombineDataValidationResults(Result, EDataValidationResult::Invalid);
		}
	}

	return CombineDataValidationResults(Result, EDataValidationResult::Valid);
}
#endif	// WITH_EDITOR

#undef LOCTEXT_NAMESPACE
