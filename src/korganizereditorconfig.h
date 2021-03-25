/*
  SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "editorconfig.h"

#include <KCalendarCore/IncidenceBase>

namespace IncidenceEditorNG
{
/**
 * @brief The KOrganizerEditorConfig class
 */
class INCIDENCEEDITOR_EXPORT KOrganizerEditorConfig : public IncidenceEditorNG::EditorConfig
{
public:
    KOrganizerEditorConfig();
    ~KOrganizerEditorConfig() override;

    KConfigSkeleton *config() const override;
    Q_REQUIRED_RESULT QString fullName() const override;
    Q_REQUIRED_RESULT QString email() const override;
    Q_REQUIRED_RESULT bool thatIsMe(const QString &email) const override;
    Q_REQUIRED_RESULT QStringList allEmails() const override;
    Q_REQUIRED_RESULT QStringList fullEmails() const override;
    Q_REQUIRED_RESULT bool showTimeZoneSelectorInIncidenceEditor() const override;
    Q_REQUIRED_RESULT QDateTime defaultDuration() const override;
    Q_REQUIRED_RESULT QDateTime startTime() const override;
    Q_REQUIRED_RESULT bool defaultAudioFileReminders() const override;
    Q_REQUIRED_RESULT QUrl audioFilePath() const override;
    Q_REQUIRED_RESULT int reminderTime() const override;
    Q_REQUIRED_RESULT int reminderTimeUnits() const override;
    Q_REQUIRED_RESULT bool defaultTodoReminders() const override;
    Q_REQUIRED_RESULT bool defaultEventReminders() const override;
    Q_REQUIRED_RESULT QStringList activeDesignerFields() const override;
    Q_REQUIRED_RESULT QStringList &templates(KCalendarCore::IncidenceBase::IncidenceType type) override;
};
} // IncidenceEditors

