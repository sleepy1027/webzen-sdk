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
 */
class WebzenContextProvider : ContentProvider() {
    override fun onCreate(): Boolean {
        context?.applicationContext?.let { appContext = it }
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
