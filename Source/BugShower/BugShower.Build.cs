// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BugShower : ModuleRules
{

	public BugShower(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public dependency modules
		// ?¼ë¸”ë¦??˜ì¡´??ëª¨ë“ˆ??
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",    // Enhanced Input System - ?¥ìƒ???…ë ¥ ?œìŠ¤??
			"UMG",              // UI Widget system - UI ?„ì ¯ ?œìŠ¤??
			"Slate",            // UI framework - UI ?„ë ˆ?„ì›Œ??
			"SlateCore",        // UI core - UI ì½”ì–´
			"PhysicsCore",      // Physics system - ë¬¼ë¦¬ ?œìŠ¤??
			"AssetRegistry",     // Asset loading for ItemResourceManager - ItemResourceManager???ì…‹ ë¡œë”©??
            "AIModule", "NavigationSystem"
        });

        // Additional include paths
        // ì¶”ê? ?¸í´ë£¨ë“œ ê²½ë¡œ
        PublicIncludePaths.AddRange(new string[] { "BugShower" });

        //¹°¸® °ü·Ã
        PublicIncludePaths.AddRange(new string[] { "PhysicsCore" });


        PrivateDependencyModuleNames.AddRange(new string[] { });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
    
}
