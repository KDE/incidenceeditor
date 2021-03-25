/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Libkdepim/MultiplyingLine>
#include <PimCommonAkonadi/AddresseeLineEdit>

#include <KCalendarCore/Attendee>

#include <QCheckBox>
#include <QToolButton>

class QKeyEvent;

namespace IncidenceEditorNG
{
class AttendeeData;

class AttendeeComboBox : public QToolButton
{
    Q_OBJECT
public:
    explicit AttendeeComboBox(QWidget *parent);

    void addItem(const QIcon &icon, const QString &text);
    void addItems(const QStringList &texts);

    Q_REQUIRED_RESULT int currentIndex() const;

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
    QVector<QPair<QString, QIcon>> mList;
    int mCurrentIndex = -1;
};

class AttendeeLineEdit : public PimCommon::AddresseeLineEdit
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
    enum AttendeeActions { EventActions, TodoActions };

    explicit AttendeeLine(QWidget *parent);
    ~AttendeeLine() override
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
    void changed(const KCalendarCore::Attendee &oldAttendee, const KCalendarCore::Attendee &newAttendee);
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

