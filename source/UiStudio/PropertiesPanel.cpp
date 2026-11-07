#include "PropertiesPanel.h"

#include <QColorDialog>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace
{
    QColor ParseHex(const QString& v)
    {
        if (!v.startsWith('#'))
            return QColor();
        const QString h = v.mid(1);
        if (h.size() == 8)          // rrggbbaa
        {
            QColor c("#" + h.left(6));
            c.setAlpha(h.mid(6, 2).toInt(nullptr, 16));
            return c;
        }
        return QColor(v);
    }

    QIcon Swatch(const QColor& c)
    {
        QPixmap pm(16, 16);
        pm.fill(c.isValid() ? c : QColor(60, 63, 72));
        return QIcon(pm);
    }
}

PropertiesPanel::PropertiesPanel(QWidget* parent) : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(6);

    m_title = new QLabel;
    QFont f = m_title->font();
    f.setBold(true);
    f.setPointSize(f.pointSize() + 2);
    m_title->setFont(f);
    m_where = new QLabel;
    m_where->setStyleSheet("color:#8f97aa");
    m_message = new QLabel;
    m_message->setWordWrap(true);
    m_message->setStyleSheet("color:#8f97aa");
    root->addWidget(m_title);
    root->addWidget(m_where);

    m_actions = new QWidget;
    auto* ah = new QHBoxLayout(m_actions);
    ah->setContentsMargins(0, 0, 0, 0);
    auto addButton = [&](const QString& text, const QString& tip, Action a) {
        auto* b = new QPushButton(text);
        b->setToolTip(tip);
        connect(b, &QPushButton::clicked, this, [this, a] { emit ActionRequested(a); });
        ah->addWidget(b);
        return b;
    };
    addButton(tr("Duplicate"), tr("Make a copy of this element right after it"), Action::Duplicate);
    m_up = addButton(tr("▲"), tr("Move this element up (earlier in its parent)"), Action::MoveUp);
    m_down = addButton(tr("▼"), tr("Move this element down (later in its parent)"), Action::MoveDown);
    addButton(tr("Delete"), tr("Remove this element and everything inside it"), Action::Delete);
    ah->addStretch(1);
    root->addWidget(m_actions);

    m_scroll = new QScrollArea;
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    root->addWidget(m_scroll, 1);

    m_adder = new QWidget;
    auto* adl = new QHBoxLayout(m_adder);
    adl->setContentsMargins(0, 0, 0, 0);
    m_addCombo = new QComboBox;
    m_addCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    auto* add = new QPushButton(tr("＋  Add property"));
    add->setToolTip(tr("Add the property chosen in the list to this element"));
    connect(add, &QPushButton::clicked, this, [this] {
        const int i = m_addCombo->currentIndex();
        if (i >= 0 && i < m_missing.size())
            emit Added(m_missing[i].name, m_missing[i].defaultValue);
    });
    adl->addWidget(m_addCombo, 1);
    adl->addWidget(add);
    root->addWidget(m_adder);
    root->addWidget(m_message);

    ShowNothing(tr("Click an element in the preview or in the page structure to edit it here."));
}

void PropertiesPanel::ShowNothing(const QString& message)
{
    m_title->setText(tr("Properties"));
    m_where->clear();
    m_actions->hide();
    m_adder->hide();
    m_scroll->setWidget(new QWidget);
    m_message->setText(message);
    m_message->show();
}

