/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#pragma once

#include "incidenceeditor_export.h"

#include <Akonadi/Calendar/IncidenceChanger>
#include <KIdentityManagement/Identity>
#include <MailTransportAkonadi/MessageQueueJob>
namespace IncidenceEditorNG
{
class OpenComposerJob;
class IndividualMailDialog;

class IndividualMessageQueueJob : public MailTransport::MessageQueueJob
{
    Q_OBJECT
public:
    explicit IndividualMessageQueueJob(const KIdentityManagement::Identity &identity,
                                       const KCalendarCore::Attendee::List &update,
                                       const KCalendarCore::Attendee::List &edit,
                                       QObject *parent);

    void start() override;

private:
    void startQueueJob(const QStringList &messageTo, const QStringList &to, const QStringList &messageCc, const QStringList &cc);
    void startComposerJob(const QStringList &to, const QStringList &cc);
    void handleJobFinished(KJob *job);
    KCalendarCore::Attendee::List mUpdate;
    KCalendarCore::Attendee::List mEdit;
    KIdentityManagement::Identity mIdentity;
    MailTransport::MessageQueueJob *mQueueJob = nullptr;
    OpenComposerJob *mComposerJob = nullptr;
};

class IndividualMailITIPHandlerDialogDelegate : public Akonadi::ITIPHandlerDialogDelegate
{
    Q_OBJECT
public:
    explicit IndividualMailITIPHandlerDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent);

    void openDialogIncidenceCreated(Recipient recipient,
                                    const QString &question,
                                    Action action = ActionAsk,
                                    const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                    const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send"))) override;

    void openDialogIncidenceModified(bool attendeeStatusChanged,
                                     Recipient recipient,
                                     const QString &question,
                                     Action action = ActionAsk,
                                     const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                     const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send"))) override;

    void openDialogIncidenceDeleted(Recipient recipient,
                                    const QString &question,
                                    Action action = ActionAsk,
                                    const KGuiItem &buttonYes = KGuiItem(i18nc("@action:button dialog positive answer", "Send Email")),
                                    const KGuiItem &buttonNo = KGuiItem(i18nc("@action:button dialog negative answer", "Do Not Send"))) override;

Q_SIGNALS:
    void setEdit(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &edit);
    void setUpdate(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &update);

protected:
    void
    openDialog(const QString &question, const KCalendarCore::Attendee::List &attendees, Action action, const KGuiItem &buttonYes, const KGuiItem &buttonNo);

private:
    void onDialogClosed(int result);
    IndividualMailDialog *mDialog = nullptr;
};

class INCIDENCEEDITOR_EXPORT IndividualMailComponentFactory : public Akonadi::ITIPHandlerComponentFactory
{
    Q_OBJECT
public:
    explicit IndividualMailComponentFactory(QObject *parent = nullptr);
    MailTransport::MessageQueueJob *
    createMessageQueueJob(const KCalendarCore::IncidenceBase::Ptr &incidence, const KIdentityManagement::Identity &identity, QObject *parent) override;

    Akonadi::ITIPHandlerDialogDelegate *
    createITIPHanderDialogDelegate(const KCalendarCore::Incidence::Ptr &incidence, KCalendarCore::iTIPMethod method, QWidget *parent) override;

public Q_SLOTS:
    void onSetEdit(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &edit);
    void onSetUpdate(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Attendee::List &update);

private:
    QHash<QString, KCalendarCore::Attendee::List> mEdit;
    QHash<QString, KCalendarCore::Attendee::List> mUpdate;
};
}
