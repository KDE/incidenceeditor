/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencerecurrence.h"
#include "incidencedatetime.h"
#include "ui_dialogdesktop.h"

#include "incidenceeditor_debug.h"
#include <QLocale>

using namespace IncidenceEditorNG;

enum {
    // Keep in sync with mRecurrenceEndCombo
    RecurrenceEndNever = 0,
    RecurrenceEndOn,
    RecurrenceEndAfter
};

/**

Description of available recurrence types:

0 - None
1 -
2 -
3 - rDaily
4 - rWeekly
5 - rMonthlyPos  - 3rd Saturday of month, last Wednesday of month...
6 - rMonthlyDay  - 17th day of month
7 - rYearlyMonth - 10th of July
8 - rYearlyDay   - on the 117th day of the year
9 - rYearlyPos   - 1st Wednesday of July
*/

enum {
    // Indexes of the month combo, keep in sync with descriptions.
    ComboIndexMonthlyDay = 0, // 11th of June
    ComboIndexMonthlyDayInverted, // 20th of June ( 11 to end )
    ComboIndexMonthlyPos, // 1st Monday of the Month
    ComboIndexMonthlyPosInverted // Last Monday of the Month
};

enum {
    // Indexes of the year combo, keep in sync with descriptions.
    ComboIndexYearlyMonth = 0,
    ComboIndexYearlyMonthInverted,
    ComboIndexYearlyPos,
    ComboIndexYearlyPosInverted,
    ComboIndexYearlyDay
};

static void setExDateTimesFromExDates(KCalendarCore::Recurrence *r, KCalendarCore::DateList exDates)
{
        KCalendarCore::DateTimeList dts;
        QDateTime dt = r->startDateTime();
        for (const auto &e : exDates) {
            dt.setDate(e);
            dts.append(dt);
        }
        r->setExDateTimes(dts);
}

IncidenceRecurrence::IncidenceRecurrence(IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui)
    : mUi(ui)
    , mDateTime(dateTime)
    , mMonthlyInitialType(0)
    , mYearlyInitialType(0)
{
    setObjectName(QStringLiteral("IncidenceRecurrence"));
    // Set some sane defaults
    mUi->mRecurrenceTypeCombo->setCurrentIndex(RecurrenceTypeNone);
    mUi->mRecurrenceEndCombo->setCurrentIndex(RecurrenceEndNever);
    mUi->mRecurrenceEndStack->setCurrentIndex(0);
    mUi->mRepeatStack->setCurrentIndex(0);
    mUi->mEndDurationEdit->setValue(1);
    handleEndAfterOccurrencesChange(1);
    toggleRecurrenceWidgets(RecurrenceTypeNone);
    fillCombos();
    const QList<QLineEdit *> lineEdits{mUi->mExceptionDateEdit->lineEdit(), mUi->mRecurrenceEndDate->lineEdit()};
    for (QLineEdit *lineEdit : lineEdits) {
        if (lineEdit) {
            lineEdit->setClearButtonEnabled(false);
        }
    }

    connect(mDateTime, &IncidenceDateTime::startDateTimeToggled, this, &IncidenceRecurrence::handleDateTimeToggle);

    connect(mDateTime, &IncidenceDateTime::startDateChanged, this, &IncidenceRecurrence::handleStartDateChange);

    connect(mUi->mExceptionAddButton, &QPushButton::clicked, this, &IncidenceRecurrence::addException);
    connect(mUi->mExceptionRemoveButton, &QPushButton::clicked, this, &IncidenceRecurrence::removeExceptions);
    connect(mUi->mExceptionDateEdit, &KDateComboBox::dateChanged, this, &IncidenceRecurrence::handleExceptionDateChange);
    connect(mUi->mExceptionList, &QListWidget::itemSelectionChanged, this, &IncidenceRecurrence::updateRemoveExceptionButton);
    connect(mUi->mRecurrenceTypeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &IncidenceRecurrence::handleRecurrenceTypeChange);
    connect(mUi->mEndDurationEdit, qOverload<int>(&QSpinBox::valueChanged), this, &IncidenceRecurrence::handleEndAfterOccurrencesChange);
    connect(mUi->mFrequencyEdit, qOverload<int>(&QSpinBox::valueChanged), this, &IncidenceRecurrence::handleFrequencyChange);

    // Check the dirty status when the user changes values.
    connect(mUi->mRecurrenceTypeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &IncidenceRecurrence::checkDirtyStatus);
    connect(mUi->mFrequencyEdit, qOverload<int>(&QSpinBox::valueChanged), this, &IncidenceRecurrence::checkDirtyStatus);
    connect(mUi->mFrequencyEdit, qOverload<int>(&QSpinBox::valueChanged), this, &IncidenceRecurrence::checkDirtyStatus);
    connect(mUi->mWeekDayCombo, &IncidenceEditorNG::KWeekdayCheckCombo::checkedItemsChanged, this, &IncidenceRecurrence::checkDirtyStatus);
    connect(mUi->mMonthlyCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &IncidenceRecurrence::checkDirtyStatus);
    connect(mUi->mYearlyCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &IncidenceRecurrence::checkDirtyStatus);
    connect(mUi->mRecurrenceEndCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &IncidenceRecurrence::checkDirtyStatus);
    connect(mUi->mEndDurationEdit, qOverload<int>(&QSpinBox::valueChanged), this, &IncidenceRecurrence::checkDirtyStatus);
    connect(mUi->mRecurrenceEndDate, &KDateComboBox::dateChanged, this, &IncidenceRecurrence::checkDirtyStatus);
    connect(mUi->mThisAndFutureCheck, &QCheckBox::stateChanged, this, &IncidenceRecurrence::checkDirtyStatus);
}

