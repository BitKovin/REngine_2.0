#include "UiDatabase.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void UiDatabase::Load(const QStringList& searchFolders)
{
    m_tags.clear();
    m_fromFile = false;
    for (const QString& folder : searchFolders)
    {
        QFile f(folder + "/SaveData/UIStudio/uidb.json");
        if (!f.open(QIODevice::ReadOnly))
            continue;
        const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
        const QJsonObject reg = root["registries"].toObject();
        const QJsonArray tags = reg["tags"].toArray();
        if (tags.isEmpty())
            continue;

        for (const QJsonValue& tv : tags)
        {
            const QJsonObject t = tv.toObject();
            if (t["legacy"].toBool())
                continue;
            TagInfo info;
            info.name = t["name"].toString();
            info.category = t["category"].toString();
            info.description = t["description"].toString();
            for (const QJsonValue& av : t["attributes"].toArray())
            {
                const QJsonObject a = av.toObject();
                if (a["name"].toString().contains('<'))
                    continue;                                // pattern entries such as param-<name>
                info.attributes.push_back({ a["name"].toString(), a["type"].toString(), a["description"].toString(), a["default"].toVariant().toString() });
            }
            m_tags.push_back(info);
        }
        for (const QJsonValue& cv : reg["components"].toArray())
        {
            const QJsonObject c = cv.toObject();
            TagInfo info;
            info.name = c["tag"].toString();
            info.category = "component";
            info.description = tr_component(c["path"].toString());
            info.isComponent = true;
            for (const char* common : { "id", "class" })
                info.attributes.push_back({ common, "string", QString(), QString() });
            for (const QJsonValue& pv : c["parameters"].toArray())
            {
                const QJsonObject p = pv.toObject();
                info.attributes.push_back({ p["name"].toString(), "string", QString("Component parameter"), p["default"].toVariant().toString() });
            }
            m_tags.push_back(info);
        }
        m_fromFile = true;
        return;
    }
    LoadFallback();
}

QString UiDatabase::tr_component(const QString& path)
{
    return QString("Reusable component (%1)").arg(path);
}

void UiDatabase::LoadFallback()
{
    const QVector<AttrInfo> common = { { "id", "string", "Name used to find this element", {} }, { "class", "string", "Style classes, separated by spaces", {} } };
    auto add = [&](const char* name, const char* cat, const char* desc, QVector<AttrInfo> extra) {
        TagInfo t;
        t.name = name;
        t.category = cat;
        t.description = desc;
        t.attributes = common + extra;
        m_tags.push_back(t);
    };
    const AttrInfo text{ "text", "text", "The words shown", {} };
    add("Panel", "layout", "Container with a box (background, border, radius)", {});
    add("Label", "text", "Text", { text });
    add("Button", "input", "Button", { text });
    add("TextButton", "input", "Button that is just text", { text });
    add("Image", "image", "Picture", { { "src", "path", "Image file", {} } });
    add("Slider", "input", "Slider", {});
    add("ProgressBar", "paint", "Progress bar", {});
}

const TagInfo* UiDatabase::Find(const QString& tag) const
{
    for (const TagInfo& t : m_tags)
        if (t.name == tag)
            return &t;
    return nullptr;
}

bool UiDatabase::TagHasAttribute(const QString& tag, const QString& attribute) const
{
    if (const TagInfo* t = Find(tag))
        for (const AttrInfo& a : t->attributes)
            if (a.name == attribute)
                return true;
    return false;
}
