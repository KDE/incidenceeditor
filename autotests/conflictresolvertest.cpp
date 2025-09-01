/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "conflictresolvertest.h"
#include "conflictresolver.h"

#include <KCalendarCore/Duration>
#include <KCalendarCore/Event>
#include <KCalendarCore/Period>

#include <QTest>
#include <QWidget>

using namespace IncidenceEditorNG;
using namespace Qt::Literals::StringLiterals;
void ConflictResolverTest::insertAttendees()
{
    for (const CalendarSupport::FreeBusyItem::Ptr &item : std::as_const(attendees)) {
        resolver->insertAttendee(item);
    }
}

void ConflictResolverTest::addAttendee(const QString &email, const KCalendarCore::FreeBusy::Ptr &fb, KCalendarCore::Attendee::Role role)
{
    QString const name = u"attendee %1"_s.arg(attendees.count());
    CalendarSupport::FreeBusyItem::Ptr const item(
        new CalendarSupport::FreeBusyItem(KCalendarCore::Attendee(name, email, false, KCalendarCore::Attendee::Accepted, role), nullptr));
    item->setFreeBusy(KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(*fb.data())));
    attendees << item;
}

void ConflictResolverTest::initTestCase()
{
    parent = new QWidget;
    init();
}

void ConflictResolverTest::init()
{
    base = QDateTime::currentDateTime().addDays(1);
    end = base.addSecs(10 * 60 * 60);
    resolver = new ConflictResolver(parent, parent);
}

void ConflictResolverTest::cleanup()
{
    delete resolver;
    resolver = nullptr;
    attendees.clear();
}

void ConflictResolverTest::simpleTest()
{
    KCalendarCore::Period const meeting(end.addSecs(-3 * 60 * 60), KCalendarCore::Duration(2 * 60 * 60));
    addAttendee(u"albert@einstein.net"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << meeting)));

    insertAttendees();

    static const int resolution = 15 * 60;
    resolver->setResolution(resolution);
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QCOMPARE(resolver->availableSlots().size(), 2);

    KCalendarCore::Period const first = resolver->availableSlots().at(0);
    QCOMPARE(first.start(), base);
    QCOMPARE(first.end(), meeting.start());

    KCalendarCore::Period const second = resolver->availableSlots().at(1);
    QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Continue);
    QCOMPARE(second.start(), meeting.end().addSecs(resolution)); // add 15 minutes because the
    // free block doesn't start until
    // the next timeslot
    QCOMPARE(second.end(), end);
}

void ConflictResolverTest::stillPrettySimpleTest()
{
    KCalendarCore::Period const meeting1(base, KCalendarCore::Duration(2 * 60 * 60));
    KCalendarCore::Period const meeting2(base.addSecs(60 * 60), KCalendarCore::Duration(2 * 60 * 60));
    KCalendarCore::Period const meeting3(end.addSecs(-3 * 60 * 60), KCalendarCore::Duration(2 * 60 * 60));
    addAttendee(u"john.f@kennedy.com"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << meeting1 << meeting3)));
    addAttendee(u"elvis@rock.com"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << meeting2 << meeting3)));
    addAttendee(u"albert@einstein.net"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << meeting3)));

    insertAttendees();

    static const int resolution = 15 * 60;
    resolver->setResolution(resolution);
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QCOMPARE(resolver->availableSlots().size(), 2);

    KCalendarCore::Period const first = resolver->availableSlots().at(0);
    QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Continue);
    QCOMPARE(first.start(), meeting2.end().addSecs(resolution));
    QCOMPARE(first.end(), meeting3.start());

    KCalendarCore::Period const second = resolver->availableSlots().at(1);
    QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Continue);
    QCOMPARE(second.start(), meeting3.end().addSecs(resolution)); // add 15 minutes because the
    // free block doesn't start until
    // the next timeslot
    QCOMPARE(second.end(), end);
}

#define _time(h, m) QDateTime(base.date(), QTime(h, m))