// this method must be at the top of this file in order to ensure
// that its message to translators appears before any usages of this method.
KLocalizedString IncidenceRecurrence::subsOrdinal(const KLocalizedString &text, int number) const
{
    QString q = i18nc(
        "In several of the messages below, "
        "an ordinal number is substituted into the message. "
        "Translate this as \"0\" if English ordinal suffixes "
        "should be added (1st, 22nd, 123rd); "
        "translate this as \"1\" if just the number itself "
        "should be substituted (1, 22, 123).",
        "0");
    if (q == QLatin1Char('0')) {
        const QString ordinal = numberToString(number);
        return text.subs(ordinal);
    } else {
        return text.subs(number);
    }
}

void IncidenceRecurrence::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_ASSERT(incidence);

    mLoadedIncidence = incidence;
    // We must be sure that the date/time in mDateTime is the correct date time.
    // So don't depend on CombinedIncidenceEditor or whatever external factor to
    // load the date/time before loading the recurrence

    mCurrentDate = mLoadedIncidence->dateTime(KCalendarCore::IncidenceBase::RoleRecurrenceStart).date();

    mDateTime->load(incidence);
    fillCombos();
    setDefaults();

    // This is an exception
    if (mLoadedIncidence->hasRecurrenceId()) {
        handleRecurrenceTypeChange(RecurrenceTypeException);
        mUi->mThisAndFutureCheck->setChecked(mLoadedIncidence->thisAndFuture());
        mWasDirty = false;
        return;
    }

    int f = 0;
    KCalendarCore::Recurrence *r = nullptr;
    if (mLoadedIncidence->recurrenceType() != KCalendarCore::Recurrence::rNone) {
        r = mLoadedIncidence->recurrence();
        f = r->frequency();
    }

    switch (mLoadedIncidence->recurrenceType()) {
    case KCalendarCore::Recurrence::rNone:
        mUi->mRecurrenceTypeCombo->setCurrentIndex(RecurrenceTypeNone);
        handleRecurrenceTypeChange(RecurrenceTypeNone);
        break;
    case KCalendarCore::Recurrence::rDaily:
        mUi->mRecurrenceTypeCombo->setCurrentIndex(RecurrenceTypeDaily);
        handleRecurrenceTypeChange(RecurrenceTypeDaily);
        setFrequency(f);
        break;
    case KCalendarCore::Recurrence::rWeekly: {
        mUi->mRecurrenceTypeCombo->setCurrentIndex(RecurrenceTypeWeekly);
        handleRecurrenceTypeChange(RecurrenceTypeWeekly);
        QBitArray disableDays(7 /*size*/, false /*default value*/);
        // dayOfWeek returns between 1 and 7
        disableDays.setBit(currentDate().dayOfWeek() - 1, true);
        mUi->mWeekDayCombo->setDays(r->days(), disableDays);
        setFrequency(f);
        break;
    }
    case KCalendarCore::Recurrence::rMonthlyPos: // Fall through
    case KCalendarCore::Recurrence::rMonthlyDay:
        mUi->mRecurrenceTypeCombo->setCurrentIndex(RecurrenceTypeMonthly);
        handleRecurrenceTypeChange(RecurrenceTypeMonthly);
        selectMonthlyItem(r, mLoadedIncidence->recurrenceType());
        setFrequency(f);
        break;
    case KCalendarCore::Recurrence::rYearlyMonth: // Fall through
    case KCalendarCore::Recurrence::rYearlyPos: // Fall through
    case KCalendarCore::Recurrence::rYearlyDay:
        mUi->mRecurrenceTypeCombo->setCurrentIndex(RecurrenceTypeYearly);
        handleRecurrenceTypeChange(RecurrenceTypeYearly);
        selectYearlyItem(r, mLoadedIncidence->recurrenceType());
        setFrequency(f);
        break;
    default:
        break;
    }

    if (mLoadedIncidence->recurs() && r) {
        setDuration(r->duration());
        if (r->duration() == 0) {
            mUi->mRecurrenceEndDate->setDate(r->endDate());
        }
    }

    r = mLoadedIncidence->recurrence();
    if (r->allDay()) {
        setExceptionDates(r->exDates());
    } else {
        if (!r->exDateTimes().isEmpty()) {
            setExceptionDateTimes(r->exDateTimes());
        }
        else if (!r->exDates().isEmpty()) {
            // Compatibility: IncidenceEditorNG <= v5.16.3 stored EXDATES as
            // dates only. Upgrade to date-times.
            setExceptionDates(r->exDates());
            setExDateTimesFromExDates(r, r->exDates());
            r->setExDates({});
        }
    }
    handleDateTimeToggle();
    mWasDirty = false;
}

