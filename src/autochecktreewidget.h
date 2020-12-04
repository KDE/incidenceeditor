/*
  SPDX-FileCopyrightText: 2007 Mathias Soeken <msoeken@tzi.de>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef INCIDENCEEDITOR_AUTOCHECKTREEWIDGET_H
#define INCIDENCEEDITOR_AUTOCHECKTREEWIDGET_H

#include <QTreeWidget>

namespace IncidenceEditorNG {
/**
  A tree widget which supports auto selecting child items, when clicking
  an item of the tree.

  Further you can find an item by passing a path as QStringList with
  the method itemByPath().

  @author Mathias Soeken <msoeken@tzi.de>
*/
class AutoCheckTreeWidget : public QTreeWidget
{
    Q_OBJECT

    Q_PROPERTY(bool autoCheckChildren READ autoCheckChildren WRITE setAutoCheckChildren)

    Q_PROPERTY(bool autoCheck READ autoCheck WRITE setAutoCheck)

public:
    /**
      Default constructor. The default behavior is like a QTreeWidget, so you
      have to activate the autoCheckChildren property manually.
    */
    explicit AutoCheckTreeWidget(QWidget *parent = nullptr);

    ~AutoCheckTreeWidget() override;

    /**
      Returns QTreeWidgetItem which matches the path, if available.

      @param path The path
      @returns a item which is represented by the path, if available.
    */
    QTreeWidgetItem *itemByPath(const QStringList &path) const;

    /**
      Returns a path by a given item as QStringList.

      @param item The item
      @returns a string list which is the represented path of the item.
    */
    Q_REQUIRED_RESULT QStringList pathByItem(QTreeWidgetItem *item) const;

    /**
      @returns whether autoCheckChildren is enabled or not.
               Default value is false.
    */
    Q_REQUIRED_RESULT bool autoCheckChildren() const;

    /**
      enables or disables autoCheckChildren behavior.

      @param autoCheckChildren if true, children of items are auto checked or
                               not, otherwise. Default value is false.
    */
    void setAutoCheckChildren(bool autoCheckChildren);

    /**
      @returns whether newly added items have checkboxes by default.
               Default value is true.
    */
    Q_REQUIRED_RESULT bool autoCheck() const;

    /**
      Sets whether newly added items have checkboxes by default.

      @param autoCheck if true, newly added items have unchecked checkboxes
                       by default, otherwise not. Default value is true.
    */
    void setAutoCheck(bool autoCheck);

protected:
    QTreeWidgetItem *findItem(QTreeWidgetItem *parent, const QString &text) const;

protected Q_SLOTS:
    void slotRowsInserted(const QModelIndex &parent, int start, int end);
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};
}

#endif
