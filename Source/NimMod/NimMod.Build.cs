// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NimMod : ModuleRules
{
	public NimMod(TargetInfo Target)
	{
        //PrivateIncludePaths.AddRange(
        //    new string[] { 
        //        "ShooterGame/Classes/Player",
        //        "ShooterGame/Private",
        //        "ShooterGame/Private/UI",
        //        "ShooterGame/Private/UI/Menu",
        //        "ShooterGame/Private/UI/Style",
        //        "ShooterGame/Private/UI/Widgets",
        //    }
        //);

		PublicDependencyModuleNames.AddRange
        (
            new string[] 
            { 
                "Core",
				"CoreUObject",
				"Engine",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"AssetRegistry",
                "AIModule",
                "InputCore" 
            }
        );

        PrivateDependencyModuleNames.AddRange
        (
            new string[] 
            {
				"InputCore",
				"Slate",
				"SlateCore"/*,
				"ShooterGameLoadingScreen",*/
			}
        );

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");

        if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Linux) || (Target.Platform == UnrealTargetPlatform.Mac))
        {
            if (UEBuildConfiguration.bCompileSteamOSS == true)
            {
                DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
            }
        }
        //else if (Target.Platform == UnrealTargetPlatform.PS4)
        //{
        //    DynamicallyLoadedModuleNames.Add("OnlineSubsystemPS4");
        //}
        //else if (Target.Platform == UnrealTargetPlatform.XboxOne)
        //{
        //    DynamicallyLoadedModuleNames.Add("OnlineSubsystemLive");
        //}
	}
}
