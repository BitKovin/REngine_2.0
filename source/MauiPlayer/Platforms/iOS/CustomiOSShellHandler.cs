using Microsoft.Maui.Controls.Handlers.Compatibility;
using Microsoft.Maui.Controls.Platform.Compatibility;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UIKit;

namespace MauiPlayer.Platforms.iOS
{
    public class CustomiOSShellHandler : ShellRenderer
    {
        IShellFlyoutRenderer flyoutRenderer;

        protected override IShellFlyoutRenderer CreateFlyoutRenderer()
        {
            flyoutRenderer = base.CreateFlyoutRenderer();
            return flyoutRenderer;
        }

        public override void ViewWillAppear(bool animated)
        {
            base.ViewWillAppear(animated);
            var type = flyoutRenderer.GetType();
            var property = type.GetProperty("PanGestureRecognizer", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
            var value = property.GetValue(flyoutRenderer);

            UIPanGestureRecognizer recognizer = value as UIPanGestureRecognizer;
            recognizer.Enabled = false;
        }
    }
}
