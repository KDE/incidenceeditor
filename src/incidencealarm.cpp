/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencealarm.h"
#include "alarmdialog.h"
#include "alarmpresets.h"
#include "incidencedatetime.h"
#include "ui_dialogdesktop.h"

#include <CalendarSupport/KCalPrefs>

using namespace IncidenceEditorNG;
using namespace CalendarSupport;

IncidenceAlarm::IncidenceAlarm(IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui)
    : mUi(ui)
    , mDateTime(dateTime)
{
    setObjectName(QStringLiteral("IncidenceAlarm"));

    mUi->mAlarmPresetCombo->insertItems(0, AlarmPresets::availablePresets());
    mUi->mAlarmPresetCombo->setCurrentIndex(AlarmPresets::defaultPresetIndex());
    updateButtons();

    connect(mDateTime, &IncidenceDateTime::startDateTimeToggled, this, &IncidenceAlarm::handleDateTimeToggle);
    connect(mDateTime, &IncidenceDateTime::endDateTimeToggled, this, &IncidenceAlarm::handleDateTimeToggle);
    connect(mUi->mAlarmAddPresetButton, &QPushButton::clicked, this, &IncidenceAlarm::newAlarmFromPreset);
    connect(mUi->mAlarmList, &QListWidget::itemSelectionChanged, this, &IncidenceAlarm::updateButtons);
    connect(mUi->mAlarmList, &QListWidget::itemDoubleClicked, this, &IncidenceAlarm::editCurrentAlarm);
    connect(mUi->mAlarmNewButton, &QPushButton::clicked, this, &IncidenceAlarm::newAlarm);
    connect(mUi->mAlarmConfigureButton, &QPushButton::clicked, this, &IncidenceAlarm::editCurrentAlarm);
    connect(mUi->mAlarmToggleButton, &QPushButton::clicked, this, &IncidenceAlarm::toggleCurrentAlarm);
    connect(mUi->mAlarmRemoveButton, &QPushButton::clicked, this, &IncidenceAlarm::removeCurrentAlarm);
}

void IncidenceAlarm::load(const KCalendarCore::Incidence::Ptr &incidence)
{
    mLoadedIncidence = incidence;
    // We must be sure that the date/time in mDateTime is the correct date time.
    // So don't depend on CombinedIncidenceEditor or whatever external factor to
    // load the date/time before loading the recurrence
    mDateTime->load(incidence);

    mAlarms.clear();
    const auto lstAlarms = incidence->alarms();
    for (const KCalendarCore::Alarm::Ptr &alarm : lstAlarms) {
        mAlarms.append(KCalendarCore::Alarm::Ptr(new KCalendarCore::Alarm(*alarm.data())));
    }

    mIsTodo = incidence->type() == KCalendarCore::Incidence::TypeTodo;
    if (mIsTodo) {
        mUi->mAlarmPresetCombo->clear();
        mUi->mAlarmPresetCombo->addItems(AlarmPresets::availablePresets(AlarmPresets::BeforeEnd));
    } else {
        mUi->mAlarmPresetCombo->clear();
        mUi->mAlarmPresetCombo->addItems(AlarmPresets::availablePresets(AlarmPresets::BeforeStart));
    }
    mUi->mAlarmPresetCombo->setCurrentIndex(AlarmPresets::defaultPresetIndex());

    handleDateTimeToggle();
    mWasDirty = false;

    updateAlarmList();
}

void IncidenceAlarm::save(const KCalendarCore::Incidence::Ptr &incidence)
{
    incidence->clearAlarms();
    const KCalendarCore::Alarm::List::ConstIterator end(mAlarms.constEnd());
    for (KCalendarCore::Alarm::List::ConstIterator it = mAlarms.constBegin(); it != end; ++it) {
        KCalendarCore::Alarm::Ptr al(new KCalendarCore::Alarm(*(*it)));
        al->setParent(incidence.data());
        // We need to make sure that both lists are the same in the end for isDirty.
        Q_ASSERT(*al == *(*it));
        incidence->addAlarm(al);
    }
}

