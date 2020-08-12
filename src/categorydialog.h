/*
  SPDX-FileCopyrightText: 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileContributor: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef INCIDENCEEDITOR_CATEGORYDIALOG_H
#define INCIDENCEEDITOR_CATEGORYDIALOG_H

#include <QDialog>

class CategoryWidgetBase;

namespace CalendarSupport {
class CategoryConfig;
}

namespace IncidenceEditorNG {
class AutoCheckTreeWidget;

class CategoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CategoryWidget(CalendarSupport::CategoryConfig *config, QWidget *parent = nullptr);
    ~CategoryWidget();

    void setCategories(const QStringList &categoryList = QStringList());
    void setCategoryList(const QStringList &categories);

    void setSelected(const QStringList &selList);
    Q_REQUIRED_RESULT QStringList selectedCategories() const;
    Q_REQUIRED_RESULT QStringList selectedCategories(QString &categoriesStr);

    void setAutoselectChildren(bool autoselectChildren);

    void hideButton();
    void hideHeader();

    Q_REQUIRED_RESULT AutoCheckTreeWidget *listView() const;

public Q_SLOTS:
    void clear();

private:
    void handleTextChanged(const QString &newText);
    void handleSelectionChanged();
    void handleColorChanged(const QColor &);
    void addCategory();
    void removeCategory();
    QStringList mCategoryList;
    CategoryWidgetBase *mWidgets = nullptr;
    CalendarSupport::CategoryConfig *mCategoryConfig = nullptr;
};

class CategoryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CategoryDialog(CalendarSupport::CategoryConfig *cfg, QWidget *parent = nullptr);
    ~CategoryDialog();

    QStringList selectedCategories() const;
    void setCategoryList(const QStringList &categories);

    void setAutoselectChildren(bool autoselectChildren);
    void setSelected(const QStringList &selList);

public Q_SLOTS:
    void slotOk();
    void slotApply();
    void updateCategoryConfig();

Q_SIGNALS:
    void categoriesSelected(const QString &);
    void categoriesSelected(const QStringList &);

private:
    CategoryWidget *mWidgets = nullptr;
    CalendarSupport::CategoryConfig *mCategoryConfig = nullptr;
    class CategorySelectDialogPrivate;
    CategorySelectDialogPrivate *d;
};
}

#endif
