using UnrealBuildTool;
using System.Collections.Generic;

public class GaiaEditorTarget : TargetRules
{
	public GaiaEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.AddRange(new string[] { "GaiaGame" });
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
	}
}
