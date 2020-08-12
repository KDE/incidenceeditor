/*
  SPDX-FileCopyrightText: 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef INCIDENCEEDITOR_CATEGORYEDITDIALOG_H
#define INCIDENCEEDITOR_CATEGORYEDITDIALOG_H

#include <QDialog>

class QTreeWidgetItem;

namespace Ui {
class CategoryEditDialog_base;
}

namespace CalendarSupport {
class CategoryConfig;
}

namespace IncidenceEditorNG {
class CategoryEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CategoryEditDialog(CalendarSupport::CategoryConfig *categoryConfig, QWidget *parent = nullptr);

    ~CategoryEditDialog();

public Q_SLOTS:
    void reload();
    virtual void show();

protected Q_SLOTS:
    void slotOk();
    void slotApply();
    void slotCancel();
    void slotTextChanged(const QString &text);
    void slotSelectionChanged();
    void add();
    void addSubcategory();
    void remove();
    void editItem();
    void expandIfToplevel(QTreeWidgetItem *item);

Q_SIGNALS:
    void categoryConfigChanged();

protected:
    void fillList();

private:
    void deleteItem(QTreeWidgetItem *item, QList<QTreeWidgetItem *> &to_remove);
    CalendarSupport::CategoryConfig *mCategoryConfig = nullptr;
    Ui::CategoryEditDialog_base *mWidgets = nullptr;
};
}

#endif
