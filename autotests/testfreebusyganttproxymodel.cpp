/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testfreebusyganttproxymodel.h"
#include "freebusyganttproxymodel.h"

#include <CalendarSupport/FreeBusyItem>
#include <CalendarSupport/FreeBusyItemModel>

#include <KCalendarCore/Attendee>
#include <KGanttGraphicsView>

#include <QAbstractItemModelTester>
#include <QStandardPaths>
#include <QTest>
#include <QTimeZone>
QTEST_MAIN(FreeBusyGanttProxyModelTest)

using namespace IncidenceEditorNG;
using namespace Qt::Literals::StringLiterals;
void FreeBusyGanttProxyModelTest::initTestCase()
{
    qputenv("TZ", "UTC");
    QStandardPaths::setTestModeEnabled(true);
}

void FreeBusyGanttProxyModelTest::testModelValidity()
{
    auto fbModel = new CalendarSupport::FreeBusyItemModel();
    auto ganttModel = new FreeBusyGanttProxyModel();
    ganttModel->setSourceModel(fbModel);
    auto modelTest = new QAbstractItemModelTester(ganttModel);

    Q_UNUSED(modelTest)

    QVERIFY(ganttModel->rowCount() == 0);

    const QDateTime dt1(QDate(2010, 8, 24), QTime(7, 0, 0), QTimeZone::utc());
    const QDateTime dt2(QDate(2010, 8, 24), QTime(16, 0, 0), QTimeZone::utc());
    KCalendarCore::Attendee const a1(u"fred"_s, QStringLiteral("fred@example.com"));
    KCalendarCore::FreeBusy::Ptr const fb1(new KCalendarCore::FreeBusy());

    fb1->addPeriod(dt1, KCalendarCore::Duration(60 * 60));
    fb1->addPeriod(dt2, KCalendarCore::Duration(60 * 60));

    CalendarSupport::FreeBusyItem::Ptr const item1(new CalendarSupport::FreeBusyItem(a1, nullptr));
    item1->setFreeBusy(fb1);

    const QDateTime dt3(QDate(2010, 8, 25), QTime(7, 0, 0), QTimeZone::utc());
    const QDateTime dt4(QDate(2010, 8, 25), QTime(16, 0, 0), QTimeZone::utc());
    KCalendarCore::Attendee const a2(u"joe"_s, QStringLiteral("joe@example.com"));
    KCalendarCore::FreeBusy::Ptr const fb2(new KCalendarCore::FreeBusy());

    fb2->addPeriod(dt3, KCalendarCore::Duration(60 * 60));
    fb2->addPeriod(dt4, KCalendarCore::Duration(60 * 60));

    CalendarSupport::FreeBusyItem::Ptr const item2(new CalendarSupport::FreeBusyItem(a2, nullptr));
    item2->setFreeBusy(fb2);

    fbModel->addItem(item1);
    fbModel->addItem(item2);

    QCOMPARE(ganttModel->rowCount(), 2);

    QModelIndex const parent0 = ganttModel->index(0, 0);
    QModelIndex const parent1 = ganttModel->index(1, 0);
    QModelIndex const parent2 = ganttModel->index(2, 0);
    QVERIFY(parent0.isValid());
    QVERIFY(parent1.isValid());
    QVERIFY(parent2.isValid() == false);

    QModelIndex const source_parent0 = fbModel->index(0, 0);
    QCOMPARE(parent0.data(), source_parent0.data());
    QCOMPARE(parent0.data(KGantt::ItemTypeRole).toInt(), (int)KGantt::TypeMulti);

    QModelIndex const source_parent1 = fbModel->index(1, 0);
    QCOMPARE(parent1.data(), source_parent1.data());
    QCOMPARE(parent1.data(KGantt::ItemTypeRole).toInt(), (int)KGantt::TypeMulti);

    QModelIndex const child0_0 = ganttModel->index(0, 0, parent0);
    QModelIndex const child0_1 = ganttModel->index(1, 0, parent0);
    QVERIFY(child0_0.isValid());
    QVERIFY(child0_1.isValid());

    QCOMPARE(child0_0.data(KGantt::ItemTypeRole).toInt(), (int)KGantt::TypeTask);
    QCOMPARE(child0_0.data(KGantt::StartTimeRole).toDateTime(), dt1);
    QCOMPARE(child0_1.data(KGantt::ItemTypeRole).toInt(), (int)KGantt::TypeTask);
    QCOMPARE(child0_1.data(KGantt::StartTimeRole).toDateTime(), dt2);

    QModelIndex const child1_0 = ganttModel->index(0, 0, parent1);
    QModelIndex const child1_1 = ganttModel->index(1, 0, parent1);
    QVERIFY(child1_0.isValid());
    QVERIFY(child1_1.isValid());

    QCOMPARE(child1_0.data(KGantt::ItemTypeRole).toInt(), (int)KGantt::TypeTask);
    QCOMPARE(child1_0.data(KGantt::StartTimeRole).toDateTime(), dt3);
    QCOMPARE(child1_1.data(KGantt::ItemTypeRole).toInt(), (int)KGantt::TypeTask);
    QCOMPARE(child1_1.data(KGantt::StartTimeRole).toDateTime(), dt4);
    delete fbModel;
    delete ganttModel;
    delete modelTest;
}

#include "moc_testfreebusyganttproxymodel.cpp"
