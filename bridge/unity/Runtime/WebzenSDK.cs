using System;

namespace Webzen
{
    /// <summary>
    /// Public API surface for the Webzen SDK Unity package. Everything below
    /// is routed to a per-platform bridge that talks to the native artifact
    /// dropped into Plugins/ by scripts/build_android.sh, build_ios.sh or
    /// build_windows.ps1 -- this class itself has no platform-specific code.
    /// </summary>
    public static class WebzenSDK
    {
        public delegate void LoginCallback(bool success, string userId, string accessToken, string errorMessage);

        private static readonly IWebzenSdkBridge Bridge = CreateBridge();

        public static void Initialize(string baseUrl) => Bridge.Initialize(baseUrl);

        public static void Login(string providerId, string providerToken, LoginCallback callback) =>
            Bridge.Login(providerId, providerToken, callback);

        private static IWebzenSdkBridge CreateBridge()
        {
#if UNITY_ANDROID && !UNITY_EDITOR
            return new WebzenAndroidBridge();
#elif UNITY_IOS && !UNITY_EDITOR
            return new WebzenIOSBridge();
#elif (UNITY_STANDALONE_WIN || UNITY_EDITOR_WIN)
            return new WebzenWindowsBridge();
#else
            return new WebzenUnsupportedBridge();
#endif
        }

        private sealed class WebzenUnsupportedBridge : IWebzenSdkBridge
        {
            private const string Message = "Webzen SDK has no native implementation for this platform/editor combination.";
            public void Initialize(string baseUrl) => throw new PlatformNotSupportedException(Message);
            public void Login(string providerId, string providerToken, LoginCallback callback) => throw new PlatformNotSupportedException(Message);
        }
    }
}