bool IncidenceAlarm::isDirty() const
{
    if (mLoadedIncidence->alarms().count() != mAlarms.count()) {
        return true;
    }

    if (!mLoadedIncidence->alarms().isEmpty()) {
        const KCalendarCore::Alarm::List initialAlarms = mLoadedIncidence->alarms();

        if (initialAlarms.count() != mAlarms.count()) {
            return true; // The number of alarms has changed
        }

        // Note: Not the most efficient algorithm but I'm assuming that we're only
        //       dealing with a couple, at most tens of alarms. The idea is we check
        //       if all currently enabled alarms are also in the incidence. The
        //       disabled alarms are not changed by our code at all, so we assume that
        //       they're still there.
        for (const KCalendarCore::Alarm::Ptr &alarm : std::as_const(mAlarms)) {
            bool found = false;
            for (const KCalendarCore::Alarm::Ptr &initialAlarm : std::as_const(initialAlarms)) {
                if (*alarm == *initialAlarm) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                // There was an alarm in the mLoadedIncidence->alarms() that wasn't found
                // in mLastAlarms. This means that one of the alarms was modified.
                return true;
            }
        }
    }

    return false;
}

void IncidenceAlarm::editCurrentAlarm()
{
    KCalendarCore::Alarm::Ptr currentAlarm = mAlarms.at(mUi->mAlarmList->currentRow());

    QPointer<AlarmDialog> dialog(new AlarmDialog(mLoadedIncidence->type(), mUi->mTabWidget));
    dialog->load(currentAlarm);

    dialog->setAllowBeginReminders(mDateTime->startDateTimeEnabled());
    dialog->setAllowEndReminders(mDateTime->endDateTimeEnabled());

    if (dialog->exec() == QDialog::Accepted) {
        dialog->save(currentAlarm);
        updateAlarmList();
        checkDirtyStatus();
    }
    delete dialog;
}

void IncidenceAlarm::handleDateTimeToggle()
{
    QWidget *parent = mUi->mAlarmPresetCombo->parentWidget(); // the parent of a toplevel widget
    if (parent) {
        parent->setEnabled(mDateTime->startDateTimeEnabled() || mDateTime->endDateTimeEnabled());
    }

    mUi->mAlarmPresetCombo->setEnabled(mDateTime->endDateTimeEnabled());
    mUi->mAlarmAddPresetButton->setEnabled(mDateTime->endDateTimeEnabled());

    mUi->mQuickAddReminderLabel->setEnabled(mDateTime->endDateTimeEnabled());
}

void IncidenceAlarm::newAlarm()
{
    QPointer<AlarmDialog> dialog(new AlarmDialog(mLoadedIncidence->type(), mUi->mTabWidget));
    const int reminderOffset = KCalPrefs::instance()->reminderTime();

    if (reminderOffset >= 0) {
        dialog->setOffset(reminderOffset);
    } else {
        dialog->setOffset(DEFAULT_REMINDER_OFFSET);
    }
    dialog->setUnit(AlarmDialog::Minutes);
    if (mIsTodo && mDateTime->endDateTimeEnabled()) {
        dialog->setWhen(AlarmDialog::BeforeEnd);
    } else {
        dialog->setWhen(AlarmDialog::BeforeStart);
    }

    dialog->setAllowBeginReminders(mDateTime->startDateTimeEnabled());
    dialog->setAllowEndReminders(mDateTime->endDateTimeEnabled());

    if (dialog->exec() == QDialog::Accepted) {
        KCalendarCore::Alarm::Ptr newAlarm(new KCalendarCore::Alarm(nullptr));
        dialog->save(newAlarm);
        newAlarm->setEnabled(true);
        mAlarms.append(newAlarm);
        updateAlarmList();
        checkDirtyStatus();
    }
    delete dialog;
}

void IncidenceAlarm::newAlarmFromPreset()
{
    if (mIsTodo) {
        mAlarms.append(AlarmPresets::preset(AlarmPresets::BeforeEnd, mUi->mAlarmPresetCombo->currentText()));
    } else {
        mAlarms.append(AlarmPresets::preset(AlarmPresets::BeforeStart, mUi->mAlarmPresetCombo->currentText()));
    }

    updateAlarmList();
    checkDirtyStatus();
}

