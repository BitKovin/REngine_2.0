using Android.App;
using Android.Content.PM;
using Android.OS;
using Android.Views;

namespace MauiPlayer
{
    [Activity(
       Theme = "@style/Maui.SplashTheme",
       MainLauncher = true,
       ConfigurationChanges = ConfigChanges.ScreenSize | ConfigChanges.Orientation,
       // ← lock into landscape (or use SensorLandscape if you want either landscape)
       ScreenOrientation = ScreenOrientation.SensorLandscape
       )]
    public class MainActivity : MauiAppCompatActivity
    {

        protected override void OnCreate(Bundle? savedInstanceState)
        {
            base.OnCreate(savedInstanceState);

            Platform.Init(this, savedInstanceState);

            EnableFullscreen();

        }

        public override void OnWindowFocusChanged(bool hasFocus)
        {
            base.OnWindowFocusChanged(hasFocus);
            if (hasFocus)
                EnableFullscreen();
        }

        void EnableFullscreen()
        {
            var window = Window;

#if ANDROID_R_OR_LATER
            if (Build.VERSION.SdkInt >= BuildVersionCodes.R)
            {
                // Let content extend into system bars
                window.SetDecorFitsSystemWindows(false);

                var controller = window.InsetsController;
                controller?.Hide(WindowInsets.Type.StatusBars() | WindowInsets.Type.NavigationBars());
                controller?.SystemBarsBehavior =
                    WindowInsetsControllerBehavior.ShowTransientBarsBySwipe;
            }
            else
#endif
            {
                // Pre‑Android 11
                window.DecorView.SystemUiVisibility = (StatusBarVisibility)(
                    SystemUiFlags.Fullscreen
                  | SystemUiFlags.HideNavigation
                  | SystemUiFlags.ImmersiveSticky
                  | SystemUiFlags.LayoutStable
                  | SystemUiFlags.LayoutHideNavigation
                  | SystemUiFlags.LayoutFullscreen);
            }

            // Make bars transparent so your layout truly sits underneath
            window.SetStatusBarColor(Android.Graphics.Color.Transparent);
            window.SetNavigationBarColor(Android.Graphics.Color.Transparent);

            // Allow drawing into display cut‑outs (notches)
            var attrs = window.Attributes;
            attrs.LayoutInDisplayCutoutMode = LayoutInDisplayCutoutMode.ShortEdges;
            window.Attributes = attrs;

            // Also no limits layout (just in case)
            window.AddFlags(WindowManagerFlags.LayoutNoLimits);
        }

    }
}
