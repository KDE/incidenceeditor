/*
  SPDX-FileCopyrightText: 2007 Mathias Soeken <msoeken@tzi.de>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "autochecktreewidget.h"

using namespace IncidenceEditorNG;

//@cond PRIVATE
class Q_DECL_HIDDEN AutoCheckTreeWidget::Private
{
public:
    bool mAutoCheckChildren = false;
    bool mAutoCheck = true;
};
//@endcond

AutoCheckTreeWidget::AutoCheckTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , d(new Private())
{
    connect(model(), &QAbstractItemModel::rowsInserted,
            this, &AutoCheckTreeWidget::slotRowsInserted);
    connect(model(), &QAbstractItemModel::dataChanged, this,
            &AutoCheckTreeWidget::slotDataChanged);

    setColumnCount(2);
}

AutoCheckTreeWidget::~AutoCheckTreeWidget()
{
    delete d;
}

QTreeWidgetItem *AutoCheckTreeWidget::itemByPath(const QStringList &path) const
{
    QStringList newPath = path;
    QTreeWidgetItem *item = nullptr;

    while (!newPath.isEmpty()) {
        item = findItem(item, newPath.takeFirst());
        if (!item) {
            return nullptr;
        }
    }

    return item;
}

QStringList AutoCheckTreeWidget::pathByItem(QTreeWidgetItem *item) const
{
    QStringList path;
    QTreeWidgetItem *current = item;

    while (current) {
        path.prepend(current->text(0));
        current = current->parent();
    }

    return path;
}

bool AutoCheckTreeWidget::autoCheckChildren() const
{
    return d->mAutoCheckChildren;
}

void AutoCheckTreeWidget::setAutoCheckChildren(bool autoCheckChildren)
{
    d->mAutoCheckChildren = autoCheckChildren;
}

bool AutoCheckTreeWidget::autoCheck() const
{
    return d->mAutoCheck;
}

void AutoCheckTreeWidget::setAutoCheck(bool autoCheck)
{
    d->mAutoCheck = autoCheck;
}

QTreeWidgetItem *AutoCheckTreeWidget::findItem(QTreeWidgetItem *parent, const QString &text) const
{
    if (parent) {
        const int nbChild{
            parent->childCount()
        };
        for (int i = 0; i < nbChild; ++i) {
            if (parent->child(i)->text(0) == text) {
                return parent->child(i);
            }
        }
    } else {
        for (int i = 0; i < topLevelItemCount(); ++i) {
            if (topLevelItem(i)->text(0) == text) {
                return topLevelItem(i);
            }
        }
    }

    return nullptr;
}

void AutoCheckTreeWidget::slotRowsInserted(const QModelIndex &parent, int start, int end)
{
    if (d->mAutoCheck) {
        QTreeWidgetItem *item = itemFromIndex(parent);
        QTreeWidgetItem *child = nullptr;
        if (item) {
            QBrush b(Qt::yellow);
            item->setBackground(0, b);
            for (int i = start; i < qMax(end, item->childCount()); ++i) {
                child = item->child(i);
                child->setFlags(child->flags() | Qt::ItemIsUserCheckable);
                child->setCheckState(0, Qt::Unchecked);
            }
        } else { /* top level item has been inserted */
            for (int i = start; i < qMax(end, topLevelItemCount()); ++i) {
                child = topLevelItem(i);
                child->setFlags(child->flags() | Qt::ItemIsUserCheckable);
                child->setCheckState(0, Qt::Unchecked);
            }
        }
    }
}

void AutoCheckTreeWidget::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!d->mAutoCheckChildren) {
        return;
    }

    QTreeWidgetItem *item1 = itemFromIndex(topLeft);
    QTreeWidgetItem *item2 = itemFromIndex(bottomRight);

    if (item1 == item2) {
        for (int i = 0; i < item1->childCount(); ++i) {
            item1->child(i)->setCheckState(0, item1->checkState(0));
        }
    }
}
