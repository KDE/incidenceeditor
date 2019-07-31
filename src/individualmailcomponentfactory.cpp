/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#include "individualmailcomponentfactory.h"
#include "individualmaildialog.h"
#include "opencomposerjob.h"

#include <CalendarSupport/KCalPrefs>

#include <KMessageBox>

#include <QDBusConnection>
#include <QDBusConnectionInterface>

using namespace IncidenceEditorNG;

// IndividualMessageQueueJob

IndividualMessageQueueJob::IndividualMessageQueueJob(const KIdentityManagement::Identity &identity, const KCalendarCore::Attendee::List &update, const KCalendarCore::Attendee::List &edit, QObject *parent)
    : MailTransport::MessageQueueJob(parent)
    , mUpdate(update)
    , mEdit(edit)
    , mIdentity(identity)
    , mQueueJob(nullptr)
    , mComposerJob(nullptr)
{
}

void IndividualMessageQueueJob::start()
{
    QSet<QString> attendeesTo(QSet<QString>::fromList(addressAttribute().to()));
    QSet<QString> attendeesCc(QSet<QString>::fromList(addressAttribute().cc()));

    QStringList attendeesAutoTo, attendeesAutoCc;
    for (const KCalendarCore::Attendee &attendee : qAsConst(mUpdate)) {
        if (attendeesTo.contains(attendee.email())) {
            attendeesAutoTo.append(attendee.fullName());
        }
        if (attendeesCc.contains(attendee.email())) {
            attendeesAutoCc.append(attendee.fullName());
        }
    }
    if (!attendeesAutoTo.isEmpty() || !attendeesAutoCc.isEmpty()
        || !addressAttribute().bcc().isEmpty()) {
        startQueueJob(attendeesAutoTo, addressAttribute().to(), attendeesAutoCc, addressAttribute().cc());
    }

    QStringList attendeesComposerTo, attendeesComposerCc;
    for (const KCalendarCore::Attendee &attendee : qAsConst(mEdit)) {
        if (attendeesTo.contains(attendee.email())) {
            attendeesComposerTo.append(attendee.fullName());
        }
        if (attendeesCc.contains(attendee.email())) {
            attendeesComposerCc.append(attendee.fullName());
        }
    }
    if (!attendeesComposerTo.isEmpty() || !attendeesComposerCc.isEmpty()) {
        startComposerJob(attendeesComposerTo, attendeesComposerCc);
    }

    // No subjob has been started
    if (!mQueueJob && !mComposerJob) {
        emitResult();
    }
}

void IndividualMessageQueueJob::startQueueJob(const QStringList &messageTo, const QStringList &to, const QStringList &messageCc, const QStringList &cc)
{
    KMime::Message::Ptr msg(message());
    msg->to()->fromUnicodeString(messageTo.join(QStringLiteral(", ")), "utf-8");
    msg->cc()->fromUnicodeString(messageCc.join(QStringLiteral(", ")), "utf-8");
    msg->assemble();

    mQueueJob = new MailTransport::MessageQueueJob(this);
    mQueueJob->setMessage(msg);
    mQueueJob->transportAttribute().setTransportId(mIdentity.isNull() ? transportAttribute().transportId() : mIdentity.transport().toInt());
    mQueueJob->addressAttribute().setFrom(addressAttribute().from());
    mQueueJob->addressAttribute().setTo(to);
    mQueueJob->addressAttribute().setCc(cc);
    mQueueJob->addressAttribute().setBcc(addressAttribute().bcc());

    if (mIdentity.disabledFcc()) {
        mQueueJob->sentBehaviourAttribute().setSentBehaviour(MailTransport::SentBehaviourAttribute::Delete);
    } else {
        const Akonadi::Collection sentCollection(mIdentity.fcc().toLongLong());
        if (sentCollection.isValid()) {
            mQueueJob->sentBehaviourAttribute().setSentBehaviour(MailTransport::SentBehaviourAttribute::MoveToCollection);
            mQueueJob->sentBehaviourAttribute().setMoveToCollection(sentCollection);
        } else {
            mQueueJob->sentBehaviourAttribute().setSentBehaviour(
                MailTransport::SentBehaviourAttribute::MoveToDefaultSentCollection);
        }
    }

    connect(mQueueJob, &MailTransport::MessageQueueJob::finished, this,
            &IndividualMessageQueueJob::handleJobFinished);
    mQueueJob->start();
}

void IndividualMessageQueueJob::startComposerJob(const QStringList &to, const QStringList &cc)
{
    mComposerJob
        = new OpenComposerJob(this, to.join(QStringLiteral(", ")), cc.join(QStringLiteral(
                                                                               ", ")),
                              QString(), message(), mIdentity);
    connect(mComposerJob, &OpenComposerJob::finished, this,
            &IndividualMessageQueueJob::handleJobFinished);
    mComposerJob->start();
}

void IndividualMessageQueueJob::handleJobFinished(KJob *job)
{
    if (job->error()) {
        if (job == mQueueJob && mComposerJob) {
            mComposerJob->kill();
            mComposerJob = nullptr;
        } else if (job == mComposerJob && mQueueJob) {
            mQueueJob->kill();
            mQueueJob = nullptr;
        }
        setError(job->error());
        setErrorText(job->errorString());
        emitResult();
        return;
    }
    if (job == mQueueJob) {
        if (!mComposerJob) {
            emitResult();
        }
        mQueueJob = nullptr;
    } else {
        if (!mQueueJob) {
            emitResult();
        }
        mComposerJob = nullptr;
    }
}

