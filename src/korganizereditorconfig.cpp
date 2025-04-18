/*
  SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "korganizereditorconfig.h"

#include <CalendarSupport/KCalPrefs>

#include <KIdentityManagementCore/Identity>
#include <KIdentityManagementCore/IdentityManager>

using namespace IncidenceEditorNG;

KOrganizerEditorConfig::KOrganizerEditorConfig()

{
}

KOrganizerEditorConfig::~KOrganizerEditorConfig() = default;

KConfigSkeleton *KOrganizerEditorConfig::config() const
{
    return CalendarSupport::KCalPrefs::instance();
}

QString KOrganizerEditorConfig::fullName() const
{
    return CalendarSupport::KCalPrefs::instance()->fullName();
}

QString KOrganizerEditorConfig::email() const
{
    return CalendarSupport::KCalPrefs::instance()->email();
}

bool KOrganizerEditorConfig::thatIsMe(const QString &email) const
{
    return CalendarSupport::KCalPrefs::instance()->thatIsMe(email);
}

QStringList KOrganizerEditorConfig::allEmails() const
{
    return CalendarSupport::KCalPrefs::instance()->allEmails();
}

QList<EditorConfig::Organizer> KOrganizerEditorConfig::allOrganizers() const
{
    // TODO add activities support here too
    const auto *manager = KIdentityManagementCore::IdentityManager::self();
    QList<EditorConfig::Organizer> organizers;
    std::transform(manager->begin(), manager->end(), std::back_inserter(organizers), [](const auto &identity) {
        return EditorConfig::Organizer{identity.fullName(), identity.fullEmailAddr(), identity.pgpAutoSign(), identity.pgpAutoEncrypt()};
    });
    return organizers;
}

bool KOrganizerEditorConfig::showTimeZoneSelectorInIncidenceEditor() const
{
    return CalendarSupport::KCalPrefs::instance()->showTimeZoneSelectorInIncidenceEditor();
}

QDateTime KOrganizerEditorConfig::defaultDuration() const
{
    return CalendarSupport::KCalPrefs::instance()->defaultDuration();
}

QDateTime KOrganizerEditorConfig::startTime() const
{
    return CalendarSupport::KCalPrefs::instance()->startTime();
}

bool KOrganizerEditorConfig::defaultAudioFileReminders() const
{
    return CalendarSupport::KCalPrefs::instance()->defaultAudioFileReminders();
}

QUrl KOrganizerEditorConfig::audioFilePath() const
{
    return QUrl::fromLocalFile(CalendarSupport::KCalPrefs::instance()->audioFilePath());
}

int KOrganizerEditorConfig::reminderTime() const
{
    return CalendarSupport::KCalPrefs::instance()->reminderTime();
}

int KOrganizerEditorConfig::reminderTimeUnits() const
{
    return CalendarSupport::KCalPrefs::instance()->reminderTimeUnits();
}

bool KOrganizerEditorConfig::defaultTodoReminders() const
{
    return CalendarSupport::KCalPrefs::instance()->defaultTodoReminders();
}

bool KOrganizerEditorConfig::defaultEventReminders() const
{
    return CalendarSupport::KCalPrefs::instance()->defaultEventReminders();
}

QStringList &KOrganizerEditorConfig::templates(KCalendarCore::IncidenceBase::IncidenceType type)
{
    if (type == KCalendarCore::IncidenceBase::TypeEvent) {
        // TODO remove mEventTemplates+etc from Prefs::instance()
        return CalendarSupport::KCalPrefs::instance()->mEventTemplates;
    }
    if (type == KCalendarCore::IncidenceBase::TypeTodo) {
        return CalendarSupport::KCalPrefs::instance()->mTodoTemplates;
    }
    if (type == KCalendarCore::IncidenceBase::TypeJournal) {
        return CalendarSupport::KCalPrefs::instance()->mJournalTemplates;
    }
    return EditorConfig::templates(type);
}
