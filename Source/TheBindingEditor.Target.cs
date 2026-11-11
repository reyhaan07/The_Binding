using UnrealBuildTool;

public class TheBindingEditorTarget : TargetRules
{
	public TheBindingEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("TheBinding");
	}
}
