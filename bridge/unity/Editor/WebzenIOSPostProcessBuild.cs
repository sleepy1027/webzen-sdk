#if UNITY_IOS
using UnityEditor;
using UnityEditor.Callbacks;
using UnityEditor.iOS.Xcode;

namespace Webzen.Editor
{
    /// <summary>
    /// WebzenSDK.xcframework (see Plugins/iOS, staged by scripts/build_ios.sh)
    /// ships REGISTER_COMMAND-registered commands that nothing in the app
    /// calls directly -- Xcode's linker drops them as "unreferenced" unless
    /// the app target links with -force_load. Doing this by hand in every
    /// Xcode project a game generates doesn't scale, so it's applied here on
    /// every build instead.
    /// </summary>
    public static class WebzenIOSPostProcessBuild
    {
        [PostProcessBuild(1)]
        public static void OnPostProcessBuild(BuildTarget target, string pathToBuiltProject)
        {
            if (target != BuildTarget.iOS) return;

            var projectPath = PBXProject.GetPBXProjectPath(pathToBuiltProject);
            var project = new PBXProject();
            project.ReadFromFile(projectPath);

            var mainTargetGuid = project.GetUnityMainTargetGuid();
            // $(PROJECT_DIR) is wherever Unity's Xcode export placed
            // Frameworks/WebzenSDK.xcframework/<slice>/libwebzen_core.a;
            // -force_load needs the concrete slice, and Xcode resolves that
            // via the framework search path it already set up for us.
            project.AddBuildProperty(mainTargetGuid, "OTHER_LDFLAGS", "-force_load $(BUILT_PRODUCTS_DIR)/WebzenSDK.framework/WebzenSDK");

            project.WriteToFile(projectPath);
        }
    }
}
#endif
