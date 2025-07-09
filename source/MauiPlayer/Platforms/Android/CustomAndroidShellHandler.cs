using Android.Views;
using Microsoft.Maui.Controls.Handlers.Compatibility;
using Microsoft.Maui.Controls.Platform.Compatibility;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MauiPlayer.Platforms.Android
{
    class CustomAndroidShellHandler : ShellRenderer
    {
        protected override IShellFlyoutRenderer CreateShellFlyoutRenderer()
        {
            var flyoutRenderer = base.CreateShellFlyoutRenderer();
            flyoutRenderer.AndroidView.Touch += AndroidView_Touch;

            return flyoutRenderer;
        }

        private void AndroidView_Touch(object? sender, global::Android.Views.View.TouchEventArgs e)
        {
            if (e.Event.Action == MotionEventActions.Move)
            {
                e.Handled = true;
            }
            else
            {
                e.Handled = false;
            }
        }
    }
}
