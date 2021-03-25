/*
    SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
    SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Libkdepim/KCheckComboBox>

#include <QBitArray>
#include <QDate>

namespace IncidenceEditorNG
{
// FIXME: This class assumes all weeks have 7 days. We should use KCalenderSystem instead.
/**
 * A combobox that is populated with the days of the week from the current
 * KCalenderSystem. The days are checkable.
 * @note: KCalenderSystem numbers weekdays starting with 1, however this widget is 0 indexed and handles the conversion to the 1 based system internally. Use
 * this widget as a normal 0 indexed container.
 * @see KCalenderSystem
 */
class KWeekdayCheckCombo : public KPIM::KCheckComboBox
{
    Q_OBJECT
public:
    /**
     * @param first5Checked if true the first 5 weekdays will be checked by default
     */
    explicit KWeekdayCheckCombo(QWidget *parent = nullptr, bool first5Checked = false);
    ~KWeekdayCheckCombo() override;

    /**
     * Retrieve the checked days
     * @param days a 7 bit array indicating the checked days (bit 0 = Monday, value 1 = checked).
     */
    Q_REQUIRED_RESULT QBitArray days() const;

    /**
     * Set the checked days on this combobox
     * @param days a 7 bit array indicating the days to check/uncheck (bit 0 = Monday, value 1 = check).
     * @param disableDays if not empty, the corresponding days will be disabled, all others enabled (bit 0 = Monday, value 1 = disable).
     * @see days()
     */
    void setDays(const QBitArray &days, const QBitArray &disableDays = QBitArray());

    /**
     * Returns the index of the weekday represented by the
     * QDate object.
     */
    int weekdayIndex(const QDate &date) const;
};
}
