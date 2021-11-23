/*
  SPDX-FileCopyrightText: 2009 Sebastian Sauer <sebsauer@kdab.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "editorconfig.h"
#include "korganizereditorconfig.h"

#include <QCoreApplication>

using namespace IncidenceEditorNG;

class IncidenceEditorNG::EditorConfigPrivate
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

EditorConfig *EditorConfigPrivate::config = nullptr;

EditorConfig::EditorConfig()
    : d(new EditorConfigPrivate)
{
}

EditorConfig::~EditorConfig() = default;

EditorConfig *EditorConfig::instance()
{
    if (!EditorConfigPrivate::config) {
        // No one called setEditorConfig(), so we default to a KorganizerEditorConfig.
        EditorConfig::setEditorConfig(new IncidenceEditorNG::KOrganizerEditorConfig);
    }

    return EditorConfigPrivate::config;
}

void EditorConfig::setEditorConfig(EditorConfig *config)
{
    delete EditorConfigPrivate::config;
    EditorConfigPrivate::config = config;
    qAddPostRoutine(EditorConfigPrivate::cleanup_config);
}

QString EditorConfig::fullName() const
{
    if (EditorConfigPrivate::config != this) {
        return EditorConfigPrivate::config->fullName();
    }
    return {};
}

QString EditorConfig::email() const
{
    if (EditorConfigPrivate::config != this) {
        return EditorConfigPrivate::config->email();
    }
    return {};
}

bool EditorConfig::thatIsMe(const QString &mail) const
{
    if (EditorConfigPrivate::config != this) {
        return EditorConfigPrivate::config->thatIsMe(mail);
    }
    return false;
}

QStringList EditorConfig::allEmails() const
{
    if (EditorConfigPrivate::config != this) {
        return EditorConfigPrivate::config->allEmails();
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
    if (EditorConfigPrivate::config != this) {
        return EditorConfigPrivate::config->fullEmails();
    }
    return {};
}

bool EditorConfig::showTimeZoneSelectorInIncidenceEditor() const
{
    if (EditorConfigPrivate::config != this) {
        return EditorConfigPrivate::config->showTimeZoneSelectorInIncidenceEditor();
    }
    return true;
}

QStringList &EditorConfig::templates(KCalendarCore::IncidenceBase::IncidenceType type)
{
    return d->mTemplates[type];
}
