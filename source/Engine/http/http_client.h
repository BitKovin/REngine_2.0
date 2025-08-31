#pragma once
// http_client.h
// Cross-platform (httplib + emscripten) HTTP client API.
// One header, two platform-specific .cpp files:
//  - http_client_httplib.cpp   -> uses yhirose/cpp-httplib (httplib)
//  - http_client_emscripten.cpp -> uses emscripten_fetch (Emscripten)
//
// Usage model (frame-friendly):
//   auto id = http_client::start_request(req);
//   // each frame: if (http_client::is_done(id)) { http_client::Response r; http_client::get_response(id, r); ... }

#include <cstdint>
#include <string>
#include <unordered_map>

namespace http_client {

    using RequestId = uint64_t;
    constexpr RequestId INVALID_REQUEST = 0;

    enum class Method {
        GET,
        POST,
        PUT,
        DELETE_,
        PATCH,
        HEAD,
        OPTIONS
    };

    enum class Status {
        Pending,
        Success,
        Error,
        Cancelled,
        Invalid
    };

    struct Request {
        Method method = Method::GET;
        std::string url; // full URL including scheme, e.g. "https://example.com/path?x=1"
        std::unordered_map<std::string, std::string> headers;
        std::string body;    // optional request body (POST/PUT/PATCH/etc)
        int timeout_ms = 0;  // 0 -> library default / no explicit timeout
    };

    struct Response {
        Status status = Status::Invalid;
        int status_code = 0; // HTTP status code (200, 404, etc). 0 if unavailable.
        std::string body;
        std::unordered_map<std::string, std::string> headers; // best-effort; in Emscripten headers might be empty
        std::string error; // text in case of Error
    };

    // Initialize / shutdown resources. Optional, but helpful.
    void init();
    void shutdown();

    // start a request, returns a RequestId. Request starts immediately for both native and emscripten implementations.
    RequestId start_request(const Request& req);

    // Polling helpers:
    // is_done: returns true if finished (Success/Error/Cancelled). false => still pending.
    bool is_done(RequestId id);

    // get_response fills `out` with the response. Returns true when it successfully retrieved the response.
    // By default it consumes the stored result (removes internal state). Set consume=false to keep it.
    bool get_response(RequestId id, Response& out, bool consume = true);

    // Cancel a pending request. For native this attempts to mark as cancelled (thread may still complete).
    // For Emscripten it will attempt to abort the fetch. After cancellation is_done() will become true and
    // get_response() will return a Response with status == Cancelled (or Error).
    void cancel_request(RequestId id);

} // namespace http_client
