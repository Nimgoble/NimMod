// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NimMod : ModuleRules
{
	public NimMod(TargetInfo Target)
	{

		PublicDependencyModuleNames.AddRange
        (
            new string[] 
            { 
                "Core",
				"CoreUObject",
				"Engine",
                "Networking",
                "Sockets",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"AssetRegistry",
                "AIModule",
                "InputCore",
                "UMG",
                "HTTP",
                "Json",
                "VaRestPlugin"
            }
        );

        PrivateDependencyModuleNames.AddRange
        (
            new string[] 
            {
                "CoreUObject",
				"InputCore",
                "Networking",
                "Sockets",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"Slate",
				"SlateCore",
                "GameplayDebugger",
                "HTTP",
                "Json",
                "VaRestPlugin"/*,
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