QWidget* PropertiesPanel::MakeEditor(const QString& name, const QString& value, const QString& type)
{
    if (type == "bool")
    {
        auto* combo = new QComboBox;
        combo->addItems({ "true", "false" });
        combo->setCurrentText(value.compare("false", Qt::CaseInsensitive) == 0 ? "false" : "true");
        connect(combo, &QComboBox::activated, this, [this, name, combo] { emit ValueChanged(name, combo->currentText()); });
        return combo;
    }
    auto* holder = new QWidget;
    auto* h = new QHBoxLayout(holder);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(4);
    auto* edit = new QLineEdit(value);
    // Only report real changes, when the user finishes typing.
    connect(edit, &QLineEdit::editingFinished, this, [this, name, edit, last = value]() mutable {
        if (edit->text() != last)
        {
            last = edit->text();
            emit ValueChanged(name, last);
        }
    });
    h->addWidget(edit, 1);
    if (type == "color")
    {
        auto* pick = new QPushButton;
        pick->setIcon(Swatch(ParseHex(value)));
        pick->setFixedWidth(30);
        pick->setToolTip(tr("Choose a colour"));
        connect(pick, &QPushButton::clicked, this, [this, name, edit, pick] {
            const QColor start = ParseHex(edit->text());
            const QColor c = QColorDialog::getColor(start.isValid() ? start : QColor("#ffffff"), this, tr("Choose a colour"),
                                                    QColorDialog::ShowAlphaChannel);
            if (!c.isValid())
                return;
            QString hex = c.name(QColor::HexRgb);
            if (c.alpha() != 255)
                hex += QString("%1").arg(c.alpha(), 2, 16, QChar('0'));
            edit->setText(hex);
            pick->setIcon(Swatch(c));
            emit ValueChanged(name, hex);
        });
        h->addWidget(pick);
    }
    return holder;
}

void PropertiesPanel::ShowElement(const QString& tag, const QString& location, const QVector<QPair<QString, QString>>& values,
                                  const QVector<AttrInfo>& known, bool canMoveUp, bool canMoveDown)
{
    m_title->setText(tag);
    m_where->setText(location);
    m_message->hide();
    m_actions->show();
    m_up->setEnabled(canMoveUp);
    m_down->setEnabled(canMoveDown);

    auto* form = new QWidget;
    auto* grid = new QGridLayout(form);
    grid->setContentsMargins(0, 4, 0, 4);
    grid->setVerticalSpacing(8);
    grid->setColumnStretch(1, 1);
    int row = 0;
    for (const auto& entry : values)
    {
        const QString name = entry.first;
        const AttrInfo* info = nullptr;
        for (const AttrInfo& a : known)
            if (a.name == name)
                info = &a;
        auto* label = new QLabel(QString("<b>%1</b>").arg(name));
        if (info && !info->description.isEmpty())
        {
            label->setToolTip(info->description);
            label->setText(QString("<b>%1</b><br><span style=\"color:#8f97aa\">%2</span>").arg(name, info->description.toHtmlEscaped()));
            label->setWordWrap(true);
        }
        grid->addWidget(label, row, 0, Qt::AlignTop);
        grid->addWidget(MakeEditor(name, entry.second, info ? info->type : QString("string")), row, 1, Qt::AlignTop);
        auto* remove = new QPushButton("✕");
        remove->setFixedWidth(28);
        remove->setToolTip(tr("Remove this property from the element"));
        connect(remove, &QPushButton::clicked, this, [this, name] { emit Removed(name); });
        grid->addWidget(remove, row, 2, Qt::AlignTop);
        ++row;
    }
    if (values.isEmpty())
    {
        auto* none = new QLabel(tr("This element has no properties yet. Use “Add property” below."));
        none->setWordWrap(true);
        none->setStyleSheet("color:#8f97aa");
        grid->addWidget(none, 0, 0, 1, 3);
        row = 1;
    }
    grid->setRowStretch(row, 1);
    m_scroll->setWidget(form);

    m_missing.clear();
    m_addCombo->clear();
    for (const AttrInfo& a : known)
    {
        bool present = false;
        for (const auto& v : values)
            present = present || v.first == a.name;
        if (present)
            continue;
        m_missing.push_back(a);
        m_addCombo->addItem(a.description.isEmpty() ? a.name : QString("%1  -  %2").arg(a.name, a.description));
    }
    m_adder->setVisible(!m_missing.isEmpty());
}
