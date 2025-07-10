using System;
using System.IO;
using System.Net;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Maui.Storage;

namespace MauiPlatform
{
    public static class LocalWebServer
    {
        static HttpListener? _listener;
        static CancellationTokenSource? _cts;

        /// <summary>
        /// Starts the local HTTP server on the given port.
        /// </summary>
        public static async Task StartAsync(int port = 9696)
        {
            if (_listener != null) return; // already running

            _cts = new CancellationTokenSource();
            _listener = new HttpListener();
            _listener.Prefixes.Add($"http://*:{port}/");
            _listener.Start();


            // fire-and-forget the request loop
            _ = Task.Run(() => HandleRequestsAsync(_cts.Token), _cts.Token);

            await Task.CompletedTask;
        }

        /// <summary>
        /// Stops the server.
        /// </summary>
        public static void Stop()
        {
            _cts?.Cancel();
            try { _listener?.Stop(); }
            catch { /* swallow */ }
            _listener = null;
            _cts = null;
        }

        static async Task HandleRequestsAsync(CancellationToken token)
        {
            while (!token.IsCancellationRequested)
            {
                HttpListenerContext ctx;
                try
                {
                    ctx = await _listener!.GetContextAsync();
                }
                catch (Exception)
                {
                    break; // shutdown or error
                }

                _ = Task.Run(() => ProcessContextAsync(ctx), token);
            }
        }

        static async Task ProcessContextAsync(HttpListenerContext ctx)
        {
            var req = ctx.Request;
            var resp = ctx.Response;

            resp.Headers.Add("Cross-Origin-Opener-Policy", "same-origin");
            resp.Headers.Add("Cross-Origin-Embedder-Policy", "require-corp");
            resp.Headers.Add("Cache-Control", "no-store, must-revalidate");
            resp.Headers.Add("Permissions-Policy", "cross-origin-isolated=(*)");

            var urlPath = req.Url!.AbsolutePath.TrimStart('/');
            if (string.IsNullOrEmpty(urlPath))
                urlPath = "index.html";

            try
            {
                using var stream = await FileSystem.OpenAppPackageFileAsync(urlPath);
                resp.ContentType = GetContentType(urlPath);
                resp.SendChunked = true;                  // chunked → no Length needed
                await stream.CopyToAsync(resp.OutputStream);
                resp.StatusCode = (int)HttpStatusCode.OK;
            }
            catch (FileNotFoundException)
            {
                resp.StatusCode = (int)HttpStatusCode.NotFound;
                await using var w = new StreamWriter(resp.OutputStream);
                await w.WriteAsync("404 - not found");
            }
            catch (Exception ex)
            {
                resp.StatusCode = (int)HttpStatusCode.InternalServerError;
                await using var w = new StreamWriter(resp.OutputStream);
                await w.WriteAsync($"500 - {ex.Message}");
            }
            finally
            {
                resp.OutputStream.Close();
            }
        }


        static string GetContentType(string path)
        {
            var ext = Path.GetExtension(path).ToLowerInvariant();
            return ext switch
            {
                ".html" or ".htm" => "text/html",
                ".js" => "application/javascript",
                ".wasm" => "application/wasm",
                ".css" => "text/css",
                ".png" => "image/png",
                ".jpg" or ".jpeg" => "image/jpeg",
                ".svg" => "image/svg+xml",
                _ => "application/octet-stream",
            };
        }



    }
}