void IncidenceAlarm::removeCurrentAlarm()
{
    Q_ASSERT(mUi->mAlarmList->selectedItems().size() == 1);
    const int curAlarmIndex = mUi->mAlarmList->currentRow();
    delete mUi->mAlarmList->takeItem(curAlarmIndex);
    mAlarms.remove(curAlarmIndex);

    updateAlarmList();
    updateButtons();
    checkDirtyStatus();
}

void IncidenceAlarm::toggleCurrentAlarm()
{
    Q_ASSERT(mUi->mAlarmList->selectedItems().size() == 1);
    const int curAlarmIndex = mUi->mAlarmList->currentRow();
    KCalendarCore::Alarm::Ptr alarm = mAlarms.at(curAlarmIndex);
    alarm->setEnabled(!alarm->enabled());

    updateButtons();
    updateAlarmList();
    checkDirtyStatus();
}

void IncidenceAlarm::updateAlarmList()
{
    const int prevEnabledAlarmCount = mEnabledAlarmCount;
    mEnabledAlarmCount = 0;

    const QModelIndex currentIndex = mUi->mAlarmList->currentIndex();
    mUi->mAlarmList->clear();
    for (const KCalendarCore::Alarm::Ptr &alarm : std::as_const(mAlarms)) {
        mUi->mAlarmList->addItem(stringForAlarm(alarm));
        if (alarm->enabled()) {
            ++mEnabledAlarmCount;
        }
    }

    mUi->mAlarmList->setCurrentIndex(currentIndex);
    if (prevEnabledAlarmCount != mEnabledAlarmCount) {
        Q_EMIT alarmCountChanged(mEnabledAlarmCount);
    }
}

void IncidenceAlarm::updateButtons()
{
    if (mUi->mAlarmList->count() > 0 && !mUi->mAlarmList->selectedItems().isEmpty()) {
        mUi->mAlarmConfigureButton->setEnabled(true);
        mUi->mAlarmRemoveButton->setEnabled(true);
        mUi->mAlarmToggleButton->setEnabled(true);
        KCalendarCore::Alarm::Ptr selAlarm;
        if (mUi->mAlarmList->currentIndex().isValid()) {
            selAlarm = mAlarms.at(mUi->mAlarmList->currentIndex().row());
        }
        if (selAlarm && selAlarm->enabled()) {
            mUi->mAlarmToggleButton->setText(i18nc("Disable currently selected reminder", "Disable"));
        } else {
            mUi->mAlarmToggleButton->setText(i18nc("Enable currently selected reminder", "Enable"));
        }
    } else {
        mUi->mAlarmConfigureButton->setEnabled(false);
        mUi->mAlarmRemoveButton->setEnabled(false);
        mUi->mAlarmToggleButton->setEnabled(false);
    }
}

