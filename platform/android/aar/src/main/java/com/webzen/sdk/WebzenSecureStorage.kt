package com.webzen.sdk

import androidx.security.crypto.EncryptedSharedPreferences
import androidx.security.crypto.MasterKey

/**
 * Backing store for AndroidSecureStorage (platform/android/adapter), called
 * over JNI from android_adapter.cpp. Keystore-backed via
 * EncryptedSharedPreferences, which has no C++ equivalent -- this is why
 * secure storage is an OS adapter instead of common-core logic.
 */
internal object WebzenSecureStorage {
    private const val PREFS_NAME = "webzen_sdk_secure_prefs"

    private val prefs by lazy {
        val masterKey = MasterKey.Builder(WebzenContextProvider.appContext)
            .setKeyScheme(MasterKey.KeyScheme.AES256_GCM)
            .build()

        EncryptedSharedPreferences.create(
            WebzenContextProvider.appContext,
            PREFS_NAME,
            masterKey,
            EncryptedSharedPreferences.PrefKeyEncryptionScheme.AES256_SIV,
            EncryptedSharedPreferences.PrefValueEncryptionScheme.AES256_GCM
        )
    }

    @JvmStatic
    fun set(key: String, value: String): Boolean = prefs.edit().putString(key, value).commit()

    @JvmStatic
    fun get(key: String): String? = prefs.getString(key, null)

    @JvmStatic
    fun remove(key: String) {
        prefs.edit().remove(key).apply()
    }
}
