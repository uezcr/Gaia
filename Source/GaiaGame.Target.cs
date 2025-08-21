using UnrealBuildTool;
using System.Collections.Generic;

public class GaiaGameTarget : TargetRules
{
	public GaiaGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		
		ExtraModuleNames.AddRange(new string[] { "GaiaGame" });
		
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
	}
}