void IncidenceRecurrence::writeToIncidence(const KCalendarCore::Incidence::Ptr &incidence) const
{
    // clear out any old settings;
    KCalendarCore::Recurrence *r = incidence->recurrence();
    r->unsetRecurs(); // Why not clear() ?

    const RecurrenceType recurrenceType = currentRecurrenceType();

    if (recurrenceType == RecurrenceTypeException) {
        incidence->setThisAndFuture(mUi->mThisAndFutureCheck->isChecked());
        return;
    }

    if (recurrenceType == RecurrenceTypeNone || !mUi->mRecurrenceTypeCombo->isEnabled()) {
        return;
    }

    const int lDuration = duration();
    QDate endDate;
    if (lDuration == 0) {
        endDate = mUi->mRecurrenceEndDate->date();
    }

    if (recurrenceType == RecurrenceTypeDaily) {
        r->setDaily(mUi->mFrequencyEdit->value());
    } else if (recurrenceType == RecurrenceTypeWeekly) {
        r->setWeekly(mUi->mFrequencyEdit->value(), mUi->mWeekDayCombo->days());
    } else if (recurrenceType == RecurrenceTypeMonthly) {
        r->setMonthly(mUi->mFrequencyEdit->value());

        if (mUi->mMonthlyCombo->currentIndex() == ComboIndexMonthlyDay) {
            // Every nth
            r->addMonthlyDate(dayOfMonthFromStart());
        } else if (mUi->mMonthlyCombo->currentIndex() == ComboIndexMonthlyDayInverted) {
            // Every (last - n)th last day
            r->addMonthlyDate(-dayOfMonthFromEnd());
        } else if (mUi->mMonthlyCombo->currentIndex() == ComboIndexMonthlyPos) {
            // Every ith weekday
            r->addMonthlyPos(monthWeekFromStart(), weekday());
        } else {
            // Every (last - i)th last weekday
            r->addMonthlyPos(-monthWeekFromEnd(), weekday());
        }
    } else if (recurrenceType == RecurrenceTypeYearly) {
        r->setYearly(mUi->mFrequencyEdit->value());

        if (mUi->mYearlyCombo->currentIndex() == ComboIndexYearlyMonth) {
            // Every nth of month
            r->addYearlyDate(dayOfMonthFromStart());
            r->addYearlyMonth(currentDate().month());
        } else if (mUi->mYearlyCombo->currentIndex() == ComboIndexYearlyMonthInverted) {
            // Every (last - n)th last day of month
            r->addYearlyDate(-dayOfMonthFromEnd());
            r->addYearlyMonth(currentDate().month());
        } else if (mUi->mYearlyCombo->currentIndex() == ComboIndexYearlyPos) {
            // Every ith weekday of month
            r->addYearlyMonth(currentDate().month());
            r->addYearlyPos(monthWeekFromStart(), weekday());
        } else if (mUi->mYearlyCombo->currentIndex() == ComboIndexYearlyPosInverted) {
            // Every (last - i)th last weekday of month
            r->addYearlyMonth(currentDate().month());
            r->addYearlyPos(-monthWeekFromEnd(), weekday());
        } else {
            // The lth day of the year (l : 1 - 366)
            r->addYearlyDay(dayOfYearFromStart());
        }
    }

    r->setDuration(lDuration);
    if (lDuration == 0) {
        r->setEndDate(endDate);
    }

    if (r->allDay()) {
        r->setExDates(mExceptionDates);
    } else {
        setExDateTimesFromExDates(r, mExceptionDates);
    }
}

void IncidenceRecurrence::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    writeToIncidence(incidence);
    mMonthlyInitialType = mUi->mMonthlyCombo->currentIndex();
    mYearlyInitialType = mUi->mYearlyCombo->currentIndex();
}

