// Native implementation using yhirose/cpp-httplib (single-header).
// Requires the cpp-httplib header to be in your include path:
//   https://github.com/yhirose/cpp-httplib (put httplib.h somewhere accessible)
//
// If you want HTTPS support, define CPPHTTPLIB_OPENSSL_SUPPORT (and link OpenSSL libs).

#include "http_client.h"

#ifndef __EMSCRIPTEN__


#include <atomic>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <httplib.h> 

namespace http_client {
	namespace detail {

		struct RequestState {
			Request req;
			Response resp;
			std::atomic<bool> pending{ true };
			std::atomic<bool> cancelled{ false };
			// thread will keep a shared_ptr to this state while running
			std::shared_ptr<std::thread> worker;
		};

		static std::mutex g_mutex;
		static std::unordered_map<RequestId, std::shared_ptr<RequestState>> g_requests;
		static std::atomic<RequestId> g_next_id{ 1 };

		static std::string method_to_string(Method m) {
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

		static bool parse_url(const std::string& url, bool& is_https, std::string& host, int& port, std::string& path) {
			// minimal URL parser: scheme://host[:port][/path...]
			auto pos = url.find("://");
			if (pos == std::string::npos) return false;
			std::string scheme = url.substr(0, pos);
			is_https = (scheme == "https");
			size_t host_start = pos + 3;
			size_t path_start = url.find('/', host_start);
			std::string hostport;
			if (path_start == std::string::npos) {
				hostport = url.substr(host_start);
				path = "/";
			}
			else {
				hostport = url.substr(host_start, path_start - host_start);
				path = url.substr(path_start);
			}
			// optional port
			auto colon = hostport.find(':');
			if (colon == std::string::npos) {
				host = hostport;
				port = is_https ? 443 : 80;
			}
			else {
				host = hostport.substr(0, colon);
				std::string portstr = hostport.substr(colon + 1);
				try { port = std::stoi(portstr); }
				catch (...) { return false; }
			}
			return !host.empty();
		}

		static void perform_request_impl(RequestId id, std::shared_ptr<RequestState> st) {
			if (!st) return;
			if (st->cancelled.load()) {
				st->resp.status = Status::Cancelled;
				st->pending = false;
				return;
			}

			bool is_https = false;
			std::string host;
			int port = 0;
			std::string path;
			if (!parse_url(st->req.url, is_https, host, port, path)) {
				st->resp.status = Status::Error;
				st->resp.error = "Malformed URL: " + st->req.url;
				st->pending = false;
				return;
			}

			try {
				// Create client (SSL if necessary and supported)
				std::unique_ptr<httplib::Client> client;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
				if (is_https) {
					client = std::make_unique<httplib::SSLClient>(host.c_str(), port);
				}
				else {
					client = std::make_unique<httplib::Client>(host.c_str(), port);
				}
#else
				if (is_https) {
					st->resp.status = Status::Error;
					st->resp.error = "HTTPS requested but cpp-httplib built without OpenSSL support.";
					st->pending = false;
					return;
				}
				else {
					client = std::make_unique<httplib::Client>(host.c_str(), port);
				}
#endif

				// set optional timeouts (seconds, microseconds)
				if (st->req.timeout_ms > 0) {
					int secs = st->req.timeout_ms / 1000;
					int usecs = (st->req.timeout_ms % 1000) * 1000;
					client->set_read_timeout(secs, usecs);
					client->set_write_timeout(secs, usecs);
				}

				// Build headers
				httplib::Headers headers;
				for (auto const& kv : st->req.headers) {
					headers.insert(kv);
				}

				httplib::Result res;

				// perform request according to method
				switch (st->req.method) {
				case Method::GET:
					res = client->Get(path.c_str(), headers);
					break;
				case Method::HEAD:
					res = client->Head(path.c_str(), headers);
					break;
				case Method::OPTIONS:
					res = client->Options(path.c_str(), headers);
					break;
				case Method::POST: {
					const char* content_type = "application/octet-stream";
					auto it = st->req.headers.find("Content-Type");
					if (it != st->req.headers.end()) content_type = it->second.c_str();
					res = client->Post(path.c_str(), headers, st->req.body, content_type);
					break;
				}
				case Method::PUT: {
					const char* content_type = "application/octet-stream";
					auto it = st->req.headers.find("Content-Type");
					if (it != st->req.headers.end()) content_type = it->second.c_str();
					res = client->Put(path.c_str(), headers, st->req.body, content_type);
					break;
				}
				case Method::PATCH: {
					const char* content_type = "application/octet-stream";
					auto it = st->req.headers.find("Content-Type");
					if (it != st->req.headers.end()) content_type = it->second.c_str();
					res = client->Patch(path.c_str(), headers, st->req.body, content_type);
					break;
				}
				case Method::DELETE_: {
					// cpp-httplib supports Delete(path, headers) and also Delete(path, headers, body, content_type)
					if (!st->req.body.empty()) {
						const char* content_type = "application/octet-stream";
						auto it = st->req.headers.find("Content-Type");
						if (it != st->req.headers.end()) content_type = it->second.c_str();
						res = client->Delete(path.c_str(), headers, st->req.body, content_type);
					}
					else {
						res = client->Delete(path.c_str(), headers);
					}
					break;
				}
				}

				if (st->cancelled.load()) {
					st->resp.status = Status::Cancelled;
					st->pending = false;
					return;
				}

				if (!res) {
					st->resp.status = Status::Error;
					st->resp.error = "No response (connection failure or timeout)";
				}
				else {
					st->resp.status_code = res->status;
					st->resp.body = res->body;
					// convert headers (std::multimap) into unordered_map keeping last value for each header
					for (auto const& kv : res->headers) {
						st->resp.headers[kv.first] = kv.second;
					}
					st->resp.status = Status::Success;
				}
			}
			catch (const std::exception& ex) {
				st->resp.status = Status::Error;
				st->resp.error = std::string("Exception: ") + ex.what();
			}
			catch (...) {
				st->resp.status = Status::Error;
				st->resp.error = "Unknown exception";
			}

			st->pending = false;
		}

	} // namespace detail

