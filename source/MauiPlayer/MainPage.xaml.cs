using MauiPlatform;

namespace MauiPlayer
{
    public partial class MainPage : ContentPage
    {
        int count = 0;

        public MainPage()
        {

            LocalWebServer.StartAsync().Wait();

            InitializeComponent();
        }
    }
}
