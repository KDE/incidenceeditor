/*
  SPDX-FileCopyrightText: 2009 Sebastian Sauer <sebsauer@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "incidenceeditor_export.h"

#include <KCalendarCore/IncidenceBase>

#include <QUrl>

#include <QDateTime>
#include <QStringList>

class KConfigSkeleton;

namespace IncidenceEditorNG
{
/**
 * Configuration details. An application can inherit from this class
 * to provide application specific configurations to the editor.
 *
 */
class INCIDENCEEDITOR_EXPORT EditorConfig
{
public:
    EditorConfig();
    virtual ~EditorConfig();

    static EditorConfig *instance();
    static void setEditorConfig(EditorConfig *);

    virtual KConfigSkeleton *config() const = 0;

    /// Return the own full name.
    Q_REQUIRED_RESULT virtual QString fullName() const;

    /// Return the own mail address.
    Q_REQUIRED_RESULT virtual QString email() const;

    /// Return true if the given email belongs to the user.
    virtual bool thatIsMe(const QString &email) const;

    /// Returns all email addresses for the user.
    Q_REQUIRED_RESULT virtual QStringList allEmails() const;

    /// Returns all email addresses together with the full username for the user.
    Q_REQUIRED_RESULT virtual QStringList fullEmails() const;

    /// Show timezone selectors in the event and todo editor dialog.
    Q_REQUIRED_RESULT virtual bool showTimeZoneSelectorInIncidenceEditor() const;

    Q_REQUIRED_RESULT virtual QDateTime defaultDuration() const
    {
        return QDateTime(QDate(1752, 1, 1), QTime(2, 0));
    }

    Q_REQUIRED_RESULT virtual QDateTime startTime() const
    {
        return QDateTime(QDate(1752, 1, 1), QTime(10, 0));
    }

    Q_REQUIRED_RESULT virtual bool defaultAudioFileReminders() const
    {
        return false;
    }

    Q_REQUIRED_RESULT virtual QUrl audioFilePath() const
    {
        return QUrl();
    }

    Q_REQUIRED_RESULT virtual int reminderTime() const
    {
        return 15;
    }

    Q_REQUIRED_RESULT virtual int reminderTimeUnits() const
    {
        return 0;
    }

    Q_REQUIRED_RESULT virtual bool defaultTodoReminders() const
    {
        return false;
    }

    Q_REQUIRED_RESULT virtual bool defaultEventReminders() const
    {
        return false;
    }

    Q_REQUIRED_RESULT virtual QStringList activeDesignerFields() const
    {
        return QStringList();
    }

    Q_REQUIRED_RESULT virtual QStringList &templates(KCalendarCore::IncidenceBase::IncidenceType type);

private:
    class Private;
    Private *const d;
};
}

