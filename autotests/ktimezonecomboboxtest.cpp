/*
  SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "ktimezonecomboboxtest.h"
#include "../src/ktimezonecombobox.h"

#include "qtest.h"

QTEST_MAIN(KTimeZoneComboBoxTest)

void KTimeZoneComboBoxTest::test_timeSpec()
{
    IncidenceEditorNG::KTimeZoneComboBox combo;
    combo.selectLocalTimeZone();
    if (combo.selectedTimeZone().id() != "UTC") {
        QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());
    } else {
        QCOMPARE(combo.selectedTimeZone(), QTimeZone::utc());
    }

    combo.selectTimeZone(QTimeZone());
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());

    combo.setFloating(true);
    QCOMPARE(combo.selectedTimeZone(), QTimeZone::systemTimeZone());
}
