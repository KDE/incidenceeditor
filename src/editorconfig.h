/*
  SPDX-FileCopyrightText: 2009 Sebastian Sauer <sebsauer@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "incidenceeditor_export.h"

#include <KCalendarCore/IncidenceBase>

#include <QUrl>

#include <QDateTime>
#include <QList>
#include <QStringList>

#include <memory>

class KConfigSkeleton;

namespace IncidenceEditorNG
{
class EditorConfigPrivate;

/**
 * Configuration details. An application can inherit from this class
 * to provide application specific configurations to the editor.
 *
 */
class INCIDENCEEDITOR_EXPORT EditorConfig
{
public:
    struct Organizer {
        QString name;
        QString email;
        bool sign = false;
        bool encrypt = false;
    };

    EditorConfig();
    virtual ~EditorConfig();

    static EditorConfig *instance();
    static void setEditorConfig(EditorConfig *);

    virtual KConfigSkeleton *config() const = 0;

    /// Return the own full name.
    [[nodiscard]] virtual QString fullName() const;

    /// Return the own mail address.
    [[nodiscard]] virtual QString email() const;

    /// Return true if the given email belongs to the user.
    virtual bool thatIsMe(const QString &email) const;

    /// Returns all email addresses for the user.
    [[nodiscard]] virtual QStringList allEmails() const;

    /// Returns all email addresses together with the full username for the user.
    [[nodiscard]] virtual QList<Organizer> allOrganizers() const;

    /// Show timezone selectors in the event and todo editor dialog.
    [[nodiscard]] virtual bool showTimeZoneSelectorInIncidenceEditor() const;

    [[nodiscard]] virtual QDateTime defaultDuration() const
    {
        return QDateTime(QDate(1752, 1, 1), QTime(2, 0));
    }

    [[nodiscard]] virtual QDateTime startTime() const
    {
        return QDateTime(QDate(1752, 1, 1), QTime(10, 0));
    }

    [[nodiscard]] virtual bool defaultAudioFileReminders() const
    {
        return false;
    }

    [[nodiscard]] virtual QUrl audioFilePath() const
    {
        return {};
    }

    [[nodiscard]] virtual int reminderTime() const
    {
        return 15;
    }

    [[nodiscard]] virtual int reminderTimeUnits() const
    {
        return 0;
    }

    [[nodiscard]] virtual bool defaultTodoReminders() const
    {
        return false;
    }

    [[nodiscard]] virtual bool defaultEventReminders() const
    {
        return false;
    }

    [[nodiscard]] virtual QStringList activeDesignerFields() const
    {
        return {};
    }

    [[nodiscard]] virtual QStringList &templates(KCalendarCore::IncidenceBase::IncidenceType type);

private:
    std::unique_ptr<EditorConfigPrivate> const d;
};
}
