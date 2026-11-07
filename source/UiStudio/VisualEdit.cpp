// Visual editing: pick an element, change it in the Properties panel, drag widgets in, drag elements around.
// Every change is computed by XmlModel as a small text splice and applied through the editor, so Ctrl+Z undoes it,
// and the rest of the file is left exactly as the author wrote it.
#include "MainWindow.h"

#include "CodeEditor.h"
#include "GameSession.h"
#include "PageTree.h"
#include "PreviewView.h"
#include "PropertiesPanel.h"
#include "TreeRoles.h"
#include "XmlModel.h"

#include <QDockWidget>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextCursor>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>

void MainWindow::BuildVisualEditing(QDockWidget* leftTabsWith, QDockWidget* rightTabsWith)
{
    auto makeDock = [this](const QString& title, QWidget* content, Qt::DockWidgetArea area, const QString& name) {
        auto* dock = new QDockWidget(title, this);
        dock->setObjectName(name);
        dock->setWidget(content);
        dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        addDockWidget(area, dock);
        return dock;
    };

    // Palette: widgets that can be dragged into the page.
    auto* paletteBox = new QWidget;
    auto* pl = new QVBoxLayout(paletteBox);
    pl->setContentsMargins(6, 6, 6, 6);
    pl->setSpacing(6);
    auto* search = new QLineEdit;
    search->setPlaceholderText(tr("Search widgets…"));
    search->setClearButtonEnabled(true);
    m_palette = new PaletteList;
    m_palette->setToolTip(tr("Drag a widget onto the preview or into the page structure.\nDouble-click to add it inside the selected element."));
    pl->addWidget(search);
    pl->addWidget(m_palette, 1);
    connect(search, &QLineEdit::textChanged, this, [this](const QString& t) { FillPalette(t); });
    connect(m_palette, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* it) {
        const QString tag = it->data(Qt::UserRole).toString();
        if (tag.isEmpty())
            return;
        QTreeWidgetItem* target = m_hierarchy->currentItem();
        if (!target)
        {
            statusBar()->showMessage(tr("Click the element you want to add it to first (in the preview or the page structure)."), 6000);
            return;
        }
        InsertTag(tag, target, 0);
    });
    m_paletteDock = makeDock(tr("Add widgets"), paletteBox, Qt::LeftDockWidgetArea, "palette");
    tabifyDockWidget(leftTabsWith, m_paletteDock);

    // Properties.
    m_properties = new PropertiesPanel;
    m_propertiesDock = makeDock(tr("Properties"), m_properties, Qt::RightDockWidgetArea, "properties");
    tabifyDockWidget(rightTabsWith, m_propertiesDock);

    connect(m_properties, &PropertiesPanel::ValueChanged, this, [this](const QString& name, const QString& value) {
        QTreeWidgetItem* item = m_hierarchy->currentItem();
        CodeEditor* ed = nullptr;
        XmlModel model;
        XmlNode* node = nullptr;
        if (item && ParseItem(item, ed, model, node))
            ApplyEdits(ed, model.SetAttribute(node, name, value), tr("Changed %1").arg(name));
    });
    connect(m_properties, &PropertiesPanel::Removed, this, [this](const QString& name) {
        QTreeWidgetItem* item = m_hierarchy->currentItem();
        CodeEditor* ed = nullptr;
        XmlModel model;
        XmlNode* node = nullptr;
        if (item && ParseItem(item, ed, model, node) && ApplyEdits(ed, model.RemoveAttribute(node, name), tr("Removed %1").arg(name)))
            QTimer::singleShot(0, this, [this] { UpdateProperties(); });
    });
    connect(m_properties, &PropertiesPanel::Added, this, [this](const QString& name, const QString& value) {
        QTreeWidgetItem* item = m_hierarchy->currentItem();
        CodeEditor* ed = nullptr;
        XmlModel model;
        XmlNode* node = nullptr;
        if (item && ParseItem(item, ed, model, node) && ApplyEdits(ed, model.SetAttribute(node, name, value), tr("Added %1").arg(name)))
            QTimer::singleShot(0, this, [this] { UpdateProperties(); });
    });
    connect(m_properties, &PropertiesPanel::ActionRequested, this, [this](PropertiesPanel::Action action) {
        QTreeWidgetItem* item = m_hierarchy->currentItem();
        CodeEditor* ed = nullptr;
        XmlModel model;
        XmlNode* node = nullptr;
        if (!item || !ParseItem(item, ed, model, node))
            return;
        QVector<TextEdit> edits;
        QString what;
        switch (action)
        {
        case PropertiesPanel::Action::Duplicate: edits = model.Duplicate(node); what = tr("Duplicated element"); break;
        case PropertiesPanel::Action::MoveUp: edits = model.MoveUpDown(node, true); what = tr("Moved element up"); break;
        case PropertiesPanel::Action::MoveDown: edits = model.MoveUpDown(node, false); what = tr("Moved element down"); break;
        case PropertiesPanel::Action::Delete:
            if (QMessageBox::question(this, tr("Delete element"),
                                      tr("Delete this <%1> and everything inside it?\nYou can undo this with Ctrl+Z in the code editor.").arg(node->tag)) != QMessageBox::Yes)
                return;
            edits = model.DeleteNode(node);
            what = tr("Deleted element");
            break;
        }
        if (!edits.isEmpty() && ApplyEdits(ed, edits, what))
            ClearSelection();
    });

    connect(m_hierarchy, &PageTree::TagDropped, this, [this](const QString& tag, QTreeWidgetItem* target, PageTree::Where where) {
        InsertTag(tag, target, int(where));
    });
    connect(m_hierarchy, &PageTree::ElementDropped, this, [this](QTreeWidgetItem* source, QTreeWidgetItem* target, PageTree::Where where) {
        MoveElement(source, target, int(where));
    });
    connect(m_preview, &PreviewView::TagDropped, this, [this](const QString& tag, double x, double y) {
        if (!m_session->IsReady())
            return;
        m_dropTag = tag;
        m_session->RequestJson(QString("ui_pick_at %1 %2").arg(qRound(x)).arg(qRound(y)), "SaveData/ui_inspect/pick.json", "pick");
    });

    // Play mode: the preview forwards the pointer and keys to the game.
    connect(m_preview, &PreviewView::PointerMoved, this, [this](double x, double y) {
        if (!m_play || !m_session->IsReady() || (m_lastMove.isValid() && m_lastMove.elapsed() < 40))
            return;
        m_lastMove.start();
        m_session->Send(QString("ui_mouse move %1 %2").arg(qRound(x)).arg(qRound(y)));
    });
    connect(m_preview, &PreviewView::PointerPressed, this, [this](double x, double y) {
        if (m_play && m_session->IsReady())
            m_session->Send(QString("ui_mouse down %1 %2").arg(qRound(x)).arg(qRound(y)));
    });
    connect(m_preview, &PreviewView::PointerReleased, this, [this] {
        if (m_play && m_session->IsReady())
            m_session->Send("ui_mouse up");
    });
    connect(m_preview, &PreviewView::ActionKey, this, [this](const QString& action) {
        if (m_play && m_session->IsReady())
            m_session->Send("ui_action " + action);
    });

    FillPalette(QString());
}

