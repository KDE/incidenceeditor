/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Libkdepim/MultiplyingLine>

#include <KCalendarCore/Attendee>

namespace IncidenceEditorNG
{
class AttendeeData : public KPIM::MultiplyingLineData, public KCalendarCore::Attendee
{
public:
    using Ptr = QSharedPointer<AttendeeData>;
    using List = QList<AttendeeData::Ptr>;

    AttendeeData(const QString &name,
                 const QString &email,
                 bool rsvp = false,
                 Attendee::PartStat status = Attendee::None,
                 Attendee::Role role = Attendee::ReqParticipant,
                 const QString &uid = QString())
        : KCalendarCore::Attendee(name, email, rsvp, status, role, uid)
    {
    }

    explicit AttendeeData(const KCalendarCore::Attendee &attendee)
        : KCalendarCore::Attendee(attendee)
    {
    }

    void clear() override;
    Q_REQUIRED_RESULT bool isEmpty() const override;

    /**
     * Return a copy of the attendee data
     */
    Q_REQUIRED_RESULT KCalendarCore::Attendee attendee() const;
};
}

