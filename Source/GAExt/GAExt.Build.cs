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
                "Core", "CoreUObject", "Engine",

                "ModularGameplay", "GameplayTags", "GameFeatures",

                "GameplayTasks", "GameplayAbilities",

                "GFCore",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "NetCore",

                "DeveloperSettings",
            }
        );

        SetupIrisSupport(Target);
    }
}
