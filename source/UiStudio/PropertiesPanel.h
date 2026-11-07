#pragma once

#include "UiDatabase.h"

#include <QPair>
#include <QVector>
#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;
class QScrollArea;

// Form for the selected element: one editor per attribute, an "Add property" list and element actions.
// It only reports what the user changed; the main window turns that into a source edit.
class PropertiesPanel : public QWidget
{
    Q_OBJECT
public:
    enum class Action { Duplicate, Delete, MoveUp, MoveDown };

    explicit PropertiesPanel(QWidget* parent = nullptr);

    void ShowNothing(const QString& message);
    // values: the attributes written in the file, in file order. known: what the engine documents for this tag.
    void ShowElement(const QString& tag, const QString& location, const QVector<QPair<QString, QString>>& values,
                     const QVector<AttrInfo>& known, bool canMoveUp, bool canMoveDown);

signals:
    void ValueChanged(const QString& name, const QString& value);
    void Removed(const QString& name);
    void Added(const QString& name, const QString& value);
    void ActionRequested(PropertiesPanel::Action action);

private:
    QWidget* MakeEditor(const QString& name, const QString& value, const QString& type);

    QLabel* m_title = nullptr;
    QLabel* m_where = nullptr;
    QLabel* m_message = nullptr;
    QWidget* m_actions = nullptr;
    QPushButton* m_up = nullptr;
    QPushButton* m_down = nullptr;
    QScrollArea* m_scroll = nullptr;
    QWidget* m_adder = nullptr;
    QComboBox* m_addCombo = nullptr;
    QVector<AttrInfo> m_missing;
};
