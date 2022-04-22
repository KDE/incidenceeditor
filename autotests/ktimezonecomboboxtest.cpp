/*
  SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "ktimezonecomboboxtest.h"
#include "../src/ktimezonecombobox.h"

#include "qtest.h"

#include <QTimeZone>

QTEST_MAIN(KTimeZoneComboBoxTest)

const auto TEST_TZ = "Asia/Tokyo";      // Not UTC, not Paris.

void KTimeZoneComboBoxTest::initTestCase()
{
    qputenv("TZ", TEST_TZ);
}

void KTimeZoneComboBoxTest::test_timeSpec()
{
    IncidenceEditorNG::KTimeZoneComboBox combo;
    combo.selectLocalTimeZone();
    QVERIFY(!combo.isFloating());
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());

    combo.selectTimeZone(QTimeZone());
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());

    combo.setFloating(true);
    QVERIFY(combo.isFloating());
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());
}

void KTimeZoneComboBoxTest::test_selectTimeZoneFor()
{
    IncidenceEditorNG::KTimeZoneComboBox combo;

    // Floating
    QDateTime dt(QDate(2021, 12, 12), QTime(12, 0, 0));
    QCOMPARE(dt.timeSpec(), Qt::LocalTime);
    combo.selectTimeZoneFor(dt);
    QVERIFY(combo.isFloating());

    // System time zone.
    QDateTime dtSys(QDate(2021, 12, 12), QTime(12, 0, 0), QTimeZone::systemTimeZone());
    combo.selectTimeZoneFor(dtSys);
    QVERIFY(!combo.isFloating());
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());

    // UTC.
    QDateTime dtUtc = QDateTime::currentDateTimeUtc();
    combo.selectTimeZoneFor(dtUtc);
    QVERIFY(!combo.isFloating());
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::utc());

    // General case.
    const QDateTime dtParis(QDate(2021, 12, 12), QTime(12, 0, 0), QTimeZone("Europe/Paris"));
    QCOMPARE(dtParis.timeSpec(), Qt::TimeZone);
    combo.selectTimeZoneFor(dtParis);
    QVERIFY(!combo.isFloating());
    QCOMPARE(combo.selectedTimeZone().id(), "Europe/Paris");
}

void KTimeZoneComboBoxTest::test_applyTimeZoneTo()
{
    IncidenceEditorNG::KTimeZoneComboBox combo;
    QDateTime dt = QDateTime::currentDateTime();

    combo.selectTimeZoneFor(QDateTime(QDate(2021, 12, 12), QTime(12, 0, 0), Qt::LocalTime));
    combo.applyTimeZoneTo(dt);
    QCOMPARE(dt.timeSpec(), Qt::LocalTime);

    combo.selectTimeZoneFor(QDateTime(QDate(2021, 12, 12), QTime(12, 0, 0), QTimeZone::systemTimeZone()));
    combo.applyTimeZoneTo(dt);
    QCOMPARE(dt.timeSpec(), Qt::TimeZone);
    QCOMPARE(dt.timeZone(), QTimeZone::systemTimeZone());

    combo.selectTimeZoneFor(QDateTime::currentDateTimeUtc());
    combo.applyTimeZoneTo(dt);
    QCOMPARE(dt.timeSpec(), Qt::TimeZone);
    QCOMPARE(dt.timeZone(), QTimeZone::utc());

    combo.selectTimeZoneFor(QDateTime(QDate(2021, 12, 12), QTime(12, 0, 0), QTimeZone("Europe/Paris")));
    combo.applyTimeZoneTo(dt);
    QCOMPARE(dt.timeSpec(), Qt::TimeZone);
    QCOMPARE(dt.timeZone().id(), "Europe/Paris");
}

/**
 * For the user's convenience, the first three items are the system time zone,
 * "floating", and UTC.
 */
void KTimeZoneComboBoxTest::test_convenience()
{
    IncidenceEditorNG::KTimeZoneComboBox combo;
    combo.setCurrentIndex(0);
    QVERIFY(!combo.isFloating());
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());

    combo.setCurrentIndex(1);
    QVERIFY(combo.isFloating());

    combo.setCurrentIndex(2);
    QVERIFY(!combo.isFloating());
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::utc());
}
