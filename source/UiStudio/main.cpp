#include "MainWindow.h"
#include "Theme.h"

#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("UI Studio");
    app.setOrganizationName("UiStudio");
    Theme::Apply(app);

    MainWindow window;
    window.show();
    window.OpenFromArguments(app.arguments());
    return app.exec();
}
