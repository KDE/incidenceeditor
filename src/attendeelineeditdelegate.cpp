/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#include "attendeelineeditdelegate.h"

#include "attendeeline.h"

#include <KLocalizedString>

#include <QAbstractItemView>
#include <QHelpEvent>
#include <QToolTip>
#include <QWhatsThis>

using namespace IncidenceEditorNG;

AttendeeLineEditDelegate::AttendeeLineEditDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    mToolTip = i18nc("@info:tooltip", "Enter the name or email address of the attendee.");
    mWhatsThis = i18nc("@info:whatsthis",
                       "The email address or name of the attendee. An invitation "
                       "can be sent to the user if an email address is provided.");
}

QWidget *AttendeeLineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    auto editor = new AttendeeLineEdit(parent);
    connect(editor, &AttendeeLineEdit::leftPressed, this, &AttendeeLineEditDelegate::leftPressed);
    connect(editor, &AttendeeLineEdit::rightPressed, this, &AttendeeLineEditDelegate::rightPressed);
    editor->setToolTip(mToolTip);
    editor->setWhatsThis(mWhatsThis);
    editor->setCompletionMode(mCompletionMode);
    editor->setClearButtonEnabled(true);

    return editor;
}

void AttendeeLineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto lineedit = static_cast<AttendeeLineEdit *>(editor);
    lineedit->setText(index.model()->data(index, Qt::EditRole).toString());
}

void AttendeeLineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto lineedit = static_cast<AttendeeLineEdit *>(editor);
    model->setData(index, lineedit->text(), Qt::EditRole);
}

void AttendeeLineEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

void AttendeeLineEditDelegate::leftPressed()
{
    Q_EMIT closeEditor(static_cast<QWidget *>(QObject::sender()), QAbstractItemDelegate::EditPreviousItem);
}

void AttendeeLineEditDelegate::rightPressed()
{
    Q_EMIT closeEditor(static_cast<QWidget *>(QObject::sender()), QAbstractItemDelegate::EditNextItem);
}

void AttendeeLineEditDelegate::setCompletionMode(KCompletion::CompletionMode mode)
{
    mCompletionMode = mode;
}

bool AttendeeLineEditDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!event || !view) {
        return false;
    }
    switch (event->type()) {
#ifndef QT_NO_TOOLTIP
    case QEvent::ToolTip:
        QToolTip::showText(event->globalPos(), mToolTip, view);
        return true;
#endif
#ifndef QT_NO_WHATSTHIS
    case QEvent::QueryWhatsThis:
        return true;
    case QEvent::WhatsThis:
        QWhatsThis::showText(event->globalPos(), mWhatsThis, view);
        return true;
#endif
    default:
        break;
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}
