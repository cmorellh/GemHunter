// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

using UnrealBuildTool;

public class GemHunterCore : ModuleRules
{
	public GemHunterCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// GemHunterCore holds runtime data types and subsystems only.
		// It must never depend on the GemHunter game module, so the data
		// layer stays reusable and testable on its own.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