void ConflictResolverTest::akademy2010()
{
    // based off akademy 2010 schedule

    // first event was at 9:30, so lets align our start time there
    base.setTime(QTime(9, 30));
    end = base.addSecs(8 * 60 * 60);
    KCalendarCore::Period const opening(_time(9, 30), _time(9, 45));
    KCalendarCore::Period const keynote(_time(9, 45), _time(10, 30));

    KCalendarCore::Period const sevenPrinciples(_time(10, 30), _time(11, 15));
    KCalendarCore::Period const commAsService(_time(10, 30), _time(11, 15));

    KCalendarCore::Period const kdeForums(_time(11, 15), _time(11, 45));
    KCalendarCore::Period const oviStore(_time(11, 15), _time(11, 45));

    // 10 min break

    KCalendarCore::Period const highlights(_time(12, 0), _time(12, 45));
    KCalendarCore::Period const styles(_time(12, 0), _time(12, 45));

    KCalendarCore::Period const wikimedia(_time(12, 45), _time(13, 15));
    KCalendarCore::Period const avalanche(_time(12, 45), _time(13, 15));

    KCalendarCore::Period const pimp(_time(13, 15), _time(13, 45));
    KCalendarCore::Period const direction(_time(13, 15), _time(13, 45));

    // lunch 1 hr 25 min lunch

    KCalendarCore::Period const blurr(_time(15, 15), _time(16, 00));
    KCalendarCore::Period const plasma(_time(15, 15), _time(16, 00));

    //  for ( int i = 1; i < 80; ++i ) {
    // adds 80 people (adds the same 8 people 10 times)
    addAttendee(
        u"akademyattendee1@email.com"_s,
        KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << keynote << oviStore << wikimedia << direction)));
    addAttendee(
        u"akademyattendee2@email.com"_s,
        KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << keynote << commAsService << highlights << pimp)));
    addAttendee(u"akademyattendee3@email.com"_s,
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << kdeForums << styles << pimp << plasma)));
    addAttendee(u"akademyattendee4@email.com"_s,
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << keynote << oviStore << pimp << blurr)));
    addAttendee(u"akademyattendee5@email.com"_s,
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << keynote << oviStore << highlights << avalanche)));
    addAttendee(u"akademyattendee6@email.com"_s,
                KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << keynote << commAsService << highlights)));
    addAttendee(u"akademyattendee7@email.com"_s,
                KCalendarCore::FreeBusy::Ptr(
                    new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << kdeForums << styles << avalanche << pimp << plasma)));
    addAttendee(
        u"akademyattendee8@email.com"_s,
        KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << opening << keynote << oviStore << wikimedia << blurr)));
    //  }

    insertAttendees();

    const int resolution = 5 * 60;
    resolver->setResolution(resolution);
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    // QBENCHMARK {
    resolver->findAllFreeSlots();
    // }

    QCOMPARE(resolver->availableSlots().size(), 3);
    QEXPECT_FAIL("", "Got broken in revision f17b9a8c975588ad7cf4ce8b94ab8e32ac193ed8", Abort);
    QCOMPARE(resolver->availableSlots().at(0).duration(), KCalendarCore::Duration(10 * 60));
    QCOMPARE(resolver->availableSlots().at(1).duration(), KCalendarCore::Duration(1 * 60 * 60 + 25 * 60));
    QVERIFY(resolver->availableSlots().at(2).start() > plasma.end());
}

void ConflictResolverTest::testPeriodIsLargerThenTimeframe()
{
    base.setDate(QDate(2010, 7, 29));
    base.setTime(QTime(7, 30));

    end.setDate(QDate(2010, 7, 29));
    end.setTime(QTime(8, 30));

    KCalendarCore::Period const testEvent(_time(5, 45), _time(8, 45));

    addAttendee(u"kdabtest1@demo.kolab.org"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << testEvent)));
    addAttendee(u"kdabtest2@demo.kolab.org"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List())));

    insertAttendees();
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QCOMPARE(resolver->availableSlots().size(), 0);
}

void ConflictResolverTest::testPeriodBeginsBeforeTimeframeBegins()
{
    base.setDate(QDate(2010, 7, 29));
    base.setTime(QTime(7, 30));

    end.setDate(QDate(2010, 7, 29));
    end.setTime(QTime(9, 30));

    KCalendarCore::Period const testEvent(_time(5, 45), _time(8, 45));

    addAttendee(u"kdabtest1@demo.kolab.org"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << testEvent)));
    addAttendee(u"kdabtest2@demo.kolab.org"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List())));

    insertAttendees();
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QCOMPARE(resolver->availableSlots().size(), 1);
    KCalendarCore::Period const freeslot = resolver->availableSlots().at(0);
    QCOMPARE(freeslot.start(), _time(8, 45));
    QCOMPARE(freeslot.end(), end);
}

void ConflictResolverTest::testPeriodEndsAfterTimeframeEnds()
{
    base.setDate(QDate(2010, 7, 29));
    base.setTime(QTime(7, 30));

    end.setDate(QDate(2010, 7, 29));
    end.setTime(QTime(9, 30));

    KCalendarCore::Period const testEvent(_time(8, 00), _time(9, 45));

    addAttendee(u"kdabtest1@demo.kolab.org"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << testEvent)));
    addAttendee(u"kdabtest2@demo.kolab.org"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List())));

    insertAttendees();
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QCOMPARE(resolver->availableSlots().size(), 1);
    KCalendarCore::Period const freeslot = resolver->availableSlots().at(0);
    QCOMPARE(freeslot.duration(), KCalendarCore::Duration(30 * 60));
    QCOMPARE(freeslot.start(), base);
    QCOMPARE(freeslot.end(), _time(8, 00));
}

void ConflictResolverTest::testPeriodEndsAtSametimeAsTimeframe()
{
    base.setDate(QDate(2010, 7, 29));
    base.setTime(QTime(7, 45));

    end.setDate(QDate(2010, 7, 29));
    end.setTime(QTime(8, 45));

    KCalendarCore::Period const testEvent(_time(5, 45), _time(8, 45));

    addAttendee(u"kdabtest1@demo.kolab.org"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List() << testEvent)));
    addAttendee(u"kdabtest2@demo.kolab.org"_s, KCalendarCore::FreeBusy::Ptr(new KCalendarCore::FreeBusy(KCalendarCore::Period::List())));

    insertAttendees();
    resolver->setEarliestDateTime(base);
    resolver->setLatestDateTime(end);
    resolver->findAllFreeSlots();

    QCOMPARE(resolver->availableSlots().size(), 0);
}

QTEST_MAIN(ConflictResolverTest)

#include "moc_conflictresolvertest.cpp"
