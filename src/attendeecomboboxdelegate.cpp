/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#include "attendeecomboboxdelegate.h"

#include "attendeeline.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QHelpEvent>
#include <QMenu>
#include <QToolTip>
#include <QWhatsThis>

using namespace IncidenceEditorNG;

AttendeeComboBoxDelegate::AttendeeComboBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    connect(this, &AttendeeComboBoxDelegate::closeEditor, this, &AttendeeComboBoxDelegate::doCloseEditor);
}

void AttendeeComboBoxDelegate::addItem(const QIcon &icon, const QString &text)
{
    QPair<QIcon, QString> pair;
    pair.first = icon;
    pair.second = text;
    mEntries << pair;
}

void AttendeeComboBoxDelegate::clear()
{
    mEntries.clear();
}

void AttendeeComboBoxDelegate::setToolTip(const QString &tT)
{
    mToolTip = tT;
}

void AttendeeComboBoxDelegate::setWhatsThis(const QString &wT)
{
    mWhatsThis = wT;
}

void AttendeeComboBoxDelegate::setStandardIndex(int index)
{
    mStandardIndex = index;
}

QWidget *AttendeeComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /* option */, const QModelIndex & /* index */) const
{
    auto editor = new AttendeeComboBox(parent);

    for (const QPair<QIcon, QString> &pair : std::as_const(mEntries)) {
        editor->addItem(pair.first, pair.second);
    }

    connect(editor, &AttendeeComboBox::leftPressed, this, &AttendeeComboBoxDelegate::leftPressed);
    connect(editor, &AttendeeComboBox::rightPressed, this, &AttendeeComboBoxDelegate::rightPressed);

    editor->setPopupMode(QToolButton::MenuButtonPopup);
    editor->setToolTip(mToolTip);
    editor->setWhatsThis(mWhatsThis);
    return editor;
}

void AttendeeComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto comboBox = static_cast<AttendeeComboBox *>(editor);
    int value = index.model()->data(index, Qt::EditRole).toUInt();
    if (value >= mEntries.count()) {
        value = mStandardIndex;
    }
    comboBox->setCurrentIndex(value);
}

void AttendeeComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto comboBox = static_cast<AttendeeComboBox *>(editor);
    model->setData(index, comboBox->currentIndex(), Qt::EditRole);
    comboBox->menu()->close();
}

void AttendeeComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}

void AttendeeComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton myOption;

    int value = index.model()->data(index).toUInt();
    if (value >= mEntries.count()) {
        value = mStandardIndex;
    }

    myOption.rect = option.rect;
    myOption.state = option.state;
    myOption.icon = mEntries[value].first;
    myOption.iconSize = myOption.icon.actualSize(option.rect.size());

    QApplication::style()->drawControl(QStyle::CE_PushButton, &myOption, painter);
}

bool AttendeeComboBoxDelegate::eventFilter(QObject *editor, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        auto comboBox = static_cast<AttendeeComboBox *>(editor);
        comboBox->showMenu();
        return editor->eventFilter(editor, event);
    }

    return QStyledItemDelegate::eventFilter(editor, event);
}

void AttendeeComboBoxDelegate::doCloseEditor(QWidget *editor)
{
    auto comboBox = static_cast<AttendeeComboBox *>(editor);
    comboBox->menu()->close();
}

void AttendeeComboBoxDelegate::leftPressed()
{
    Q_EMIT closeEditor(static_cast<QWidget *>(QObject::sender()), QAbstractItemDelegate::EditPreviousItem);
}

void AttendeeComboBoxDelegate::rightPressed()
{
    Q_EMIT closeEditor(static_cast<QWidget *>(QObject::sender()), QAbstractItemDelegate::EditNextItem);
}

bool AttendeeComboBoxDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!event || !view) {
        return false;
    }
    switch (event->type()) {
#ifndef QT_NO_TOOLTIP
    case QEvent::ToolTip: {
        QToolTip::showText(event->globalPos(), mToolTip, view);
        return true;
    }
#endif
#ifndef QT_NO_WHATSTHIS
    case QEvent::QueryWhatsThis:
        return true;
    case QEvent::WhatsThis: {
        QWhatsThis::showText(event->globalPos(), mWhatsThis, view);
        return true;
    }
#endif
    default:
        break;
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}
