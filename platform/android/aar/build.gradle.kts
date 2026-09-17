plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.webzen.sdk"
    compileSdk = 34

    defaultConfig {
        minSdk = 24

        // These become the .so files Gradle packages at jni/<abi>/libwebzen_core.so
        // inside the .aar -- this ndk block is the only reason a consumer of the
        // .aar never has to touch a .so directly.
        ndk {
            abiFilters += listOf("arm64-v8a", "armeabi-v7a", "x86_64")
        }

        externalNativeBuild {
            cmake {
                // Builds the whole project (core + platform/android), but only
                // the `webzen_core` target's output is packaged into the .aar
                // (the `targets` filter below) -- the demo/test targets built
                // by the same CMake project never ship.
                targets += "webzen_core"
                arguments += listOf(
                    "-DWEBZEN_BUILD_TESTS=OFF",
                    "-DANDROID_STL=c++_shared"
                )
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../../../CMakeLists.txt")
            version = "3.24.0+"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }
}

dependencies {
    // Backs WebzenSecureStorage.kt only -- nothing else here uses AndroidX.
    implementation("androidx.security:security-crypto:1.1.0-alpha06")
}
