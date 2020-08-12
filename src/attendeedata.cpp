/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "attendeedata.h"

using namespace IncidenceEditorNG;

void AttendeeData::clear()
{
    setName(QString());
    setEmail(QString());
    setRole(Attendee::ReqParticipant);
    setStatus(Attendee::None);
    setRSVP(false);
    setUid(QString());
}

bool AttendeeData::isEmpty() const
{
    return name().isEmpty() && email().isEmpty();
}

KCalendarCore::Attendee AttendeeData::attendee() const
{
    return KCalendarCore::Attendee(*this);
}
