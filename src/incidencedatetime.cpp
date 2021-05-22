/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencedatetime.h"
#include "incidenceeditor_debug.h"
#include "ui_dialogdesktop.h"

#include <CalendarSupport/KCalPrefs>

#include <KCalUtils/IncidenceFormatter>

#include <QTimeZone>

using namespace IncidenceEditorNG;

/**
 * Returns true if the incidence's dates are equal to the default ones specified in config.
 */
static bool incidenceHasDefaultTimes(const KCalendarCore::Incidence::Ptr &incidence)
{
    if (!incidence || incidence->allDay()) {
        return false;
    }

    QTime defaultDuration = CalendarSupport::KCalPrefs::instance()->defaultDuration().time();
    if (!defaultDuration.isValid()) {
        return false;
    }

    QTime defaultStart = CalendarSupport::KCalPrefs::instance()->mStartTime.time();
    if (!defaultStart.isValid()) {
        return false;
    }

    if (incidence->dtStart().time() == defaultStart) {
        if (incidence->type() == KCalendarCore::Incidence::TypeJournal) {
            return true; // no duration to compare with
        }

        const QDateTime start = incidence->dtStart();
        const QDateTime end = incidence->dateTime(KCalendarCore::Incidence::RoleEnd);
        if (!end.isValid() || !start.isValid()) {
            return false;
        }

        const int durationInSeconds = defaultDuration.hour() * 3600 + defaultDuration.minute() * 60;
        return start.secsTo(end) == durationInSeconds;
    }

    return false;
}

IncidenceDateTime::IncidenceDateTime(Ui::EventOrTodoDesktop *ui)
    : IncidenceEditor(nullptr)
    , mUi(ui)
    , mTimezoneCombosWereVisibile(false)
{
    setTimeZonesVisibility(false);
    setObjectName(QStringLiteral("IncidenceDateTime"));

    mUi->mTimeZoneLabel->setVisible(!mUi->mWholeDayCheck->isChecked());
    connect(mUi->mTimeZoneLabel, &QLabel::linkActivated, this, &IncidenceDateTime::toggleTimeZoneVisibility);
    mUi->mTimeZoneLabel->setContextMenuPolicy(Qt::NoContextMenu);

    const QList<QLineEdit *> lineEdits{mUi->mStartDateEdit->lineEdit(),
                                       mUi->mEndDateEdit->lineEdit(),
                                       mUi->mStartTimeEdit->lineEdit(),
                                       mUi->mEndTimeEdit->lineEdit()};
    for (QLineEdit *lineEdit : lineEdits) {
        if (lineEdit) {
            lineEdit->setClearButtonEnabled(false);
        }
    }

    connect(mUi->mFreeBusyCheck, &QCheckBox::toggled, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mWholeDayCheck, &QCheckBox::toggled, this, &IncidenceDateTime::enableTimeEdits);
    connect(mUi->mWholeDayCheck, &QCheckBox::toggled, this, &IncidenceDateTime::checkDirtyStatus);

    connect(this, &IncidenceDateTime::startDateChanged, this, &IncidenceDateTime::updateStartToolTips);
    connect(this, &IncidenceDateTime::startTimeChanged, this, &IncidenceDateTime::updateStartToolTips);
    connect(this, &IncidenceDateTime::endDateChanged, this, &IncidenceDateTime::updateEndToolTips);
    connect(this, &IncidenceDateTime::endTimeChanged, this, &IncidenceDateTime::updateEndToolTips);
    connect(mUi->mWholeDayCheck, &QCheckBox::toggled, this, &IncidenceDateTime::updateStartToolTips);
    connect(mUi->mWholeDayCheck, &QCheckBox::toggled, this, &IncidenceDateTime::updateEndToolTips);
    connect(mUi->mStartCheck, &QCheckBox::toggled, this, &IncidenceDateTime::updateStartToolTips);
    connect(mUi->mEndCheck, &QCheckBox::toggled, this, &IncidenceDateTime::updateEndToolTips);
}

IncidenceDateTime::~IncidenceDateTime()
{
}

