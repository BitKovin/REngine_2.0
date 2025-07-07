using Microsoft.Maui.Controls;

namespace MyMauiWasmApp
{
    public partial class App : Application
    {
        public App()
        {
            InitializeComponent();
            // Ensure WebServerService is started
            _ = App.Current.Services.GetService<WebServerService>();
            MainPage = new MainPage();
        }
    }
}
