using UnrealBuildTool;

public class TheBinding : ModuleRules
{
	public TheBinding(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Lets code include headers by folder, e.g. #include "Player/BreathComponent.h"
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule"      // noise events the Bound One will hear (UAISense_Hearing)
		});
	}
}
