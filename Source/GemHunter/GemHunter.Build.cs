// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

using UnrealBuildTool;

public class GemHunter : ModuleRules
{
	public GemHunter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GemHunterCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
