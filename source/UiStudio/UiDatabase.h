#pragma once

#include <QMap>
#include <QString>
#include <QVector>

// What the engine says it understands: tags, their attributes and the project's own components.
// Read from the offline catalogue uidb.json (written by the game); a short built-in list is used when it is missing.
struct AttrInfo
{
    QString name;
    QString type;           // string, bool, number, float, color, vec2, path, text, actions, ...
    QString description;
    QString defaultValue;
};

struct TagInfo
{
    QString name;
    QString category;       // layout, text, input, image, paint, component
    QString description;
    bool isComponent = false;
    QVector<AttrInfo> attributes;
};

class UiDatabase
{
public:
    // Looks at <gameFolder>/SaveData/UIStudio/uidb.json for each folder in `searchFolders`.
    void Load(const QStringList& searchFolders);
    bool FromFile() const { return m_fromFile; }
    const QVector<TagInfo>& Tags() const { return m_tags; }
    const TagInfo* Find(const QString& tag) const;
    bool TagHasAttribute(const QString& tag, const QString& attribute) const;

private:
    void LoadFallback();
    static QString tr_component(const QString& path);

    QVector<TagInfo> m_tags;
    bool m_fromFile = false;
};