QString IncidenceAlarm::stringForAlarm(const KCalendarCore::Alarm::Ptr &alarm)
{
    Q_ASSERT(alarm);

    QString action;
    switch (alarm->type()) {
    case KCalendarCore::Alarm::Display:
        action = i18nc("Alarm action", "Display a dialog");
        break;
    case KCalendarCore::Alarm::Procedure:
        action = i18nc("Alarm action", "Execute a script");
        break;
    case KCalendarCore::Alarm::Email:
        action = i18nc("Alarm action", "Send an email");
        break;
    case KCalendarCore::Alarm::Audio:
        action = i18nc("Alarm action", "Play an audio file");
        break;
    default:
        action = i18nc("Alarm action", "Invalid Reminder.");
        return action;
    }

    const int offset = alarm->hasStartOffset() ? alarm->startOffset().asSeconds() / 60 : alarm->endOffset().asSeconds() / 60; // make minutes

    QString offsetUnitTranslated = i18ncp("The reminder is set to X minutes before/after the event", "1 minute", "%1 minutes", qAbs(offset));

    int useoffset = offset;
    if (offset % (24 * 60) == 0 && offset != 0) { // divides evenly into days?
        useoffset = offset / 60 / 24;
        offsetUnitTranslated = i18ncp("The reminder is set to X days before/after the event", "1 day", "%1 days", qAbs(useoffset));
    } else if (offset % 60 == 0 && offset != 0) { // divides evenly into hours?
        useoffset = offset / 60;
        offsetUnitTranslated = i18ncp("The reminder is set to X hours before/after the event", "1 hour", "%1 hours", qAbs(useoffset));
    }

    QString repeatStr;
    if (alarm->repeatCount() > 0) {
        repeatStr = i18nc("The reminder is configured to repeat after snooze", "(Repeats)");
    }

    if (alarm->enabled()) {
        if (useoffset > 0 && alarm->hasStartOffset()) {
            if (mIsTodo) {
                // i18n: These series of strings are used to show the user a description of
                // the alarm. %1 is replaced by one of the actions above, %2 is replaced by
                // one of the time units above, %3 is the (Repeats) part that will be used
                // in case of repetition of the alarm.
                return i18n("%1 %2 after the to-do started %3", action, offsetUnitTranslated, repeatStr);
            } else {
                return i18n("%1 %2 after the event started %3", action, offsetUnitTranslated, repeatStr);
            }
        } else if (useoffset < 0 && alarm->hasStartOffset()) {
            if (mIsTodo) {
                return i18n("%1 %2 before the to-do starts %3", action, offsetUnitTranslated, repeatStr);
            } else {
                return i18n("%1 %2 before the event starts %3", action, offsetUnitTranslated, repeatStr);
            }
        } else if (useoffset > 0 && alarm->hasEndOffset()) {
            if (mIsTodo) {
                return i18n("%1 %2 after the to-do is due %3", action, offsetUnitTranslated, repeatStr);
            } else {
                return i18n("%1 %2 after the event ends %3", action, offsetUnitTranslated, repeatStr);
            }
        } else if (useoffset < 0 && alarm->hasEndOffset()) {
            if (mIsTodo) {
                return i18n("%1 %2 before the to-do is due %3", action, offsetUnitTranslated, repeatStr);
            } else {
                return i18n("%1 %2 before the event ends %3", action, offsetUnitTranslated, repeatStr);
            }
        }
    } else {
        if (useoffset > 0 && alarm->hasStartOffset()) {
            if (mIsTodo) {
                return i18n("%1 %2 after the to-do started %3 (Disabled)", action, offsetUnitTranslated, repeatStr);
            } else {
                return i18n("%1 %2 after the event started %3 (Disabled)", action, offsetUnitTranslated, repeatStr);
            }
        } else if (useoffset < 0 && alarm->hasStartOffset()) {
            if (mIsTodo) {
                return i18n("%1 %2 before the to-do starts %3 (Disabled)", action, offsetUnitTranslated, repeatStr);
            } else {
                return i18n("%1 %2 before the event starts %3 (Disabled)", action, offsetUnitTranslated, repeatStr);
            }
        } else if (useoffset > 0 && alarm->hasEndOffset()) {
            if (mIsTodo) {
                return i18n("%1 %2 after the to-do is due %3 (Disabled)", action, offsetUnitTranslated, repeatStr);
            } else {
                return i18n("%1 %2 after the event ends %3 (Disabled)", action, offsetUnitTranslated, repeatStr);
            }
        } else if (useoffset < 0 && alarm->hasEndOffset()) {
            if (mIsTodo) {
                return i18n("%1 %2 before the to-do is due %3 (Disabled)", action, offsetUnitTranslated, repeatStr);
            } else {
                return i18n("%1 %2 before the event ends %3 (Disabled)", action, offsetUnitTranslated, repeatStr);
            }
        }
    }

    // useoffset == 0
    if (alarm->enabled()) {
        if (mIsTodo && alarm->hasStartOffset()) {
            return i18n("%1 when the to-do starts", action);
        } else if (alarm->hasStartOffset()) {
            return i18n("%1 when the event starts", action);
        } else if (mIsTodo && alarm->hasEndOffset()) {
            return i18n("%1 when the to-do is due", action);
        } else {
            return i18n("%1 when the event ends", action);
        }
    } else {
        if (mIsTodo && alarm->hasStartOffset()) {
            return i18n("%1 when the to-do starts (Disabled)", action);
        } else if (alarm->hasStartOffset()) {
            return i18n("%1 when the event starts (Disabled)", action);
        } else if (mIsTodo && alarm->hasEndOffset()) {
            return i18n("%1 when the to-do is due (Disabled)", action);
        } else {
            return i18n("%1 when the event ends (Disabled)", action);
        }
    }
}