bool IncidenceDateTime::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        if (obj == mUi->mStartDateEdit) {
            qCDebug(INCIDENCEEDITOR_LOG) << "emitting startDateTime: " << mUi->mStartDateEdit;
            Q_EMIT startDateFocus(obj);
        } else if (obj == mUi->mEndDateEdit) {
            qCDebug(INCIDENCEEDITOR_LOG) << "emitting endDateTime: " << mUi->mEndDateEdit;
            Q_EMIT endDateFocus(obj);
        } else if (obj == mUi->mStartTimeEdit) {
            qCDebug(INCIDENCEEDITOR_LOG) << "emitting startTimeTime: " << mUi->mStartTimeEdit;
            Q_EMIT startTimeFocus(obj);
        } else if (obj == mUi->mEndTimeEdit) {
            qCDebug(INCIDENCEEDITOR_LOG) << "emitting endTimeTime: " << mUi->mEndTimeEdit;
            Q_EMIT endTimeFocus(obj);
        }

        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void IncidenceDateTime::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    if (mLoadedIncidence && *mLoadedIncidence == *incidence) {
        return;
    }

    const bool isTemplate = incidence->customProperty("kdepim", "isTemplate") == QLatin1String("true");
    const bool templateOverridesTimes = incidenceHasDefaultTimes(mLoadedIncidence);

    mLoadedIncidence = incidence;
    mLoadingIncidence = true;

    // We can only handle events or todos.
    if (KCalendarCore::Todo::Ptr todo = IncidenceDateTime::incidence<KCalendarCore::Todo>()) {
        load(todo, isTemplate, templateOverridesTimes);
    } else if (KCalendarCore::Event::Ptr event = IncidenceDateTime::incidence<KCalendarCore::Event>()) {
        load(event, isTemplate, templateOverridesTimes);
    } else if (KCalendarCore::Journal::Ptr journal = IncidenceDateTime::incidence<KCalendarCore::Journal>()) {
        load(journal, isTemplate, templateOverridesTimes);
    } else {
        qCDebug(INCIDENCEEDITOR_LOG) << "Not an Incidence.";
    }

    // Set the initial times before calling enableTimeEdits, as enableTimeEdits
    // assumes that the initial times are initialized.
    mInitialStartDT = currentStartDateTime();
    mInitialEndDT = currentEndDateTime();

    enableTimeEdits();

    if (mUi->mTimeZoneComboStart->currentIndex() == 0) { // Floating
        mInitialStartDT.setTimeZone(QTimeZone::systemTimeZone());
    }

    if (mUi->mTimeZoneComboEnd->currentIndex() == 0) { // Floating
        mInitialEndDT.setTimeZone(QTimeZone::systemTimeZone());
    }

    mWasDirty = false;
    mLoadingIncidence = false;
}

void IncidenceDateTime::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    if (KCalendarCore::Todo::Ptr todo = IncidenceDateTime::incidence<KCalendarCore::Todo>(incidence)) {
        save(todo);
    } else if (KCalendarCore::Event::Ptr event = IncidenceDateTime::incidence<KCalendarCore::Event>(incidence)) {
        save(event);
    } else if (KCalendarCore::Journal::Ptr journal = IncidenceDateTime::incidence<KCalendarCore::Journal>(incidence)) {
        save(journal);
    } else {
        Q_ASSERT_X(false, "IncidenceDateTimeEditor::save", "Only implemented for todos, events and journals");
    }
}

bool IncidenceDateTime::isDirty() const
{
    if (KCalendarCore::Todo::Ptr todo = IncidenceDateTime::incidence<KCalendarCore::Todo>()) {
        return isDirty(todo);
    } else if (KCalendarCore::Event::Ptr event = IncidenceDateTime::incidence<KCalendarCore::Event>()) {
        return isDirty(event);
    } else if (KCalendarCore::Journal::Ptr journal = IncidenceDateTime::incidence<KCalendarCore::Journal>()) {
        return isDirty(journal);
    } else {
        Q_ASSERT_X(false, "IncidenceDateTimeEditor::isDirty", "Only implemented for todos and events");
        return false;
    }
}

void IncidenceDateTime::setActiveDate(const QDate &activeDate)
{
    mActiveDate = activeDate;
}

QDate IncidenceDateTime::startDate() const
{
    return currentStartDateTime().date();
}

QDate IncidenceDateTime::endDate() const
{
    return currentEndDateTime().date();
}

QTime IncidenceDateTime::startTime() const
{
    return currentStartDateTime().time();
}

QTime IncidenceDateTime::endTime() const
{
    return currentEndDateTime().time();
}

/// private slots for General

void IncidenceDateTime::setTimeZonesVisibility(bool visible)
{
    static const QString tz(i18nc("@action show or hide the time zone widgets", "Time zones"));
    QString placeholder(QStringLiteral("<a href=\"hide\">&lt;&lt; %1</a>"));
    if (visible) {
        placeholder = placeholder.arg(tz);
    } else {
        placeholder = QStringLiteral("<a href=\"show\">%1 &gt;&gt;</a>");
        placeholder = placeholder.arg(tz);
    }
    mUi->mTimeZoneLabel->setText(placeholder);

    mUi->mTimeZoneComboStart->setVisible(visible);
    mUi->mTimeZoneComboEnd->setVisible(visible && type() != KCalendarCore::Incidence::TypeJournal);
}

void IncidenceDateTime::toggleTimeZoneVisibility()
{
    setTimeZonesVisibility(!mUi->mTimeZoneComboStart->isVisible());
}

void IncidenceDateTime::updateStartTime(const QTime &newTime)
{
    if (!newTime.isValid()) {
        return;
    }

    QDateTime endDateTime = currentEndDateTime();
    const int secsep = mCurrentStartDateTime.secsTo(endDateTime);
    mCurrentStartDateTime.setTime(newTime);
    if (mUi->mEndCheck->isChecked()) {
        // Only update the end time when it is actually enabled, adjust end time so
        // that the event/todo has the same duration as before.
        endDateTime = mCurrentStartDateTime.addSecs(secsep);
        mUi->mEndTimeEdit->setTime(endDateTime.time());
        mUi->mEndDateEdit->setDate(endDateTime.date());
    }

    Q_EMIT startTimeChanged(mCurrentStartDateTime.time());
    checkDirtyStatus();
}

