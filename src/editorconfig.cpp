/*
  SPDX-FileCopyrightText: 2009 Sebastian Sauer <sebsauer@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "editorconfig.h"
#include "korganizereditorconfig.h"

#include <QCoreApplication>

using namespace IncidenceEditorNG;

class EditorConfig::Private
{
public:
    static EditorConfig *config;
    static void cleanup_config()
    {
        delete config;
        config = nullptr;
    }

    QHash<KCalendarCore::IncidenceBase::IncidenceType, QStringList> mTemplates;
};

EditorConfig *EditorConfig::Private::config = nullptr;

EditorConfig::EditorConfig()
    : d(new Private)
{
}

EditorConfig::~EditorConfig()
{
    delete d;
}

EditorConfig *EditorConfig::instance()
{
    if (!Private::config) {
        // No one called setEditorConfig(), so we default to a KorganizerEditorConfig.
        EditorConfig::setEditorConfig(new IncidenceEditorNG::KOrganizerEditorConfig);
    }

    return Private::config;
}

void EditorConfig::setEditorConfig(EditorConfig *config)
{
    delete Private::config;
    Private::config = config;
    qAddPostRoutine(Private::cleanup_config);
}

QString EditorConfig::fullName() const
{
    if (Private::config != this) {
        return Private::config->fullName();
    }
    return QString();
}

QString EditorConfig::email() const
{
    if (Private::config != this) {
        return Private::config->email();
    }
    return QString();
}

bool EditorConfig::thatIsMe(const QString &mail) const
{
    if (Private::config != this) {
        return Private::config->thatIsMe(mail);
    }
    return false;
}

QStringList EditorConfig::allEmails() const
{
    if (Private::config != this) {
        return Private::config->allEmails();
    }

    QStringList mails;
    const QString m = email();
    if (!m.isEmpty()) {
        mails << m;
    }
    return mails;
}

QStringList EditorConfig::fullEmails() const
{
    if (Private::config != this) {
        return Private::config->fullEmails();
    }
    return QStringList();
}

bool EditorConfig::showTimeZoneSelectorInIncidenceEditor() const
{
    if (Private::config != this) {
        return Private::config->showTimeZoneSelectorInIncidenceEditor();
    }
    return true;
}

QStringList &EditorConfig::templates(KCalendarCore::IncidenceBase::IncidenceType type)
{
    return d->mTemplates[type];
}
