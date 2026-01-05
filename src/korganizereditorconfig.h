/*
  SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include "editorconfig.h"
#include "incidenceeditor_private_export.h"

#include <KCalendarCore/IncidenceBase>

namespace IncidenceEditorNG
{
class INCIDENCEEDITOR_TESTS_EXPORT KOrganizerEditorConfig : public IncidenceEditorNG::EditorConfig
{
public:
    KOrganizerEditorConfig();
    ~KOrganizerEditorConfig() override;

    KConfigSkeleton *config() const override;
    [[nodiscard]] QString fullName() const override;
    [[nodiscard]] QString email() const override;
    [[nodiscard]] bool thatIsMe(const QString &email) const override;
    [[nodiscard]] QStringList allEmails() const override;
    [[nodiscard]] QList<Organizer> allOrganizers() const override;
    [[nodiscard]] bool showTimeZoneSelectorInIncidenceEditor() const override;
    [[nodiscard]] QDateTime defaultDuration() const override;
    [[nodiscard]] QDateTime startTime() const override;
    [[nodiscard]] bool defaultAudioFileReminders() const override;
    [[nodiscard]] QUrl audioFilePath() const override;
    [[nodiscard]] int reminderTime() const override;
    [[nodiscard]] int reminderTimeUnits() const override;
    [[nodiscard]] bool defaultTodoReminders() const override;
    [[nodiscard]] bool defaultEventReminders() const override;
    [[nodiscard]] QStringList &templates(KCalendarCore::IncidenceBase::IncidenceType type) override;
};
} // IncidenceEditors
