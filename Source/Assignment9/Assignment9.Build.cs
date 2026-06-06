// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Assignment9 : ModuleRules
{
	public Assignment9(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" , "NavigationSystem", 
			
			//온라인
			"OnlineSubsystem", "OnlineSubsystemNull", "OnlineSubsystemUtils"

        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		PublicIncludePaths.AddRange(new string[] { "Assignment9" });
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
