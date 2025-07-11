/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor-ng.h"

#include <KLocalizedString>
#include <QDate>
namespace Ui
{
class EventOrTodoDesktop;
}

namespace IncidenceEditorNG
{
class IncidenceDateTime;

/// Keep this in sync with the values in mUi->mRecurrenceTypeCombo
enum RecurrenceType {
    RecurrenceTypeNone = 0,
    RecurrenceTypeDaily,
    RecurrenceTypeWeekly,
    RecurrenceTypeMonthly,
    RecurrenceTypeYearly,
    RecurrenceTypeUnknown, // keep this one at the end of the ones which are also in the combobox
    RecurrenceTypeException
};

class IncidenceRecurrence : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual

    IncidenceRecurrence(IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui);

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    [[nodiscard]] bool isDirty() const override;
    [[nodiscard]] bool isValid() const override;

    void focusInvalidField() override;

    [[nodiscard]] RecurrenceType currentRecurrenceType() const;

Q_SIGNALS:
    void recurrenceChanged(IncidenceEditorNG::RecurrenceType type);

private:
    void addException();
    void fillCombos();
    void handleDateTimeToggle();
    void handleEndAfterOccurrencesChange(int currentValue);
    void handleExceptionDateChange(const QDate &currentDate);
    void handleFrequencyChange();
    void handleRecurrenceTypeChange(int currentIndex);
    void removeExceptions();
    void updateRemoveExceptionButton();
    void updateWeekDays(const QDate &newStartDate);
    void handleStartDateChange(const QDate &);

    /**
       I needed save() to be const, so created this func.
       save() calls this now, and changes members outside.
    */
    void writeToIncidence(const KCalendarCore::Incidence::Ptr &incidence) const;

    [[nodiscard]] KLocalizedString subsOrdinal(const KLocalizedString &text, int number) const;
    /**
     * Return the day in the month/year on which the event recurs, starting at the
     * beginning/end. Both return a positive number.
     */
    [[nodiscard]] short dayOfMonthFromStart() const;
    [[nodiscard]] short dayOfMonthFromEnd() const;
    [[nodiscard]] short dayOfYearFromStart() const; // We don't need from end for year
    [[nodiscard]] int duration() const;

    /** Returns the week number (1-5) of the month in which the start date occurs. */
    [[nodiscard]] short monthWeekFromStart() const;
    [[nodiscard]] short monthWeekFromEnd() const;

    /** DO NOT USE THIS METHOD DIRECTLY
        use subsOrdinal() instead for i18n * */
    [[nodiscard]] QString numberToString(int number) const;
    void selectMonthlyItem(KCalendarCore::Recurrence *recurrence, ushort recurenceType);
    void selectYearlyItem(KCalendarCore::Recurrence *recurrence, ushort recurenceType);
    void setDefaults();
    void setDuration(int duration);
    void setExceptionDates(const KCalendarCore::DateList &dates);
    void setExceptionDateTimes(const KCalendarCore::DateTimeList &dateTimes);
    void setFrequency(int freq);
    void toggleRecurrenceWidgets(int recurrenceType);
    /** Returns an array with the weekday on which the event occurs set to 1 */
    [[nodiscard]] QBitArray weekday() const;

    /**
     * Return how many times the weekday represented by @param date occurs in
     * the month of @param date.
     */
    [[nodiscard]] int weekdayCountForMonth(const QDate &date) const;

    [[nodiscard]] QDate currentDate() const;

private:
    Ui::EventOrTodoDesktop *const mUi;
    QDate mCurrentDate;
    IncidenceDateTime *const mDateTime;
    KCalendarCore::DateList mExceptionDates;

    // So we can easily detect if the user changed the type,
    // without going through complicated recurrence logic:
    int mMonthlyInitialType = -1;
    int mYearlyInitialType = -1;
};
}