bool IncidenceRecurrence::isDirty() const
{
    const RecurrenceType recurrenceType = currentRecurrenceType();
    if (mLoadedIncidence->recurs() && recurrenceType == RecurrenceTypeNone) {
        return true;
    }

    if (recurrenceType == RecurrenceTypeException) {
        return mLoadedIncidence->thisAndFuture() != mUi->mThisAndFutureCheck->isChecked();
    }

    if (!mLoadedIncidence->recurs() && recurrenceType != IncidenceEditorNG::RecurrenceTypeNone) {
        return true;
    }

    // The incidence is not recurring and that hasn't changed, so don't check the
    // other values.
    if (recurrenceType == RecurrenceTypeNone) {
        return false;
    }

    const KCalendarCore::Recurrence *recurrence = mLoadedIncidence->recurrence();
    switch (recurrence->recurrenceType()) {
    case KCalendarCore::Recurrence::rDaily:
        if (recurrenceType != RecurrenceTypeDaily || mUi->mFrequencyEdit->value() != recurrence->frequency()) {
            return true;
        }

        break;
    case KCalendarCore::Recurrence::rWeekly:
        if (recurrenceType != RecurrenceTypeWeekly || mUi->mFrequencyEdit->value() != recurrence->frequency()
            || mUi->mWeekDayCombo->days() != recurrence->days()) {
            return true;
        }
        break;
    case KCalendarCore::Recurrence::rMonthlyDay:
        if (recurrenceType != RecurrenceTypeMonthly || mUi->mFrequencyEdit->value() != recurrence->frequency()
            || mUi->mMonthlyCombo->currentIndex() != mMonthlyInitialType) {
            return true;
        }
        break;
    case KCalendarCore::Recurrence::rMonthlyPos:
        if (recurrenceType != RecurrenceTypeMonthly || mUi->mFrequencyEdit->value() != recurrence->frequency()
            || mUi->mMonthlyCombo->currentIndex() != mMonthlyInitialType) {
            return true;
        }
        break;
    case KCalendarCore::Recurrence::rYearlyDay:
        if (recurrenceType != RecurrenceTypeYearly || mUi->mFrequencyEdit->value() != recurrence->frequency()
            || mUi->mYearlyCombo->currentIndex() != mYearlyInitialType) {
            return true;
        }
        break;
    case KCalendarCore::Recurrence::rYearlyMonth:
        if (recurrenceType != RecurrenceTypeYearly || mUi->mFrequencyEdit->value() != recurrence->frequency()
            || mUi->mYearlyCombo->currentIndex() != mYearlyInitialType) {
            return true;
        }
        break;
    case KCalendarCore::Recurrence::rYearlyPos:
        if (recurrenceType != RecurrenceTypeYearly || mUi->mFrequencyEdit->value() != recurrence->frequency()
            || mUi->mYearlyCombo->currentIndex() != mYearlyInitialType) {
            return true;
        }
        break;
    }

    // Recurrence end
    // -1 means "recurs forever"
    if (recurrence->duration() == -1 && mUi->mRecurrenceEndCombo->currentIndex() != RecurrenceEndNever) {
        return true;
    } else if (recurrence->duration() == 0) {
        // 0 means "end date is set"
        if (mUi->mRecurrenceEndCombo->currentIndex() != RecurrenceEndOn || recurrence->endDate() != mUi->mRecurrenceEndDate->date()) {
            return true;
        }
    } else if (recurrence->duration() > 0) {
        if (mUi->mEndDurationEdit->value() != recurrence->duration() || mUi->mRecurrenceEndCombo->currentIndex() != RecurrenceEndAfter) {
            return true;
        }
    }

    // Exception dates
    if (recurrence->allDay()) {
        if (mExceptionDates != recurrence->exDates()) {
            return true;
        }
    } else {
        KCalendarCore::DateList dates;
        for (const auto &dt : recurrence->exDateTimes()) {
            dates.append(dt.date());
        }
        if (mExceptionDates != dates) {
            return true;
        }
    }

    return false;
}

void IncidenceRecurrence::focusInvalidField()
{
    KCalendarCore::Incidence::Ptr incidence(mLoadedIncidence->clone());
    writeToIncidence(incidence);
    if (incidence->recurs()) {
        if (mUi->mRecurrenceEndCombo->currentIndex() == RecurrenceEndOn && !mUi->mRecurrenceEndDate->date().isValid()) {
            mUi->mRecurrenceEndDate->setFocus();
        }
    }
}

bool IncidenceRecurrence::isValid() const
{
    mLastErrorString.clear();
    if (currentRecurrenceType() == IncidenceEditorNG::RecurrenceTypeException) {
        // Nothing you can do wrong here
        return true;
    }
    KCalendarCore::Incidence::Ptr incidence(mLoadedIncidence->clone());

    // Write start and end dates to the incidence
    mDateTime->save(incidence);

    // Write new recurring parameters to incidence
    writeToIncidence(incidence);

    // Check if the incidence will occur at least once
    if (incidence->recurs()) {
        // dtStart for events, dtDue for to-dos
        const QDateTime referenceDate = incidence->dateTime(KCalendarCore::Incidence::RoleRecurrenceStart);

        if (referenceDate.isValid()) {
            if (!(incidence->recurrence()->recursOn(referenceDate.date(), referenceDate.timeZone())
                  || incidence->recurrence()->getNextDateTime(referenceDate).isValid())) {
                mLastErrorString = i18n(
                    "A recurring event or to-do must occur at least once. "
                    "Adjust the recurring parameters.");
                qCDebug(INCIDENCEEDITOR_LOG) << mLastErrorString;
                return false;
            }
        } else {
            mLastErrorString = i18n("The incidence's start date is invalid.");
            qCDebug(INCIDENCEEDITOR_LOG) << mLastErrorString;
            return false;
        }

        if (mUi->mRecurrenceEndCombo->currentIndex() == RecurrenceEndOn && !mUi->mRecurrenceEndDate->date().isValid()) {
            mLastErrorString = i18nc("@info", "The recurrence end date is invalid.");
            qCDebug(INCIDENCEEDITOR_LOG) << mLastErrorString;
            return false;
        }
    }

    return true;
}

void IncidenceRecurrence::addException()
{
    const QDate date = mUi->mExceptionDateEdit->date();
    if (!date.isValid()) {
        qCWarning(INCIDENCEEDITOR_LOG) << "Refusing to add invalid date";
        return;
    }

    const QString dateStr = QLocale().toString(date);
    if (mUi->mExceptionList->findItems(dateStr, Qt::MatchExactly).isEmpty()) {
        mExceptionDates.append(date);
        mUi->mExceptionList->addItem(dateStr);
    }

    mUi->mExceptionAddButton->setEnabled(false);
    checkDirtyStatus();
}

