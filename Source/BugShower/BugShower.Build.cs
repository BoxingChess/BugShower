// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BugShower : ModuleRules
{
    public BugShower(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG", "Slate", "SlateCore", "AIModule", "NavigationSystem" });

        // 추가로 필요한 경로
        PublicIncludePaths.AddRange(new string[] { "BugShower" });

        //물리 관련
        PublicIncludePaths.AddRange(new string[] { "PhysicsCore" });

        //UI 관련
        //PublicIncludePaths.AddRange(new string[] { "UMG", "Slate", "SlateCore" });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
    
}
