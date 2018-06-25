/*
  Copyright (c) 2010 Kevin Ottens <ervin@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef INCIDENCEEDITOR_KORGANIZEREDITORCONFIG_H
#define INCIDENCEEDITOR_KORGANIZEREDITORCONFIG_H

#include "editorconfig.h"

#include <KCalCore/IncidenceBase>

namespace IncidenceEditorNG {
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
    Q_REQUIRED_RESULT QStringList &templates(KCalCore::IncidenceBase::IncidenceType type) override;
};
} // IncidenceEditors

#endif
