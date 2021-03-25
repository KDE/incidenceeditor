/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "attendeedata.h"
#include "attendeeline.h"

#include <Libkdepim/MultiplyingLineEditor>

namespace IncidenceEditorNG
{
class AttendeeLineFactory : public KPIM::MultiplyingLineFactory
{
    Q_OBJECT
public:
    explicit AttendeeLineFactory(QObject *parent)
        : KPIM::MultiplyingLineFactory(parent)
    {
    }

    KPIM::MultiplyingLine *newLine(QWidget *parent) override
    {
        return new AttendeeLine(parent);
    }
};

class AttendeeEditor : public KPIM::MultiplyingLineEditor
{
    Q_OBJECT
public:
    explicit AttendeeEditor(QWidget *parent = nullptr);

    AttendeeData::List attendees() const;

    void addAttendee(const KCalendarCore::Attendee &attendee);
    void removeAttendee(const AttendeeData::Ptr &attendee);

    void setActions(AttendeeLine::AttendeeActions actions);

Q_SIGNALS:
    void countChanged(int);
    void changed(const KCalendarCore::Attendee &oldAttendee, const KCalendarCore::Attendee &newAttendee);
    void editingFinished(KPIM::MultiplyingLine *);

protected Q_SLOTS:
    void slotLineAdded(KPIM::MultiplyingLine *);
    void slotCalculateTotal();
};
}

