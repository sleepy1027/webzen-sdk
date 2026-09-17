#include "auth/auth_service.hpp"

#include "core/sdk.hpp"

#include <glaze/glaze.hpp>

namespace webzen::auth {

namespace {
constexpr std::string_view kTokenStorageKey = "webzen.auth.token";
}

AuthService::AuthService(HttpClient& httpClient, adapter::PlatformAdapter platform)
    : httpClient_(httpClient), platform_(platform) {
    currentToken_ = LoadPersistedToken();
}

asio::awaitable<AuthResult> AuthService::Login(RequestLogin request) {
    if (request.DeviceId.empty() && platform_.DeviceInfo) {
        request.DeviceId = platform_.DeviceInfo->DeviceId();
    }

    std::string body;
    if (glz::write_json(request, body)) {
        co_return AuthResult{.Success = false, .Error = AuthError::InvalidCredentials};
    }

    HttpRequest httpRequest{
        .Method = "POST",
        .Url = Sdk::Instance().BaseUrl() + "/v1/auth/login",
        .Headers = {{"Content-Type", "application/json"}},
        .Body = std::move(body),
    };

    const HttpResult httpResult = co_await httpClient_.Send(std::move(httpRequest));

    if (httpResult.Error != HttpError::None) {
        co_return AuthResult{.Success = false, .Error = AuthError::NetworkError};
    }
    if (httpResult.Response.StatusCode == 401 || httpResult.Response.StatusCode == 403) {
        co_return AuthResult{.Success = false, .Error = AuthError::InvalidCredentials};
    }
    if (!httpResult.Response.Ok()) {
        co_return AuthResult{.Success = false, .Error = AuthError::NetworkError};
    }

    AuthTokenDto token;
    if (glz::read_json(token, httpResult.Response.Body)) {
        co_return AuthResult{.Success = false, .Error = AuthError::NetworkError};
    }

    currentToken_ = token;
    PersistToken(token);
    co_return AuthResult{.Success = true, .Error = AuthError::None, .Token = token};
}

asio::awaitable<AuthResult> AuthService::RefreshSession() {
    const auto persisted = currentToken_ ? currentToken_ : LoadPersistedToken();
    if (!persisted || persisted->RefreshToken.empty()) {
        co_return AuthResult{.Success = false, .Error = AuthError::NotAuthenticated};
    }

    std::string body;
    if (glz::write_json(*persisted, body)) {
        co_return AuthResult{.Success = false, .Error = AuthError::NotAuthenticated};
    }

    HttpRequest httpRequest{
        .Method = "POST",
        .Url = Sdk::Instance().BaseUrl() + "/v1/auth/refresh",
        .Headers = {{"Content-Type", "application/json"}},
        .Body = std::move(body),
    };

    const HttpResult httpResult = co_await httpClient_.Send(std::move(httpRequest));
    if (httpResult.Error != HttpError::None || !httpResult.Response.Ok()) {
        co_return AuthResult{.Success = false, .Error = AuthError::NetworkError};
    }

    AuthTokenDto token;
    if (glz::read_json(token, httpResult.Response.Body)) {
        co_return AuthResult{.Success = false, .Error = AuthError::NetworkError};
    }

    currentToken_ = token;
    PersistToken(token);
    co_return AuthResult{.Success = true, .Error = AuthError::None, .Token = token};
}

asio::awaitable<void> AuthService::Logout() {
    currentToken_.reset();
    if (platform_.SecureStorage) {
        platform_.SecureStorage->Remove(kTokenStorageKey);
    }
    co_return;
}

std::optional<AuthTokenDto> AuthService::CurrentToken() const {
    return currentToken_;
}

void AuthService::PersistToken(const AuthTokenDto& token) {
    if (!platform_.SecureStorage) {
        return;
    }
    std::string json;
    if (!glz::write_json(token, json)) {
        platform_.SecureStorage->Set(kTokenStorageKey, json);
    }
}

std::optional<AuthTokenDto> AuthService::LoadPersistedToken() const {
    if (!platform_.SecureStorage) {
        return std::nullopt;
    }
    const auto stored = platform_.SecureStorage->Get(kTokenStorageKey);
    if (!stored) {
        return std::nullopt;
    }
    AuthTokenDto token;
    if (glz::read_json(token, *stored)) {
        return std::nullopt;
    }
    return token;
}

AuthService& GetAuthService() {
    static AuthService instance(Sdk::Instance().Http(), Sdk::Instance().Platform());
    return instance;
}

}  // namespace webzen::auth
