package com.webzen.sdk

import android.annotation.SuppressLint
import android.os.Build
import android.provider.Settings

/** Backing calls for AndroidDeviceInfo (platform/android/adapter), called over JNI. */
internal object WebzenDeviceInfo {
    @SuppressLint("HardwareIds")
    @JvmStatic
    fun deviceId(): String =
        Settings.Secure.getString(WebzenContextProvider.appContext.contentResolver, Settings.Secure.ANDROID_ID) ?: "unknown"

    @JvmStatic
    fun osVersion(): String = "Android ${Build.VERSION.RELEASE} (API ${Build.VERSION.SDK_INT})"

    @JvmStatic
    fun appVersion(): String {
        val context = WebzenContextProvider.appContext
        val packageInfo = context.packageManager.getPackageInfo(context.packageName, 0)
        return packageInfo.versionName ?: "unknown"
    }
}