// ---------------------------------------------------------------------------------------------------------------------
void MainWindow::FillPalette(const QString& filter)
{
    m_palette->clear();
    struct Group { const char* key; const char* title; };
    const Group groups[] = { { "layout", "Layout and containers" }, { "text", "Text" }, { "image", "Pictures" }, { "input", "Buttons and input" },
                             { "paint", "Shapes and progress" }, { "component", "Your components" } };
    for (const Group& g : groups)
    {
        QVector<const TagInfo*> tags;
        for (const TagInfo& t : m_db.Tags())
            if (t.category == g.key && (filter.isEmpty() || t.name.contains(filter, Qt::CaseInsensitive) || t.description.contains(filter, Qt::CaseInsensitive)))
                tags.push_back(&t);
        if (tags.isEmpty())
            continue;
        auto* header = new QListWidgetItem(tr(g.title));
        header->setFlags(Qt::ItemIsEnabled);
        QFont f = header->font();
        f.setBold(true);
        header->setFont(f);
        header->setForeground(QColor("#8fb1f5"));
        m_palette->addItem(header);
        for (const TagInfo* t : tags)
        {
            auto* item = new QListWidgetItem(t->description.isEmpty() ? t->name : QString("%1\n%2").arg(t->name, t->description));
            item->setData(Qt::UserRole, t->name);
            item->setToolTip(t->description);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
            m_palette->addItem(item);
        }
    }
}

