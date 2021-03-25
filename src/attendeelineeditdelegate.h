/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#pragma once

#include <QModelIndex>
#include <QString>
#include <QStyledItemDelegate>

#include <KCompletion>

namespace IncidenceEditorNG
{
/** show a AttendeeLineEdit as editor */
class AttendeeLineEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AttendeeLineEditDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    virtual void setCompletionMode(KCompletion::CompletionMode mode);

public Q_SLOTS:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    void rightPressed();
    void leftPressed();
    QString mToolTip;
    QString mWhatsThis;
    KCompletion::CompletionMode mCompletionMode = KCompletion::CompletionPopup;
};
}

