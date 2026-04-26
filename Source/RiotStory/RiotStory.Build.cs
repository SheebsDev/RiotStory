// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RiotStory : ModuleRules
{
	public RiotStory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"GameplayMessageRuntime",
			"GameplayTasks",
			"UMG",
			"Slate",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"RiotStory",
			"RiotStory/Conversation",
			"RiotStory/Interaction",
			"RiotStory/Inventory",
			"RiotStory/Variant_Shooter",
			"RiotStory/Variant_Shooter/AI",
			"RiotStory/Variant_Shooter/Tasks",
			"RiotStory/Variant_Shooter/UI",
			"RiotStory/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
