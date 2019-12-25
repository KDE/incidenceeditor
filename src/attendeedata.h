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

#ifndef INCIDENCEEDITOR_ATTENDEEDATA_H
#define INCIDENCEEDITOR_ATTENDEEDATA_H

#include <Libkdepim/MultiplyingLine>

#include <KCalendarCore/Attendee>

namespace IncidenceEditorNG {
class AttendeeData : public KPIM::MultiplyingLineData, public KCalendarCore::Attendee
{
public:
    typedef QSharedPointer<AttendeeData> Ptr;
    typedef QList<AttendeeData::Ptr> List;

    AttendeeData(const QString &name, const QString &email, bool rsvp = false, Attendee::PartStat status = Attendee::None, Attendee::Role role = Attendee::ReqParticipant, const QString &uid = QString())
        : KCalendarCore::Attendee(name, email, rsvp, status, role, uid)
    {
    }

    explicit AttendeeData(const KCalendarCore::Attendee &attendee) : KCalendarCore::Attendee(attendee)
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

#endif // INCIDENCEEDITOR_ATTENDEEDATA_H