void IncidenceDateTime::updateStartDate(const QDate &newDate)
{
    if (!newDate.isValid()) {
        return;
    }

    const bool dateChanged = mCurrentStartDateTime.date() != newDate;

    QDateTime endDateTime = currentEndDateTime();
    int daysep = mCurrentStartDateTime.daysTo(endDateTime);
    mCurrentStartDateTime.setDate(newDate);
    if (mUi->mEndCheck->isChecked()) {
        // Only update the end time when it is actually enabled, adjust end time so
        // that the event/todo has the same duration as before.
        endDateTime.setDate(mCurrentStartDateTime.date().addDays(daysep));
        mUi->mEndDateEdit->setDate(endDateTime.date());
    }

    checkDirtyStatus();

    if (dateChanged) {
        Q_EMIT startDateChanged(mCurrentStartDateTime.date());
    }
}

void IncidenceDateTime::updateStartSpec()
{
    const QDate prevDate = mCurrentStartDateTime.date();

    if (mUi->mEndCheck->isChecked() && currentEndDateTime().timeZone() == mCurrentStartDateTime.timeZone()) {
        mUi->mTimeZoneComboEnd->selectTimeZone(mUi->mTimeZoneComboStart->selectedTimeZone());
    }

    mCurrentStartDateTime.setTimeZone(mUi->mTimeZoneComboStart->selectedTimeZone());

    const bool dateChanged = mCurrentStartDateTime.date() != prevDate;

    if (dateChanged) {
        Q_EMIT startDateChanged(mCurrentStartDateTime.date());
    }

    if (type() == KCalendarCore::Incidence::TypeJournal) {
        checkDirtyStatus();
    }
}

/// private slots for Todo

void IncidenceDateTime::enableStartEdit(bool enable)
{
    mUi->mStartDateEdit->setEnabled(enable);

    if (mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked()) {
        mUi->mWholeDayCheck->setEnabled(true);
        setTimeZoneLabelEnabled(!mUi->mWholeDayCheck->isChecked());
    } else {
        mUi->mWholeDayCheck->setEnabled(false);
        mUi->mWholeDayCheck->setChecked(false);
        setTimeZoneLabelEnabled(false);
    }

    if (enable) {
        mUi->mStartTimeEdit->setEnabled(!mUi->mWholeDayCheck->isChecked());
        mUi->mTimeZoneComboStart->setEnabled(!mUi->mWholeDayCheck->isChecked());
    } else {
        mUi->mStartTimeEdit->setEnabled(false);
        mUi->mTimeZoneComboStart->setEnabled(false);
    }

    mUi->mTimeZoneComboStart->setFloating(!mUi->mTimeZoneComboStart->isEnabled());
    checkDirtyStatus();
}

void IncidenceDateTime::enableEndEdit(bool enable)
{
    mUi->mEndDateEdit->setEnabled(enable);

    if (mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked()) {
        mUi->mWholeDayCheck->setEnabled(true);
        setTimeZoneLabelEnabled(!mUi->mWholeDayCheck->isChecked());
    } else {
        mUi->mWholeDayCheck->setEnabled(false);
        mUi->mWholeDayCheck->setChecked(false);
        setTimeZoneLabelEnabled(false);
    }

    if (enable) {
        mUi->mEndTimeEdit->setEnabled(!mUi->mWholeDayCheck->isChecked());
        mUi->mTimeZoneComboEnd->setEnabled(!mUi->mWholeDayCheck->isChecked());
    } else {
        mUi->mEndTimeEdit->setEnabled(false);
        mUi->mTimeZoneComboEnd->setEnabled(false);
    }

    mUi->mTimeZoneComboEnd->setFloating(!mUi->mTimeZoneComboEnd->isEnabled());
    checkDirtyStatus();
}

bool IncidenceDateTime::timeZonesAreLocal(const QDateTime &start, const QDateTime &end)
{
    // Returns false if the incidence start or end timezone is not the local zone.

    if ((start.isValid() && start.timeZone() != QTimeZone::systemTimeZone()) || (end.isValid() && end.timeZone() != QTimeZone::systemTimeZone())) {
        return false;
    } else {
        return true;
    }
}

