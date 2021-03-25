/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <CalendarSupport/FreeBusyItem>

#include <KCalendarCore/Attendee>
#include <KCalendarCore/FreeBusy>

#include <QObject>

namespace IncidenceEditorNG
{
class ConflictResolver;
}

class ConflictResolverTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void simpleTest();
    void stillPrettySimpleTest();
    void akademy2010();
    void testPeriodBeginsBeforeTimeframeBegins();
    void testPeriodEndsAfterTimeframeEnds();
    void testPeriodIsLargerThenTimeframe();
    void testPeriodEndsAtSametimeAsTimeframe();

private:
    void insertAttendees();
    void
    addAttendee(const QString &email, const KCalendarCore::FreeBusy::Ptr &fb, KCalendarCore::Attendee::Role role = KCalendarCore::Attendee::ReqParticipant);
    QList<CalendarSupport::FreeBusyItem::Ptr> attendees;
    QWidget *parent;
    IncidenceEditorNG::ConflictResolver *resolver;
    QDateTime base, end;
};

