#include "webzen/net/http_client.hpp"

#include <asio/post.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>

#include <curl/curl.h>

#include <cstddef>

namespace webzen::net {

namespace {

std::size_t WriteCallback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata) {
    auto* body = static_cast<std::string*>(userdata);
    body->append(ptr, size * nmemb);
    return size * nmemb;
}

// The actual (blocking) transfer. Runs on HttpClient::workerPool_, never on
// the caller's executor -- see the comment on RunOnWorker below.
HttpResult PerformBlocking(const HttpRequest& request) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return HttpResult{.Error = HttpError::NetworkError};
    }

    std::string responseBody;
    struct curl_slist* headers = nullptr;
    for (const auto& [name, value] : request.Headers) {
        headers = curl_slist_append(headers, (name + ": " + value).c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, request.Url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.Method.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, static_cast<long>(request.Timeout.count()));
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    if (!request.Body.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.Body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(request.Body.size()));
    }

    const CURLcode code = curl_easy_perform(curl);

    HttpResult result;
    if (code == CURLE_OK) {
        long statusCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
        result.Response.StatusCode = statusCode;
        result.Response.Body = std::move(responseBody);
        result.Error = HttpError::None;
    } else if (code == CURLE_OPERATION_TIMEDOUT) {
        result.Error = HttpError::Timeout;
    } else {
        result.Error = HttpError::NetworkError;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return result;
}

// libcurl's easy API blocks the calling thread for the whole transfer.
// Integrating curl_multi with asio's reactor would avoid the extra thread
// hop, but it roughly doubles the code for this scaffold's benefit; offload
// to a small worker pool and resume the coroutine on `executor` instead.
asio::awaitable<HttpResult> RunOnWorker(asio::any_io_executor executor, asio::thread_pool& pool, HttpRequest request) {
    co_return co_await asio::async_initiate<decltype(asio::use_awaitable), void(HttpResult)>(
        [executor, &pool, request = std::move(request)](auto handler) mutable {
            using Handler = decltype(handler);
            asio::post(pool, [executor, request = std::move(request), handler = std::forward<Handler>(handler)]() mutable {
                HttpResult result = PerformBlocking(request);
                asio::post(executor, [handler = std::move(handler), result = std::move(result)]() mutable {
                    handler(std::move(result));
                });
            });
        },
        asio::use_awaitable);
}

}  // namespace

HttpClient::HttpClient(asio::any_io_executor executor, RetryPolicy retryPolicy)
    : executor_(std::move(executor)), workerPool_(2), retryPolicy_(retryPolicy) {}

asio::awaitable<HttpResult> HttpClient::SendOnce(const HttpRequest& request) {
    co_return co_await RunOnWorker(executor_, workerPool_, request);
}

asio::awaitable<HttpResult> HttpClient::Send(HttpRequest request) {
    HttpResult lastResult;
    for (int attempt = 1; attempt <= retryPolicy_.MaxAttempts; ++attempt) {
        lastResult = co_await SendOnce(request);

        const bool retryable = lastResult.Error != HttpError::None || lastResult.Response.StatusCode >= 500;
        if (!retryable || attempt == retryPolicy_.MaxAttempts) {
            co_return lastResult;
        }

        asio::steady_timer timer(executor_, retryPolicy_.BaseBackoff * attempt);
        co_await timer.async_wait(asio::use_awaitable);
    }
    co_return lastResult;
}

}  // namespace webzen::net
