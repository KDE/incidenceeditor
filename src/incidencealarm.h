/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "incidenceeditor-ng.h"

namespace Ui
{
class EventOrTodoDesktop;
}

namespace IncidenceEditorNG
{
class IncidenceDateTime;

class IncidenceAlarm : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual
    IncidenceAlarm(IncidenceDateTime *dateTime, Ui::EventOrTodoDesktop *ui);

    void load(const KCalendarCore::Incidence::Ptr &incidence) override;
    void save(const KCalendarCore::Incidence::Ptr &incidence) override;
    Q_REQUIRED_RESULT bool isDirty() const override;

Q_SIGNALS:
    void alarmCountChanged(int newCount);

private:
    void editCurrentAlarm();
    void handleDateTimeToggle();
    void newAlarm();
    void newAlarmFromPreset();
    void removeCurrentAlarm();
    void toggleCurrentAlarm();
    void updateAlarmList();
    void updateButtons();
    QString stringForAlarm(const KCalendarCore::Alarm::Ptr &alarm);

private:
    Ui::EventOrTodoDesktop *const mUi;

    KCalendarCore::Alarm::List mAlarms;
    IncidenceDateTime *mDateTime = nullptr;
    int mEnabledAlarmCount = 0;
    bool mIsTodo = false;
};
}