void IncidenceRecurrence::fillCombos()
{
    if (!currentDate().isValid()) {
        // Can happen if you're editing with keyboard
        return;
    }

    // Next the monthly combo. This contains the following elements:
    // - nth day of the month
    // - (month.lastDay() - n)th day of the month
    // - the ith ${weekday} of the month
    // - the (month.weekCount() - i)th day of the month
    const int currentMonthlyIndex = mUi->mMonthlyCombo->currentIndex();
    mUi->mMonthlyCombo->clear();
    const QDate date = mDateTime->startDate();

    QString item = subsOrdinal(ki18nc("example: the 30th", "the %1"), dayOfMonthFromStart()).toString();
    mUi->mMonthlyCombo->addItem(item);

    item = subsOrdinal(ki18nc("example: the 4th to last day", "the %1 to last day"), dayOfMonthFromEnd()).toString();
    mUi->mMonthlyCombo->addItem(item);

    item = subsOrdinal(ki18nc("example: the 5th Wednesday", "the %1 %2"), monthWeekFromStart())
               .subs(QLocale::system().dayName(date.dayOfWeek(), QLocale::QLocale::LongFormat))
               .toString();
    mUi->mMonthlyCombo->addItem(item);

    if (monthWeekFromEnd() == 1) {
        item = ki18nc("example: the last Wednesday", "the last %1").subs(QLocale::system().dayName(date.dayOfWeek(), QLocale::LongFormat)).toString();
    } else {
        item = subsOrdinal(ki18nc("example: the 5th to last Wednesday", "the %1 to last %2"), monthWeekFromEnd())
                   .subs(QLocale::system().dayName(date.dayOfWeek(), QLocale::LongFormat))
                   .toString();
    }
    mUi->mMonthlyCombo->addItem(item);
    mUi->mMonthlyCombo->setCurrentIndex(currentMonthlyIndex == -1 ? 0 : currentMonthlyIndex);

    // Finally the yearly combo. This contains the following options:
    // - ${n}th of ${long-month-name}
    // - ${month.lastDay() - n}th last day of ${long-month-name}
    // - the ${i}th ${weekday} of ${long-month-name}
    // - the ${month.weekCount() - i}th day of ${long-month-name}
    // - the ${m}th day of the year
    const int currentYearlyIndex = mUi->mYearlyCombo->currentIndex();
    mUi->mYearlyCombo->clear();
    const QString longMonthName = QLocale::system().monthName(date.month(), QLocale::LongFormat);
    item = subsOrdinal(ki18nc("example: the 5th of June", "the %1 of %2"), date.day()).subs(longMonthName).toString();
    mUi->mYearlyCombo->addItem(item);

    item = subsOrdinal(ki18nc("example: the 3rd to last day of June", "the %1 to last day of %2"), dayOfMonthFromEnd()).subs(longMonthName).toString();
    mUi->mYearlyCombo->addItem(item);

    item = subsOrdinal(ki18nc("example: the 4th Wednesday of June", "the %1 %2 of %3"), monthWeekFromStart())
               .subs(QLocale::system().dayName(date.dayOfWeek(), QLocale::LongFormat))
               .subs(longMonthName)
               .toString();
    mUi->mYearlyCombo->addItem(item);

    if (monthWeekFromEnd() == 1) {
        item = ki18nc("example: the last Wednesday of June", "the last %1 of %2")
                   .subs(QLocale::system().dayName(date.dayOfWeek(), QLocale::LongFormat))
                   .subs(longMonthName)
                   .toString();
    } else {
        item = subsOrdinal(ki18nc("example: the 4th to last Wednesday of June", "the %1 to last %2 of %3 "), monthWeekFromEnd())
                   .subs(QLocale::system().dayName(date.dayOfWeek(), QLocale::LongFormat))
                   .subs(longMonthName)
                   .toString();
    }
    mUi->mYearlyCombo->addItem(item);

    item = subsOrdinal(ki18nc("example: the 15th day of the year", "the %1 day of the year"), date.dayOfYear()).toString();
    mUi->mYearlyCombo->addItem(item);
    mUi->mYearlyCombo->setCurrentIndex(currentYearlyIndex == -1 ? 0 : currentYearlyIndex);
}

void IncidenceRecurrence::handleDateTimeToggle()
{
    QWidget *parent = mUi->mRepeatStack->parentWidget(); // Take the parent of a toplevel widget;
    if (parent) {
        parent->setEnabled(mDateTime->startDateTimeEnabled());
    }
}

void IncidenceRecurrence::handleEndAfterOccurrencesChange(int currentValue)
{
    mUi->mRecurrenceOccurrencesLabel->setText(i18ncp("Recurrence ends after n occurrences", "occurrence", "occurrences", currentValue));
}

void IncidenceRecurrence::handleExceptionDateChange(const QDate &currentDate)
{
    const QDate date = mUi->mExceptionDateEdit->date();
    const QString dateStr = QLocale().toString(date);

    mUi->mExceptionAddButton->setEnabled(currentDate >= mDateTime->startDate() && mUi->mExceptionList->findItems(dateStr, Qt::MatchExactly).isEmpty());
}

void IncidenceRecurrence::handleFrequencyChange()
{
    handleRecurrenceTypeChange(currentRecurrenceType());
}

