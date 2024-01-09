// Copyright (C) 2024 owoDra

using UnrealBuildTool;

public class GAExt : ModuleRules
{
	public GAExt(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory,
                ModuleDirectory + "/GAExt",
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "ModularGameplay",
                "GameplayTags",
                "GameplayTasks",
                "GameplayAbilities",
                "GameFeatures",
                "DeveloperSettings",
                "NetCore",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "GFCore",
            }
        );

        SetupIrisSupport(Target);
    }
}