	// public API

	void init() {
		// nothing necessary for native implementation currently
	}

	void shutdown() {
		std::lock_guard<std::mutex> lock(detail::g_mutex);
		// mark all pending as cancelled; threads will finish in background
		for (auto& kv : detail::g_requests) {
			kv.second->cancelled = true;
		}
		// we don't join threads to avoid blocking shutdown; the OS will clean up after process exit.
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

		// spawn a worker thread (native can use threads)
		auto thr = std::make_shared<std::thread>(
			[id, st]() {
				detail::perform_request_impl(id, st);
			}
		);
		// detach so we don't have to manage join; RequestState holds thread pointer only to keep it alive if needed
		thr->detach();
		st->worker = thr;

		return id;
	}

	bool is_done(RequestId id) {
		std::lock_guard<std::mutex> lock(detail::g_mutex);
		auto it = detail::g_requests.find(id);
		if (it == detail::g_requests.end()) return true; // unknown => treat as done/invalid
		return !(it->second->pending.load());
	}

	bool get_response(RequestId id, Response& out, bool consume) {
		std::shared_ptr<detail::RequestState> st;
		{
			std::lock_guard<std::mutex> lock(detail::g_mutex);
			auto it = detail::g_requests.find(id);
			if (it == detail::g_requests.end()) return false;
			st = it->second;
			if (st->pending.load()) return false; // still pending
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
		// we cannot forcefully interrupt httplib blocking IO easily; marking cancelled will cause it
		// to return Cancelled when possible. The thread may still continue until network I/O completes.
	}

} // namespace http_client

#endif // !__EMSCRIPTEN__