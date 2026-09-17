#pragma once

#include <asio/any_io_executor.hpp>
#include <asio/awaitable.hpp>
#include <asio/thread_pool.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace webzen {

struct HttpRequest {
    std::string Method = "GET";
    std::string Url;
    std::vector<std::pair<std::string, std::string>> Headers;
    std::string Body;
    std::chrono::milliseconds Timeout{10000};
};

struct HttpResponse {
    long StatusCode = 0;
    std::string Body;

    [[nodiscard]] bool Ok() const { return StatusCode >= 200 && StatusCode < 300; }
};

enum class HttpError {
    None,
    Timeout,
    NetworkError,
};

struct HttpResult {
    HttpResponse Response;
    HttpError Error = HttpError::None;

    [[nodiscard]] bool Ok() const { return Error == HttpError::None && Response.Ok(); }
};

struct RetryPolicy {
    int MaxAttempts = 3;
    std::chrono::milliseconds BaseBackoff{200};
};

// Owns the retry/timeout/error-mapping policy that used to be duplicated in
// each OS library. libcurl does the actual transfer; see http_client.cpp for
// why that call happens on a worker thread rather than through curl_multi.
class HttpClient {
public:
    explicit HttpClient(asio::any_io_executor executor, RetryPolicy retryPolicy = {});

    asio::awaitable<HttpResult> Send(HttpRequest request);

private:
    asio::awaitable<HttpResult> SendOnce(const HttpRequest& request);

    asio::any_io_executor executor_;
    asio::thread_pool workerPool_;
    RetryPolicy retryPolicy_;
};

}  // namespace webzen
