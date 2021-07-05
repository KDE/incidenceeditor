/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-enterprise.h>

#include "alarmpresets.h"
#include "incidencedefaults.h"
#include "incidenceeditor_debug.h"

#include <CalendarSupport/KCalPrefs>
#include <akonadi/calendar/calendarsettings.h> //krazy:exclude=camelcase this is a generated file

#include <KContacts/Addressee>

#include <KCalendarCore/Alarm>
#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Todo>

#include <KEmailAddress>

#include <KIO/Job>
#include <KIO/StoredTransferJob>
#include <KLocalizedString>

#include <QFile>
#include <QUrl>

using namespace CalendarSupport;
using namespace IncidenceEditorNG;
using namespace KCalendarCore;

namespace IncidenceEditorNG
{
enum { UNSPECIFED_PRIORITY = 0 };

class IncidenceDefaultsPrivate
{
public:
    /// Members
    KCalendarCore::Attachment::List mAttachments;
    QVector<KCalendarCore::Attendee> mAttendees;
    QStringList mEmails;
    QString mGroupWareDomain;
    KCalendarCore::Incidence::Ptr mRelatedIncidence;
    QDateTime mStartDt;
    QDateTime mEndDt;
    bool mCleanupTemporaryFiles;

    /// Methods
    KCalendarCore::Person organizerAsPerson() const;
    KCalendarCore::Attendee organizerAsAttendee(const KCalendarCore::Person &organizer) const;

    void todoDefaults(const KCalendarCore::Todo::Ptr &todo) const;
    void eventDefaults(const KCalendarCore::Event::Ptr &event) const;
    void journalDefaults(const KCalendarCore::Journal::Ptr &journal) const;
};
}

KCalendarCore::Person IncidenceDefaultsPrivate::organizerAsPerson() const
{
    const QString invalidEmail = IncidenceDefaults::invalidEmailAddress();

    KCalendarCore::Person organizer;
    organizer.setName(i18nc("@label", "no (valid) identities found"));
    organizer.setEmail(invalidEmail);

    if (mEmails.isEmpty()) {
        // Don't bother any longer, either someone forget to call setFullEmails, or
        // the user has no identities configured.
        return organizer;
    }

    if (!mGroupWareDomain.isEmpty()) {
        // Check if we have an identity with an email that ends with the groupware
        // domain.
        for (const QString &fullEmail : std::as_const(mEmails)) {
            QString name;
            QString email;
            const bool success = KEmailAddress::extractEmailAddressAndName(fullEmail, email, name);
            if (success && email.endsWith(mGroupWareDomain)) {
                organizer.setName(name);
                organizer.setEmail(email);
                break;
            }
        }
    }

    if (organizer.email() == invalidEmail) {
        // Either, no groupware was used, or we didn't find a groupware email address.
        // Now try to
        for (const QString &fullEmail : std::as_const(mEmails)) {
            QString name;
            QString email;
            const bool success = KEmailAddress::extractEmailAddressAndName(fullEmail, email, name);
            if (success) {
                organizer.setName(name);
                organizer.setEmail(email);
                break;
            }
        }
    }

    return organizer;
}

KCalendarCore::Attendee IncidenceDefaultsPrivate::organizerAsAttendee(const KCalendarCore::Person &organizer) const
{
    KCalendarCore::Attendee organizerAsAttendee;
    // Really, the appropriate values (even the fall back values) should come from
    // organizer. (See organizerAsPerson for more details).
    organizerAsAttendee.setName(organizer.name());
    organizerAsAttendee.setEmail(organizer.email());
    // NOTE: Don't set the status to None, this value is not supported by the attendee
    //       editor atm.
    organizerAsAttendee.setStatus(KCalendarCore::Attendee::Accepted);
    organizerAsAttendee.setRole(KCalendarCore::Attendee::ReqParticipant);
    return organizerAsAttendee;
}

void IncidenceDefaultsPrivate::eventDefaults(const KCalendarCore::Event::Ptr &event) const
{
    QDateTime startDT;
    if (mStartDt.isValid()) {
        startDT = mStartDt;
    } else {
        startDT = QDateTime::currentDateTime();

        if (KCalPrefs::instance()->startTime().isValid()) {
            startDT.setTime(KCalPrefs::instance()->startTime().time());
        }
    }

    const QTime defaultDurationTime = KCalPrefs::instance()->defaultDuration().time();
    const int defaultDuration = (defaultDurationTime.hour() * 3600) + (defaultDurationTime.minute() * 60);

    const QDateTime endDT = mEndDt.isValid() ? mEndDt : startDT.addSecs(defaultDuration);

    event->setDtStart(startDT);
    event->setDtEnd(endDT);
    event->setTransparency(KCalendarCore::Event::Opaque);

    if (KCalPrefs::instance()->defaultEventReminders()) {
        event->addAlarm(AlarmPresets::defaultAlarm(AlarmPresets::BeforeStart));
    }
}