void IncidenceRecurrence::handleRecurrenceTypeChange(int currentIndex)
{
    toggleRecurrenceWidgets(currentIndex);
    QString labelFreq;
    QString freqKey;
    int frequency = mUi->mFrequencyEdit->value();
    switch (currentIndex) {
    case 2:
        labelFreq = i18ncp("repeat every N >weeks<", "week", "weeks", frequency);
        freqKey = QLatin1Char('w');
        break;
    case 3:
        labelFreq = i18ncp("repeat every N >months<", "month", "months", frequency);
        freqKey = QLatin1Char('m');
        break;
    case 4:
        labelFreq = i18ncp("repeat every N >years<", "year", "years", frequency);
        freqKey = QLatin1Char('y');
        break;
    default:
        labelFreq = i18ncp("repeat every N >days<", "day", "days", frequency);
        freqKey = QLatin1Char('d');
    }

    const QString labelEvery = ki18ncp(
                                   "repeat >every< N years/months/...; "
                                   "dynamic context 'type': 'd' days, 'w' weeks, "
                                   "'m' months, 'y' years",
                                   "every",
                                   "every")
                                   .subs(frequency)
                                   .inContext(QStringLiteral("type"), freqKey)
                                   .toString();
    mUi->mFrequencyLabel->setText(labelEvery);
    mUi->mRecurrenceRuleLabel->setText(labelFreq);

    Q_EMIT recurrenceChanged(static_cast<RecurrenceType>(currentIndex));
}

void IncidenceRecurrence::removeExceptions()
{
    const QList<QListWidgetItem *> selectedExceptions = mUi->mExceptionList->selectedItems();
    for (QListWidgetItem *selectedException : selectedExceptions) {
        const int row = mUi->mExceptionList->row(selectedException);
        mExceptionDates.removeAt(row);
        delete mUi->mExceptionList->takeItem(row);
    }

    handleExceptionDateChange(mUi->mExceptionDateEdit->date());
    checkDirtyStatus();
}

void IncidenceRecurrence::updateRemoveExceptionButton()
{
    mUi->mExceptionRemoveButton->setEnabled(!mUi->mExceptionList->selectedItems().isEmpty());
}

void IncidenceRecurrence::updateWeekDays(const QDate &newStartDate)
{
    const int oldStartDayIndex = mUi->mWeekDayCombo->weekdayIndex(mCurrentDate);
    const int newStartDayIndex = mUi->mWeekDayCombo->weekdayIndex(newStartDate);

    if (oldStartDayIndex >= 0) {
        mUi->mWeekDayCombo->setItemCheckState(oldStartDayIndex, Qt::Unchecked);
        mUi->mWeekDayCombo->setItemEnabled(oldStartDayIndex, true);
    }

    if (newStartDayIndex >= 0) {
        mUi->mWeekDayCombo->setItemCheckState(newStartDayIndex, Qt::Checked);
        mUi->mWeekDayCombo->setItemEnabled(newStartDayIndex, false);
    }

    if (newStartDate.isValid()) {
        mCurrentDate = newStartDate;
    }
}

short IncidenceRecurrence::dayOfMonthFromStart() const
{
    return currentDate().day();
}

short IncidenceRecurrence::dayOfMonthFromEnd() const
{
    const QDate start = currentDate();
    return start.daysInMonth() - start.day() + 1;
}

short IncidenceRecurrence::dayOfYearFromStart() const
{
    return currentDate().dayOfYear();
}

int IncidenceRecurrence::duration() const
{
    if (mUi->mRecurrenceEndCombo->currentIndex() == RecurrenceEndNever) {
        return -1;
    } else if (mUi->mRecurrenceEndCombo->currentIndex() == RecurrenceEndAfter) {
        return mUi->mEndDurationEdit->value();
    } else {
        // 0 means "end date set"
        return 0;
    }
}

short IncidenceRecurrence::monthWeekFromStart() const
{
    const QDate date = currentDate();
    int count;
    if (date.isValid()) {
        count = 1;
        QDate tmp = date.addDays(-7);
        while (tmp.month() == date.month()) {
            tmp = tmp.addDays(-7); // Count backward
            ++count;
        }
    } else {
        // date can be invalid if you're editing the date with your keyboard
        count = -1;
    }

    // 1 is the first week, 4/5 is the last week of the month
    return count;
}

short IncidenceRecurrence::monthWeekFromEnd() const
{
    const QDate date = currentDate();
    int count;
    if (date.isValid()) {
        count = 1;
        QDate tmp = date.addDays(7);
        while (tmp.month() == date.month()) {
            tmp = tmp.addDays(7); // Count forward
            ++count;
        }
    } else {
        // date can be invalid if you're editing the date with your keyboard
        count = -1;
    }

    // 1 is the last week, 4/5 is the first week of the month
    return count;
}

QString IncidenceRecurrence::numberToString(int number) const
{
    // The code in here was adapted from an article by Johnathan Wood, see:
    // http://www.blackbeltcoder.com/Articles/strings/converting-numbers-to-ordinal-strings

    static QString _numSuffixes[] = {QStringLiteral("th"),
                                     QStringLiteral("st"),
                                     QStringLiteral("nd"),
                                     QStringLiteral("rd"),
                                     QStringLiteral("th"),
                                     QStringLiteral("th"),
                                     QStringLiteral("th"),
                                     QStringLiteral("th"),
                                     QStringLiteral("th"),
                                     QStringLiteral("th")};

    int i = (number % 100);
    int j = (i > 10 && i < 20) ? 0 : (number % 10);
    return QString::number(number) + _numSuffixes[j];
}

