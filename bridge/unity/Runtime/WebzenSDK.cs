using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine;

namespace Webzen
{
    // JsonUtility has no [JsonProperty]-style renaming, so field names here
    // are the literal snake_case JSON keys the native side (core/request.hpp,
    // WEBZEN_JSON) expects -- deliberately not idiomatic C# naming, to avoid
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

    // The four fields every command's result starts with (core/request.hpp's
    // webzen::Result). A command with nothing more to report uses this
    // directly; one that returns more (e.g. auth.login) has its own
    // ResultXXX with these same four fields plus its own -- see ResultLogin.
    [Serializable]
    public class Result
    {
        public string request_id;
        public bool success;
        public string error_code;
        public string error_message;
    }

    [Serializable]
    public sealed class ResultLogin : Result
    {
        public string user_id;
        public string access_token;
        public string refresh_token;
        public long expires_at;
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
        // per-call routing (and, since each pending call may expect a
        // different TResult, the actual JSON -> TResult parsing) on the
        // managed side instead.
        private static readonly object Gate = new();
        private static readonly Dictionary<string, Action<string>> PendingCallbacks = new();
        private static readonly ResultCallback Trampoline = OnResult;

        // TResult : Result, so every command's result can be deserialized
        // through this one method -- pass Result itself for a command with
        // nothing more to report (e.g. RequestInitialize), or a richer type
        // like ResultLogin for one that returns more.
        public static void Dispatch<TResult>(Request request, Action<TResult> callback) where TResult : Result
        {
            lock (Gate) {
                PendingCallbacks[request.request_id] = json => callback(JsonUtility.FromJson<TResult>(json));
            }
            Webzen_DispatchCommand(request.CommandId, JsonUtility.ToJson(request), Trampoline);
        }

        public static void Shutdown() => Webzen_Shutdown();

        [MonoPInvokeCallback(typeof(ResultCallback))]
        private static void OnResult(string resultJson)
        {
            // Parsed only for request_id here -- JsonUtility silently
            // ignores JSON fields a type doesn't declare, so reading the
            // base Result shape out of a richer ResultXXX payload is safe.
            Result envelope;
            try {
                envelope = JsonUtility.FromJson<Result>(resultJson);
            } catch (Exception) {
                return;
            }

            if (envelope == null || string.IsNullOrEmpty(envelope.request_id)) {
                return;
            }

            Action<string> handler;
            lock (Gate) {
                if (!PendingCallbacks.TryGetValue(envelope.request_id, out handler)) {
                    return;
                }
                PendingCallbacks.Remove(envelope.request_id);
            }

            // Fires on the SDK's own worker thread, not Unity's main thread;
            // the caller is responsible for hopping back to the main thread
            // before touching Unity APIs.
            handler?.Invoke(resultJson);
        }
    }
}
