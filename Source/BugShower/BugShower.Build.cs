// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BugShower : ModuleRules
{
	public BugShower(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public dependency modules
		// 퍼블릭 의존성 모듈들
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",    // Enhanced Input System - 향상된 입력 시스템
			"UMG",              // UI Widget system - UI 위젯 시스템
			"Slate",            // UI framework - UI 프레임워크
			"SlateCore",        // UI core - UI 코어
			"PhysicsCore",      // Physics system - 물리 시스템
			"AssetRegistry"     // Asset loading for ItemResourceManager - ItemResourceManager의 에셋 로딩용
		});

        // Additional include paths
        // 추가 인클루드 경로
        PublicIncludePaths.AddRange(new string[] { "BugShower" });

		// Physics includes
		// 물리 인클루드
        PublicIncludePaths.AddRange(new string[] { "PhysicsCore" });

		// UI includes (already in PublicDependencyModuleNames)
		// UI 인클루드 (이미 PublicDependencyModuleNames에 포함됨)
        //PublicIncludePaths.AddRange(new string[] { "UMG", "Slate", "SlateCore" });

        PrivateDependencyModuleNames.AddRange(new string[] { });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