QTreeWidgetItem* MainWindow::ItemByPath(const QString& path) const
{
    QTreeWidgetItemIterator it(m_hierarchy);
    while (*it)
    {
        if ((*it)->data(0, RolePath).toString() == path)
            return *it;
        ++it;
    }
    return nullptr;
}

void MainWindow::ClearSelection()
{
    m_selectedPath.clear();
    m_preview->SetSelection(QRectF());
    m_properties->ShowNothing(tr("Saved. Click an element to keep editing."));
}

// ---------------------------------------------------------------------------------------------------------------------
CodeEditor* MainWindow::EditorForItem(QTreeWidgetItem* item)
{
    const QString file = item->data(0, RoleFile).toString();
    if (file.isEmpty() || item->data(0, RoleLine).toInt() <= 0)
        return nullptr;
    const QString local = FileOfGamePath(file);
    if (!QFileInfo::exists(local))
        return nullptr;
    const QString canonical = QFileInfo(local).absoluteFilePath();
    for (int i = 0; i < m_tabs->count(); ++i)
    {
        auto* e = qobject_cast<CodeEditor*>(m_tabs->widget(i));
        if (e && QFileInfo(e->FilePath()).absoluteFilePath() == canonical)
            return e;
    }
    return OpenFile(local);
}

bool MainWindow::ParseItem(QTreeWidgetItem* item, CodeEditor*& editor, XmlModel& model, XmlNode*& node)
{
    editor = EditorForItem(item);
    if (!editor)
    {
        statusBar()->showMessage(tr("This element does not come from a page file, so it cannot be edited here."), 6000);
        return false;
    }
    QString error;
    if (!model.Parse(editor->toPlainText(), &error))
    {
        statusBar()->showMessage(tr("The page has a mistake (%1). Fix it in the code editor first.").arg(error), 8000);
        return false;
    }
    node = model.FindByPosition(item->data(0, RoleLine).toInt(), item->data(0, RoleCol).toInt());
    if (!node)
    {
        statusBar()->showMessage(tr("Could not find this element in the file. Save, then pick it again."), 6000);
        return false;
    }
    return true;
}

bool MainWindow::ApplyEdits(CodeEditor* editor, const QVector<TextEdit>& edits, const QString& what)
{
    if (edits.isEmpty())
        return false;
    QVector<TextEdit> sorted = edits;
    std::sort(sorted.begin(), sorted.end(), [](const TextEdit& a, const TextEdit& b) { return a.pos > b.pos; });
    QTextCursor c(editor->document());
    c.beginEditBlock();
    for (const TextEdit& e : sorted)
    {
        c.setPosition(e.pos);
        if (e.removeLen > 0)
            c.setPosition(e.pos + e.removeLen, QTextCursor::KeepAnchor);
        if (e.removeLen > 0 || !e.insert.isEmpty())
            c.insertText(e.insert);
    }
    c.endEditBlock();
    if (!SaveEditor(editor))
        return false;
    statusBar()->showMessage(tr("%1. Press Ctrl+Z in the code editor to undo.").arg(what), 6000);
    return true;
}

