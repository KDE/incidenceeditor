/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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

IndividualMessageQueueJob::IndividualMessageQueueJob(const KIdentityManagementCore::Identity &identity,
                                                     const KCalendarCore::Attendee::List &update,
                                                     const KCalendarCore::Attendee::List &edit,
                                                     QObject *parent)
    : Akonadi::MessageQueueJob(parent)
    , mUpdate(update)
    , mEdit(edit)
    , mIdentity(identity)
{
}

void IndividualMessageQueueJob::start()
{
    const auto to = addressAttribute().to();
    QSet<QString> const attendeesTo(to.begin(), to.end());
    const auto cc = addressAttribute().cc();
    QSet<QString> const attendeesCc(cc.begin(), cc.end());

    QStringList attendeesAutoTo;
    QStringList attendeesAutoCc;
    for (const KCalendarCore::Attendee &attendee : std::as_const(mUpdate)) {
        if (attendeesTo.contains(attendee.email())) {
            attendeesAutoTo.append(attendee.fullName());
        }
        if (attendeesCc.contains(attendee.email())) {
            attendeesAutoCc.append(attendee.fullName());
        }
    }
    if (!attendeesAutoTo.isEmpty() || !attendeesAutoCc.isEmpty() || !addressAttribute().bcc().isEmpty()) {
        startQueueJob(attendeesAutoTo, addressAttribute().to(), attendeesAutoCc, addressAttribute().cc());
    }

    QStringList attendeesComposerTo;
    QStringList attendeesComposerCc;
    for (const KCalendarCore::Attendee &attendee : std::as_const(mEdit)) {
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
    KMime::Message::Ptr const msg(message());
    msg->to()->fromUnicodeString(messageTo.join(QLatin1StringView(", ")));
    msg->cc()->fromUnicodeString(messageCc.join(QLatin1StringView(", ")));
    msg->assemble();

    mQueueJob = new Akonadi::MessageQueueJob(this);
    mQueueJob->setMessage(msg);
    mQueueJob->transportAttribute().setTransportId(mIdentity.isNull() ? transportAttribute().transportId() : mIdentity.transport().toInt());
    mQueueJob->addressAttribute().setFrom(addressAttribute().from());
    mQueueJob->addressAttribute().setTo(to);
    mQueueJob->addressAttribute().setCc(cc);
    mQueueJob->addressAttribute().setBcc(addressAttribute().bcc());

    if (mIdentity.disabledFcc()) {
        mQueueJob->sentBehaviourAttribute().setSentBehaviour(Akonadi::SentBehaviourAttribute::Delete);
    } else {
        const Akonadi::Collection sentCollection(mIdentity.fcc().toLongLong());
        if (sentCollection.isValid()) {
            mQueueJob->sentBehaviourAttribute().setSentBehaviour(Akonadi::SentBehaviourAttribute::MoveToCollection);
            mQueueJob->sentBehaviourAttribute().setMoveToCollection(sentCollection);
        } else {
            mQueueJob->sentBehaviourAttribute().setSentBehaviour(Akonadi::SentBehaviourAttribute::MoveToDefaultSentCollection);
        }
    }

    connect(mQueueJob, &Akonadi::MessageQueueJob::finished, this, &IndividualMessageQueueJob::handleJobFinished);
    mQueueJob->start();
}

void IndividualMessageQueueJob::startComposerJob(const QStringList &to, const QStringList &cc)
{
    mComposerJob = new OpenComposerJob(this, to.join(QLatin1StringView(", ")), cc.join(QLatin1StringView(", ")), QString(), message(), mIdentity);
    connect(mComposerJob, &OpenComposerJob::finished, this, &IndividualMessageQueueJob::handleJobFinished);
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

IndividualMailITIPHandlerDialogDelegate::IndividualMailITIPHandlerDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence,
                                                                                 KCalendarCore::iTIPMethod method,
                                                                                 QWidget *parent)
    : Akonadi::ITIPHandlerDialogDelegate(incidence, method, parent)

{
}

void IndividualMailITIPHandlerDialogDelegate::openDialog(const QString &question,
                                                         const KCalendarCore::Attendee::List &attendees,
                                                         Action action,
                                                         const KGuiItem &buttonYes,
                                                         const KGuiItem &buttonNo)
{
    switch (action) {
    case ActionSendMessage:
        Q_EMIT setUpdate(mIncidence, attendees);
        Q_EMIT dialogClosed(KMessageBox::ButtonCode::PrimaryAction, mMethod, mIncidence);
        break;
    case ActionDontSendMessage:
        Q_EMIT dialogClosed(KMessageBox::ButtonCode::SecondaryAction, mMethod, mIncidence);
        break;
    default:
        switch (CalendarSupport::KCalPrefs::instance()->sendPolicy()) {
        case (CalendarSupport::KCalPrefs::InvitationPolicySend):
            Q_EMIT setUpdate(mIncidence, attendees);
            Q_EMIT dialogClosed(KMessageBox::ButtonCode::PrimaryAction, mMethod, mIncidence);
            break;
        case (CalendarSupport::KCalPrefs::InvitationPolicyDontSend):
            Q_EMIT dialogClosed(KMessageBox::ButtonCode::SecondaryAction, mMethod, mIncidence);
            break;
        case (CalendarSupport::KCalPrefs::InvitationPolicyAsk):
        default:
            mDialog = new IndividualMailDialog(question, attendees, buttonYes, buttonNo, mParent);
            connect(mDialog, &QDialog::finished, this, &IndividualMailITIPHandlerDialogDelegate::onDialogClosed);
            mDialog->show();
            break;
        }
        break;
    }
}

void IndividualMailITIPHandlerDialogDelegate::openDialogIncidenceCreated(Recipient recipient,
                                                                         const QString &question,
                                                                         Action action,
                                                                         const KGuiItem &buttonYes,
                                                                         const KGuiItem &buttonNo)
{
    if (recipient == Attendees) {
        openDialog(question, mIncidence->attendees(), action, buttonYes, buttonNo);
    } else {
        KCalendarCore::Attendee const organizer(mIncidence->organizer().name(), mIncidence->organizer().email());
        openDialog(question, KCalendarCore::Attendee::List() << organizer, action, buttonYes, buttonNo);
    }
}

void IndividualMailITIPHandlerDialogDelegate::openDialogIncidenceModified(bool attendeeStatusChanged,
                                                                          Recipient recipient,
                                                                          const QString &question,
                                                                          Action action,
                                                                          const KGuiItem &buttonYes,
                                                                          const KGuiItem &buttonNo)
{
    Q_UNUSED(attendeeStatusChanged)
    if (recipient == Attendees) {
        openDialog(question, mIncidence->attendees(), action, buttonYes, buttonNo);
    } else {
        KCalendarCore::Attendee const organizer(mIncidence->organizer().name(), mIncidence->organizer().email());
        openDialog(question, KCalendarCore::Attendee::List() << organizer, action, buttonYes, buttonNo);
    }
}

void IndividualMailITIPHandlerDialogDelegate::openDialogIncidenceDeleted(Recipient recipient,
                                                                         const QString &question,
                                                                         Action action,
                                                                         const KGuiItem &buttonYes,
                                                                         const KGuiItem &buttonNo)
{
    if (recipient == Attendees) {
        openDialog(question, mIncidence->attendees(), action, buttonYes, buttonNo);
    } else {
        KCalendarCore::Attendee const organizer(mIncidence->organizer().name(), mIncidence->organizer().email());
        openDialog(question, KCalendarCore::Attendee::List() << organizer, action, buttonYes, buttonNo);
    }
}

void IndividualMailITIPHandlerDialogDelegate::onDialogClosed(int result)
{
    if (result == QDialogButtonBox::Yes) {
        Q_EMIT setEdit(mIncidence, mDialog->editAttendees());
        Q_EMIT setUpdate(mIncidence, mDialog->updateAttendees());
        Q_EMIT dialogClosed(KMessageBox::ButtonCode::PrimaryAction, mMethod, mIncidence);
    } else {
        Q_EMIT dialogClosed(KMessageBox::ButtonCode::SecondaryAction, mMethod, mIncidence);
    }
}

// IndividualMailJobFactory
IndividualMailComponentFactory::IndividualMailComponentFactory(QObject *parent)
    : Akonadi::ITIPHandlerComponentFactory(parent)
{
}

Akonadi::MessageQueueJob *IndividualMailComponentFactory::createMessageQueueJob(const KCalendarCore::IncidenceBase::Ptr &incidence,
                                                                                const KIdentityManagementCore::Identity &identity,
                                                                                QObject *parent)
{
    return new IndividualMessageQueueJob(identity, mUpdate.take(incidence->uid()), mEdit.take(incidence->uid()), parent);
}

Akonadi::ITIPHandlerDialogDelegate *IndividualMailComponentFactory::createITIPHanderDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence,
                                                                                                   KCalendarCore::iTIPMethod method,
                                                                                                   QWidget *parent)
{
    auto askDelegator = new IndividualMailITIPHandlerDialogDelegate(incidence, method, parent);
    connect(askDelegator, &IndividualMailITIPHandlerDialogDelegate::setEdit, this, &IndividualMailComponentFactory::onSetEdit);
    connect(askDelegator, &IndividualMailITIPHandlerDialogDelegate::setUpdate, this, &IndividualMailComponentFactory::onSetUpdate);

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

#include "moc_individualmailcomponentfactory.cpp"