void IncidenceDefaultsPrivate::journalDefaults(const KCalendarCore::Journal::Ptr &journal) const
{
    const QDateTime startDT = mStartDt.isValid() ? mStartDt : QDateTime::currentDateTime();
    journal->setDtStart(startDT);
    journal->setAllDay(true);
}

void IncidenceDefaultsPrivate::todoDefaults(const KCalendarCore::Todo::Ptr &todo) const
{
    KCalendarCore::Todo::Ptr relatedTodo = mRelatedIncidence.dynamicCast<KCalendarCore::Todo>();
    if (relatedTodo) {
        todo->setCategories(relatedTodo->categories());
    }

    if (mEndDt.isValid()) {
        todo->setDtDue(mEndDt, true /** first */);
    } else if (relatedTodo && relatedTodo->hasDueDate()) {
        todo->setDtDue(relatedTodo->dtDue(true), true /** first */);
        todo->setAllDay(relatedTodo->allDay());
    } else if (relatedTodo) {
        todo->setDtDue(QDateTime());
    } else {
        todo->setDtDue(QDateTime::currentDateTime().addDays(1), true /** first */);
    }

    if (mStartDt.isValid()) {
        todo->setDtStart(mStartDt);
    } else if (relatedTodo && !relatedTodo->hasStartDate()) {
        todo->setDtStart(QDateTime());
    } else if (relatedTodo && relatedTodo->hasStartDate() && relatedTodo->dtStart() <= todo->dtDue()) {
        todo->setDtStart(relatedTodo->dtStart());
        todo->setAllDay(relatedTodo->allDay());
    } else if (!mEndDt.isValid() || (QDateTime::currentDateTime() < mEndDt)) {
        todo->setDtStart(QDateTime::currentDateTime());
    } else {
        todo->setDtStart(mEndDt.addDays(-1));
    }

    todo->setCompleted(false);
    todo->setPercentComplete(0);

    // I had a bunch of to-dos and couldn't distinguish between those that had priority '5'
    // because I wanted, and those that had priority '5' because it was set by default
    // and I forgot to unset it.
    // So don't be smart and try to guess a good default priority for the user, just use unspecified.
    todo->setPriority(UNSPECIFED_PRIORITY);

    if (KCalPrefs::instance()->defaultTodoReminders()) {
        todo->addAlarm(AlarmPresets::defaultAlarm(AlarmPresets::BeforeEnd));
    }
}

/// IncidenceDefaults

IncidenceDefaults::IncidenceDefaults(bool cleanupAttachmentTemporaryFiles)
    : d_ptr(new IncidenceDefaultsPrivate)
{
    d_ptr->mCleanupTemporaryFiles = cleanupAttachmentTemporaryFiles;
}

IncidenceDefaults::IncidenceDefaults(const IncidenceDefaults &other)
    : d_ptr(new IncidenceDefaultsPrivate)
{
    *d_ptr = *other.d_ptr;
}

IncidenceDefaults::~IncidenceDefaults()
{
    delete d_ptr;
}

IncidenceDefaults &IncidenceDefaults::operator=(const IncidenceDefaults &other)
{
    if (&other != this) {
        *d_ptr = *other.d_ptr;
    }
    return *this;
}

void IncidenceDefaults::setAttachments(const QStringList &attachments,
                                       const QStringList &attachmentMimetypes,
                                       const QStringList &attachmentLabels,
                                       bool inlineAttachment)
{
    Q_D(IncidenceDefaults);
    d->mAttachments.clear();

    QStringList::ConstIterator it;
    int i = 0;
    for (it = attachments.constBegin(); it != attachments.constEnd(); ++it, ++i) {
        if (!(*it).isEmpty()) {
            QString mimeType;
            if (attachmentMimetypes.count() > i) {
                mimeType = attachmentMimetypes[i];
            }

            KCalendarCore::Attachment attachment;
            if (inlineAttachment) {
                auto job = KIO::storedGet(QUrl::fromUserInput(*it));
                if (job->exec()) {
                    const QByteArray data = job->data();
                    attachment = KCalendarCore::Attachment(data.toBase64(), mimeType);

                    if (i < attachmentLabels.count()) {
                        attachment.setLabel(attachmentLabels[i]);
                    }
                } else {
                    qCCritical(INCIDENCEEDITOR_LOG) << "Error downloading uri " << *it << job->errorString();
                }

                if (d_ptr->mCleanupTemporaryFiles) {
                    QFile file(*it);
                    if (!file.remove()) {
                        qCCritical(INCIDENCEEDITOR_LOG) << "Uname to remove file " << *it;
                    }
                }
            } else {
                attachment = KCalendarCore::Attachment(*it, mimeType);
                if (i < attachmentLabels.count()) {
                    attachment.setLabel(attachmentLabels[i]);
                }
            }

            if (!attachment.isEmpty()) {
                if (attachment.label().isEmpty()) {
                    if (attachment.isUri()) {
                        attachment.setLabel(attachment.uri());
                    } else {
                        attachment.setLabel(i18nc("@label attachment contains binary data", "[Binary data]"));
                    }
                }
                d->mAttachments << attachment;
                attachment.setShowInline(inlineAttachment);
            }
        }
    }
}

