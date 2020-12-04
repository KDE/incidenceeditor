/*
  SPDX-FileCopyrightText: 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef INCIDENCEEDITOR_CATEGORYSELECTDIALOG_H
#define INCIDENCEEDITOR_CATEGORYSELECTDIALOG_H

#include <QDialog>

class CategorySelectWidgetBase;

namespace CalendarSupport {
class CategoryConfig;
}

namespace IncidenceEditorNG {
class AutoCheckTreeWidget;

class CategorySelectWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CategorySelectWidget(CalendarSupport::CategoryConfig *config, QWidget *parent = nullptr);
    ~CategorySelectWidget() override;

    void setCategories(const QStringList &categoryList = QStringList());
    void setCategoryList(const QStringList &categories);

    void setSelected(const QStringList &selList);
    Q_REQUIRED_RESULT QStringList selectedCategories() const;
    Q_REQUIRED_RESULT QStringList selectedCategories(QString &categoriesStr);

    void setAutoselectChildren(bool autoselectChildren);

    void hideButton();
    void hideHeader();

    AutoCheckTreeWidget *listView() const;

public Q_SLOTS:
    void clear();

Q_SIGNALS:
    void editCategories();

private:
    QStringList mCategoryList;
    CategorySelectWidgetBase *mWidgets = nullptr;
    CalendarSupport::CategoryConfig * const mCategoryConfig;
};

class CategorySelectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CategorySelectDialog(CalendarSupport::CategoryConfig *cfg, QWidget *parent = nullptr);
    ~CategorySelectDialog();

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
    void editCategories();

private:
    CategorySelectWidget *mWidgets = nullptr;

    class CategorySelectDialogPrivate;
    CategorySelectDialogPrivate *d;
};
}

#endif
