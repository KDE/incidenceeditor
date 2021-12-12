/*
  SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "ktimezonecomboboxtest.h"
#include "../src/ktimezonecombobox.h"

#include "qtest.h"

#include <QTimeZone>

QTEST_MAIN(KTimeZoneComboBoxTest)

void KTimeZoneComboBoxTest::test_timeSpec()
{
    IncidenceEditorNG::KTimeZoneComboBox combo;
    combo.selectLocalTimeZone();
    QVERIFY(!combo.isFloating());
    if (combo.selectedTimeZone().id() != "UTC") {
        QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());
    } else {
        QCOMPARE(combo.selectedTimeZone(), QTimeZone::utc());
    }

    combo.selectTimeZone(QTimeZone());
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());

    combo.setFloating(true);
    QVERIFY(combo.isFloating());
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());
}

void KTimeZoneComboBoxTest::test_dateTime()
{
    IncidenceEditorNG::KTimeZoneComboBox combo;

    // Floating
    QDateTime dt(QDate(2021, 12, 12), QTime(12, 0, 0));
    QCOMPARE(dt.timeSpec(), Qt::LocalTime);
    combo.selectTimeZoneFor(dt);
    QVERIFY(combo.isFloating());

    // Non-floating
    const QDateTime dtParis(QDate(2021, 12, 12), QTime(12, 0, 0), QTimeZone("Europe/Paris"));
    QCOMPARE(dtParis.timeSpec(), Qt::TimeZone);
    combo.selectTimeZoneFor(dtParis);
    QVERIFY(!combo.isFloating());
    QCOMPARE(combo.selectedTimeZone().id(), "Europe/Paris");
    combo.applyTimeZoneTo(dt);
    QCOMPARE(dt.timeSpec(), Qt::TimeZone);
    QCOMPARE(dt.timeZone().id(), "Europe/Paris");
}
