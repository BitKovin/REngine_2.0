using EmbedIO;
using EmbedIO.Actions;
using EmbedIO.Sessions;
using EmbedIO.Files;
using Swan.Logging;
using System.IO;

public class WebServerService
{
    private WebServer _server;
    private readonly string[] _assetList;

    public WebServerService()
    {
        _assetList = typeof(WebServerService)
          .Assembly.GetManifestResourceNames()
          .Where(n => n.Contains("wwwroot."))
          .ToArray();

        _server = CreateServer();
        _server.StateChanged += (s, e) => $"Server state: {e.NewState}".Info();
        _server.RunAsync();
    }

    private WebServer CreateServer()
    {
        return new WebServer(o => o.WithUrlPrefix("http://127.0.0.1:9696/").WithMode(HttpListenerMode.EmbedIO))
          .WithLocalSessionManager()
          .WithModule(new ActionModule("/", HttpVerbs.Get, ctx => ServeAsset(ctx)))
          .WithModule(new ActionModule("/", HttpVerbs.Any, ctx => ctx.SendDataAsync(new { Message = "OK" })));
    }

    private async Task ServeAsset(IHttpContext ctx)
    {
        var path = ctx.RequestedPath.Trim('/');
        if (string.IsNullOrEmpty(path)) path = "index.html";

        // locate the embedded asset in Resources/Raw/wwwroot
        var resourceName = typeof(WebServerService)
          .Assembly
          .GetManifestResourceNames()
          .FirstOrDefault(n => n.EndsWith($"wwwroot.{path}", StringComparison.OrdinalIgnoreCase));

        if (resourceName is null)
        {
            ctx.Response.StatusCode = 404;
            await ctx.SendStringAsync("Not found", "text/plain", System.Text.Encoding.UTF8);
            return;
        }

        using var stream = typeof(WebServerService).Assembly.GetManifestResourceStream(resourceName);
        var ext = Path.GetExtension(path).ToLowerInvariant();
        var contentType = ext switch
        {
            ".html" => "text/html",
            ".js" => "application/javascript",
            ".wasm" => "application/wasm",
            ".css" => "text/css",
            _ => "application/octet-stream"
        };
        ctx.Response.ContentType = contentType;
        await stream.CopyToAsync(ctx.Response.OutputStream);
    }
}