void IncidenceDateTime::enableTimeEdits()
{
    // NOTE: assumes that the initial times are initialized.
    const bool wholeDayChecked = mUi->mWholeDayCheck->isChecked();

    setTimeZoneLabelEnabled(!wholeDayChecked);

    if (mUi->mStartCheck->isChecked()) {
        mUi->mStartTimeEdit->setEnabled(!wholeDayChecked);
        mUi->mTimeZoneComboStart->setEnabled(!wholeDayChecked);
        mUi->mTimeZoneComboStart->setFloating(wholeDayChecked, mInitialStartDT.timeZone());
    }
    if (mUi->mEndCheck->isChecked()) {
        mUi->mEndTimeEdit->setEnabled(!wholeDayChecked);
        mUi->mTimeZoneComboEnd->setEnabled(!wholeDayChecked);
        mUi->mTimeZoneComboEnd->setFloating(wholeDayChecked, mInitialEndDT.timeZone());
    }

    /**
       When editing a whole-day event, unchecking mWholeDayCheck shouldn't set both
       times to 00:00. DTSTART must always be smaller than DTEND
     */
    if (sender() == mUi->mWholeDayCheck && !wholeDayChecked // Somebody unchecked it, the incidence will now have time.
        && mUi->mStartCheck->isChecked() && mUi->mEndCheck->isChecked() // The incidence has both start and end/due dates
        && currentStartDateTime() == currentEndDateTime()) { // DTSTART == DTEND. This is illegal, lets correct it.
        // Not sure about the best time here... doesn't really matter, when someone unchecks mWholeDayCheck, she will
        // always want to set a time.
        mUi->mStartTimeEdit->setTime(QTime(0, 0));
        mUi->mEndTimeEdit->setTime(QTime(1, 0));
    }

    const bool currentlyVisible = mUi->mTimeZoneLabel->text().contains(QLatin1String("&lt;&lt;"));
    setTimeZonesVisibility(!wholeDayChecked && mTimezoneCombosWereVisibile);
    mTimezoneCombosWereVisibile = currentlyVisible;
    if (!wholeDayChecked && !timeZonesAreLocal(currentStartDateTime(), currentEndDateTime())) {
        setTimeZonesVisibility(true);
        mTimezoneCombosWereVisibile = true;
    }
}

bool IncidenceDateTime::isDirty(const KCalendarCore::Todo::Ptr &todo) const
{
    Q_ASSERT(todo);

    const bool hasDateTimes = mUi->mStartCheck->isChecked() || mUi->mEndCheck->isChecked();

    // First check the start time/date of the todo
    if (todo->hasStartDate() != mUi->mStartCheck->isChecked()) {
        return true;
    }

    if ((hasDateTimes && todo->allDay()) != mUi->mWholeDayCheck->isChecked()) {
        return true;
    }

    if (todo->hasDueDate() != mUi->mEndCheck->isChecked()) {
        return true;
    }

    if (mUi->mStartCheck->isChecked()) {
        // Use mActiveStartTime. This is the QTimeZone selected on load coming from
        // the combobox. We use this one as it can slightly differ (e.g. missing
        // country code in the incidence time spec) from the incidence.
        if (currentStartDateTime() != mInitialStartDT) {
            return true;
        }
    }

    if (mUi->mEndCheck->isChecked() && currentEndDateTime() != mInitialEndDT) {
        return true;
    }

    return false;
}

/// Event specific methods

bool IncidenceDateTime::isDirty(const KCalendarCore::Event::Ptr &event) const
{
    if (event->allDay() != mUi->mWholeDayCheck->isChecked()) {
        return true;
    }

    if (mUi->mFreeBusyCheck->isChecked() && event->transparency() != KCalendarCore::Event::Opaque) {
        return true;
    }

    if (!mUi->mFreeBusyCheck->isChecked() && event->transparency() != KCalendarCore::Event::Transparent) {
        return true;
    }

    if (event->allDay()) {
        if (mUi->mStartDateEdit->date() != mInitialStartDT.date() || mUi->mEndDateEdit->date() != mInitialEndDT.date()) {
            return true;
        }
    } else {
        if (currentStartDateTime() != mInitialStartDT || currentEndDateTime() != mInitialEndDT
            || currentStartDateTime().timeZone() != mInitialStartDT.timeZone() || currentEndDateTime().timeZone() != mInitialEndDT.timeZone()) {
            return true;
        }
    }

    return false;
}

bool IncidenceDateTime::isDirty(const KCalendarCore::Journal::Ptr &journal) const
{
    if (journal->allDay() != mUi->mWholeDayCheck->isChecked()) {
        return true;
    }

    if (journal->allDay()) {
        if (mUi->mStartDateEdit->date() != mInitialStartDT.date()) {
            return true;
        }
    } else {
        if (currentStartDateTime() != mInitialStartDT) {
            return true;
        }
    }

    return false;
}

/// Private methods

QDateTime IncidenceDateTime::currentStartDateTime() const
{
    return QDateTime(mUi->mStartDateEdit->date(), mUi->mStartTimeEdit->time(), mUi->mTimeZoneComboStart->selectedTimeZone());
}

QDateTime IncidenceDateTime::currentEndDateTime() const
{
    return QDateTime(mUi->mEndDateEdit->date(), mUi->mEndTimeEdit->time(), mUi->mTimeZoneComboEnd->selectedTimeZone());
}