void IncidenceDefaults::setAttendees(const QStringList &attendees)
{
    Q_D(IncidenceDefaults);
    d->mAttendees.clear();
    QStringList::ConstIterator it;
    for (it = attendees.begin(); it != attendees.end(); ++it) {
        QString name, email;
        KContacts::Addressee::parseEmailAddress(*it, name, email);
        d->mAttendees << KCalendarCore::Attendee(name, email, true, KCalendarCore::Attendee::NeedsAction);
    }
}

void IncidenceDefaults::setFullEmails(const QStringList &fullEmails)
{
    Q_D(IncidenceDefaults);
    d->mEmails = fullEmails;
}

void IncidenceDefaults::setGroupWareDomain(const QString &domain)
{
    Q_D(IncidenceDefaults);
    d->mGroupWareDomain = domain;
}

void IncidenceDefaults::setRelatedIncidence(const KCalendarCore::Incidence::Ptr &incidence)
{
    Q_D(IncidenceDefaults);
    d->mRelatedIncidence = incidence;
}

void IncidenceDefaults::setStartDateTime(const QDateTime &startDT)
{
    Q_D(IncidenceDefaults);
    d->mStartDt = startDT;
}

void IncidenceDefaults::setEndDateTime(const QDateTime &endDT)
{
    Q_D(IncidenceDefaults);
    d->mEndDt = endDT;
}

void IncidenceDefaults::setDefaults(const KCalendarCore::Incidence::Ptr &incidence) const
{
    Q_D(const IncidenceDefaults);

    // First some general defaults
    incidence->setSummary(QString(), false);
    incidence->setLocation(QString(), false);
    incidence->setCategories(QStringList());
    incidence->setSecrecy(KCalendarCore::Incidence::SecrecyPublic);
    incidence->setStatus(KCalendarCore::Incidence::StatusNone);
    incidence->setAllDay(false);
    incidence->setCustomStatus(QString());
    incidence->setResources(QStringList());
    incidence->setPriority(0);

    if (d->mRelatedIncidence) {
        incidence->setRelatedTo(d->mRelatedIncidence->uid());
    }

    incidence->clearAlarms();
    incidence->clearAttachments();
    incidence->clearAttendees();
    incidence->clearComments();
    incidence->clearContacts();
    incidence->clearRecurrence();

    const KCalendarCore::Person organizerAsPerson = d->organizerAsPerson();
#if KDEPIM_ENTERPRISE_BUILD
    incidence->addAttendee(d->organizerAsAttendee(organizerAsPerson));
#endif
    for (const KCalendarCore::Attendee &attendee : std::as_const(d->mAttendees)) {
        incidence->addAttendee(attendee);
    }
    // Ical standard: No attendees -> must not have an organizer!
    if (incidence->attendeeCount()) {
        incidence->setOrganizer(organizerAsPerson);
    }

    for (const KCalendarCore::Attachment &attachment : std::as_const(d->mAttachments)) {
        incidence->addAttachment(attachment);
    }

    switch (incidence->type()) {
    case KCalendarCore::Incidence::TypeEvent:
        d->eventDefaults(incidence.dynamicCast<KCalendarCore::Event>());
        break;
    case KCalendarCore::Incidence::TypeTodo:
        d->todoDefaults(incidence.dynamicCast<KCalendarCore::Todo>());
        break;
    case KCalendarCore::Incidence::TypeJournal:
        d->journalDefaults(incidence.dynamicCast<KCalendarCore::Journal>());
        break;
    default:
        qCDebug(INCIDENCEEDITOR_LOG) << "Unsupported incidence type, keeping current values. Type: " << static_cast<int>(incidence->type());
    }
}

/** static */
IncidenceDefaults IncidenceDefaults::minimalIncidenceDefaults(bool cleanupAttachmentTempFiles)
{
    IncidenceDefaults defaults(cleanupAttachmentTempFiles);

    // Set the full emails manually here, to avoid that we get dependencies on
    // KCalPrefs all over the place.
    defaults.setFullEmails(CalendarSupport::KCalPrefs::instance()->fullEmails());

    // NOTE: At some point this should be generalized. That is, we now use the
    //       freebusy url as a hack, but this assumes that the user has only one
    //       groupware account. Which doesn't have to be the case necessarily.
    //       This method should somehow depend on the calendar selected to which
    //       the incidence is added.
    if (CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication()) {
        defaults.setGroupWareDomain(QUrl(Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl()).host());
    }
    return defaults;
}

/** static */
QString IncidenceDefaults::invalidEmailAddress()
{
    static const QString invalidEmail(i18nc("@label invalid email address marker", "invalid@email.address"));
    return invalidEmail;
}
