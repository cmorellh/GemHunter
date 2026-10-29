// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

using UnrealBuildTool;
using System.Collections.Generic;

public class GemHunterTarget : TargetRules
{
	public GemHunterTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.AddRange(new string[] { "GemHunterCore", "GemHunter" });
	}
}