void IncidenceRecurrence::selectMonthlyItem(KCalendarCore::Recurrence *recurrence, ushort recurenceType)
{
    Q_ASSERT(recurenceType == KCalendarCore::Recurrence::rMonthlyPos || recurenceType == KCalendarCore::Recurrence::rMonthlyDay);

    if (recurenceType == KCalendarCore::Recurrence::rMonthlyPos) {
        QList<KCalendarCore::RecurrenceRule::WDayPos> rmp = recurrence->monthPositions();
        if (rmp.isEmpty()) {
            return; // Use the default values. Probably marks the editor as dirty
        }

        if (rmp.first().pos() > 0) { // nth day
            // TODO if ( rmp.first().pos() != mDateTime->startDate().day() ) { warn user }
            // NOTE: This silently changes the recurrence when:
            //       rmp.first().pos() != mDateTime->startDate().day()
            mUi->mMonthlyCombo->setCurrentIndex(ComboIndexMonthlyPos);
        } else { // (month.last() - n)th day
            // TODO: Handle recurrences we cannot represent
            // QDate startDate = mDateTime->startDate();
            // const int dayFromEnd = startDate.daysInMonth() - startDate.day();
            // if ( qAbs( rmp.first().pos() ) != dayFromEnd ) { /* warn user */ }
            mUi->mMonthlyCombo->setCurrentIndex(ComboIndexMonthlyPosInverted);
        }
    } else { // Monthly by day
        // check if we have any setting for which day (vcs import is broken and
        // does not set any day, thus we need to check)
        const int day = recurrence->monthDays().isEmpty() ? currentDate().day() : recurrence->monthDays().at(0);

        // Days from the end are after the ones from the begin, so correct for the
        // negative sign and add 30 (index starting at 0)
        // TODO: Do similar checks as in the monthlyPos case
        if (day > 0 && day <= 31) {
            mUi->mMonthlyCombo->setCurrentIndex(ComboIndexMonthlyDay);
        } else if (day < 0) {
            mUi->mMonthlyCombo->setCurrentIndex(ComboIndexMonthlyDayInverted);
        }
    }

    // So we can easily detect if the user changed the type, without going through this logic ^
    mMonthlyInitialType = mUi->mMonthlyCombo->currentIndex();
}

void IncidenceRecurrence::selectYearlyItem(KCalendarCore::Recurrence *recurrence, ushort recurenceType)
{
    Q_ASSERT(recurenceType == KCalendarCore::Recurrence::rYearlyDay || recurenceType == KCalendarCore::Recurrence::rYearlyMonth
             || recurenceType == KCalendarCore::Recurrence::rYearlyPos);

    if (recurenceType == KCalendarCore::Recurrence::rYearlyDay) {
        /*
        const int day = recurrence->yearDays().isEmpty() ? currentDate().dayOfYear() :
                                                           recurrence->yearDays().first();
        */
        // TODO Check if day has actually the same value as in the combo.
        mUi->mYearlyCombo->setCurrentIndex(ComboIndexYearlyDay);
    } else if (recurenceType == KCalendarCore::Recurrence::rYearlyMonth) {
        const int day = recurrence->yearDates().isEmpty() ? currentDate().day() : recurrence->yearDates().at(0);

        /*
        int month = currentDate().month();
        if ( !recurrence->yearMonths().isEmpty() ) {
          month = recurrence->yearMonths().first();
        }
        */

        // TODO check month and day to be correct values with respect to what is
        //      presented in the combo box.
        if (day > 0) {
            mUi->mYearlyCombo->setCurrentIndex(ComboIndexYearlyMonth);
        } else {
            mUi->mYearlyCombo->setCurrentIndex(ComboIndexYearlyMonthInverted);
        }
    } else { // KCalendarCore::Recurrence::rYearlyPos
        /*
        int month = currentDate().month();
        if ( !recurrence->yearMonths().isEmpty() ) {
          month = recurrence->yearMonths().first();
        }
        */

        // count is the nth weekday of the month or the ith last weekday of the month.
        int count = (currentDate().day() - 1) / 7;
        if (!recurrence->yearPositions().isEmpty()) {
            count = recurrence->yearPositions().at(0).pos();
        }

        // TODO check month,count and day to be correct values with respect to what is
        //      presented in the combo box.
        if (count > 0) {
            mUi->mYearlyCombo->setCurrentIndex(ComboIndexYearlyPos);
        } else {
            mUi->mYearlyCombo->setCurrentIndex(ComboIndexYearlyPosInverted);
        }
    }

    // So we can easily detect if the user changed the type, without going through this logic ^
    mYearlyInitialType = mUi->mYearlyCombo->currentIndex();
}

void IncidenceRecurrence::setDefaults()
{
    mUi->mRecurrenceEndCombo->setCurrentIndex(RecurrenceEndNever);
    mUi->mRecurrenceEndDate->setDate(currentDate());
    mUi->mRecurrenceTypeCombo->setCurrentIndex(RecurrenceTypeNone);

    setFrequency(1);

    // -1 because we want between 0 and 6
    const int day = currentDate().dayOfWeek() - 1;

    QBitArray checkDays(7, false);
    checkDays.setBit(day);

    QBitArray disableDays(7, false);
    disableDays.setBit(day);

    mUi->mWeekDayCombo->setDays(checkDays, disableDays);

    mUi->mMonthlyCombo->setCurrentIndex(0); // Recur on the nth of the month
    mUi->mYearlyCombo->setCurrentIndex(0); // Recur on the nth of the month
}

