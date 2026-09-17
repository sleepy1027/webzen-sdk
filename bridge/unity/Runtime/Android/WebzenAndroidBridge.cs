#if UNITY_ANDROID
using UnityEngine;

namespace Webzen
{
    /// <summary>
    /// Calls the Kotlin com.webzen.sdk.WebzenSDK facade (see
    /// platform/android/aar) via Unity's AndroidJavaObject reflection
    /// bridge. WebzenSDK.aar is dropped into Plugins/Android by
    /// scripts/build_android.sh and Unity packages it into the APK/AAB
    /// automatically -- nothing else to configure for Android.
    /// </summary>
    internal sealed class WebzenAndroidBridge : IWebzenSdkBridge
    {
        private readonly AndroidJavaClass _sdkClass = new AndroidJavaClass("com.webzen.sdk.WebzenSDK");

        public void Initialize(string baseUrl) => _sdkClass.CallStatic("initialize", baseUrl);

        public void Login(string providerId, string providerToken, WebzenSDK.LoginCallback callback)
        {
            _sdkClass.CallStatic("login", providerId, providerToken, new LoginCallbackProxy(callback));
        }

        // Implements the Kotlin fun interface WebzenSDK.LoginCallback so the
        // JVM can call back into managed code across the JNI boundary.
        private sealed class LoginCallbackProxy : AndroidJavaProxy
        {
            private readonly WebzenSDK.LoginCallback _callback;

            public LoginCallbackProxy(WebzenSDK.LoginCallback callback) : base("com.webzen.sdk.WebzenSDK$LoginCallback")
            {
                _callback = callback;
            }

            // Signature/name must match WebzenSDK.LoginCallback.onResult in Kotlin exactly.
            public void onResult(bool success, string userId, string accessToken, string errorMessage)
            {
                _callback?.Invoke(success, userId, accessToken, errorMessage);
            }
        }
    }
}
#endif
