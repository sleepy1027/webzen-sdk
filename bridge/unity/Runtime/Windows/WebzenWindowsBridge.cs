#if UNITY_STANDALONE_WIN || UNITY_EDITOR_WIN
using System;
using System.Runtime.InteropServices;
using AOT;

namespace Webzen
{
    /// <summary>
    /// Calls webzen/sdk_c_api.h in webzen_core.dll (dropped into
    /// Plugins/x86_64 by scripts/build_windows.ps1) via P/Invoke. Same C ABI
    /// as the Android/iOS bridges, just loaded as a plain native DLL instead
    /// of through JNI or a statically-linked framework.
    /// </summary>
    internal sealed class WebzenWindowsBridge : IWebzenSdkBridge
    {
        private const string DllName = "webzen_core";

        [DllImport(DllName)] private static extern void Webzen_Initialize(string baseUrl);
        [DllImport(DllName)] private static extern void Webzen_DispatchCommand(string commandId, string jsonPayload, CommandCallback callback, IntPtr userData);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void CommandCallback(int success, string jsonPayload, IntPtr userData);

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
