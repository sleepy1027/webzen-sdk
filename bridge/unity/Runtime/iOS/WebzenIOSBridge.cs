#if UNITY_IOS
using System;
using System.Runtime.InteropServices;
using AOT;

namespace Webzen
{
    /// <summary>
    /// Calls webzen/sdk_c_api.h directly (the same flat C ABI the Android
    /// JNI bridge calls into) via [DllImport("__Internal")] -- on iOS,
    /// everything statically linked into the app (including
    /// WebzenSDK.xcframework, dropped into Plugins/iOS by
    /// scripts/build_ios.sh) is reachable that way. See docs/BUILDING.md for
    /// the required -force_load Xcode setting; Editor/WebzenIOSPostProcessBuild.cs
    /// sets it automatically on build.
    /// </summary>
    internal sealed class WebzenIOSBridge : IWebzenSdkBridge
    {
        [DllImport("__Internal")] private static extern void Webzen_Initialize(string baseUrl);
        [DllImport("__Internal")] private static extern void Webzen_DispatchCommand(string commandId, string jsonPayload, CommandCallback callback, IntPtr userData);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void CommandCallback(int success, string jsonPayload, IntPtr userData);

        // GCHandle keeps the managed callback alive across the native call;
        // freed once the native side invokes it exactly once.
        private static readonly System.Collections.Generic.Dictionary<IntPtr, WebzenSDK.LoginCallback> PendingLogins = new();

        public void Initialize(string baseUrl) => Webzen_Initialize(baseUrl);

        public void Login(string providerId, string providerToken, WebzenSDK.LoginCallback callback)
        {
            var json = $"{{\"provider_id\":\"{providerId}\",\"provider_token\":\"{providerToken}\",\"device_id\":\"\"}}";
            var handle = GCHandle.Alloc(callback);
            Webzen_DispatchCommand("auth.login", json, OnLoginResult, GCHandle.ToIntPtr(handle));
        }

        [MonoPInvokeCallback(typeof(CommandCallback))]
        private static void OnLoginResult(int success, string jsonPayload, IntPtr userData)
        {
            var handle = GCHandle.FromIntPtr(userData);
            var callback = (WebzenSDK.LoginCallback)handle.Target;
            handle.Free();

            // Minimal hand-rolled parse to avoid pulling in a JSON dependency
            // for two fields; swap for Unity's JsonUtility if this grows.
            bool ok = success != 0;
            callback?.Invoke(ok, ExtractField(jsonPayload, "user_id"), ExtractField(jsonPayload, "access_token"), ok ? null : ExtractField(jsonPayload, "error"));
        }

        private static string ExtractField(string json, string key)
        {
            var marker = $"\"{key}\":\"";
            var start = json.IndexOf(marker, StringComparison.Ordinal);
            if (start < 0) return null;
            start += marker.Length;
            var end = json.IndexOf('"', start);
            return end < 0 ? null : json.Substring(start, end - start);
        }
    }
}
#endif
