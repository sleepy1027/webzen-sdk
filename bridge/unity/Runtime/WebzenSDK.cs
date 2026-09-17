using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine;

namespace Webzen
{
    // JsonUtility has no [JsonProperty]-style renaming, so field names here
    // are the literal snake_case JSON keys the native side (core/request.hpp,
    // SDK_FIELD) expects -- deliberately not idiomatic C# naming, to avoid
    // pulling in Newtonsoft just for key renaming.
    [Serializable]
    public abstract class Request
    {
        public string request_id = Guid.NewGuid().ToString("N");

        // Not serialized (JsonUtility only serializes fields) -- read by
        // WebzenSDK.Dispatch to pick the command id, not sent as JSON.
        internal abstract string CommandId { get; }
    }

    [Serializable]
    public sealed class RequestInitialize : Request
    {
        public string base_url;
        internal override string CommandId => "core.initialize";
    }

    [Serializable]
    public sealed class RequestLogin : Request
    {
        public string provider_id;
        public string provider_token;
        public string device_id = "";
        internal override string CommandId => "auth.login";
    }

    // The one shape every Dispatch callback receives, whatever the command
    // (core/request.hpp's webzen::Result, mirrored field for field). Parse
    // `data` yourself once you know which request you sent.
    [Serializable]
    public sealed class Result
    {
        public string request_id;
        public bool success;
        public string error_code;
        public string error_message;
        public string data;
    }

    /// <summary>
    /// The entire native-facing surface of the Webzen SDK Unity package: one
    /// Dispatch method calling straight into the native artifact dropped
    /// into Plugins/ by scripts/build_android.sh, build_ios.sh or
    /// build_windows.ps1. There is deliberately no per-OS bridge class --
    /// Android's libwebzen_core.so (bundled inside WebzenSDK.aar), iOS's
    /// statically-linked WebzenSDK.xcframework and Windows' webzen_core.dll
    /// all export the exact same C ABI (core/sdk_c_api.h), so the only
    /// platform difference is the DllImport library name below.
    /// </summary>
    public static class WebzenSDK
    {
#if UNITY_IOS && !UNITY_EDITOR
        private const string LibName = "__Internal";
#else
        private const string LibName = "webzen_core";
#endif

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ResultCallback(string resultJson);

        [DllImport(LibName)]
        private static extern void Webzen_DispatchCommand(string commandId, string requestJson, ResultCallback callback);

        [DllImport(LibName)]
        private static extern void Webzen_Shutdown();

        // Correlates an async result back to its caller via Result.request_id
        // (echoed from Request.request_id by the native side) instead of a
        // per-call native user_data -- see core/request.hpp. One static
        // trampoline is registered for every call; the dictionary does the
        // per-call routing on the managed side instead.
        private static readonly object Gate = new();
        private static readonly Dictionary<string, Action<Result>> PendingCallbacks = new();
        private static readonly ResultCallback Trampoline = OnResult;

        public static void Dispatch(Request request, Action<Result> callback)
        {
            lock (Gate) {
                PendingCallbacks[request.request_id] = callback;
            }
            Webzen_DispatchCommand(request.CommandId, JsonUtility.ToJson(request), Trampoline);
        }

        public static void Shutdown() => Webzen_Shutdown();

        [MonoPInvokeCallback(typeof(ResultCallback))]
        private static void OnResult(string resultJson)
        {
            Result result;
            try {
                result = JsonUtility.FromJson<Result>(resultJson);
            } catch (Exception) {
                return;
            }

            if (result == null || string.IsNullOrEmpty(result.request_id)) {
                return;
            }

            Action<Result> callback;
            lock (Gate) {
                if (!PendingCallbacks.TryGetValue(result.request_id, out callback)) {
                    return;
                }
                PendingCallbacks.Remove(result.request_id);
            }

            // Fires on the SDK's own worker thread, not Unity's main thread;
            // the caller is responsible for hopping back to the main thread
            // before touching Unity APIs.
            callback?.Invoke(result);
        }
    }
}
