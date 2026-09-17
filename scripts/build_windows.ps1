# Builds webzen_core.dll/.lib and stages them for both engine bridges.
# Requires VCPKG_ROOT (provides libcurl for the x64-windows triplet) and a
# Visual Studio 2022 install with the "Desktop development with C++" workload.
$ErrorActionPreference = "Stop"

if (-not $env:VCPKG_ROOT) {
    throw "Set VCPKG_ROOT to your vcpkg checkout"
}

$RootDir = Split-Path -Parent $PSScriptRoot
Set-Location $RootDir

cmake --preset windows-x64
cmake --build --preset windows-x64 --target webzen_core

$BuildDir = Join-Path $RootDir "build\windows-x64"
$Dll = Get-ChildItem -Recurse -Path $BuildDir -Filter "webzen_core.dll" | Select-Object -First 1
$Lib = Get-ChildItem -Recurse -Path $BuildDir -Filter "webzen_core.lib" | Select-Object -First 1

if (-not $Dll -or -not $Lib) {
    throw "webzen_core.dll/.lib not found under $BuildDir -- did the build succeed?"
}

$UnityDest = Join-Path $RootDir "bridge\unity\Plugins\x86_64"
$UnrealDest = Join-Path $RootDir "bridge\unreal\ThirdParty\WebzenCore\Win64"
$UnrealIncludeDest = Join-Path $RootDir "bridge\unreal\ThirdParty\WebzenCore\include\webzen"
New-Item -ItemType Directory -Force -Path $UnityDest, $UnrealDest, $UnrealIncludeDest | Out-Null

Copy-Item $Dll.FullName -Destination $UnityDest -Force
Copy-Item $Dll.FullName -Destination $UnrealDest -Force
Copy-Item $Lib.FullName -Destination $UnrealDest -Force

# The plugin ships inside a separate Unreal project once copied there, so it
# can't reach back into this repo's core/include -- it needs its own copy of
# just the public C ABI headers.
Copy-Item (Join-Path $RootDir "core\include\webzen\sdk_c_api.h") -Destination $UnrealIncludeDest -Force
Copy-Item (Join-Path $RootDir "core\include\webzen\export.h") -Destination $UnrealIncludeDest -Force

Write-Host "Windows DLL/LIB staged:"
Write-Host "  $UnityDest\webzen_core.dll"
Write-Host "  $UnrealDest\webzen_core.dll (+ .lib, + include\webzen headers)"
