// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class PretendEmpyres : ModuleRules
{
	public PretendEmpyres(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" , "XmlParser"});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

      PublicIncludePaths.Add("PretendEmpyres/Public");
      PrivateIncludePaths.Add("PretendEmpyres/Private");

      // Uncomment if you are using Slate UI
      // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

      // Uncomment if you are using online features
      // PrivateDependencyModuleNames.Add("OnlineSubsystem");

      // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
   }
}
