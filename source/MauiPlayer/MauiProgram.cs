using Microsoft.Extensions.Logging;

namespace MauiPlayer
{
    public static class MauiProgram
    {
        public static MauiApp CreateMauiApp()
        {
            var builder = MauiApp.CreateBuilder();
            builder
                .UseMauiApp<App>()
                .ConfigureFonts(fonts =>
                {
                    fonts.AddFont("OpenSans-Regular.ttf", "OpenSansRegular");
                    fonts.AddFont("OpenSans-Semibold.ttf", "OpenSansSemibold");
                });

            builder.ConfigureMauiHandlers(handlers =>
            {
#if IOS
                handlers.AddHandler<Shell, Platforms.iOS.CustomiOSShellHandler>();
#elif ANDROID
                handlers.AddHandler<Shell, Platforms.Android.CustomAndroidShellHandler>();
#endif
            });

#if DEBUG
    		builder.Logging.AddDebug();
#endif

            return builder.Build();
        }
    }
}