void IncidenceDateTime::load(const KCalendarCore::Event::Ptr &event, bool isTemplate, bool templateOverridesTimes)
{
    // First en/disable the necessary ui bits and pieces
    mUi->mStartCheck->setVisible(false);
    mUi->mStartCheck->setChecked(true); // Set to checked so we can reuse enableTimeEdits.
    mUi->mEndCheck->setVisible(false);
    mUi->mEndCheck->setChecked(true); // Set to checked so we can reuse enableTimeEdits.

    // Start time
    connect(mUi->mStartTimeEdit, &KTimeComboBox::timeChanged, this,
            &IncidenceDateTime::updateStartTime); // when editing with mouse, or up/down arrows
    connect(mUi->mStartTimeEdit, &KTimeComboBox::timeEdited, this,
            &IncidenceDateTime::updateStartTime); // When editing with any key except up/down
    connect(mUi->mStartDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::updateStartDate);
    connect(mUi->mTimeZoneComboStart,
            static_cast<void (IncidenceEditorNG::KTimeZoneComboBox::*)(int)>(&IncidenceEditorNG::KTimeZoneComboBox::currentIndexChanged),
            this,
            &IncidenceDateTime::updateStartSpec);

    // End time
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeEdited, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::endTimeChanged);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeEdited, this, &IncidenceDateTime::endTimeChanged);
    connect(mUi->mEndDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::endDateChanged);
    connect(mUi->mTimeZoneComboEnd,
            static_cast<void (IncidenceEditorNG::KTimeZoneComboBox::*)(int)>(&IncidenceEditorNG::KTimeZoneComboBox::currentIndexChanged),
            this,
            &IncidenceDateTime::checkDirtyStatus);
    mUi->mWholeDayCheck->setChecked(event->allDay());
    enableTimeEdits();

    if (isTemplate) {
        if (templateOverridesTimes) {
            // We only use the template times if the user didn't override them.
            setTimes(event->dtStart(), event->dtEnd());
        }
    } else {
        QDateTime startDT = event->dtStart();
        QDateTime endDT = event->dtEnd();
        setDateTimes(startDT, endDT);
    }

    switch (event->transparency()) {
    case KCalendarCore::Event::Transparent:
        mUi->mFreeBusyCheck->setChecked(false);
        break;
    case KCalendarCore::Event::Opaque:
        mUi->mFreeBusyCheck->setChecked(true);
        break;
    }
}

void IncidenceDateTime::load(const KCalendarCore::Journal::Ptr &journal, bool isTemplate, bool templateOverridesTimes)
{
    // First en/disable the necessary ui bits and pieces
    mUi->mStartCheck->setVisible(false);
    mUi->mStartCheck->setChecked(true); // Set to checked so we can reuse enableTimeEdits.
    mUi->mEndCheck->setVisible(false);
    mUi->mEndCheck->setChecked(true); // Set to checked so we can reuse enableTimeEdits.
    mUi->mEndDateEdit->setVisible(false);
    mUi->mEndTimeEdit->setVisible(false);
    mUi->mTimeZoneComboEnd->setVisible(false);
    mUi->mEndLabel->setVisible(false);
    mUi->mFreeBusyCheck->setVisible(false);

    // Start time
    connect(mUi->mStartTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::updateStartTime);
    connect(mUi->mStartDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::updateStartDate);
    connect(mUi->mTimeZoneComboStart,
            static_cast<void (IncidenceEditorNG::KTimeZoneComboBox::*)(int)>(&IncidenceEditorNG::KTimeZoneComboBox::currentIndexChanged),
            this,
            &IncidenceDateTime::updateStartSpec);
    mUi->mWholeDayCheck->setChecked(journal->allDay());
    enableTimeEdits();

    if (isTemplate) {
        if (templateOverridesTimes) {
            // We only use the template times if the user didn't override them.
            setTimes(journal->dtStart(), QDateTime());
        }
    } else {
        QDateTime startDT = journal->dtStart();

        // Convert UTC to local timezone, if needed (i.e. for kolab #204059)
        if (startDT.timeZone() == QTimeZone::utc()) {
            startDT = startDT.toLocalTime();
        }
        setDateTimes(startDT, QDateTime());
    }
}

