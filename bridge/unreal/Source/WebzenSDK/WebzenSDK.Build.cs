using System.IO;
using UnrealBuildTool;

// Links the prebuilt native core (see ThirdParty/WebzenCore, staged by
// scripts/build_android.sh / build_ios.sh / build_windows.ps1) into the
// game. This module's own C++ is a thin bridge -- see
// WebzenSDKSubsystem.cpp -- exactly like the Unity package's Runtime/*.
public class WebzenSDK : ModuleRules
{
	public WebzenSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "Json" });

		string ThirdPartyDir = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "WebzenCore");
		PublicIncludePaths.Add(Path.Combine(ThirdPartyDir, "include"));

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.Add("Launch"); // FJavaWrapper / AndroidJNI live here

			// Embeds WebzenSDK.aar (and applies its own AndroidManifest.xml
			// merge, permissions, etc.) into the packaged APK/AAB.
			string UplPath = Path.Combine(ModuleDirectory, "..", "..", "WebzenSDK_UPL_Android.xml");
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", UplPath);
		}
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			string XcframeworkPath = Path.Combine(ThirdPartyDir, "IOS", "WebzenSDK.xcframework");
			PublicAdditionalFrameworks.Add(new Framework("WebzenSDK", XcframeworkPath));

			// REGISTER_COMMAND commands aren't referenced directly by any
			// symbol the app calls (CommandRegistry finds them by string at
			// runtime), so Xcode's linker drops them unless force-loaded --
			// same requirement as the Unity bridge's iOS postprocessor.
			string UplPath = Path.Combine(ModuleDirectory, "..", "..", "WebzenSDK_UPL_IOS.xml");
			AdditionalPropertiesForReceipt.Add("IOSPlugin", UplPath);
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string Win64Dir = Path.Combine(ThirdPartyDir, "Win64");
			PublicAdditionalLibraries.Add(Path.Combine(Win64Dir, "webzen_core.lib"));

			string Dll = Path.Combine(Win64Dir, "webzen_core.dll");
			RuntimeDependencies.Add(Dll);
			PublicDelayLoadDLLs.Add("webzen_core.dll");
		}
	}
}