// IndividualMailAskDelegator

IndividualMailITIPHandlerDialogDelegate::IndividualMailITIPHandlerDialogDelegate(
    const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent)
    : Akonadi::ITIPHandlerDialogDelegate(incidence, method, parent)
    , mDialog(nullptr)
{
}

void IndividualMailITIPHandlerDialogDelegate::openDialog(const QString &question, const KCalendarCore::Attendee::List &attendees, Action action, const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    switch (action) {
    case ActionSendMessage:
        Q_EMIT setUpdate(mIncidence, attendees);
        Q_EMIT dialogClosed(KMessageBox::Yes, mMethod, mIncidence);
        break;
    case ActionDontSendMessage:
        Q_EMIT dialogClosed(KMessageBox::No, mMethod, mIncidence);
        break;
    default:
        switch (CalendarSupport::KCalPrefs::instance()->sendPolicy()) {
        case (CalendarSupport::KCalPrefs::InvitationPolicySend):
            Q_EMIT setUpdate(mIncidence, attendees);
            Q_EMIT dialogClosed(KMessageBox::Yes, mMethod, mIncidence);
            break;
        case (CalendarSupport::KCalPrefs::InvitationPolicyDontSend):
            Q_EMIT dialogClosed(KMessageBox::No, mMethod, mIncidence);
            break;
        case (CalendarSupport::KCalPrefs::InvitationPolicyAsk):
        default:
            mDialog = new IndividualMailDialog(question, attendees, buttonYes, buttonNo, mParent);
            connect(mDialog, &QDialog::finished, this,
                    &IndividualMailITIPHandlerDialogDelegate::onDialogClosed);
            mDialog->show();
            break;
        }
        break;
    }
}

void IndividualMailITIPHandlerDialogDelegate::openDialogIncidenceCreated(Recipient recipient, const QString &question, Action action, const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    if (recipient == Attendees) {
        openDialog(question, mIncidence->attendees(), action, buttonYes, buttonNo);
    } else {
        KCalendarCore::Attendee organizer(mIncidence->organizer().name(), mIncidence->organizer().email());
        openDialog(question, KCalendarCore::Attendee::List() << organizer, action, buttonYes, buttonNo);
    }
}

void IndividualMailITIPHandlerDialogDelegate::openDialogIncidenceModified(
    bool attendeeStatusChanged, Recipient recipient, const QString &question, Action action, const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    Q_UNUSED(attendeeStatusChanged);
    if (recipient == Attendees) {
        openDialog(question, mIncidence->attendees(), action, buttonYes, buttonNo);
    } else {
        KCalendarCore::Attendee organizer(mIncidence->organizer().name(), mIncidence->organizer().email());
        openDialog(question, KCalendarCore::Attendee::List() << organizer, action, buttonYes, buttonNo);
    }
}

void IndividualMailITIPHandlerDialogDelegate::openDialogIncidenceDeleted(Recipient recipient, const QString &question, Action action, const KGuiItem &buttonYes, const KGuiItem &buttonNo)
{
    if (recipient == Attendees) {
        openDialog(question, mIncidence->attendees(), action, buttonYes, buttonNo);
    } else {
        KCalendarCore::Attendee organizer(mIncidence->organizer().name(), mIncidence->organizer().email());
        openDialog(question, KCalendarCore::Attendee::List() << organizer, action, buttonYes, buttonNo);
    }
}

void IndividualMailITIPHandlerDialogDelegate::onDialogClosed(int result)
{
    if (result == QDialogButtonBox::Yes) {
        Q_EMIT setEdit(mIncidence, mDialog->editAttendees());
        Q_EMIT setUpdate(mIncidence, mDialog->updateAttendees());
        Q_EMIT dialogClosed(KMessageBox::Yes, mMethod, mIncidence);
    } else {
        Q_EMIT dialogClosed(KMessageBox::No, mMethod, mIncidence);
    }
}

// IndividualMailJobFactory
IndividualMailComponentFactory::IndividualMailComponentFactory(QObject *parent)
    : Akonadi::ITIPHandlerComponentFactory(parent)
{
}

MailTransport::MessageQueueJob *IndividualMailComponentFactory::createMessageQueueJob(
    const KCalendarCore::IncidenceBase::Ptr &incidence, const KIdentityManagement::Identity &identity, QObject *parent)
{
    return new IndividualMessageQueueJob(identity, mUpdate.take(incidence->uid()),
                                         mEdit.take(incidence->uid()), parent);
}

Akonadi::ITIPHandlerDialogDelegate *IndividualMailComponentFactory::createITIPHanderDialogDelegate(
    const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent)
{
    IndividualMailITIPHandlerDialogDelegate *askDelegator
        = new IndividualMailITIPHandlerDialogDelegate(incidence, method, parent);
    connect(askDelegator, &IndividualMailITIPHandlerDialogDelegate::setEdit, this,
            &IndividualMailComponentFactory::onSetEdit);
    connect(askDelegator, &IndividualMailITIPHandlerDialogDelegate::setUpdate, this,
            &IndividualMailComponentFactory::onSetUpdate);

    return askDelegator;
}

void IndividualMailComponentFactory::onSetEdit(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &edit)
{
    mEdit[incidence->uid()] = edit;
}

void IndividualMailComponentFactory::onSetUpdate(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &update)
{
    mUpdate[incidence->uid()] = update;
}