void IncidenceDateTime::load(const KCalendarCore::Todo::Ptr &todo, bool isTemplate, bool templateOverridesTimes)
{
    // First en/disable the necessary ui bits and pieces
    mUi->mStartCheck->setVisible(true);
    mUi->mStartCheck->setChecked(todo->hasStartDate());
    mUi->mStartDateEdit->setEnabled(todo->hasStartDate());
    mUi->mStartTimeEdit->setEnabled(todo->hasStartDate());
    mUi->mTimeZoneComboStart->setEnabled(todo->hasStartDate());

    mUi->mEndLabel->setText(i18nc("@label The due date/time of a to-do", "Due:"));
    mUi->mEndCheck->setVisible(true);
    mUi->mEndCheck->setChecked(todo->hasDueDate());
    mUi->mEndDateEdit->setEnabled(todo->hasDueDate());
    mUi->mEndTimeEdit->setEnabled(todo->hasDueDate());
    mUi->mTimeZoneComboEnd->setEnabled(todo->hasDueDate());

    // These fields where not enabled in the old code either:
    mUi->mFreeBusyCheck->setVisible(false);

    const bool hasDateTimes = mUi->mEndCheck->isChecked() || mUi->mStartCheck->isChecked();
    mUi->mWholeDayCheck->setChecked(hasDateTimes && todo->allDay());
    mUi->mWholeDayCheck->setEnabled(hasDateTimes);

    // Connect to the right logic
    connect(mUi->mStartCheck, &QCheckBox::toggled, this, &IncidenceDateTime::enableStartEdit);
    connect(mUi->mStartCheck, &QCheckBox::toggled, this, &IncidenceDateTime::startDateTimeToggled);
    connect(mUi->mStartDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::updateStartDate);
    connect(mUi->mStartTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::updateStartTime);
    connect(mUi->mStartTimeEdit, &KTimeComboBox::timeEdited, this, &IncidenceDateTime::updateStartTime);
    connect(mUi->mTimeZoneComboStart,
            static_cast<void (IncidenceEditorNG::KTimeZoneComboBox::*)(int)>(&IncidenceEditorNG::KTimeZoneComboBox::currentIndexChanged),
            this,
            &IncidenceDateTime::updateStartSpec);

    connect(mUi->mEndCheck, &QCheckBox::toggled, this, &IncidenceDateTime::enableEndEdit);
    connect(mUi->mEndCheck, &QCheckBox::toggled, this, &IncidenceDateTime::endDateTimeToggled);
    connect(mUi->mEndDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeEdited, this, &IncidenceDateTime::checkDirtyStatus);
    connect(mUi->mEndDateEdit, &KDateComboBox::dateChanged, this, &IncidenceDateTime::endDateChanged);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeChanged, this, &IncidenceDateTime::endTimeChanged);
    connect(mUi->mEndTimeEdit, &KTimeComboBox::timeEdited, this, &IncidenceDateTime::endTimeChanged);
    connect(mUi->mTimeZoneComboEnd,
            static_cast<void (IncidenceEditorNG::KTimeZoneComboBox::*)(int)>(&IncidenceEditorNG::KTimeZoneComboBox::currentIndexChanged),
            this,
            &IncidenceDateTime::checkDirtyStatus);
    const QDateTime rightNow = QDateTime::currentDateTime();

    if (isTemplate) {
        if (templateOverridesTimes) {
            // We only use the template times if the user didn't override them.
            setTimes(todo->dtStart(), todo->dateTime(KCalendarCore::Incidence::RoleEnd));
        }
    } else {
        const QDateTime endDT = todo->hasDueDate() ? todo->dtDue(true /** first */) : rightNow;
        const QDateTime startDT = todo->hasStartDate() ? todo->dtStart(true /** first */) : rightNow;
        setDateTimes(startDT, endDT);
    }
}

void IncidenceDateTime::save(const KCalendarCore::Event::Ptr &event)
{
    event->setAllDay(mUi->mWholeDayCheck->isChecked());
    event->setDtStart(currentStartDateTime());
    event->setDtEnd(currentEndDateTime());

    // Free == Event::Transparent
    // Busy == Event::Opaque
    event->setTransparency(mUi->mFreeBusyCheck->isChecked() ? KCalendarCore::Event::Opaque : KCalendarCore::Event::Transparent);
}

void IncidenceDateTime::save(const KCalendarCore::Todo::Ptr &todo)
{
    if (mUi->mStartCheck->isChecked()) {
        todo->setDtStart(currentStartDateTime());
        todo->setAllDay(mUi->mWholeDayCheck->isChecked());
        if (currentStartDateTime() != mInitialStartDT) {
            // We don't offer any way to edit the current completed occurrence.
            // So, if the start date changes, reset the dtRecurrence
            todo->setDtRecurrence(currentStartDateTime());
        }
    } else {
        todo->setDtStart(QDateTime());
    }

    if (mUi->mEndCheck->isChecked()) {
        todo->setDtDue(currentEndDateTime(), true /* first */);
        todo->setAllDay(mUi->mWholeDayCheck->isChecked());
    } else {
        todo->setDtDue(QDateTime());
    }
}

void IncidenceDateTime::save(const KCalendarCore::Journal::Ptr &journal)
{
    journal->setAllDay(mUi->mWholeDayCheck->isChecked());
    journal->setDtStart(currentStartDateTime());
}

void IncidenceDateTime::setDateTimes(const QDateTime &start, const QDateTime &end)
{
    if (start.isValid()) {
        mUi->mStartDateEdit->setDate(start.date());
        mUi->mStartTimeEdit->setTime(start.time());
        mUi->mTimeZoneComboStart->selectTimeZone(start.timeZone());
    } else {
        QDateTime dt = QDateTime::currentDateTime();
        mUi->mStartDateEdit->setDate(dt.date());
        mUi->mStartTimeEdit->setTime(dt.time());
        mUi->mTimeZoneComboStart->selectTimeZone(dt.timeZone());
    }

    if (end.isValid()) {
        mUi->mEndDateEdit->setDate(end.date());
        mUi->mEndTimeEdit->setTime(end.time());
        mUi->mTimeZoneComboEnd->selectTimeZone(end.timeZone());
    } else {
        QDateTime dt(QDate::currentDate(), QTime::currentTime().addSecs(60 * 60));
        mUi->mEndDateEdit->setDate(dt.date());
        mUi->mEndTimeEdit->setTime(dt.time());
        mUi->mTimeZoneComboEnd->selectTimeZone(dt.timeZone());
    }

    mCurrentStartDateTime = currentStartDateTime();
    Q_EMIT startDateChanged(start.date());
    Q_EMIT startTimeChanged(start.time());
    Q_EMIT endDateChanged(end.date());
    Q_EMIT endTimeChanged(end.time());

    updateStartToolTips();
    updateEndToolTips();
}

