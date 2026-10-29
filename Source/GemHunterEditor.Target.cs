// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

using UnrealBuildTool;
using System.Collections.Generic;

public class GemHunterEditorTarget : TargetRules
{
	public GemHunterEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.AddRange(new string[] { "GemHunterCore", "GemHunter" });
	}
}