void IncidenceRecurrence::setDuration(int duration)
{
    if (duration == -1) { // No end date
        mUi->mRecurrenceEndCombo->setCurrentIndex(RecurrenceEndNever);
        mUi->mRecurrenceEndStack->setCurrentIndex(0);
    } else if (duration == 0) {
        mUi->mRecurrenceEndCombo->setCurrentIndex(RecurrenceEndOn);
        mUi->mRecurrenceEndStack->setCurrentIndex(1);
    } else {
        mUi->mRecurrenceEndCombo->setCurrentIndex(RecurrenceEndAfter);
        mUi->mRecurrenceEndStack->setCurrentIndex(2);
        mUi->mEndDurationEdit->setValue(duration);
    }
}

void IncidenceRecurrence::setExceptionDates(const KCalendarCore::DateList &dates)
{
    mUi->mExceptionList->clear();
    mExceptionDates.clear();
    KCalendarCore::DateList::ConstIterator dit;
    for (const auto &d : dates) {
        mUi->mExceptionList->addItem(QLocale().toString(d));
        mExceptionDates.append(d);
    }
}

void IncidenceRecurrence::setExceptionDateTimes(const KCalCore::DateTimeList &dateTimes)
{
    mUi->mExceptionList->clear();
    mExceptionDates.clear();
    for (const auto &dt : dateTimes) {
        mUi->mExceptionList->addItem(QLocale().toString(dt.date()));
        mExceptionDates.append(dt.date());
    }
}

void IncidenceRecurrence::setFrequency(int frequency)
{
    if (frequency < 1) {
        frequency = 1;
    }

    mUi->mFrequencyEdit->setValue(frequency);
}

void IncidenceRecurrence::toggleRecurrenceWidgets(int recurrenceType)
{
    bool enable = (recurrenceType != RecurrenceTypeNone) && (recurrenceType != RecurrenceTypeException);
    mUi->mRecurrenceTypeCombo->setVisible(recurrenceType != RecurrenceTypeException);
    mUi->mRepeatLabel->setVisible(recurrenceType != RecurrenceTypeException);
    mUi->mRecurrenceEndLabel->setVisible(enable);
    mUi->mOnLabel->setVisible(enable && recurrenceType != RecurrenceTypeDaily);
    if (!enable) {
        // So we can hide the exceptions labels and not trigger column resizing.
        mUi->mRepeatLabel->setMinimumSize(mUi->mExceptionsLabel->sizeHint());
    }

    mUi->mFrequencyLabel->setVisible(enable);
    mUi->mFrequencyEdit->setVisible(enable);
    mUi->mRecurrenceRuleLabel->setVisible(enable);
    mUi->mRepeatStack->setVisible(enable && recurrenceType != RecurrenceTypeDaily);
    mUi->mRepeatStack->setCurrentIndex(recurrenceType);
    mUi->mRecurrenceEndCombo->setVisible(enable);
    mUi->mEndDurationEdit->setVisible(enable);
    mUi->mRecurrenceEndStack->setVisible(enable);

    // Exceptions widgets
    mUi->mExceptionsLabel->setVisible(enable);
    mUi->mExceptionDateEdit->setVisible(enable);
    mUi->mExceptionAddButton->setVisible(enable);
    mUi->mExceptionAddButton->setEnabled(mUi->mExceptionDateEdit->date() >= currentDate());
    mUi->mExceptionRemoveButton->setVisible(enable);
    mUi->mExceptionRemoveButton->setEnabled(!mUi->mExceptionList->selectedItems().isEmpty());
    mUi->mExceptionList->setVisible(enable);
    mUi->mThisAndFutureCheck->setVisible(recurrenceType == RecurrenceTypeException);
}

QBitArray IncidenceRecurrence::weekday() const
{
    QBitArray days(7);
    // QDate::dayOfWeek() -> returns [1 - 7], 1 == monday
    days.setBit(currentDate().dayOfWeek() - 1, true);
    return days;
}

int IncidenceRecurrence::weekdayCountForMonth(const QDate &date) const
{
    Q_ASSERT(date.isValid());
    // This methods returns how often the weekday specified by @param date occurs
    // in the month represented by @param date.

    int count = 1;
    QDate tmp = date.addDays(-7);
    while (tmp.month() == date.month()) {
        tmp = tmp.addDays(-7);
        ++count;
    }

    tmp = date.addDays(7);
    while (tmp.month() == date.month()) {
        tmp = tmp.addDays(7);
        ++count;
    }

    return count;
}

RecurrenceType IncidenceRecurrence::currentRecurrenceType() const
{
    if (mLoadedIncidence && mLoadedIncidence->hasRecurrenceId()) {
        return RecurrenceTypeException;
    }

    const int currentIndex = mUi->mRecurrenceTypeCombo->currentIndex();
    Q_ASSERT_X(currentIndex >= 0 && currentIndex < RecurrenceTypeUnknown, "currentRecurrenceType", "Keep the combo-box values in sync with the enum");
    return static_cast<RecurrenceType>(currentIndex);
}

void IncidenceRecurrence::handleStartDateChange(const QDate &date)
{
    if (currentDate().isValid()) {
        fillCombos();
        updateWeekDays(date);
        mUi->mExceptionDateEdit->setDate(date);
    }
}

QDate IncidenceRecurrence::currentDate() const
{
    return mDateTime->startDate();
}
