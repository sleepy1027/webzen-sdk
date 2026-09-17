package com.webzen.sdk

import android.content.ContentProvider
import android.content.ContentValues
import android.content.Context
import android.database.Cursor
import android.net.Uri

/**
 * Captures the application [Context] at process start without requiring the
 * host game (Unity/Unreal) to call an explicit "give me your Context" init
 * method -- ContentProviders are instantiated by the platform before any
 * Activity, which is the standard trick SDKs (e.g. Firebase, WorkManager)
 * use for this. Declared in AndroidManifest.xml; never referenced directly
 * by app code.
 *
 * Also loads libwebzen_core.so. Nothing else does: there is no public
 * Kotlin facade any more (see jni_bridge.cpp) for the engine bridges to
 * call, which previously triggered the load as a side effect of their own
 * `init` block. Unity's [DllImport] and Unreal's dlopen/dlsym both need the
 * library already resident in the process by the time they run, and both
 * can run before any Activity exists, so this -- which also runs before any
 * Activity -- is the one place left that's early enough.
 */
class WebzenContextProvider : ContentProvider() {
    override fun onCreate(): Boolean {
        context?.applicationContext?.let { appContext = it }
        System.loadLibrary("webzen_core")
        return true
    }

    override fun query(uri: Uri, projection: Array<String>?, selection: String?, selectionArgs: Array<String>?, sortOrder: String?): Cursor? = null
    override fun getType(uri: Uri): String? = null
    override fun insert(uri: Uri, values: ContentValues?): Uri? = null
    override fun delete(uri: Uri, selection: String?, selectionArgs: Array<String>?): Int = 0
    override fun update(uri: Uri, values: ContentValues?, selection: String?, selectionArgs: Array<String>?): Int = 0

    companion object {
        lateinit var appContext: Context
            private set
    }
}
