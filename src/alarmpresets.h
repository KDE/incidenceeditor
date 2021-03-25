/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/Alarm>

#include <QStringList>

namespace IncidenceEditorNG
{
enum {
    // Fallback in case config is invalid
    DEFAULT_REMINDER_OFFSET = 15 // minutes
};

namespace AlarmPresets
{
enum When { BeforeStart, BeforeEnd };

/**
 * Returns the available presets.
 */
Q_REQUIRED_RESULT QStringList availablePresets(When when = BeforeStart);

/**
 * Returns a recurrence preset for given name. The name <em>must</em> be one
 * of availablePresets().
 *
 * Note: The caller takes ownership over the pointer.
 */
Q_REQUIRED_RESULT KCalendarCore::Alarm::Ptr preset(When when, const QString &name);

/**
 * Returns an Alarm configured accordingly to the default preset.
 *
 * Note: The caller takes ownership over the pointer.
 */
Q_REQUIRED_RESULT KCalendarCore::Alarm::Ptr defaultAlarm(When when);

/**
 * Returns the index of the preset in availablePresets for the given recurrence,
 * or -1 if no preset is equal to the given recurrence.
 */
Q_REQUIRED_RESULT int presetIndex(When when, const KCalendarCore::Alarm::Ptr &alarm);

/**
   Returns the index of the default preset. ( Comes from KCalPrefs ).
 */
Q_REQUIRED_RESULT int defaultPresetIndex();
}
}

