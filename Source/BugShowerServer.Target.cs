// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class BugShowerServerTarget : TargetRules
{
    public BugShowerServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.Add("BugShower");

        CppStandard = CppStandardVersion.Cpp20;
    }
}