void MainWindow::UpdateProperties()
{
    QTreeWidgetItem* item = m_hierarchy->currentItem();
    if (!item || m_selectedPath.isEmpty())
    {
        m_properties->ShowNothing(tr("Click an element in the preview or in the page structure to edit it here."));
        return;
    }
    const QString file = item->data(0, RoleFile).toString();
    CodeEditor* ed = nullptr;
    XmlModel model;
    XmlNode* node = nullptr;
    const QString tag = item->text(0).section(' ', 0, 0);
    if (file.isEmpty() || item->data(0, RoleLine).toInt() <= 0)
    {
        m_properties->ShowNothing(tr("“%1” is created by the game itself, not by a page file, so there is nothing to edit here.").arg(tag));
        return;
    }
    ed = EditorForItem(item);
    QString error;
    if (!ed || !model.Parse(ed->toPlainText(), &error))
    {
        m_properties->ShowNothing(ed ? tr("The page has a mistake (%1).\nFix it in the code editor, then pick the element again.").arg(error)
                                     : tr("The page file could not be opened."));
        return;
    }
    node = model.FindByPosition(item->data(0, RoleLine).toInt(), item->data(0, RoleCol).toInt());
    if (!node)
    {
        m_properties->ShowNothing(tr("This element is not in the file as written (for example it is made by a component).\nPick another one."));
        return;
    }
    QVector<QPair<QString, QString>> values;
    for (const XmlAttr& a : node->attrs)
        values.push_back({ a.name, a.value });
    QVector<AttrInfo> known;
    if (const TagInfo* info = m_db.Find(node->tag))
        known = info->attributes;
    for (const char* common : { "id", "class" })
    {
        bool have = false;
        for (const AttrInfo& a : known)
            have = have || a.name == common;
        if (!have)
            known.push_back({ common, "string", QString(), QString() });
    }
    const int index = node->IndexInParent();
    const int siblings = node->parent ? node->parent->children.size() : 0;
    m_properties->ShowElement(node->tag, tr("%1, line %2").arg(QFileInfo(ed->FilePath()).fileName()).arg(node->line), values, known,
                              index > 0, index >= 0 && index + 1 < siblings);
    m_propertiesDock->raise();
}

// ---------------------------------------------------------------------------------------------------------------------
void MainWindow::InsertTag(const QString& tag, QTreeWidgetItem* target, int whereInt)
{
    CodeEditor* ed = nullptr;
    XmlModel model;
    XmlNode* node = nullptr;
    if (!ParseItem(target, ed, model, node))
        return;
    const auto where = static_cast<PageTree::Where>(whereInt);
    XmlNode* parent = node;
    XmlNode* before = nullptr;
    if (where != PageTree::Where::Onto && node->parent && node->parent != model.Root())
    {
        parent = node->parent;
        const int idx = node->IndexInParent();
        before = where == PageTree::Where::Above ? node
               : (idx + 1 < parent->children.size() ? parent->children[idx + 1].get() : nullptr);
    }
    const QString snippet = XmlModel::DefaultSnippet(tag, m_db.TagHasAttribute(tag, "text"));
    if (ApplyEdits(ed, model.InsertChild(parent, before, snippet), tr("Added <%1>").arg(tag)))
        ClearSelection();
}

void MainWindow::MoveElement(QTreeWidgetItem* source, QTreeWidgetItem* target, int whereInt)
{
    if (source == target)
        return;
    for (QTreeWidgetItem* p = target->parent(); p; p = p->parent())
        if (p == source)
        {
            statusBar()->showMessage(tr("An element cannot be moved inside itself."), 5000);
            return;
        }
    CodeEditor* ed = EditorForItem(source);
    if (!ed || ed != EditorForItem(target))
    {
        statusBar()->showMessage(tr("Dragging works inside one file. This element comes from a different file."), 6000);
        return;
    }
    CodeEditor* unused = nullptr;
    XmlModel model;
    XmlNode* src = nullptr;
    if (!ParseItem(source, unused, model, src))
        return;
    XmlNode* dst = model.FindByPosition(target->data(0, RoleLine).toInt(), target->data(0, RoleCol).toInt());
    if (!dst)
        return;
    const auto where = static_cast<PageTree::Where>(whereInt);
    XmlNode* parent = dst;
    XmlNode* before = nullptr;
    if (where != PageTree::Where::Onto && dst->parent && dst->parent != model.Root())
    {
        parent = dst->parent;
        const int idx = dst->IndexInParent();
        before = where == PageTree::Where::Above ? dst
               : (idx + 1 < parent->children.size() ? parent->children[idx + 1].get() : nullptr);
    }
    const QVector<TextEdit> edits = model.Move(src, parent, before);
    if (edits.isEmpty())
    {
        statusBar()->showMessage(tr("It is already there."), 3000);
        return;
    }
    if (ApplyEdits(ed, edits, tr("Moved element")))
        ClearSelection();
}