void IncidenceDateTime::updateStartToolTips()
{
    if (mUi->mStartCheck->isChecked()) {
        QString datetimeStr = KCalUtils::IncidenceFormatter::dateTimeToString(currentStartDateTime(), mUi->mWholeDayCheck->isChecked(), false);
        mUi->mStartDateEdit->setToolTip(i18n("Starts: %1", datetimeStr));
        mUi->mStartTimeEdit->setToolTip(i18n("Starts: %1", datetimeStr));
    } else {
        mUi->mStartDateEdit->setToolTip(i18n("Starting Date"));
        mUi->mStartTimeEdit->setToolTip(i18n("Starting Time"));
    }
}

void IncidenceDateTime::updateEndToolTips()
{
    if (mUi->mStartCheck->isChecked()) {
        QString datetimeStr = KCalUtils::IncidenceFormatter::dateTimeToString(currentEndDateTime(), mUi->mWholeDayCheck->isChecked(), false);
        if (mLoadedIncidence->type() == KCalendarCore::Incidence::TypeTodo) {
            mUi->mEndDateEdit->setToolTip(i18n("Due on: %1", datetimeStr));
            mUi->mEndTimeEdit->setToolTip(i18n("Due on: %1", datetimeStr));
        } else {
            mUi->mEndDateEdit->setToolTip(i18n("Ends: %1", datetimeStr));
            mUi->mEndTimeEdit->setToolTip(i18n("Ends: %1", datetimeStr));
        }
    } else {
        if (mLoadedIncidence->type() == KCalendarCore::Incidence::TypeTodo) {
            mUi->mEndDateEdit->setToolTip(i18n("Due Date"));
            mUi->mEndTimeEdit->setToolTip(i18n("Due Time"));
        } else {
            mUi->mEndDateEdit->setToolTip(i18n("Ending Date"));
            mUi->mEndTimeEdit->setToolTip(i18n("Ending Time"));
        }
    }
}

void IncidenceDateTime::setTimes(const QDateTime &start, const QDateTime &end)
{
    // like setDateTimes(), but it set only the start/end time, not the date
    // it is used while applying a template to an event.
    mUi->mStartTimeEdit->blockSignals(true);
    mUi->mStartTimeEdit->setTime(start.time());
    mUi->mStartTimeEdit->blockSignals(false);

    mUi->mEndTimeEdit->setTime(end.time());

    mUi->mTimeZoneComboStart->selectTimeZone(start.timeZone());
    mUi->mTimeZoneComboEnd->selectTimeZone(end.timeZone());

    //   emitDateTimeStr();
}

void IncidenceDateTime::setStartDate(const QDate &newDate)
{
    mUi->mStartDateEdit->setDate(newDate);
    updateStartDate(newDate);
}

void IncidenceDateTime::setStartTime(const QTime &newTime)
{
    mUi->mStartTimeEdit->setTime(newTime);
    updateStartTime(newTime);
}

bool IncidenceDateTime::startDateTimeEnabled() const
{
    return mUi->mStartCheck->isChecked();
}

bool IncidenceDateTime::endDateTimeEnabled() const
{
    return mUi->mEndCheck->isChecked();
}

void IncidenceDateTime::focusInvalidField()
{
    if (startDateTimeEnabled()) {
        if (!mUi->mStartDateEdit->isValid()) {
            mUi->mStartDateEdit->setFocus();
            return;
        }
        if (!mUi->mWholeDayCheck->isChecked() && !mUi->mStartTimeEdit->isValid()) {
            mUi->mStartTimeEdit->setFocus();
            return;
        }
    }
    if (endDateTimeEnabled()) {
        if (!mUi->mEndDateEdit->isValid()) {
            mUi->mEndDateEdit->setFocus();
            return;
        }
        if (!mUi->mWholeDayCheck->isChecked() && !mUi->mEndTimeEdit->isValid()) {
            mUi->mEndTimeEdit->setFocus();
            return;
        }
    }
    if (startDateTimeEnabled() && endDateTimeEnabled() && currentStartDateTime() > currentEndDateTime()) {
        if (mUi->mEndDateEdit->date() < mUi->mStartDateEdit->date()) {
            mUi->mEndDateEdit->setFocus();
        } else {
            mUi->mEndTimeEdit->setFocus();
        }
    }
}

