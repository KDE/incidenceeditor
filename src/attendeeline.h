/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef INCIDENCEEDITOR_ATTENDEELINE_H
#define INCIDENCEEDITOR_ATTENDEELINE_H

#include <LibkdepimAkonadi/AddresseeLineEdit>
#include <Libkdepim/MultiplyingLine>

#include <KCalCore/Attendee>

#include <QCheckBox>
#include <QToolButton>

class QKeyEvent;

namespace IncidenceEditorNG {
class AttendeeData;

class AttendeeComboBox : public QToolButton
{
    Q_OBJECT
public:
    explicit AttendeeComboBox(QWidget *parent);

    void addItem(const QIcon &icon, const QString &text);
    void addItems(const QStringList &texts);

    int currentIndex() const;

Q_SIGNALS:
    void rightPressed();
    void leftPressed();
    void itemChanged();

public Q_SLOTS:
    /** Clears the combobox, removing all items. */
    void clear();
    void setCurrentIndex(int index);

protected:
    void keyPressEvent(QKeyEvent *ev) override;

private:
    void slotActionTriggered();
    QMenu *mMenu = nullptr;
    QVector<QPair<QString, QIcon> > mList;
    int mCurrentIndex;
};

class AttendeeLineEdit : public KPIM::AddresseeLineEdit
{
    Q_OBJECT
public:
    explicit AttendeeLineEdit(QWidget *parent);

Q_SIGNALS:
    void deleteMe();
    void leftPressed();
    void rightPressed();
    void upPressed();
    void downPressed();

protected:
    void keyPressEvent(QKeyEvent *ev) override;
};

class AttendeeLine : public KPIM::MultiplyingLine
{
    Q_OBJECT
public:
    enum AttendeeActions {
        EventActions,
        TodoActions
    };

    explicit AttendeeLine(QWidget *parent);
    virtual ~AttendeeLine()
    {
    }

    void activate() override;
    bool isActive() const override;

    bool isEmpty() const override;
    void clear() override;

    bool isModified() const override;
    void clearModified() override;

    KPIM::MultiplyingLineData::Ptr data() const override;
    void setData(const KPIM::MultiplyingLineData::Ptr &data) override;

    void fixTabOrder(QWidget *previous) override;
    QWidget *tabOut() const override;

    void setCompletionMode(KCompletion::CompletionMode) override;

    int setColumnWidth(int w) override;

    void aboutToBeDeleted() override;
    bool canDeleteLineEdit() const override;

    void setActions(AttendeeActions actions);

Q_SIGNALS:
    void changed();
    void changed(const KCalCore::Attendee::Ptr &oldAttendee, const KCalCore::Attendee::Ptr &newAttendee);
    void editingFinished(KPIM::MultiplyingLine *);

private:
    void slotTextChanged(const QString &);
    void slotHandleChange();
    void slotComboChanged();
    void dataFromFields();
    void fieldsFromData();

    AttendeeComboBox *mRoleCombo = nullptr;
    AttendeeComboBox *mStateCombo = nullptr;
    AttendeeComboBox *mResponseCombo = nullptr;
    AttendeeLineEdit *mEdit = nullptr;
    QSharedPointer<AttendeeData> mData;
    QString mUid;
    bool mModified;
};
}

#endif
