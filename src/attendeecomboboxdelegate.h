/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#pragma once

#include <QIcon>
#include <QModelIndex>
#include <QString>
#include <QStyledItemDelegate>

namespace IncidenceEditorNG
{
/**
 * class to show a Icon and Text for an Attendee
 * you have to set the Items via addItem to have a list to choose from.
 * saves the option as int in the model
 */
class AttendeeComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AttendeeComboBoxDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    bool eventFilter(QObject *editor, QEvent *event) override;

    virtual void addItem(const QIcon &, const QString &);
    virtual void clear();

    virtual void setToolTip(const QString &);
    virtual void setWhatsThis(const QString &);

    /** choose this index, if the item in the model is unknown
     */
    void setStandardIndex(int);

public Q_SLOTS:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private Q_SLOTS:
    void doCloseEditor(QWidget *editor);
    void rightPressed();
    void leftPressed();

private:
    /** all entries to choose from */
    QVector<QPair<QIcon, QString>> mEntries;
    QString mToolTip;
    QString mWhatsThis;
    /**fallback index */
    int mStandardIndex = 0;
};
}

