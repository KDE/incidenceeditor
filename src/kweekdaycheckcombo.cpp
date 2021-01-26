/*
    SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
    SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kweekdaycheckcombo.h"

#include "incidenceeditor_debug.h"

#include <QLocale>

using namespace IncidenceEditorNG;

KWeekdayCheckCombo::KWeekdayCheckCombo(QWidget *parent, bool first5Checked)
    : KCheckComboBox(parent)
{
    const int weekStart = QLocale().firstDayOfWeek();
    QStringList checkedItems;
    for (int i = 0; i < 7; ++i) {
        // i is the nr of the combobox, not the day of week!
        const int dayOfWeek = (i + weekStart + 6) % 7;

        const QString weekDayName = QLocale::system().dayName(dayOfWeek + 1, QLocale::ShortFormat);
        addItem(weekDayName);
        // by default Monday - Friday should be checked
        // which corresponds to index 0 - 4;
        if (first5Checked && dayOfWeek < 5) {
            checkedItems << weekDayName;
        }
    }
    if (first5Checked) {
        setCheckedItems(checkedItems);
    }
}

KWeekdayCheckCombo::~KWeekdayCheckCombo()
{
}

QBitArray KWeekdayCheckCombo::days() const
{
    QBitArray days(7);
    const int weekStart = QLocale().firstDayOfWeek();

    for (int i = 0; i < 7; ++i) {
        // i is the nr of the combobox, not the day of week!
        const int index = (1 + i + (7 - weekStart)) % 7;
        days.setBit(i, itemCheckState(index) == Qt::Checked);
    }

    return days;
}

int KWeekdayCheckCombo::weekdayIndex(const QDate &date) const
{
    if (!date.isValid()) {
        return -1;
    }
    const int weekStart = QLocale().firstDayOfWeek();
    const int dayOfWeek = date.dayOfWeek() - 1; // Values 1 - 7, we need 0 - 6

    // qCDebug(INCIDENCEEDITOR_LOG) << "dayOfWeek = " << dayOfWeek << " weekStart = " << weekStart
    // << "; result " << ( ( dayOfWeek + weekStart ) % 7 ) << "; date = " << date;
    return (1 + dayOfWeek + (7 - weekStart)) % 7;
}

void KWeekdayCheckCombo::setDays(const QBitArray &days, const QBitArray &disableDays)
{
    Q_ASSERT(count() == 7); // The combobox must be filled.

    QStringList checkedDays;
    const int weekStart = QLocale().firstDayOfWeek();
    for (int i = 0; i < 7; ++i) {
        // i is the nr of the combobox, not the day of week!
        const int index = (1 + i + (7 - weekStart)) % 7;

        // qCDebug(INCIDENCEEDITOR_LOG) << "Checking for i = " << i << "; index = " << index << days.testBit( i );
        // qCDebug(INCIDENCEEDITOR_LOG) << "Disabling? for i = " << i << "; index = " << index << !disableDays.testBit( i );

        if (days.testBit(i)) {
            checkedDays << itemText(index);
        }
        if (!disableDays.isEmpty()) {
            setItemEnabled(index, !disableDays.testBit(i));
        }
    }
    setCheckedItems(checkedDays);
}
