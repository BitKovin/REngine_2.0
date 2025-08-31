//requires -sFETCH

#ifdef __EMSCRIPTEN__

#include "http_client.h"

#include <emscripten/fetch.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace http_client {
    namespace detail {

        struct RequestState {
            Request req;
            Response resp;
            std::atomic<bool> pending{ true };
            std::atomic<bool> cancelled{ false };

            // Keep header strings and pointers alive here so emscripten_fetch can safely read them
            std::vector<std::string> header_strings; // actual storage for header key/value strings
            std::vector<const char*> header_ptrs;    // pointers into header_strings, null-terminated array

            emscripten_fetch_t* fetch_ptr = nullptr;
        };

        static std::mutex g_mutex;
        static std::unordered_map<RequestId, std::shared_ptr<RequestState>> g_requests;
        static std::atomic<RequestId> g_next_id{ 1 };

        static const char* method_to_cstr(Method m) {
            switch (m) {
            case Method::GET: return "GET";
            case Method::POST: return "POST";
            case Method::PUT: return "PUT";
            case Method::DELETE_: return "DELETE";
            case Method::PATCH: return "PATCH";
            case Method::HEAD: return "HEAD";
            case Method::OPTIONS: return "OPTIONS";
            }
            return "GET";
        }

        static void onsuccess(emscripten_fetch_t* fetch) {
            RequestState* st = static_cast<RequestState*>(fetch->userData);
            if (!st) {
                emscripten_fetch_close(fetch);
                return;
            }
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                st->resp.status_code = fetch->status;
                st->resp.body.assign(fetch->data, fetch->numBytes);
                st->resp.status = Status::Success;
                st->pending = false;
                st->fetch_ptr = nullptr;
            }
            emscripten_fetch_close(fetch);
        }

        static void onerror(emscripten_fetch_t* fetch) {
            RequestState* st = static_cast<RequestState*>(fetch->userData);
            if (!st) {
                emscripten_fetch_close(fetch);
                return;
            }
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                st->resp.status = Status::Error;
                st->resp.error = "Fetch error (network or CORS or blocked)";
                st->pending = false;
                st->fetch_ptr = nullptr;
            }
            emscripten_fetch_close(fetch);
        }

    } // namespace detail

    // public API

    void init() {
        // nothing needed
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(detail::g_mutex);
        for (auto& kv : detail::g_requests) {
            kv.second->cancelled = true;
            if (kv.second->fetch_ptr) {
                emscripten_fetch_close(kv.second->fetch_ptr);
                kv.second->fetch_ptr = nullptr;
            }
        }
        detail::g_requests.clear();
    }

    RequestId start_request(const Request& req) {
        auto st = std::make_shared<detail::RequestState>();
        st->req = req;
        st->resp = Response{};
        st->resp.status = Status::Pending;

        RequestId id = detail::g_next_id.fetch_add(1);
        {
            std::lock_guard<std::mutex> lock(detail::g_mutex);
            detail::g_requests[id] = st;
        }

        // Build header storage in the RequestState so it lives until the fetch completes.
        if (!req.headers.empty()) {
            st->header_strings.reserve(req.headers.size() * 2);
            st->header_ptrs.reserve(req.headers.size() * 2 + 1);
            for (const auto& kv : req.headers) {
                // Emscripten expects pairs: {"HeaderName", "HeaderValue", ... , 0}
                st->header_strings.emplace_back(kv.first);
                st->header_ptrs.push_back(st->header_strings.back().c_str());
                st->header_strings.emplace_back(kv.second);
                st->header_ptrs.push_back(st->header_strings.back().c_str());
            }
            st->header_ptrs.push_back(nullptr); // null-terminate the array
        }

        // Setup fetch attributes
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, detail::method_to_cstr(req.method));
        attr.userData = st.get();
        attr.onsuccess = detail::onsuccess;
        attr.onerror = detail::onerror;
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY; // load to memory; choose flags as needed

        // Attach requestHeaders if any (pointer is into st->header_ptrs -> will remain valid)
        if (!st->header_ptrs.empty()) {
            attr.requestHeaders = st->header_ptrs.data();
        }
        else {
            attr.requestHeaders = nullptr;
        }

        // Request body — valid at least until emscripten_fetch returns. We keep req in state.
        if (!req.body.empty()) {
            attr.requestData = st->req.body.c_str();
            attr.requestDataSize = st->req.body.size();
        }
        else {
            attr.requestData = nullptr;
            attr.requestDataSize = 0;
        }

        emscripten_fetch_t* fetch = emscripten_fetch(&attr, req.url.c_str());
        st->fetch_ptr = fetch;

        return id;
    }

    bool is_done(RequestId id) {
        std::lock_guard<std::mutex> lock(detail::g_mutex);
        auto it = detail::g_requests.find(id);
        if (it == detail::g_requests.end()) return true;
        return !(it->second->pending.load());
    }

    bool get_response(RequestId id, Response& out, bool consume) {
        std::shared_ptr<detail::RequestState> st;
        {
            std::lock_guard<std::mutex> lock(detail::g_mutex);
            auto it = detail::g_requests.find(id);
            if (it == detail::g_requests.end()) return false;
            st = it->second;
            if (st->pending.load()) return false;
            out = st->resp;
            if (consume) detail::g_requests.erase(it);
        }
        return true;
    }

    void cancel_request(RequestId id) {
        std::lock_guard<std::mutex> lock(detail::g_mutex);
        auto it = detail::g_requests.find(id);
        if (it == detail::g_requests.end()) return;
        it->second->cancelled = true;
        if (it->second->fetch_ptr) {
            emscripten_fetch_close(it->second->fetch_ptr);
            it->second->fetch_ptr = nullptr;
            it->second->resp.status = Status::Cancelled;
            it->second->pending = false;
        }
    }

} // namespace http_client

#endif // __EMSCRIPTEN__
