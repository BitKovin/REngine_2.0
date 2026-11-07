#pragma once

#include <QList>
#include <QString>

// Starter files offered by "New file". Every template is valid markup or style for the engine's UI.
struct FileTemplate
{
    QString name;
    QString description;
    QString extension;     // ".xml" or ".css"
    QString content;       // %1 is replaced by the file's base name, %2 by its game-relative folder
};

namespace Templates
{
    const QList<FileTemplate>& All();
    QString Render(const FileTemplate& t, const QString& baseName, const QString& folder);
}
