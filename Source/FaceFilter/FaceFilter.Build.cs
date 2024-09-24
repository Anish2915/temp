// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FaceFilter : ModuleRules
{
	public FaceFilter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" , "WebSockets", "ImageWrapper", "Networking" , "Sockets" });
	}
}
