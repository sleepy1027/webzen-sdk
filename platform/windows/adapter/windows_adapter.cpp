#include "platform/windows/adapter/windows_adapter.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <dpapi.h>
#include <shlobj.h>
#include <version.h>

#include <filesystem>
#include <fstream>
#include <vector>

#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Version.lib")

namespace webzen::platform::windows {

namespace {

std::filesystem::path StorageDirectory() {
    PWSTR localAppData = nullptr;
    std::filesystem::path dir;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &localAppData))) {
        dir = std::filesystem::path(localAppData) / L"WebzenSDK";
        CoTaskMemFree(localAppData);
    }
    std::filesystem::create_directories(dir);
    return dir;
}

std::filesystem::path KeyFilePath(std::string_view key) {
    return StorageDirectory() / (std::string(key) + ".bin");
}

}  // namespace

bool WindowsSecureStorage::Set(std::string_view key, std::string_view value) {
    DATA_BLOB input{};
    input.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(value.data()));
    input.cbData = static_cast<DWORD>(value.size());

    DATA_BLOB output{};
    if (!CryptProtectData(&input, L"webzen.sdk", nullptr, nullptr, nullptr, 0, &output)) {
        return false;
    }

    std::ofstream file(KeyFilePath(key), std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char*>(output.pbData), output.cbData);
    LocalFree(output.pbData);
    return file.good();
}

std::optional<std::string> WindowsSecureStorage::Get(std::string_view key) {
    std::ifstream file(KeyFilePath(key), std::ios::binary | std::ios::ate);
    if (!file) {
        return std::nullopt;
    }

    const auto size = static_cast<std::size_t>(file.tellg());
    file.seekg(0);
    std::string encrypted(size, '\0');
    file.read(encrypted.data(), static_cast<std::streamsize>(size));

    DATA_BLOB input{};
    input.pbData = reinterpret_cast<BYTE*>(encrypted.data());
    input.cbData = static_cast<DWORD>(encrypted.size());

    DATA_BLOB output{};
    if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr, 0, &output)) {
        return std::nullopt;
    }

    std::string result(reinterpret_cast<char*>(output.pbData), output.cbData);
    LocalFree(output.pbData);
    return result;
}

void WindowsSecureStorage::Remove(std::string_view key) {
    std::error_code ec;
    std::filesystem::remove(KeyFilePath(key), ec);
}

std::string WindowsDeviceInfo::DeviceId() const {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS) {
        return "unknown";
    }

    wchar_t buffer[64] = {};
    DWORD size = sizeof(buffer);
    const bool ok = RegQueryValueExW(key, L"MachineGuid", nullptr, nullptr, reinterpret_cast<BYTE*>(buffer), &size) == ERROR_SUCCESS;
    RegCloseKey(key);
    if (!ok) {
        return "unknown";
    }

    const int len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    std::string result(len > 0 ? len - 1 : 0, '\0');
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, result.data(), len, nullptr, nullptr);
    return result;
}

std::string WindowsDeviceInfo::OsVersion() const {
    OSVERSIONINFOEXW info{};
    info.dwOSVersionInfoSize = sizeof(info);
    // GetVersionEx is deprecated in favor of the version-helper API family
    // (VerifyVersionInfo), but those require a version to test *against*;
    // this just wants "whatever the running major/minor/build is" for a
    // diagnostics string, so plain GetVersionEx is enough here.
#pragma warning(push)
#pragma warning(disable : 4996)
    GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&info));
#pragma warning(pop)
    return "Windows " + std::to_string(info.dwMajorVersion) + "." + std::to_string(info.dwMinorVersion) +
           " (build " + std::to_string(info.dwBuildNumber) + ")";
}

std::string WindowsDeviceInfo::AppVersion() const {
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    DWORD handle = 0;
    const DWORD size = GetFileVersionInfoSizeW(exePath, &handle);
    if (size == 0) {
        return "unknown";
    }

    std::vector<BYTE> buffer(size);
    if (!GetFileVersionInfoW(exePath, handle, size, buffer.data())) {
        return "unknown";
    }

    VS_FIXEDFILEINFO* fileInfo = nullptr;
    UINT fileInfoLen = 0;
    if (!VerQueryValueW(buffer.data(), L"\\", reinterpret_cast<void**>(&fileInfo), &fileInfoLen) || !fileInfo) {
        return "unknown";
    }

    return std::to_string(HIWORD(fileInfo->dwFileVersionMS)) + "." + std::to_string(LOWORD(fileInfo->dwFileVersionMS)) +
           "." + std::to_string(HIWORD(fileInfo->dwFileVersionLS)) + "." + std::to_string(LOWORD(fileInfo->dwFileVersionLS));
}

}  // namespace webzen::platform::windows