bool IncidenceDateTime::isValid() const
{
    if (startDateTimeEnabled()) {
        if (!mUi->mStartDateEdit->isValid()) {
            mLastErrorString = i18nc("@info", "Invalid start date.");
            qCDebug(INCIDENCEEDITOR_LOG) << mLastErrorString;
            return false;
        }
        if (!mUi->mWholeDayCheck->isChecked() && !mUi->mStartTimeEdit->isValid()) {
            mLastErrorString = i18nc("@info", "Invalid start time.");
            qCDebug(INCIDENCEEDITOR_LOG) << mLastErrorString;
            return false;
        }
    }

    if (endDateTimeEnabled()) {
        if (!mUi->mEndDateEdit->isValid()) {
            mLastErrorString = i18nc("@info", "Invalid end date.");
            qCDebug(INCIDENCEEDITOR_LOG) << mLastErrorString;
            return false;
        }
        if (!mUi->mWholeDayCheck->isChecked() && !mUi->mEndTimeEdit->isValid()) {
            mLastErrorString = i18nc("@info", "Invalid end time.");
            qCDebug(INCIDENCEEDITOR_LOG) << mLastErrorString;
            return false;
        }
    }

    if (startDateTimeEnabled() && endDateTimeEnabled() && currentStartDateTime() > currentEndDateTime()) {
        if (mLoadedIncidence->type() == KCalendarCore::Incidence::TypeEvent) {
            mLastErrorString = i18nc("@info",
                                     "The event ends before it starts.\n"
                                     "Please correct dates and times.");
        } else if (mLoadedIncidence->type() == KCalendarCore::Incidence::TypeTodo) {
            mLastErrorString = i18nc("@info",
                                     "The to-do is due before it starts.\n"
                                     "Please correct dates and times.");
        } else if (mLoadedIncidence->type() == KCalendarCore::Incidence::TypeJournal) {
            return true;
        }

        qCDebug(INCIDENCEEDITOR_LOG) << mLastErrorString;
        return false;
    } else {
        mLastErrorString.clear();
        return true;
    }
}

void IncidenceDateTime::printDebugInfo() const
{
    qCDebug(INCIDENCEEDITOR_LOG) << "startDateTimeEnabled()          : " << startDateTimeEnabled();
    qCDebug(INCIDENCEEDITOR_LOG) << "endDateTimeEnabled()            : " << endDateTimeEnabled();
    qCDebug(INCIDENCEEDITOR_LOG) << "currentStartDateTime().isValid(): " << currentStartDateTime().isValid();
    qCDebug(INCIDENCEEDITOR_LOG) << "currentEndDateTime().isValid()  : " << currentEndDateTime().isValid();
    qCDebug(INCIDENCEEDITOR_LOG) << "currentStartDateTime()          : " << currentStartDateTime().toString();
    qCDebug(INCIDENCEEDITOR_LOG) << "currentEndDateTime()            : " << currentEndDateTime().toString();
    qCDebug(INCIDENCEEDITOR_LOG) << "Incidence type                  : " << mLoadedIncidence->type();
    qCDebug(INCIDENCEEDITOR_LOG) << "allday                          : " << mLoadedIncidence->allDay();
    qCDebug(INCIDENCEEDITOR_LOG) << "mInitialStartDT                 : " << mInitialStartDT.toString();
    qCDebug(INCIDENCEEDITOR_LOG) << "mInitialEndDT                   : " << mInitialEndDT.toString();

    qCDebug(INCIDENCEEDITOR_LOG) << "currentStartDateTime().timeZone(): " << currentStartDateTime().timeZone().id();
    qCDebug(INCIDENCEEDITOR_LOG) << "currentEndDateTime().timeZone()  : " << currentEndDateTime().timeZone().id();
    qCDebug(INCIDENCEEDITOR_LOG) << "mInitialStartDT.timeZone()       : " << mInitialStartDT.timeZone().id();
    qCDebug(INCIDENCEEDITOR_LOG) << "mInitialEndDT.timeZone()         : " << mInitialEndDT.timeZone().id();

    qCDebug(INCIDENCEEDITOR_LOG) << "dirty test1: " << (mLoadedIncidence->allDay() != mUi->mWholeDayCheck->isChecked());
    if (mLoadedIncidence->type() == KCalendarCore::Incidence::TypeEvent) {
        KCalendarCore::Event::Ptr event = mLoadedIncidence.staticCast<KCalendarCore::Event>();
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test2: " << (mUi->mFreeBusyCheck->isChecked() && event->transparency() != KCalendarCore::Event::Opaque);
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test3: " << (!mUi->mFreeBusyCheck->isChecked() && event->transparency() != KCalendarCore::Event::Transparent);
    }

    if (mLoadedIncidence->allDay()) {
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test4: "
                                     << (mUi->mStartDateEdit->date() != mInitialStartDT.date() || mUi->mEndDateEdit->date() != mInitialEndDT.date());
    } else {
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test4.1: " << (currentStartDateTime() != mInitialStartDT);
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test4.2: " << (currentEndDateTime() != mInitialEndDT);
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test4.3: " << (currentStartDateTime().timeZone() != mInitialStartDT.timeZone());
        qCDebug(INCIDENCEEDITOR_LOG) << "dirty test4.4: " << (currentEndDateTime().timeZone() != mInitialEndDT.timeZone());
    }
}

void IncidenceDateTime::setTimeZoneLabelEnabled(bool enable)
{
    mUi->mTimeZoneLabel->setVisible(enable);
}
